//
// InterAPODll.cpp -- Copyright (c) Microsoft Corporation. All rights reserved.
//
// Author:
//
// Description:
//
// InterAPODll.cpp : Implementation of DLL Exports.

#include <atlbase.h>
#include <atlcom.h>
#include <atlcoll.h>
#include <atlsync.h>
#include <mmreg.h>

#include "resource.h"
#include "InterAPODll.h"
#include <InterAPO.h>

#include <InterAPODll_i.c>

#include <EvnTrace.h>
#include "trace.h"
#include "InterAPODll.tmh"

//-------------------------------------------------------------------------
// Array of APO_REG_PROPERTIES structures implemented in this module.
// Each new APO implementation will be added to this array.
//
APO_REG_PROPERTIES const *gCoreAPOs[] =
{
    &CInterAPOMFX::sm_RegProperties.m_Properties,
    &CInterAPOSFX::sm_RegProperties.m_Properties
};

// {secret}
class CInterAPODllModule : public CAtlDllModuleT< CInterAPODllModule >
{
public :
    DECLARE_LIBID(LIBID_InterAPODlllib)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_INTERAPODLL, "{0A21D954-674A-4C09-806E-DB4FBE8F199C}")
public:
    HRESULT DllRegisterServer(BOOL bRegTypeLib = TRUE) throw();
    HRESULT DllUnregisterServer(BOOL bUnRegTypeLib = TRUE) throw();
};

// {secret}
CInterAPODllModule _AtlModule;


HRESULT CInterAPODllModule::DllRegisterServer(BOOL bRegTypeLib) throw()
{
    HRESULT hResult;
    UINT32 u32APORegIndex = 0;
    UINT32 u32APOUnregIndex = 0;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! ...");

    // Register all APOs implemented in this module.
    for (u32APORegIndex = 0; u32APORegIndex < SIZEOF_ARRAY(gCoreAPOs); u32APORegIndex++)
    {
        hResult = RegisterAPO(gCoreAPOs[u32APORegIndex]);
        if (FAILED(hResult))
        {
            goto ExitFailed;
        }
    }

    // Register the module.
    hResult = CAtlDllModuleT<CInterAPODllModule>::DllRegisterServer(bRegTypeLib);
    if (FAILED(hResult))
    {
        goto ExitFailed;
    }

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "<-- %!FUNC! hr=%!HRESULT!", hResult);

    return hResult;

ExitFailed:
    // Unregister all registered APOs if something failed.
    for (u32APOUnregIndex = 0; u32APOUnregIndex < u32APORegIndex; u32APOUnregIndex++)
    {
        ATLVERIFY(SUCCEEDED(UnregisterAPO(gCoreAPOs[u32APOUnregIndex]->clsid)));
    }

    TraceEvents(TRACE_LEVEL_ERROR, TRACE_DRIVER, "<-- %!FUNC! Failed hr=%!HRESULT!", hResult);

    return hResult;
} // DllRegisterServer

HRESULT CInterAPODllModule::DllUnregisterServer(BOOL bUnRegTypeLib) throw()
{
    HRESULT hResult;
    UINT32 u32APOUnregIndex = 0;

    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "--> %!FUNC! ...");

    // Unregister the module.
    hResult = CAtlDllModuleT<CInterAPODllModule>::UnregisterServer(bUnRegTypeLib);
    if (FAILED(hResult))
    {
        goto Exit;
    }

    // Unregister all the APOs that are implemented in this module.
    for (u32APOUnregIndex = 0; u32APOUnregIndex < SIZEOF_ARRAY(gCoreAPOs); u32APOUnregIndex++)
    {
        hResult = UnregisterAPO(gCoreAPOs[u32APOUnregIndex]->clsid);
        ATLASSERT(SUCCEEDED(hResult));
    }

Exit:
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "<-- %!FUNC! hr=%!HRESULT!", hResult);

    return hResult;
} // DllUnregisterServer

// {secret}
extern "C" BOOL WINAPI DllMain(HINSTANCE /* hInstance */, DWORD dwReason, LPVOID lpReserved)
{
    WPP_INIT_TRACING(NULL);

    if (DLL_PROCESS_ATTACH == dwReason)
    {
    }
    // do necessary cleanup only if the DLL is being unloaded dynamically
    else if ((DLL_PROCESS_DETACH == dwReason) && (NULL == lpReserved))
    {
    }

    return _AtlModule.DllMain(dwReason, lpReserved);
}


// {secret}
__control_entrypoint(DllExport)
STDAPI DllCanUnloadNow(void)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! ...");

    WPP_CLEANUP();

    return _AtlModule.DllCanUnloadNow();
}


// {secret}
_Check_return_
STDAPI DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ LPVOID FAR* ppv)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER,
        "%!FUNC! CLSID=%!GUID! IID=%!GUID! PID=%u", (LPCGUID)&rclsid, (LPCGUID)&riid, GetCurrentProcessId());

    return _AtlModule.DllGetClassObject(rclsid, riid, ppv);
}


// {secret}
__control_entrypoint(DllExport)
STDAPI DllRegisterServer(void)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER,
        "InterAMP APO: %!FUNC! built %s %s", __DATE__, __TIME__);

    // registers object, typelib and all interfaces in typelib
    HRESULT hr = _AtlModule.DllRegisterServer();
    return hr;
}


// {secret}
__control_entrypoint(DllExport)
STDAPI DllUnregisterServer(void)
{
    TraceEvents(TRACE_LEVEL_INFORMATION, TRACE_DRIVER, "%!FUNC! ...");

    HRESULT hr = _AtlModule.DllUnregisterServer();
    return hr;
}
