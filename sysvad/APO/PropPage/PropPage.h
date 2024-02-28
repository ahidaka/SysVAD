//**@@@*@@@****************************************************
//
// Microsoft Windows
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//**@@@*@@@****************************************************

//
// FileName:    InterPropPage.h
//
// Abstract:    Declaration of the CInterPropPage class
//
// ----------------------------------------------------------------------------


#pragma once

// ----------------------------------------------------------------------
// CInterPropPage class
//
class ATL_NO_VTABLE CInterPropPage :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CInterPropPage, &CLSID_InterPropPage>,
    public IDispatchImpl<IInterPropPage, &__uuidof(IInterPropPage), &LIBID_CplExtLib, /*wMajor =*/ 1, /*wMinor =*/ 0>,
    public IShellExtInit,
    public IShellPropSheetExt
{
public:
    CInterPropPage();
    ~CInterPropPage();

    DECLARE_REGISTRY_RESOURCEID(IDR_INTER_PROP_PAGE)

    BEGIN_COM_MAP(CInterPropPage)
        COM_INTERFACE_ENTRY(IInterPropPage)
        COM_INTERFACE_ENTRY(IDispatch)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IShellPropSheetExt)
    END_COM_MAP()

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct()
    {
        return S_OK;
    }

    void FinalRelease()
    {
    }

    static INT_PTR CALLBACK DialogProcPage1(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static UINT CALLBACK PropSheetPageProc(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);

    // IShellExtInit
    STDMETHOD(Initialize)(_In_opt_ LPCITEMIDLIST pidlFolder, _In_opt_ IDataObject* pdtobj, _In_opt_ HKEY hkeyProgID);

    // IShellPropSheetExt
    STDMETHOD(AddPages)(_In_ LPFNADDPROPSHEETPAGE lpfnAddPage, _In_ LPARAM lParam);
    STDMETHOD(ReplacePage)(_In_ UINT uPageID, _In_ LPFNSVADDPROPSHEETPAGE lpfnReplaceWith, _In_ LPARAM lParam);

private:
    AudioFXExtensionParams* m_pAudioFXExtParams;
    BOOL m_fDisableSysFX;
    BOOL m_fEnableInterSFX;
    BOOL m_fEnableInterMFX;
    LONG m_dwInterGainSFX;
    LONG m_dwInterGainMFX;
    LONG m_dwInterBassSFX;
    LONG m_dwInterBassMFX;
    LONG m_dwInterMidrangeSFX;
    LONG m_dwInterMidrangeMFX;
    LONG m_dwInterTrebleSFX;
    LONG m_dwInterTrebleMFX;

    HRESULT GetDeviceFriendlyName(_Outptr_result_maybenull_ LPWSTR* ppNameOut);
    HRESULT RetrieveSysFXState(BOOL *pfDisabled);
    HRESULT SetSysFXState();

    HRESULT RetrieveInterSFXState(BOOL *pfEnabled);
    HRESULT SetInterSFXState();

    HRESULT RetrieveInterMFXState(BOOL *pfEnabled);
    HRESULT SetInterMFXState();

#if 1 // Currently, we use Retrieve functions only for SFX.
    HRESULT RetrieveInterGainSFXValue(DWORD *pdwValue);
//    HRESULT SetInterGainSFXValue(DWORD dwPosValue);

//    HRESULT RetrieveInterGainMFXValue(DWORD* pdwValue);
//    HRESULT SetInterGainMFXValue(DWORD dwPosValue);

    HRESULT RetrieveInterBassSFXValue(DWORD* pdwValue);
//    HRESULT SetInterBassSFXValue(DWORD dwPosValue);

//    HRESULT RetrieveInterBassMFXValue(DWORD* pdwValue);
//    HRESULT SetInterBassMFXValue(DWORD dwPosValue);

    HRESULT RetrieveInterMidrangeSFXValue(DWORD* pdwValue);
//    HRESULT SetInterMidrangeSFXValue(DWORD dwPosValue);

//    HRESULT RetrieveInterMidrangeMFXValue(DWORD* pdwValue);
//    HRESULT SetInterMidrangeMFXValue(DWORD dwPosValue);

    HRESULT RetrieveInterTrebleSFXValue(DWORD* pdwValue);
//    HRESULT SetInterTrebleSFXValue(DWORD dwPosValue);

//    HRESULT RetrieveInterTrebleMFXValue(DWORD* pdwValue);
//    HRESULT SetInterTrebleMFXValue(DWORD dwPosValue);
#endif
    BOOL OnInitDialog(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
    BOOL OnApply(HWND hwndDlg);
    BOOL OnCheckBoxClickedDisableSysFX(HWND hwndDlg);
    BOOL OnCheckBoxClickedEnableInterSFX(HWND hwndDlg);
    BOOL OnCheckBoxClickedEnableInterMFX(HWND hwndDlg);

    const DWORD INTER_GAIN_THRESHOLD = 50;
    const DWORD INTER_EQ_THRESHOLD = 20;

//
//
//    These are functions that relates three types of elements.
//    The basic requirements are as follows
//
//    Reg is a Windows registry, unsigned 32bit variable.
//    The important point is that this value is treated as a 32 bit unsigned number when reading and writing as a registry,
//    but when interfacing with other values, it is treated as a 32 - bit signed integer.
//
//    Value is the gain number.The maximum is + 50 and the minimum is - 50.
//
//    Bar is the setting barcode value.The maximum is 100 and the minimum is 0.
//

    LONG CnvertGainFromBarToValue(DWORD bar)
    {
        return bar - INTER_GAIN_THRESHOLD;
    }

    LONG ConvertGainFromValueToBar(LONG value)
    {
        return value + INTER_GAIN_THRESHOLD;
    }

    LONG ConvertGainFromRegToValue(DWORD reg)
    {
        LONG value = (LONG32)reg;
        return value;
    }

    DWORD ConvertGainFromValueToReg(LONG value)
    {
        DWORD reg = value & 0xFFFFFFFF;
        return reg;
    }

    LONG CnvertEqLevelFromBarToValue(DWORD bar)
	{
        return bar * INTER_EQ_THRESHOLD / 50 - INTER_EQ_THRESHOLD;
	}

    LONG ConvertEqLevelFromValueToBar(LONG value)   
	{
        return (value * 50 / INTER_EQ_THRESHOLD) + 50;
    }

    LONG ConvertEqLevelFromRegToValue(DWORD reg)
	{
		LONG value = (LONG32)reg;
		return value;
	}

    DWORD ConvertEqLevelFromValueToReg(LONG value)
    {
		DWORD reg = value & 0xFFFFFFFF;
		return reg;
	}
    
    //
    //
    //
    BOOL ValidateGainValue(DWORD reg)
    {
        LONG l32 = (LONG)reg;
        return l32; // l32 > (0 - INTER_GAIN_THRESHOLD) && l32 < INTER_GAIN_THRESHOLD;
    }

    BOOL ValidateEqLevel(DWORD reg)
    {
        LONG l32 = (LONG)reg;
        return l32; // l32 > (0 - INTER_EQ_THRESHOLD) && l32 < INTER_EQ_THRESHOLD;
    }

    LONG CorrectGainValue(DWORD reg)
    {
        LONG l32 = (LONG)reg;
        LONG x;
        if (l32 < (x = 0 - INTER_GAIN_THRESHOLD))
        {
            reg = (LONG)(x = 0 - INTER_GAIN_THRESHOLD);
        }
        else if (l32 >= (x = INTER_GAIN_THRESHOLD))
        {
            reg = INTER_GAIN_THRESHOLD;
        }
        return reg;
    }

    LONG CorrectEqLevel(DWORD reg)
    {
        LONG l32 = (LONG)reg;
        LONG x;
        if (l32 < (x = 0 - INTER_EQ_THRESHOLD))
        {
            reg = (LONG) (x = 0 - INTER_EQ_THRESHOLD);
        }
        else if (l32 >= (x = INTER_EQ_THRESHOLD))
        {
            reg = INTER_EQ_THRESHOLD;
        }
        return reg;
    }

    //
    // These are functions that relates three types of elements.
    //
    // 

};

OBJECT_ENTRY_AUTO(__uuidof(InterPropPage), CInterPropPage)
