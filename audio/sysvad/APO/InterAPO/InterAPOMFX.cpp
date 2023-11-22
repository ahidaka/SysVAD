//
// InterAPOMFX.cpp -- Copyright (c) Microsoft Corporation. All rights reserved.
//
// Description:
//
//  Implementation of CInterAPOMFX
//

#include <atlbase.h>
#include <atlcom.h>
#include <atlcoll.h>
#include <atlsync.h>
#include <mmreg.h>

#include <initguid.h>
#include <audioenginebaseapo.h>
#include <baseaudioprocessingobject.h>
#include <resource.h>

#include <float.h>
#include "InterAPO.h"
#include "SysVadShared.h"
#include <CustomPropKeys.h>

#include <EvnTrace.h>
#include "trace.h"
#include "InterAPOMFX.tmh"


// Static declaration of the APO_REG_PROPERTIES structure
// associated with this APO.  The number in <> brackets is the
// number of IIDs supported by this APO.  If more than one, then additional
// IIDs are added at the end
#pragma warning (disable : 4815)
const AVRT_DATA CRegAPOProperties<1> CInterAPOMFX::sm_RegProperties(
    __uuidof(InterAPOMFX),                           // clsid of this APO
    L"CInterAPOMFX",                                 // friendly name of this APO
    L"Copyright (c) Microsoft Corporation",         // copyright info
    1,                                              // major version #
    0,                                              // minor version #
    __uuidof(IInterAPOMFX)                           // iid of primary interface
//
// If you need to change any of these attributes, uncomment everything up to
// the point that you need to change something.  If you need to add IIDs, uncomment
// everything and add additional IIDs at the end.
//
//   Enable inplace processing for this APO.
//  , DEFAULT_APOREG_FLAGS
//  , DEFAULT_APOREG_MININPUTCONNECTIONS
//  , DEFAULT_APOREG_MAXINPUTCONNECTIONS
//  , DEFAULT_APOREG_MINOUTPUTCONNECTIONS
//  , DEFAULT_APOREG_MAXOUTPUTCONNECTIONS
//  , DEFAULT_APOREG_MAXINSTANCES
//
    );

//-------------------------------------------------------------------------
// Description:
//
//  GetCurrentEffectsSetting
//      Gets the current aggregate effects-enable setting
//
// Parameters:
//
//  properties - Property store holding configurable effects settings
//
//  pkeyEnable - VT_UI4 property holding an enable/disable setting
//
//  processingMode - Audio processing mode
//
// Return values:
//  LONG - true if the effect is enabled
//
// Remarks:
//  The routine considers the value of the specified property, the well known
//  master PKEY_AudioEndpoint_Disable_SysFx property, and the specified
//  processing mode.If the processing mode is RAW then the effect is off. If
//  PKEY_AudioEndpoint_Disable_SysFx is non-zero then the effect is off.
//
LONG GetCurrentEffectsSetting(IPropertyStore* properties, PROPERTYKEY pkeyEnable, GUID processingMode)
{
    HRESULT hr;
    BOOL enabled;
    PROPVARIANT var;
    LONG currentValue = 0;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! KEY=%!GUID!,%ul Mode=%!GUID!",
        &pkeyEnable.fmtid, pkeyEnable.pid, &processingMode);

    PropVariantInit(&var);

    // Get the state of whether channel swap MFX is enabled or not. 

    // Check the master disable property defined by Windows
    hr = properties->GetValue(PKEY_AudioEndpoint_Disable_SysFx, &var);
    enabled = (SUCCEEDED(hr)) && !((var.vt == VT_UI4) && (var.ulVal != 0));

    PropVariantClear(&var);

    // Check the APO's enable property, defined by this APO.
    hr = properties->GetValue(pkeyEnable, &var);
    enabled = enabled && ((SUCCEEDED(hr)) && ((var.vt == VT_UI4) && (var.ulVal != 0)));
    if (enabled)
    {
        currentValue = var.ulVal;
    }
    PropVariantClear(&var);

    enabled = enabled && !IsEqualGUID(processingMode, AUDIO_SIGNALPROCESSINGMODE_RAW);
    if (!enabled)
    {
        currentValue = 0;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "<-- %!FUNC! hr=%!HRESULT! enabled=%ld value=%ld",
        hr, enabled, currentValue);

    return currentValue;
}

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
STDMETHODIMP_(void) CInterAPOMFX::APOProcess(
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
            ATLASSERT( IS_VALID_TYPED_READ_POINTER(pf32OutputFrames) );

            if (BUFFER_SILENT == ppInputConnections[0]->u32BufferFlags)
            {
                WriteSilence( pf32InputFrames,
                              ppInputConnections[0]->u32ValidFrameCount,
                              GetSamplesPerFrame() );
            }

            // copy to the inter buffer
            if (
                !IsEqualGUID(m_AudioProcessingMode, AUDIO_SIGNALPROCESSINGMODE_RAW) &&
                m_fEnableInterMFX
            )
            {
                TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER, "%!FUNC! DBG ValidFCnt=%lu SmpPF=%lu Coef=%f",
                    ppInputConnections[0]->u32ValidFrameCount,
                    m_u32SamplesPerFrame, m_pf32Coefficients ? (double)*m_pf32Coefficients : 0);

                GainControl(pf32OutputFrames, pf32InputFrames,
                             ppInputConnections[0]->u32ValidFrameCount,
                             GetSamplesPerFrame(),
                             m_iInterGainMFX);

                pf32InputFrames = pf32OutputFrames;
                Equalizer(pf32OutputFrames, pf32InputFrames,
                    ppInputConnections[0]->u32ValidFrameCount,
                    GetSamplesPerFrame(),
                    &m_EqMFX.m_LowFilter);

                pf32InputFrames = pf32OutputFrames;
                Equalizer(pf32OutputFrames, pf32InputFrames,
                    ppInputConnections[0]->u32ValidFrameCount,
                    GetSamplesPerFrame(),
                    &m_EqMFX.m_MidFilter);

                pf32InputFrames = pf32OutputFrames;
                Equalizer(pf32OutputFrames, pf32InputFrames,
                    ppInputConnections[0]->u32ValidFrameCount,
                    GetSamplesPerFrame(),
                    &m_EqMFX.m_HighFilter);

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
STDMETHODIMP CInterAPOMFX::GetLatency(HNSTIME* pTime)  
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
        *pTime = (m_fEnableInterMFX ? HNS_INTER : 0);
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
STDMETHODIMP CInterAPOMFX::LockForProcess(UINT32 u32NumInputConnections,
    APO_CONNECTION_DESCRIPTOR** ppInputConnections,  
    UINT32 u32NumOutputConnections, APO_CONNECTION_DESCRIPTOR** ppOutputConnections)
{
    ASSERT_NONREALTIME();
    HRESULT hr = S_OK;
    
    hr = CBaseAudioProcessingObject::LockForProcess(u32NumInputConnections,
        ppInputConnections, u32NumOutputConnections, ppOutputConnections);
    IF_FAILED_JUMP(hr, Exit);
    
    if (!IsEqualGUID(m_AudioProcessingMode, AUDIO_SIGNALPROCESSINGMODE_RAW) && m_fEnableInterMFX)
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
        "%!FUNC! MFXEn=%ld index=%lu hr=%!HRESULT!", m_fEnableInterMFX, m_iInterIndex, hr);
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

HRESULT CInterAPOMFX::Initialize(UINT32 cbDataSize, BYTE* pbyData)
{
    HRESULT                     hr = S_OK;
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

        // Save the processing mode being initialized.
        processingMode = papoSysFxInit2->AudioProcessingMode;

        // There is information in the APOInitSystemEffects2 structure that could help facilitate 
        // proprietary communication between an APO instance and the KS pin that the APO is initialized on
        // Eg, in the case that an APO is implemented as an effect proxy for the effect processing hosted inside
        // an driver (either host CPU based or offload DSP based), the example below uses a combination of 
        // IDeviceTopology, IConnector, and IKsControl interfaces to communicate with the underlying audio driver. 
        // the following following routine demonstrates how to implement how to communicate to an audio driver from a APO.
        ProprietaryCommunicationWithDriver(papoSysFxInit2);
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
    //  Get current effects settings
    //
    if (m_spAPOSystemEffectsProperties != NULL)
    {
        m_fEnableInterMFX = GetCurrentEffectsSetting(m_spAPOSystemEffectsProperties, PKEY_Endpoint_Enable_Interface_MFX, m_AudioProcessingMode);
        m_iInterGainMFX = GetCurrentEffectsSetting(m_spAPOSystemEffectsProperties, PKEY_Endpoint_Inter_Gain_Level_MFX, m_AudioProcessingMode);
        m_iInterEqLowMFX = GetCurrentEffectsSetting(m_spAPOSystemEffectsProperties, PKEY_Endpoint_Inter_EQ_Low_MFX, m_AudioProcessingMode);
        m_iInterEqMidMFX = GetCurrentEffectsSetting(m_spAPOSystemEffectsProperties, PKEY_Endpoint_Inter_EQ_Mid_MFX, m_AudioProcessingMode);
        m_iInterEqHighMFX = GetCurrentEffectsSetting(m_spAPOSystemEffectsProperties, PKEY_Endpoint_Inter_EQ_High_MFX, m_AudioProcessingMode);

        (&m_EqMFX.m_LowFilter)->Prepare(m_iInterEqLowMFX);
        (&m_EqMFX.m_MidFilter)->Prepare(m_iInterEqMidMFX);
        (&m_EqMFX.m_HighFilter)->Prepare(m_iInterEqHighMFX);
    }

    //
    //  Register for notification of registry updates
    //
    hr = m_spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
    IF_FAILED_JUMP(hr, Exit);

    hr = m_spEnumerator->RegisterEndpointNotificationCallback(this);
    IF_FAILED_JUMP(hr, Exit);

    m_bIsInitialized = true;
Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER,
        "<-- %!FUNC! hr=%!HRESULT! en=%ld MFX=%ld,%ld,%ld,%ld", hr, m_fEnableInterMFX,
        m_iInterGainMFX, m_iInterEqLowMFX, m_iInterEqMidMFX, m_iInterEqHighMFX);
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
STDMETHODIMP CInterAPOMFX::GetEffectsList(_Outptr_result_buffer_maybenull_(*pcEffects) LPGUID *ppEffectsIds, _Out_ UINT *pcEffects, _In_ HANDLE Event)
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
            { InterEffectId, m_fEnableInterMFX },
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

HRESULT CInterAPOMFX::ProprietaryCommunicationWithDriver(APOInitSystemEffects2 *_pAPOSysFxInit2)
{
    HRESULT hr = S_OK;    
    CComPtr<IMMDevice>	        spMyDevice;
    CComPtr<IDeviceTopology>    spMyDeviceTopology;
    CComPtr<IConnector>         spMyConnector;
    CComPtr<IPart>              spMyConnectorPart;
    CComPtr<IKsControl>         spKsControl;
    UINT                        uKsPinId = 0;
    UINT                        myPartId = 0;

    ULONG ulBytesReturned = 0;
    CComHeapPtr<KSMULTIPLE_ITEM> spKsMultipleItem;
    KSP_PIN ksPin = {0};

    UINT   nSoftwareIoDeviceInCollection = 0 ;
    UINT   nSoftwareIoConnectorIndex = 0 ;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! SmpPF=%lu", m_u32SamplesPerFrame);

    nSoftwareIoDeviceInCollection = _pAPOSysFxInit2->nSoftwareIoDeviceInCollection;
    nSoftwareIoConnectorIndex = _pAPOSysFxInit2->nSoftwareIoConnectorIndex;

    // Get the target IMMDevice
    hr = _pAPOSysFxInit2->pDeviceCollection->Item(nSoftwareIoDeviceInCollection, &spMyDevice);
    IF_FAILED_JUMP(hr, Exit);

    // Instantiate a device topology instance
    hr = spMyDevice->Activate(__uuidof(IDeviceTopology), CLSCTX_ALL, NULL, (void**)&spMyDeviceTopology);
    IF_FAILED_JUMP(hr, Exit);

    // retrieve connect instance
    hr = spMyDeviceTopology->GetConnector(nSoftwareIoConnectorIndex, &spMyConnector);
    IF_FAILED_JUMP(hr, Exit);

    // activate IKsControl on the IMMDevice
    hr = spMyDevice->Activate(__uuidof(IKsControl), CLSCTX_INPROC_SERVER, NULL, (void**)&spKsControl);
    IF_FAILED_JUMP(hr, Exit);

    // get KS pin id
    hr = spMyConnector->QueryInterface(__uuidof(IPart), (void**)&spMyConnectorPart);
    IF_FAILED_JUMP(hr, Exit);
    hr = spMyConnectorPart->GetLocalId(&myPartId);
    IF_FAILED_JUMP(hr, Exit);

    uKsPinId = myPartId & 0x0000ffff;

    ksPin.Property.Set = KSPROPSETID_SysVAD;
    ksPin.Property.Id = KSPROPERTY_SYSVAD_DEFAULTSTREAMEFFECTS;
    ksPin.Property.Flags = KSPROPERTY_TYPE_GET;
    ksPin.PinId = uKsPinId;

    // First, get size of array returned by driver
    hr = spKsControl->KsProperty( &ksPin.Property,
                                    sizeof(KSP_PIN),
                                    NULL,
                                    0,
                                    &ulBytesReturned );
    IF_FAILED_JUMP(hr, Exit);

    if( !spKsMultipleItem.AllocateBytes(ulBytesReturned) )
    {
        hr = E_OUTOFMEMORY;
        IF_FAILED_JUMP(hr, Exit);
    }

    // Second, now get the active effects from the driver
    hr = spKsControl->KsProperty( &ksPin.Property,
                                    sizeof(KSP_PIN),
                                    spKsMultipleItem,
                                    ulBytesReturned,
                                    &ulBytesReturned );
    IF_FAILED_JUMP(hr, Exit);

    // Upon successful return, effect guids could be found in the memory following (spKsMultipleItem.m_pData + 1)
    // and effectcount could be found in spKsMultipleItem->Count;

Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "<-- %!FUNC! hr=%!HRESULT!", hr);
    return hr;
}

//-------------------------------------------------------------------------
// Description:
//
//  Implementation of IMMNotificationClient::OnPropertyValueChanged
//
// Parameters:
//
//      pwstrDeviceId - [in] the id of the device whose property has changed
//      key - [in] the property that changed
//
// Return values:
//
//      Ignored by caller
//
// Remarks:
//
//      This method is called asynchronously.  No UI work should be done here.
//
HRESULT CInterAPOMFX::OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key)
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
    if (PK_EQUAL(key, PKEY_Endpoint_Enable_Interface_MFX) ||
        PK_EQUAL(key, PKEY_AudioEndpoint_Disable_SysFx))
    {
        // Synchronize access to the effects list and effects changed event
        m_EffectsLock.Enter();

        TraceEvents(TRACE_LEVEL_VERBOSE, TRACE_DRIVER, "%!FUNC! m_EffectsLock.Enter()");

        struct KeyControl
        {
            PROPERTYKEY key;
            LONG *value;
            BiQuadFilter* filter;
        };
        
        KeyControl controls[] =
        {
            { PKEY_Endpoint_Enable_Interface_MFX,        &m_fEnableInterMFX,   NULL  },
            { PKEY_Endpoint_Inter_Gain_Level_MFX,        &m_iInterGainMFX,     NULL  },
            { PKEY_Endpoint_Inter_EQ_Low_SFX,            &m_iInterEqLowMFX,    &m_EqMFX.m_LowFilter },
            { PKEY_Endpoint_Inter_EQ_Mid_SFX,            &m_iInterEqMidMFX,    &m_EqMFX.m_MidFilter },
            { PKEY_Endpoint_Inter_EQ_High_SFX,           &m_iInterEqHighMFX,   &m_EqMFX.m_HighFilter },
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
                nChanges, m_fEnableInterMFX, m_iInterGainMFX);
            SetEvent(m_hEffectsChangedEvent);
            nSetEvent++;
        }

        m_EffectsLock.Leave();
    }
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER,
        "<-- %!FUNC! hr=%!HRESULT! %ld en=%ld MFX=%ld,%ld,%ld,%ld", hr, nSetEvent, m_fEnableInterMFX,
        m_iInterGainMFX, m_iInterEqLowMFX, m_iInterEqMidMFX, m_iInterEqHighMFX);
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
CInterAPOMFX::~CInterAPOMFX(void)
{
    if (m_bIsInitialized)
    {
        //
        // unregister for callbacks
        //
        if (m_spEnumerator != NULL)
        {
            m_spEnumerator->UnregisterEndpointNotificationCallback(this);
        }
    }

    if (m_hEffectsChangedEvent != NULL)
    {
        CloseHandle(m_hEffectsChangedEvent);
    }

    // Free locked memory allocations
    if (NULL != m_pf32Coefficients)
    {
        AERT_Free(m_pf32Coefficients);
        m_pf32Coefficients = NULL;
    }
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! ...");
} // ~CInterAPOMFX


//-------------------------------------------------------------------------
// Description:
//
//  Validates input/output format pair during LockForProcess.
//
// Parameters:
//
//      u32NumInputConnections - [in] number of input connections attached to this APO
//      ppInputConnections - [in] format of each input connection attached to this APO
//      u32NumOutputConnections - [in] number of output connections attached to this APO
//      ppOutputConnections - [in] format of each output connection attached to this APO
//
// Return values:
//
//      S_OK                                Connections are valid.
//
// See Also:
//
//  CBaseAudioProcessingObject::LockForProcess
//
// Remarks:
//
//  This method is an internal call that is called by the default implementation of
//  CBaseAudioProcessingObject::LockForProcess().  This is called after the connections
//  are validated for simple conformance to the APO's registration properties.  It may be
//  used to verify that the APO is initialized properly and that the connections that are passed
//  agree with the data used for initialization.  Any failure code passed back from this
//  function will get returned by LockForProcess, and cause it to fail.
//
//  By default, this routine just ASSERTS and returns S_OK.
//
HRESULT CInterAPOMFX::ValidateAndCacheConnectionInfo(UINT32 u32NumInputConnections,
                APO_CONNECTION_DESCRIPTOR** ppInputConnections,
                UINT32 u32NumOutputConnections,
                APO_CONNECTION_DESCRIPTOR** ppOutputConnections)
{
    ASSERT_NONREALTIME();
    HRESULT hResult;
    CComPtr<IAudioMediaType> pFormat;
    UNCOMPRESSEDAUDIOFORMAT UncompInputFormat, UncompOutputFormat;
    FLOAT32 f32InverseChannelCount;

    UNREFERENCED_PARAMETER(u32NumInputConnections);
    UNREFERENCED_PARAMETER(u32NumOutputConnections);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! SmpPF=%lu", m_u32SamplesPerFrame);

    _ASSERTE(!m_bIsLocked);
    _ASSERTE(((0 == u32NumInputConnections) || (NULL != ppInputConnections)) &&
              ((0 == u32NumOutputConnections) || (NULL != ppOutputConnections)));

    EnterCriticalSection(&m_CritSec);

    // get the uncompressed formats and channel masks
    hResult = ppInputConnections[0]->pFormat->GetUncompressedAudioFormat(&UncompInputFormat);
    IF_FAILED_JUMP(hResult, Exit);
    
    hResult = ppOutputConnections[0]->pFormat->GetUncompressedAudioFormat(&UncompOutputFormat);
    IF_FAILED_JUMP(hResult, Exit);

    // Since we haven't overridden the IsIn{Out}putFormatSupported APIs in this example, this APO should
    // always have input channel count == output channel count.  The sampling rates should also be eqaul,
    // and formats 32-bit float.
    _ASSERTE(UncompOutputFormat.fFramesPerSecond == UncompInputFormat.fFramesPerSecond);
    _ASSERTE(UncompOutputFormat. dwSamplesPerFrame == UncompInputFormat.dwSamplesPerFrame);

    // Allocate some locked memory.  We will use these as scaling coefficients during APOProcess->GainControl
    hResult = AERT_Allocate(sizeof(FLOAT32)*m_u32SamplesPerFrame, (void**)&m_pf32Coefficients);
    IF_FAILED_JUMP(hResult, Exit);

    // Set scalars to decrease volume from 1.0 to 1.0/N where N is the number of channels
    // starting with the first channel.
    f32InverseChannelCount = 1.0f/m_u32SamplesPerFrame;
#pragma warning(push)
#pragma warning(disable:6386)
    UINT32 u32Index = 0;
    for (u32Index=0; u32Index<m_u32SamplesPerFrame; u32Index++)
    {
        m_pf32Coefficients[u32Index] = 1.0f - (FLOAT32)(f32InverseChannelCount)*u32Index;
    }
#pragma warning (pop)
    
Exit:
    LeaveCriticalSection(&m_CritSec);

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "<-- %!FUNC! hr=%!HRESULT!", hResult);

    return hResult;
}

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
// IAudioSystemEffectsCustomFormats implementation

//
// For demonstration purposes we will add 44.1KHz, 16-bit stereo and 48KHz, 16-bit
// stereo formats.  These formats should already be available in mmsys.cpl.  We
// embellish the labels to make it obvious that these formats are coming from
// the APO.
//

//
// Note that the IAudioSystemEffectsCustomFormats interface, if present, is invoked only on APOs 
// that attach directly to the connector in the 'DEFAULT' mode streaming graph. For example:
// - APOs implementing global effects
// - APOs implementing endpoint effects
// - APOs implementing DEFAULT mode effects which attach directly to a connector supporting DEFAULT processing mode

struct CUSTOM_FORMAT_ITEM
{
    WAVEFORMATEXTENSIBLE wfxFmt;
    LPCWSTR              pwszRep;
};

#define STATIC_KSDATAFORMAT_SUBTYPE_AC3\
    DEFINE_WAVEFORMATEX_GUID(WAVE_FORMAT_DOLBY_AC3_SPDIF)
DEFINE_GUIDSTRUCT("00000092-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_AC3);
#define KSDATAFORMAT_SUBTYPE_AC3 DEFINE_GUIDNAMED(KSDATAFORMAT_SUBTYPE_AC3)
 
CUSTOM_FORMAT_ITEM _rgCustomFormats[] =
{
    {{ WAVE_FORMAT_EXTENSIBLE, 2, 44100, 176400, 4, 16, sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX), 16, KSAUDIO_SPEAKER_STEREO, KSDATAFORMAT_SUBTYPE_PCM},  L"Custom #1 (really 44.1 KHz, 16-bit, stereo)"},
    {{ WAVE_FORMAT_EXTENSIBLE, 2, 48000, 192000, 4, 16, sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX), 16, KSAUDIO_SPEAKER_STEREO, KSDATAFORMAT_SUBTYPE_PCM},  L"Custom #2 (really 48 KHz, 16-bit, stereo)"}
    // The compressed AC3 format has been temporarily removed since the APO is not set up for compressed formats or EFXs yet.
    // {{ WAVE_FORMAT_EXTENSIBLE, 2, 48000, 192000, 4, 16, sizeof(WAVEFORMATEXTENSIBLE)-sizeof(WAVEFORMATEX), 16, KSAUDIO_SPEAKER_STEREO, KSDATAFORMAT_SUBTYPE_AC3},  L"Custom #3 (really 48 KHz AC-3)"}
};

#define _cCustomFormats (ARRAYSIZE(_rgCustomFormats))

//-------------------------------------------------------------------------
// Description:
//
//  Implementation of IAudioSystemEffectsCustomFormats::GetFormatCount
//
// Parameters:
//
//      pcFormats - [out] receives the number of formats to be added
//
// Return values:
//
//      S_OK        Success
//      E_POINTER   Null pointer passed
//
// Remarks:
//
STDMETHODIMP CInterAPOMFX::GetFormatCount
(
    UINT* pcFormats
)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! pcFormats=%p", pcFormats);

    if (pcFormats == NULL)
        return E_POINTER;

    *pcFormats = _cCustomFormats;
    return S_OK;
}

//-------------------------------------------------------------------------
// Description:
//
//  Implementation of IAudioSystemEffectsCustomFormats::GetFormat
//
// Parameters:
//
//      nFormat - [in] which format is being requested
//      IAudioMediaType - [in] address of a variable that will receive a ptr 
//                             to a new IAudioMediaType object
//
// Return values:
//
//      S_OK            Success
//      E_INVALIDARG    nFormat is out of range
//      E_POINTER       Null pointer passed
//
// Remarks:
//
STDMETHODIMP CInterAPOMFX::GetFormat
(
    UINT              nFormat, 
    IAudioMediaType** ppFormat
)
{
    HRESULT hr;

    IF_TRUE_ACTION_JUMP((nFormat >= _cCustomFormats), hr = E_INVALIDARG, Exit);
    IF_TRUE_ACTION_JUMP((ppFormat == NULL), hr = E_POINTER, Exit);

    *ppFormat = NULL; 

    hr = CreateAudioMediaType(  (const WAVEFORMATEX*)&_rgCustomFormats[nFormat].wfxFmt, 
                                sizeof(_rgCustomFormats[nFormat].wfxFmt),
                                ppFormat);

Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! hr=%!HRESULT!", hr);

    return hr;
}

//-------------------------------------------------------------------------
// Description:
//
//  Implementation of IAudioSystemEffectsCustomFormats::GetFormatRepresentation
//
// Parameters:
//
//      nFormat - [in] which format is being requested
//      ppwstrFormatRep - [in] address of a variable that will receive a ptr 
//                             to a new string description of the requested format
//
// Return values:
//
//      S_OK            Success
//      E_INVALIDARG    nFormat is out of range
//      E_POINTER       Null pointer passed
//
// Remarks:
//
STDMETHODIMP CInterAPOMFX::GetFormatRepresentation
(
    UINT                nFormat,
    _Outptr_ LPWSTR* ppwstrFormatRep
)
{
    HRESULT hr;
    size_t  cbRep;
    LPWSTR  pwstrLocal;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! nFormat=%lu", nFormat);

    IF_TRUE_ACTION_JUMP((nFormat >= _cCustomFormats), hr = E_INVALIDARG, Exit);
    IF_TRUE_ACTION_JUMP((ppwstrFormatRep == NULL), hr = E_POINTER, Exit);

    cbRep = (wcslen(_rgCustomFormats[nFormat].pwszRep) + 1) * sizeof(WCHAR);

    pwstrLocal = (LPWSTR)CoTaskMemAlloc(cbRep);
    IF_TRUE_ACTION_JUMP((pwstrLocal == NULL), hr = E_OUTOFMEMORY, Exit);

    hr = StringCbCopyW(pwstrLocal, cbRep, _rgCustomFormats[nFormat].pwszRep);
    if (FAILED(hr))
    {
        CoTaskMemFree(pwstrLocal);
    }
    else
    {
        *ppwstrFormatRep = pwstrLocal;
    }

Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "<-- %!FUNC! hr=%!HRESULT!", hr);
    return hr;
}

//-------------------------------------------------------------------------
// Description:
//
//  Implementation of IAudioProcessingObject::IsOutputFormatSupported
//
// Parameters:
//
//      pInputFormat - [in] A pointer to an IAudioMediaType interface. This parameter indicates the output format. This parameter must be set to NULL to indicate that the output format can be any type
//      pRequestedOutputFormat - [in] A pointer to an IAudioMediaType interface. This parameter indicates the output format that is to be verified
//      ppSupportedOutputFormat - [in] This parameter indicates the supported output format that is closest to the format to be verified
//
// Return values:
//
//      S_OK                            Success
//      S_FALSE                         The format of Input/output format pair is not supported. The ppSupportedOutPutFormat parameter returns a suggested new format
//      APOERR_FORMAT_NOT_SUPPORTED     The format is not supported. The value of ppSupportedOutputFormat does not change. 
//      E_POINTER                       Null pointer passed
//
// Remarks:
//
STDMETHODIMP CInterAPOMFX::IsOutputFormatSupported
(
    IAudioMediaType *pInputFormat, 
    IAudioMediaType *pRequestedOutputFormat, 
    IAudioMediaType **ppSupportedOutputFormat
)
{
    ASSERT_NONREALTIME();
    bool formatChanged = false;
    HRESULT hResult;
    UNCOMPRESSEDAUDIOFORMAT uncompOutputFormat;
    IAudioMediaType *recommendedFormat = NULL;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! ...");

    IF_TRUE_ACTION_JUMP((NULL == pRequestedOutputFormat) || (NULL == ppSupportedOutputFormat), hResult = E_POINTER, Exit);
    *ppSupportedOutputFormat = NULL;

    // Initial comparison to make sure the requested format is valid and consistent with the input
    // format. Because of the APO flags specified during creation, the samples per frame value will
    // not be validated.
    hResult = IsFormatTypeSupported(pInputFormat, pRequestedOutputFormat, &recommendedFormat, true);
    IF_FAILED_JUMP(hResult, Exit);

    // Check to see if a custom format from the APO was used.
    if (S_FALSE == hResult)
    {
        hResult = CheckCustomFormats(pRequestedOutputFormat);

        // If the output format is changed, make sure we track it for our return code.
        if (S_FALSE == hResult)
        {
            formatChanged = true;
        }
    }

    // now retrieve the format that IsFormatTypeSupported decided on, building upon that by adding
    // our channel count constraint.
    hResult = recommendedFormat->GetUncompressedAudioFormat(&uncompOutputFormat);
    IF_FAILED_JUMP(hResult, Exit);

    // If the requested format exactly matched our requirements,
    // just return it.
    if (!formatChanged)
    {
        *ppSupportedOutputFormat = pRequestedOutputFormat;
        (*ppSupportedOutputFormat)->AddRef();
        hResult = S_OK;
    }
    else // we're proposing something different, copy it and return S_FALSE
    {
        hResult = CreateAudioMediaTypeFromUncompressedAudioFormat(&uncompOutputFormat, ppSupportedOutputFormat);
        IF_FAILED_JUMP(hResult, Exit);
        hResult = S_FALSE;
    }

Exit:
    if (recommendedFormat)
    {
        recommendedFormat->Release();
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "<-- %!FUNC! hr=%!HRESULT!", hResult);

    return hResult;
}

HRESULT CInterAPOMFX::CheckCustomFormats(IAudioMediaType *pRequestedFormat)
{
    HRESULT hResult = S_OK;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! ...");

    for (int i = 0; i < _cCustomFormats; i++)
    {
        hResult = S_OK;
        const WAVEFORMATEX* waveFormat = pRequestedFormat->GetAudioFormat();

        if (waveFormat->wFormatTag != _rgCustomFormats[i].wfxFmt.Format.wFormatTag)
        {
            hResult = S_FALSE;
        }

        if (waveFormat->nChannels != _rgCustomFormats[i].wfxFmt.Format.nChannels)
        {
            hResult = S_FALSE;
        }

        if (waveFormat->nSamplesPerSec != _rgCustomFormats[i].wfxFmt.Format.nSamplesPerSec)
        {
            hResult = S_FALSE;
        }

        if (waveFormat->nAvgBytesPerSec != _rgCustomFormats[i].wfxFmt.Format.nAvgBytesPerSec)
        {
            hResult = S_FALSE;
        }

        if (waveFormat->nBlockAlign != _rgCustomFormats[i].wfxFmt.Format.nBlockAlign)
        {
            hResult = S_FALSE;
        }

        if (waveFormat->wBitsPerSample != _rgCustomFormats[i].wfxFmt.Format.wBitsPerSample)
        {
            hResult = S_FALSE;
        }

        if (waveFormat->cbSize != _rgCustomFormats[i].wfxFmt.Format.cbSize)
        {
            hResult = S_FALSE;
        }

        if (hResult == S_OK)
        {
            break;
        }
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "<-- %!FUNC! hr=%!HRESULT!", hResult);

    return hResult;
}