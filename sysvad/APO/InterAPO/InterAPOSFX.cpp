//
// InterAPOSFX.cpp -- Copyright (c) Microsoft Corporation. All rights reserved.
//
// Description:
//
//  Implementation of CInterAPOSFX
//

#include <atlbase.h>
#include <atlcom.h>
#include <atlcoll.h>
#include <atlsync.h>
#include <mmreg.h>

#include <audioenginebaseapo.h>
#include <baseaudioprocessingobject.h>
#include <resource.h>

#include <float.h>

#include "InterAPO.h"
#include <devicetopology.h>
#include <CustomPropKeys.h>

#include <EvnTrace.h>
#include "trace.h"
#include "InterAPOSFX.tmh"


// Static declaration of the APO_REG_PROPERTIES structure
// associated with this APO.  The number in <> brackets is the
// number of IIDs supported by this APO.  If more than one, then additional
// IIDs are added at the end
#pragma warning (disable : 4815)
const AVRT_DATA CRegAPOProperties<1> CInterAPOSFX::sm_RegProperties(
    __uuidof(InterAPOSFX),                           // clsid of this APO
    L"CInterAPOSFX",                                 // friendly name of this APO
    L"Copyright (c) Microsoft Corporation",         // copyright info
    1,                                              // major version #
    0,                                              // minor version #
    __uuidof(IInterAPOSFX)                           // iid of primary interface

//
// If you need to change any of these attributes, uncomment everything up to
// the point that you need to change something.  If you need to add IIDs, uncomment
// everything and add additional IIDs at the end.
//
//  , DEFAULT_APOREG_FLAGS
//  , DEFAULT_APOREG_MININPUTCONNECTIONS
//  , DEFAULT_APOREG_MAXINPUTCONNECTIONS
//  , DEFAULT_APOREG_MINOUTPUTCONNECTIONS
//  , DEFAULT_APOREG_MAXOUTPUTCONNECTIONS
//  , DEFAULT_APOREG_MAXINSTANCES
//
    );

#pragma AVRT_CODE_BEGIN
//-------------------------------------------------------------------------
// Description:
//
//  Do the actual processing of data.
//
// Parameters:
//
//      u32NumInputConnections      - [in] number of input connections
//      ppInputConnections          - [in] pointer to list of input APO_CONNECTION_PROPERTY pointers
//      u32NumOutputConnections      - [in] number of output connections
//      ppOutputConnections         - [in] pointer to list of output APO_CONNECTION_PROPERTY pointers
//
// Return values:
//
//      void
//
// Remarks:
//
//  This function processes data in a manner dependent on the implementing
//  object.  This routine can not fail and can not block, or call any other
//  routine that blocks, or touch pagable memory.
//
STDMETHODIMP_(void) CInterAPOSFX::APOProcess(
    UINT32 u32NumInputConnections,
    APO_CONNECTION_PROPERTY** ppInputConnections,
    UINT32 u32NumOutputConnections,
    APO_CONNECTION_PROPERTY** ppOutputConnections)
{
    UNREFERENCED_PARAMETER(u32NumInputConnections);
    UNREFERENCED_PARAMETER(u32NumOutputConnections);

    FLOAT32 *pf32InputFrames, *pf32OutputFrames;

    ATLASSERT(m_bIsLocked);

    // assert that the number of input and output connectins fits our registration properties
    ATLASSERT(m_pRegProperties->u32MinInputConnections <= u32NumInputConnections);
    ATLASSERT(m_pRegProperties->u32MaxInputConnections >= u32NumInputConnections);
    ATLASSERT(m_pRegProperties->u32MinOutputConnections <= u32NumOutputConnections);
    ATLASSERT(m_pRegProperties->u32MaxOutputConnections >= u32NumOutputConnections);

    TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER, "--> %!FUNC! In=%lu Out=%lu Flags=%08lX",
        u32NumInputConnections, u32NumOutputConnections, ppInputConnections[0]->u32BufferFlags);

    // check APO_BUFFER_FLAGS.
    switch( ppInputConnections[0]->u32BufferFlags )
    {
        case BUFFER_INVALID:
        {
            ATLASSERT(false);  // invalid flag - should never occur.  don't do anything.
            break;
        }
        case BUFFER_VALID:
        case BUFFER_SILENT:
        {
            // get input pointer to connection buffer
            pf32InputFrames = reinterpret_cast<FLOAT32*>(ppInputConnections[0]->pBuffer);
            ATLASSERT( IS_VALID_TYPED_READ_POINTER(pf32InputFrames) );

            // get output pointer to connection buffer
            pf32OutputFrames = reinterpret_cast<FLOAT32*>(ppOutputConnections[0]->pBuffer);
            ATLASSERT( IS_VALID_TYPED_WRITE_POINTER(pf32OutputFrames) );

            if (BUFFER_SILENT == ppInputConnections[0]->u32BufferFlags)
            {
                WriteSilence( pf32InputFrames,
                              ppInputConnections[0]->u32ValidFrameCount,
                              GetSamplesPerFrame() );
            }

            // copy to the inter buffer
            if (
                !IsEqualGUID(m_AudioProcessingMode, AUDIO_SIGNALPROCESSINGMODE_RAW) &&
                m_fEnableInterSFX
            )
            {
                TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER, "%!FUNC! DBG ValidFCnt=%lu SmpPF=%lu",
                    ppInputConnections[0]->u32ValidFrameCount, m_u32SamplesPerFrame);

                GainControl(pf32OutputFrames, pf32InputFrames,
                             ppInputConnections[0]->u32ValidFrameCount,
                             GetSamplesPerFrame(),
                             m_iInterGainSFX);

                pf32InputFrames = pf32OutputFrames;
                Equalizer(pf32OutputFrames, pf32InputFrames,
                    ppInputConnections[0]->u32ValidFrameCount,
                    GetSamplesPerFrame(),
                    &m_EqSFX.m_LowFilter);

                pf32InputFrames = pf32OutputFrames;
                Equalizer(pf32OutputFrames, pf32InputFrames,
                    ppInputConnections[0]->u32ValidFrameCount,
                    GetSamplesPerFrame(),
                    &m_EqSFX.m_MidFilter);

                pf32InputFrames = pf32OutputFrames;
                Equalizer(pf32OutputFrames, pf32InputFrames,
                    ppInputConnections[0]->u32ValidFrameCount,
                    GetSamplesPerFrame(),
                    &m_EqSFX.m_HighFilter);

                // we don't try to remember silence
                ppOutputConnections[0]->u32BufferFlags = BUFFER_VALID;
            }
            else
            {
                TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER, "%!FUNC! nomatch OutCon=%lu actionF=%s Coef=%lu",
                    u32NumOutputConnections,
                    ppOutputConnections[0]->pBuffer == ppInputConnections[0]->pBuffer ? "CopyFrames" : "No CopyFrame",
                    ppInputConnections[0]->u32BufferFlags);

                // copy the memory only if there is an output connection, and input/output pointers are unequal
                if ( (0 != u32NumOutputConnections) &&
                      (ppOutputConnections[0]->pBuffer != ppInputConnections[0]->pBuffer) )
                {
                    CopyFrames( pf32OutputFrames, pf32InputFrames,
                                ppInputConnections[0]->u32ValidFrameCount,
                                GetSamplesPerFrame() );
                }

                // pass along buffer flags
                ppOutputConnections[0]->u32BufferFlags = ppInputConnections[0]->u32BufferFlags;
            }

            // Set the valid frame count.
            ppOutputConnections[0]->u32ValidFrameCount = ppInputConnections[0]->u32ValidFrameCount;

            break;
        }
        default:
        {
            ATLASSERT(false);  // invalid flag - should never occur
            break;
        }
    } // switch
    TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER, "<-- %!FUNC! Flags=%08lX Count=%lu",
        ppOutputConnections[0]->u32BufferFlags, ppOutputConnections[0]->u32ValidFrameCount);
} // APOProcess
#pragma AVRT_CODE_END

//-------------------------------------------------------------------------
// Description:
//
//  Report inter added by the APO between samples given on input
//  and samples given on output.
//
// Parameters:
//
//      pTime                       - [out] hundreds-of-nanoseconds of inter added
//
// Return values:
//
//      S_OK on success, a failure code on failure
STDMETHODIMP CInterAPOSFX::GetLatency(HNSTIME* pTime)  
{  
    ASSERT_NONREALTIME();  
    HRESULT hr = S_OK;  
  
    IF_TRUE_ACTION_JUMP(NULL == pTime, hr = E_POINTER, Exit);  
  
    if (IsEqualGUID(m_AudioProcessingMode, AUDIO_SIGNALPROCESSINGMODE_RAW))
    {
        *pTime = 0;
    }
    else
    {
        *pTime = (m_fEnableInterSFX ? HNS_INTER : 0);
    }
  
Exit:
    TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER,
        "%!FUNC! ptime=%llX hr=%!HRESULT!", pTime ? *pTime : NULL, hr);
    return hr;
}

//-------------------------------------------------------------------------
// Description:
//
//  Verifies that the APO is ready to process and locks its state if so.
//
// Parameters:
//
//      u32NumInputConnections - [in] number of input connections attached to this APO
//      ppInputConnections - [in] connection descriptor of each input connection attached to this APO
//      u32NumOutputConnections - [in] number of output connections attached to this APO
//      ppOutputConnections - [in] connection descriptor of each output connection attached to this APO
//
// Return values:
//
//      S_OK                                Object is locked and ready to process.
//      E_POINTER                           Invalid pointer passed to function.
//      APOERR_INVALID_CONNECTION_FORMAT    Invalid connection format.
//      APOERR_NUM_CONNECTIONS_INVALID      Number of input or output connections is not valid on
//                                          this APO.
STDMETHODIMP CInterAPOSFX::LockForProcess(UINT32 u32NumInputConnections,
    APO_CONNECTION_DESCRIPTOR** ppInputConnections,  
    UINT32 u32NumOutputConnections, APO_CONNECTION_DESCRIPTOR** ppOutputConnections)
{
    ASSERT_NONREALTIME();
    HRESULT hr = S_OK;
    
    hr = CBaseAudioProcessingObject::LockForProcess(u32NumInputConnections,
        ppInputConnections, u32NumOutputConnections, ppOutputConnections);
    IF_FAILED_JUMP(hr, Exit);
    
    if (!IsEqualGUID(m_AudioProcessingMode, AUDIO_SIGNALPROCESSINGMODE_RAW) && m_fEnableInterSFX)
    {
        m_nInterFrames = FRAMES_FROM_HNS(HNS_INTER);
        m_iInterIndex = 0;

        m_pf32InterBuffer.Free();
        
        // Allocate one second's worth of audio
        // 
        // This allocation is being done using CoTaskMemAlloc because the inter is very large
        // This introduces a risk of glitches if the inter buffer gets paged out
        //
        // A more typical approach would be to allocate the memory using AERT_Allocate, which locks the memory
        // But for the purposes of this APO, CoTaskMemAlloc suffices, and the risk of glitches is not important
        m_pf32InterBuffer.Allocate(GetSamplesPerFrame() * m_nInterFrames);
                
        WriteSilence(m_pf32InterBuffer, m_nInterFrames, GetSamplesPerFrame());
        if (nullptr == m_pf32InterBuffer)
        {
            hr = E_OUTOFMEMORY;
            goto Exit;
        }
    }
    
Exit:
    TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER,
        "%!FUNC! SFXEn=%ld index=%lu hr=%!HRESULT!", m_fEnableInterSFX, m_iInterIndex, hr);
    return hr;
}

// The method that this long comment refers to is "Initialize()"
//-------------------------------------------------------------------------
// Description:
//
//  Generic initialization routine for APOs.
//
// Parameters:
//
//     cbDataSize - [in] the size in bytes of the initialization data.
//     pbyData - [in] initialization data specific to this APO
//
// Return values:
//
//     S_OK                         Successful completion.
//     E_POINTER                    Invalid pointer passed to this function.
//     E_INVALIDARG                 Invalid argument
//     AEERR_ALREADY_INITIALIZED    APO is already initialized
//
// Remarks:
//
//  This method initializes the APO.  The data is variable length and
//  should have the form of:
//
//    struct MyAPOInitializationData
//    {
//        APOInitBaseStruct APOInit;
//        ... // add additional fields here
//    };
//
//  If the APO needs no initialization or needs no data to initialize
//  itself, it is valid to pass NULL as the pbyData parameter and 0 as
//  the cbDataSize parameter.
//
//  As part of designing an APO, decide which parameters should be
//  immutable (set once during initialization) and which mutable (changeable
//  during the lifetime of the APO instance).  Immutable parameters must
//  only be specifiable in the Initialize call; mutable parameters must be
//  settable via methods on whichever parameter control interface(s) your
//  APO provides. Mutable values should either be set in the initialize
//  method (if they are required for proper operation of the APO prior to
//  LockForProcess) or default to reasonable values upon initialize and not
//  be required to be set before LockForProcess.
//
//  Within the mutable parameters, you must also decide which can be changed
//  while the APO is locked for processing and which cannot.
//
//  All parameters should be considered immutable as a first choice, unless
//  there is a specific scenario which requires them to be mutable; similarly,
//  no mutable parameters should be changeable while the APO is locked, unless
//  a specific scenario requires them to be.  Following this guideline will
//  simplify the APO's state diagram and implementation and prevent certain
//  types of bug.
//
//  If a parameter changes the APOs latency or MaxXXXFrames values, it must be
//  immutable.
//
//  The default version of this function uses no initialization data, but does verify
//  the passed parameters and set the m_bIsInitialized member to true.
//
//  Note: This method may not be called from a real-time processing thread.
//

HRESULT CInterAPOSFX::Initialize(UINT32 cbDataSize, BYTE* pbyData)
{
    HRESULT                     hr = S_OK;
    CComPtr<IMMDevice>	        spMyDevice;
    CComPtr<IDeviceTopology>    spMyDeviceTopology;
    CComPtr<IConnector>         spMyConnector;
    GUID                        processingMode;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! DataSize=%lu", cbDataSize);

    IF_TRUE_ACTION_JUMP( ((NULL == pbyData) && (0 != cbDataSize)), hr = E_INVALIDARG, Exit);
    IF_TRUE_ACTION_JUMP( ((NULL != pbyData) && (0 == cbDataSize)), hr = E_INVALIDARG, Exit);

    if (cbDataSize == sizeof(APOInitSystemEffects2))
    {
        //
        // Initialize for mode-specific signal processing
        //
        APOInitSystemEffects2* papoSysFxInit2 = (APOInitSystemEffects2*)pbyData;

        // Save reference to the effects property store. This saves effects settings
        // and is the communication medium between this APO and any associated UI.
        m_spAPOSystemEffectsProperties = papoSysFxInit2->pAPOSystemEffectsProperties;

        // Windows should pass a valid collection.
        ATLASSERT(papoSysFxInit2->pDeviceCollection != nullptr);
        IF_TRUE_ACTION_JUMP(papoSysFxInit2->pDeviceCollection == nullptr, hr = E_INVALIDARG, Exit);

        // Get the IDeviceTopology and IConnector interfaces to communicate with this
        // APO's counterpart audio driver. This can be used for any proprietary
        // communication.
        hr = papoSysFxInit2->pDeviceCollection->Item(papoSysFxInit2->nSoftwareIoDeviceInCollection, &spMyDevice);
        IF_FAILED_JUMP(hr, Exit);

        hr = spMyDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, NULL, (void**)&spMyDeviceTopology);
        IF_FAILED_JUMP(hr, Exit);

        hr = spMyDeviceTopology->GetConnector(papoSysFxInit2->nSoftwareIoConnectorIndex, &spMyConnector);
        IF_FAILED_JUMP(hr, Exit);

        // Save the processing mode being initialized.
        processingMode = papoSysFxInit2->AudioProcessingMode;

    }
    else if (cbDataSize == sizeof(APOInitSystemEffects))
    {
        //
        // Initialize for default signal processing
        //
        APOInitSystemEffects* papoSysFxInit = (APOInitSystemEffects*)pbyData;

        // Save reference to the effects property store. This saves effects settings
        // and is the communication medium between this APO and any associated UI.
        m_spAPOSystemEffectsProperties = papoSysFxInit->pAPOSystemEffectsProperties;

        // Assume default processing mode
        processingMode = AUDIO_SIGNALPROCESSINGMODE_DEFAULT;
    }
    else
    {
        // Invalid initialization size
        hr = E_INVALIDARG;
        goto Exit;
    }

    // Validate then save the processing mode. Note an endpoint effects APO
    // does not depend on the mode. Windows sets the APOInitSystemEffects2
    // AudioProcessingMode member to GUID_NULL for an endpoint effects APO.
    IF_TRUE_ACTION_JUMP((processingMode != AUDIO_SIGNALPROCESSINGMODE_DEFAULT        &&
                         processingMode != AUDIO_SIGNALPROCESSINGMODE_RAW            &&
                         processingMode != AUDIO_SIGNALPROCESSINGMODE_COMMUNICATIONS &&
                         processingMode != AUDIO_SIGNALPROCESSINGMODE_SPEECH         &&
                         processingMode != AUDIO_SIGNALPROCESSINGMODE_MEDIA          &&
                         processingMode != AUDIO_SIGNALPROCESSINGMODE_MOVIE), hr = E_INVALIDARG, Exit);
    m_AudioProcessingMode = processingMode;

    //
    // An APO that implements signal processing more complex than this sample
    // would configure its processing for the processingMode determined above.
    // If necessary, the APO would also use the IDeviceTopology and IConnector
    // interfaces retrieved above to communicate with its counterpart audio
    // driver to configure any additional signal processing in the driver and
    // associated hardware.
    //

    //
    //  Get the current values
    //
    if (m_spAPOSystemEffectsProperties != NULL)
    {
        m_fEnableInterSFX = GetCurrentEffectsSetting(m_spAPOSystemEffectsProperties, PKEY_Endpoint_Enable_Interface_SFX, m_AudioProcessingMode);
        m_iInterGainSFX = GetCurrentEffectsSetting(m_spAPOSystemEffectsProperties, PKEY_Endpoint_Inter_Gain_Level_SFX, m_AudioProcessingMode);
        m_iInterEqLowSFX = GetCurrentEffectsSetting(m_spAPOSystemEffectsProperties, PKEY_Endpoint_Inter_EQ_Low_SFX, m_AudioProcessingMode);
        m_iInterEqMidSFX = GetCurrentEffectsSetting(m_spAPOSystemEffectsProperties, PKEY_Endpoint_Inter_EQ_Mid_SFX, m_AudioProcessingMode);
        m_iInterEqHighSFX = GetCurrentEffectsSetting(m_spAPOSystemEffectsProperties, PKEY_Endpoint_Inter_EQ_High_SFX, m_AudioProcessingMode);

       (& m_EqSFX.m_LowFilter)->Prepare(m_iInterEqLowSFX);
       (& m_EqSFX.m_MidFilter)->Prepare(m_iInterEqMidSFX);
       (& m_EqSFX.m_HighFilter)->Prepare(m_iInterEqHighSFX);
    }

    //
    //  Register for notification of registry updates
    //
    hr = m_spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
    IF_FAILED_JUMP(hr, Exit);

    hr = m_spEnumerator->RegisterEndpointNotificationCallback(this);
    IF_FAILED_JUMP(hr, Exit);

    //m_EqSFX.

    m_bIsInitialized = true;


 Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER,
        "<-- %!FUNC! hr=%!HRESULT! en=%ld SFX=%ld,%ld,%ld,%ld", hr, m_fEnableInterSFX,
        m_iInterGainSFX, m_iInterEqLowSFX, m_iInterEqMidSFX, m_iInterEqHighSFX);
    return hr;
}

//-------------------------------------------------------------------------
//
// GetEffectsList
//
//  Retrieves the list of signal processing effects currently active and
//  stores an event to be signaled if the list changes.
//
// Parameters
//
//  ppEffectsIds - returns a pointer to a list of GUIDs each identifying a
//      class of effect. The caller is responsible for freeing this memory by
//      calling CoTaskMemFree.
//
//  pcEffects - returns a count of GUIDs in the list.
//
//  Event - passes an event handle. The APO signals this event when the list
//      of effects changes from the list returned from this function. The APO
//      uses this event until either this function is called again or the APO
//      is destroyed. The passed handle may be NULL. In this case, the APO
//      stops using any previous handle and does not signal an event.
//
// Remarks
//
//  An APO imlements this method to allow Windows to discover the current
//  effects applied by the APO. The list of effects may depend on what signal
//  processing mode the APO initialized (see AudioProcessingMode in the
//  APOInitSystemEffects2 structure) as well as any end user configuration.
//
//  If there are no effects then the function still succeeds, ppEffectsIds
//  returns a NULL pointer, and pcEffects returns a count of 0.
//
STDMETHODIMP CInterAPOSFX::GetEffectsList(_Outptr_result_buffer_maybenull_(*pcEffects) LPGUID *ppEffectsIds, _Out_ UINT *pcEffects, _In_ HANDLE Event)
{
    HRESULT hr;
    BOOL effectsLocked = FALSE;
    UINT cEffects = 0;
    
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! Event=%p", Event);

    IF_TRUE_ACTION_JUMP(ppEffectsIds == NULL, hr = E_POINTER, Exit);
    IF_TRUE_ACTION_JUMP(pcEffects == NULL, hr = E_POINTER, Exit);

    // Synchronize access to the effects list and effects changed event
    m_EffectsLock.Enter();
    effectsLocked = TRUE;

    // Always close existing effects change event handle
    if (m_hEffectsChangedEvent != NULL)
    {
        CloseHandle(m_hEffectsChangedEvent);
        m_hEffectsChangedEvent = NULL;
    }

    // If an event handle was specified, save it here (duplicated to control lifetime)
    if (Event != NULL)
    {
        if (!DuplicateHandle(GetCurrentProcess(), Event, GetCurrentProcess(), &m_hEffectsChangedEvent, EVENT_MODIFY_STATE, FALSE, 0))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            goto Exit;
        }
        TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! GotEvent=%p", Event);
    }

    // naked scope to force the initialization of list[] to be after we enter the critical section
    {
        struct EffectControl
        {
            GUID effect;
            BOOL control;
        };
        
        EffectControl list[] =
        {
            { InterEffectId, m_fEnableInterSFX },
        };
    
        if (!IsEqualGUID(m_AudioProcessingMode, AUDIO_SIGNALPROCESSINGMODE_RAW))
        {
            // count the active effects
            for (UINT i = 0; i < ARRAYSIZE(list); i++)
            {
                if (list[i].control)
                {
                    cEffects++;
                }
            }
        }

        if (0 == cEffects)
        {
            *ppEffectsIds = NULL;
            *pcEffects = 0;
        }
        else
        {
            GUID *pEffectsIds = (LPGUID)CoTaskMemAlloc(sizeof(GUID) * cEffects);
            if (pEffectsIds == nullptr)
            {
                hr = E_OUTOFMEMORY;
                goto Exit;
            }
            
            // pick up the active effects
            UINT j = 0;
            for (UINT i = 0; i < ARRAYSIZE(list); i++)
            {
                if (list[i].control)
                {
                    pEffectsIds[j++] = list[i].effect;
                }
            }

            *ppEffectsIds = pEffectsIds;
            *pcEffects = cEffects;
        }
        
        hr = S_OK;
    }
    
Exit:
    if (effectsLocked)
    {
        m_EffectsLock.Leave();
    }
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "<-- %!FUNC! cFX=%lu hr=%!HRESULT!", cEffects, hr);
    return hr;
}

//-------------------------------------------------------------------------
// Description:
//
//
//
// Parameters:
//
//
//
// Return values:
//
//
//
// Remarks:
//
//
HRESULT CInterAPOSFX::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
{
    HRESULT     hr = S_OK;
    LONG nChanges = 0;
    LONG nSetEvent = 0;

    UNREFERENCED_PARAMETER(pwstrDeviceId);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! key=%!GUID!,%ld", &key.fmtid, key.pid);

    if (!m_spAPOSystemEffectsProperties)
    {
        return hr;
    }

    // If either the master disable or our APO's enable properties changed...
    if (PK_EQUAL(key, PKEY_AudioEndpoint_Disable_SysFx) ||
        PK_EQUAL(key, PKEY_Endpoint_Enable_Interface_SFX) ||
        PK_EQUAL(key, PKEY_Endpoint_Inter_Gain_Level_SFX) ||
		PK_EQUAL(key, PKEY_Endpoint_Inter_EQ_Low_SFX) ||
        PK_EQUAL(key, PKEY_Endpoint_Inter_EQ_Mid_SFX) ||
        PK_EQUAL(key, PKEY_Endpoint_Inter_EQ_High_SFX))
    {
        // Synchronize access to the effects list and effects changed event
        m_EffectsLock.Enter();

        TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER, "%!FUNC! m_EffectsLock.Enter()");

        struct KeyControl
        {
            PROPERTYKEY key;
            LONG *value;
            BiQuadFilter *filter;
        };
        
        KeyControl controls[] =
        {
            { PKEY_Endpoint_Enable_Interface_SFX,        &m_fEnableInterSFX,   NULL },
            { PKEY_Endpoint_Inter_Gain_Level_SFX,        &m_iInterGainSFX,     NULL },
            { PKEY_Endpoint_Inter_EQ_Low_SFX,            &m_iInterEqLowSFX,    &m_EqSFX.m_LowFilter },
            { PKEY_Endpoint_Inter_EQ_Mid_SFX,            &m_iInterEqMidSFX,    &m_EqSFX.m_MidFilter },
            { PKEY_Endpoint_Inter_EQ_High_SFX,           &m_iInterEqHighSFX,   &m_EqSFX.m_HighFilter },
        };
        
        for (int i = 0; i < ARRAYSIZE(controls); i++)
        {
            LONG fOldValue;
            LONG fNewValue = true;
            
            // Get the registry values for the current processing mode
            fNewValue = GetCurrentEffectsSetting(m_spAPOSystemEffectsProperties, controls[i].key, m_AudioProcessingMode);

            // Inter in the new setting
            fOldValue = InterlockedExchange(controls[i].value, fNewValue);
            
            if (fNewValue != fOldValue)
            {
                // Notify to check the new value...
                TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! Changed[%d]: Key=%!GUID!,%ld val=%ld",
                    i, &controls[i].key.fmtid, controls[i].key.pid, fNewValue);
                if (controls[i].filter != NULL)
                {
					controls[i].filter->Prepare(fNewValue);
                }
                nChanges++;
            }
        }
        
        // If anything changed and a change event handle exists
        if ((nChanges > 0) && (m_hEffectsChangedEvent != NULL))
        {
            TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER, "%!FUNC! call SetEvent nChanges=%ld(%ld %ld)",
                nChanges, m_fEnableInterSFX, m_iInterGainSFX);
            SetEvent(m_hEffectsChangedEvent);
            nSetEvent++;
        }
        m_EffectsLock.Leave();
    }
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER,
        "<-- %!FUNC! hr=%!HRESULT! %ld en=%ld SFX=%ld,%ld,%ld,%ld", hr, nSetEvent, m_fEnableInterSFX,
        m_iInterGainSFX, m_iInterEqLowSFX, m_iInterEqMidSFX, m_iInterEqHighSFX);
    return hr;
}


//-------------------------------------------------------------------------
// Description:
//
//  Destructor.
//
// Parameters:
//
//     void
//
// Return values:
//
//      void
//
// Remarks:
//
//      This method deletes whatever was allocated.
//
//      This method may not be called from a real-time processing thread.
//
CInterAPOSFX::~CInterAPOSFX(void)
{
    //
    // unregister for callbacks
    //
    if (m_bIsInitialized)
    {
        m_spEnumerator->UnregisterEndpointNotificationCallback(this);
    }

    if (m_hEffectsChangedEvent != NULL)
    {
        CloseHandle(m_hEffectsChangedEvent);
    }
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! ...");
} // ~CInterAPOSFX
