//
// InterAPO.h -- Copyright (c) Microsoft Corporation. All rights reserved.
//
// Description:
//
//   Declaration of the CInterAPO class.
//

#pragma once

#include <audioenginebaseapo.h>
#include <BaseAudioProcessingObject.h>
#include <InterAPOInterface.h>
#include <InterAPODll.h>

#include <commonmacros.h>
#include <devicetopology.h>
#include <float.h>
#include "BiQuadFilter.h"

_Analysis_mode_(_Analysis_code_type_user_driver_)

#define PK_EQUAL(x, y)  ((x.fmtid == y.fmtid) && (x.pid == y.pid))

//
// Define a GUID identifying the type of this APO's custom effect.
//
// APOs generally should not define new GUIDs for types of effects and instead
// should use predefined effect types. Only define a new GUID if the effect is
// truly very different from all predefined types of effects.
//
// {33F1D858-3570-4B5A-81E4-14AAEC438C3A}
DEFINE_GUID(InterEffectId, 0x33f1d858, 0x3570, 0x4b5a, 0x81, 0xe4, 0x14, 0xaa, 0xec, 0x43, 0x8c, 0x3a);

// 1000 ms of inter
#define HNS_INTER HNS_PER_SECOND

#define FRAMES_FROM_HNS(hns) (ULONG)(1.0 * hns / HNS_PER_SECOND * GetFramesPerSecond() + 0.5)

LONG GetCurrentEffectsSetting(IPropertyStore* properties, PROPERTYKEY pkeyEnable, GUID processingMode);

#pragma AVRT_VTABLES_BEGIN
// Inter APO class - MFX
class CInterAPOMFX :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CInterAPOMFX, &CLSID_InterAPOMFX>,
    public CBaseAudioProcessingObject,
    public IMMNotificationClient,
    public IAudioSystemEffects2,
    // IAudioSystemEffectsCustomFormats may be optionally supported
    // by APOs that attach directly to the connector in the DEFAULT mode streaming graph
    public IAudioSystemEffectsCustomFormats, 
    public IInterAPOMFX
{
public:
    // constructor
    CInterAPOMFX()
    :   CBaseAudioProcessingObject(sm_RegProperties)
        ,   m_hEffectsChangedEvent(NULL)
        ,   m_AudioProcessingMode(AUDIO_SIGNALPROCESSINGMODE_DEFAULT)
        ,   m_fEnableInterMFX(FALSE)
        ,   m_nInterFrames(0)
        ,   m_iInterIndex(0)
        ,   m_iInterGainMFX(0)
        ,   m_iInterEqLowMFX(0)
        ,   m_iInterEqMidMFX(0)
        ,   m_iInterEqHighMFX(0)
        ,   m_EqMFX()        
    {
        m_pf32Coefficients = NULL;
    }

    virtual ~CInterAPOMFX();    // destructor

DECLARE_REGISTRY_RESOURCEID(IDR_INTERAPOMFX)

BEGIN_COM_MAP(CInterAPOMFX)
    COM_INTERFACE_ENTRY(IInterAPOMFX)
    COM_INTERFACE_ENTRY(IAudioSystemEffects)
    COM_INTERFACE_ENTRY(IAudioSystemEffects2)
    // IAudioSystemEffectsCustomFormats may be optionally supported
    // by APOs that attach directly to the connector in the DEFAULT mode streaming graph
    COM_INTERFACE_ENTRY(IAudioSystemEffectsCustomFormats)
    COM_INTERFACE_ENTRY(IMMNotificationClient)
    COM_INTERFACE_ENTRY(IAudioProcessingObjectRT)
    COM_INTERFACE_ENTRY(IAudioProcessingObject)
    COM_INTERFACE_ENTRY(IAudioProcessingObjectConfiguration)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    STDMETHOD_(void, APOProcess)(UINT32 u32NumInputConnections,
        APO_CONNECTION_PROPERTY** ppInputConnections, UINT32 u32NumOutputConnections,
        APO_CONNECTION_PROPERTY** ppOutputConnections);

    STDMETHOD(GetLatency)(HNSTIME* pTime);

    STDMETHOD(LockForProcess)(UINT32 u32NumInputConnections,
        APO_CONNECTION_DESCRIPTOR** ppInputConnections,  
        UINT32 u32NumOutputConnections, APO_CONNECTION_DESCRIPTOR** ppOutputConnections);

    STDMETHOD(Initialize)(UINT32 cbDataSize, BYTE* pbyData);

    // IAudioSystemEffects2
    STDMETHOD(GetEffectsList)(_Outptr_result_buffer_maybenull_(*pcEffects)  LPGUID *ppEffectsIds, _Out_ UINT *pcEffects, _In_ HANDLE Event);

    virtual HRESULT ValidateAndCacheConnectionInfo(
                                    UINT32 u32NumInputConnections, 
                                    APO_CONNECTION_DESCRIPTOR** ppInputConnections, 
                                    UINT32 u32NumOutputConnections, 
                                    APO_CONNECTION_DESCRIPTOR** ppOutputConnections);

    // IMMNotificationClient
    STDMETHODIMP OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) 
    { 
        UNREFERENCED_PARAMETER(pwstrDeviceId); 
        UNREFERENCED_PARAMETER(dwNewState); 
        return S_OK; 
    }
    STDMETHODIMP OnDeviceAdded(LPCWSTR pwstrDeviceId)
    { 
        UNREFERENCED_PARAMETER(pwstrDeviceId); 
        return S_OK; 
    }
    STDMETHODIMP OnDeviceRemoved(LPCWSTR pwstrDeviceId)
    { 
        UNREFERENCED_PARAMETER(pwstrDeviceId); 
        return S_OK; 
    }
    STDMETHODIMP OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
    { 
        UNREFERENCED_PARAMETER(flow); 
        UNREFERENCED_PARAMETER(role); 
        UNREFERENCED_PARAMETER(pwstrDefaultDeviceId); 
        return S_OK; 
    }
    STDMETHODIMP OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);

    // IAudioSystemEffectsCustomFormats
    // This interface may be optionally supported by APOs that attach directly to the connector in the DEFAULT mode streaming graph
    STDMETHODIMP GetFormatCount(UINT* pcFormats);
    STDMETHODIMP GetFormat(UINT nFormat, IAudioMediaType** ppFormat);
    STDMETHODIMP GetFormatRepresentation(UINT nFormat, _Outptr_ LPWSTR* ppwstrFormatRep);

    // IAudioProcessingObject
    STDMETHODIMP IsOutputFormatSupported(IAudioMediaType *pOppositeFormat, IAudioMediaType *pRequestedOutputFormat, IAudioMediaType **ppSupportedOutputFormat);

    STDMETHODIMP CheckCustomFormats(IAudioMediaType *pRequestedFormat);

public:
    LONG                                    m_fEnableInterMFX;
    GUID                                    m_AudioProcessingMode;
    CComPtr<IPropertyStore>                 m_spAPOSystemEffectsProperties;
    CComPtr<IMMDeviceEnumerator>            m_spEnumerator;
    static const CRegAPOProperties<1>       sm_RegProperties;   // registration properties

    // Locked memory
    FLOAT32                                 *m_pf32Coefficients;

    CComHeapPtr<FLOAT32>                    m_pf32InterBuffer;
    UINT32                                  m_nInterFrames;
    UINT32                                  m_iInterIndex;
    LONG                                    m_iInterGainMFX;
    LONG                                    m_iInterEqLowMFX;
    LONG                                    m_iInterEqMidMFX;
    LONG                                    m_iInterEqHighMFX;

    GraphicEqualizer						m_EqMFX;

private:
    CCriticalSection                        m_EffectsLock;
    HANDLE                                  m_hEffectsChangedEvent;

    HRESULT ProprietaryCommunicationWithDriver(APOInitSystemEffects2 *_pAPOSysFxInit2);

};
#pragma AVRT_VTABLES_END


#pragma AVRT_VTABLES_BEGIN
// Inter APO class - SFX
class CInterAPOSFX :
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CInterAPOSFX, &CLSID_InterAPOSFX>,
    public CBaseAudioProcessingObject,
    public IMMNotificationClient,
    public IAudioSystemEffects2,
    public IInterAPOSFX
{
public:
    // constructor
    CInterAPOSFX()
        : CBaseAudioProcessingObject(sm_RegProperties)
        , m_hEffectsChangedEvent(NULL)
        , m_AudioProcessingMode(AUDIO_SIGNALPROCESSINGMODE_DEFAULT)
        , m_fEnableInterSFX(FALSE)
        , m_nInterFrames(0)
        , m_iInterIndex(0)
        , m_iInterGainSFX(0)
        , m_iInterEqLowSFX(0)
        , m_iInterEqMidSFX(0)
        , m_iInterEqHighSFX(0)
        , m_EqSFX()
    {
    }

    virtual ~CInterAPOSFX();    // destructor

DECLARE_REGISTRY_RESOURCEID(IDR_INTERAPOSFX)

BEGIN_COM_MAP(CInterAPOSFX)
    COM_INTERFACE_ENTRY(IInterAPOSFX)
    COM_INTERFACE_ENTRY(IAudioSystemEffects)
    COM_INTERFACE_ENTRY(IAudioSystemEffects2)
    COM_INTERFACE_ENTRY(IMMNotificationClient)
    COM_INTERFACE_ENTRY(IAudioProcessingObjectRT)
    COM_INTERFACE_ENTRY(IAudioProcessingObject)
    COM_INTERFACE_ENTRY(IAudioProcessingObjectConfiguration)
END_COM_MAP()

DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    STDMETHOD_(void, APOProcess)(UINT32 u32NumInputConnections,
        APO_CONNECTION_PROPERTY** ppInputConnections, UINT32 u32NumOutputConnections,
        APO_CONNECTION_PROPERTY** ppOutputConnections);

    STDMETHOD(GetLatency)(HNSTIME* pTime);

    STDMETHOD(LockForProcess)(UINT32 u32NumInputConnections,
        APO_CONNECTION_DESCRIPTOR** ppInputConnections,  
        UINT32 u32NumOutputConnections, APO_CONNECTION_DESCRIPTOR** ppOutputConnections);

    STDMETHOD(Initialize)(UINT32 cbDataSize, BYTE* pbyData);

    // IAudioSystemEffects2
    STDMETHOD(GetEffectsList)(_Outptr_result_buffer_maybenull_(*pcEffects)  LPGUID *ppEffectsIds, _Out_ UINT *pcEffects, _In_ HANDLE Event);

    // IMMNotificationClient
    STDMETHODIMP OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState) 
    { 
        UNREFERENCED_PARAMETER(pwstrDeviceId); 
        UNREFERENCED_PARAMETER(dwNewState); 
        return S_OK; 
    }
    STDMETHODIMP OnDeviceAdded(LPCWSTR pwstrDeviceId)
    { 
        UNREFERENCED_PARAMETER(pwstrDeviceId); 
        return S_OK; 
    }
    STDMETHODIMP OnDeviceRemoved(LPCWSTR pwstrDeviceId)
    { 
        UNREFERENCED_PARAMETER(pwstrDeviceId); 
        return S_OK; 
    }
    STDMETHODIMP OnDefaultDeviceChanged(EDataFlow flow, ERole role, LPCWSTR pwstrDefaultDeviceId)
    { 
        UNREFERENCED_PARAMETER(flow); 
        UNREFERENCED_PARAMETER(role); 
        UNREFERENCED_PARAMETER(pwstrDefaultDeviceId); 
        return S_OK; 
    }
    STDMETHODIMP OnPropertyValueChanged(LPCWSTR pwstrDeviceId, const PROPERTYKEY key);

public:
    LONG                                    m_fEnableInterSFX;
    GUID                                    m_AudioProcessingMode;
    CComPtr<IPropertyStore>                 m_spAPOSystemEffectsProperties;
    CComPtr<IMMDeviceEnumerator>            m_spEnumerator;
    static const CRegAPOProperties<1>       sm_RegProperties;   // registration properties

    CCriticalSection                        m_EffectsLock;
    HANDLE                                  m_hEffectsChangedEvent;

    CComHeapPtr<FLOAT32>                    m_pf32InterBuffer;
    UINT32                                  m_nInterFrames;
    UINT32                                  m_iInterIndex;
    LONG                                    m_iInterGainSFX;
    LONG                                    m_iInterEqLowSFX;
    LONG                                    m_iInterEqMidSFX;
    LONG                                    m_iInterEqHighSFX;

    GraphicEqualizer						m_EqSFX;
};
#pragma AVRT_VTABLES_END

OBJECT_ENTRY_AUTO(__uuidof(InterAPOMFX), CInterAPOMFX)
OBJECT_ENTRY_AUTO(__uuidof(InterAPOSFX), CInterAPOSFX)

//
//   Declaration of the GainControl routine.
//
void GainControl(
    _Out_writes_(u32ValidFrameCount* u32SamplesPerFrame)
    FLOAT32* pf32OutputFrames,
    _In_reads_(u32ValidFrameCount* u32SamplesPerFrame)
    const FLOAT32* pf32InputFrames,
    UINT32       u32ValidFrameCount,
    UINT32       u32SamplesPerFrame,
    INT32        i32GainLevel);

//
//   Convenience methods
//

void WriteSilence(
    _Out_writes_(u32FrameCount * u32SamplesPerFrame)
        FLOAT32 *pf32Frames,
    UINT32 u32FrameCount,
    UINT32 u32SamplesPerFrame );

void CopyFrames(
    _Out_writes_(u32FrameCount * u32SamplesPerFrame)
        FLOAT32 *pf32OutFrames,
    _In_reads_(u32FrameCount * u32SamplesPerFrame)
        const FLOAT32 *pf32InFrames,
    UINT32 u32FrameCount,
    UINT32 u32SamplesPerFrame );

//
//  Call graphic equalizer
//
void Equalizer(
    _Out_writes_(u32ValidFrameCount* u32SamplesPerFrame)
    FLOAT32* pf32OutputFrames,
    _In_reads_(u32ValidFrameCount* u32SamplesPerFrame)
    const FLOAT32* pf32InputFrames,
    UINT32       u32ValidFrameCount,
    UINT32       u32SamplesPerFrame,
    BiQuadFilter *filter);
