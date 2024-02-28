//**@@@*@@@****************************************************
//
// Microsoft Windows
// Copyright (C) Microsoft Corporation. All rights reserved.
//
//**@@@*@@@****************************************************

//
// FileName:    InterPropPage.cpp
//
// Abstract:    Implementation of the CInterPropPage class
//
// ----------------------------------------------------------------------------


#include "stdafx.h"
#include <initguid.h>   // DEFINE_GUID
#include <mmdeviceapi.h>
#include <audioenginebaseapo.h>
#include <audioendpoints.h>
#include "PropPage.h"
#include <functiondiscoverykeys.h>
#include <CustomPropKeys.h>
#include "cplext_i.c"

_Analysis_mode_(_Analysis_code_type_user_driver_)

// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::CInterPropPage
//
// Description:
//      CInterPropPage constructor
// ----------------------------------------------------------------------------
CInterPropPage::CInterPropPage()
:   m_pAudioFXExtParams(NULL)
,   m_fDisableSysFX(FALSE)
,   m_fEnableInterSFX(FALSE)
,   m_fEnableInterMFX(FALSE)
,   m_dwInterGainSFX(0)
,   m_dwInterGainMFX(0)
,   m_dwInterBassSFX(0)
,   m_dwInterBassMFX(0)
,   m_dwInterMidrangeSFX(0)
,   m_dwInterMidrangeMFX(0)
,   m_dwInterTrebleSFX(0)
,   m_dwInterTrebleMFX(0)
{
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::~CInterPropPage
//
// Description:
//      CInterPropPage destructor
// ----------------------------------------------------------------------------
CInterPropPage::~CInterPropPage()
{
    SAFE_RELEASE(m_pAudioFXExtParams->pFxProperties);
    SAFE_DELETE(m_pAudioFXExtParams);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::GetDeviceFriendlyName
//
// Description:
//      Retrieves the endpoint's friendly name
//
// Parameters:
//      ppNameOut - [out] The friendly name of the endpoint
//
// Return values:
//      S_OK if successful
// ----------------------------------------------------------------------------
HRESULT CInterPropPage::GetDeviceFriendlyName
(
    _Outptr_result_maybenull_ LPWSTR* ppNameOut
)
{
    CComPtr<IMMDeviceEnumerator>    spEnumerator;
    CComPtr<IPropertyStore>         spProperties;
    CComPtr<IMMDevice>              spMMDevice;
    HRESULT                         hr = S_OK;
    PROPVARIANT                     var;

    IF_TRUE_ACTION_JUMP((ppNameOut == NULL), hr = E_POINTER, Exit);

    *ppNameOut = NULL;

    // Create device enumerator and get IMMDevice from the device ID
    hr = spEnumerator.CoCreateInstance(__uuidof(MMDeviceEnumerator));
    IF_FAILED_JUMP(hr, Exit);

    hr = spEnumerator->GetDevice(m_pAudioFXExtParams->pwstrEndpointID, &spMMDevice);
    IF_FAILED_JUMP(hr, Exit);

    // Open the PropertyStore for read access
    hr = spMMDevice->OpenPropertyStore(STGM_READ, &spProperties);
    IF_FAILED_JUMP(hr, Exit);

    PropVariantInit(&var);

    // Retrieve the friendly name of the endpoint
    hr = spProperties->GetValue(PKEY_Device_FriendlyName, &var);
    if (SUCCEEDED(hr) && (var.vt == VT_LPWSTR))
    {
        *ppNameOut = var.pwszVal;
    }
    else
    {
        PropVariantClear(&var);
    }

Exit:
    return(hr);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::RetrieveSysFXState
//
// Description:
//      Get the current state (enabled or disabled) of system effects
//
// Return values:
//      S_OK if successful
// ----------------------------------------------------------------------------
HRESULT CInterPropPage::RetrieveSysFXState(BOOL *pfDisabled)
{
    HRESULT hr = E_POINTER;

    if ((m_pAudioFXExtParams != NULL) && (m_pAudioFXExtParams->pFxProperties != NULL))
    {
        PROPVARIANT var;
        PropVariantInit(&var);

        // Get the state of whether system effects are enabled or not
        hr = m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_AudioEndpoint_Disable_SysFx, &var);
        if (SUCCEEDED(hr) && (var.vt == VT_UI4))
        {
            *pfDisabled = (var.ulVal != 0L);
        }

        PropVariantClear(&var);
    }

    return(hr);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::SetSysFXState
//
// Description:
//      Enable or disable system effects
//
// Return values:
//      S_OK if successful
// ----------------------------------------------------------------------------
HRESULT CInterPropPage::SetSysFXState()
{
    BOOL fCurrentState = 0;
    HRESULT hr = RetrieveSysFXState(&fCurrentState);
    IF_FAILED_JUMP(hr, Exit);

    if (fCurrentState != m_fDisableSysFX)
    {
        PROPVARIANT var;
        var.vt = VT_UI4;
        var.ulVal = (m_fDisableSysFX ? 1L : 0L);

        // Enable or disable SysFX
        hr = m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_AudioEndpoint_Disable_SysFx, var);
    }
    
Exit:
    return(hr);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::RetrieveInterSFXState
//
// Description:
//      Get the current state (enabled or disabled) of inter SFX
//
// Return values:
//      S_OK if successful
// ----------------------------------------------------------------------------
HRESULT CInterPropPage::RetrieveInterSFXState(BOOL *pfEnabled)
{
    HRESULT hr = E_POINTER;

    if ((m_pAudioFXExtParams != NULL) && (m_pAudioFXExtParams->pFxProperties != NULL))
    {
        PROPVARIANT var;
        PropVariantInit(&var);

        // Get the state of whether inter SFX is enabled or not
        hr = m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_Endpoint_Enable_Interface_SFX, &var);
        if (SUCCEEDED(hr) && (var.vt == VT_UI4))
        {
            *pfEnabled = (var.ulVal != 0L);
        }

        PropVariantClear(&var);
    }

    return(hr);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::SetInterSFXState
//
// Description:
//      Enable or disable inter SFX
//
// Return values:
//      S_OK if successful
// ----------------------------------------------------------------------------
HRESULT CInterPropPage::SetInterSFXState()
{
    BOOL fCurrentState = 0;
    HRESULT hr = RetrieveInterSFXState(&fCurrentState);
    IF_FAILED_JUMP(hr, Exit);

    if (fCurrentState != m_fEnableInterSFX)
    {
        PROPVARIANT var;
        var.vt = VT_UI4;
        var.ulVal = (m_fEnableInterSFX ? 1L : 0L);

        // Enable or disable inter SFX
        hr = m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Enable_Interface_SFX, var);

        //hr = m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_Endpoint_Inter_Gain_Level_SFX, &var);//??//
        //var.ulVal += 1;
        //hr = m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_Gain_Level_SFX, var);
    }

Exit:
    return(hr);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::RetrieveInterMFXState
//
// Description:
//      Get the current state (enabled or disabled) of inter MFX
//
// Return values:
//      S_OK if successful
// ----------------------------------------------------------------------------
HRESULT CInterPropPage::RetrieveInterMFXState(BOOL *pfEnabled)
{
    HRESULT hr = E_POINTER;

    if ((m_pAudioFXExtParams != NULL) && (m_pAudioFXExtParams->pFxProperties != NULL))
    {
        PROPVARIANT var;
        PropVariantInit(&var);

        // Get the state of whether channel inter MFX is enabled or not
        hr = m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_Endpoint_Enable_Interface_MFX, &var);
        if (SUCCEEDED(hr) && (var.vt == VT_UI4))
        {
            *pfEnabled = (var.ulVal != 0L);
        }

        PropVariantClear(&var);
    }

    return(hr);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::SetInterMFXState
//
// Description:
//      Enable or disable inter MFX
//
// Return values:
//      S_OK if successful
// ----------------------------------------------------------------------------
HRESULT CInterPropPage::SetInterMFXState()
{
    BOOL fCurrentState = 0;
    HRESULT hr = RetrieveInterMFXState(&fCurrentState);
    IF_FAILED_JUMP(hr, Exit);

    if (fCurrentState != m_fEnableInterMFX)
    {
        PROPVARIANT var;
        var.vt = VT_UI4;
        var.ulVal = (m_fEnableInterMFX ? 1L : 0L);

        // Enable or disable inter MFX
        hr = m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Enable_Interface_MFX, var);

        // Enabling or disabling the inter effect can be done with the engine running
        // It does not invalidate anything which should be locked by IAudioProcessingObjectConfiguration::LockForProcess
        // In particular, it does not impact IAudioProcessingObject::GetLatency
        // So we do not need to call IAudioEndpointFormatControl::ResetToDefault here
    }

Exit:
    return(hr);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::RetrieveInterGainSFXValue
//
// Description:
//      Get the current gain value EQ SFX
//
// Return values:
//      S_OK if successful
// ----------------------------------------------------------------------------
HRESULT CInterPropPage::RetrieveInterGainSFXValue(DWORD *pdwValue)
{
    HRESULT hr = E_POINTER;

    if ((m_pAudioFXExtParams != NULL) && (m_pAudioFXExtParams->pFxProperties != NULL) && (pdwValue != NULL))
    {
        PROPVARIANT var;
        PropVariantInit(&var);

        // Get the state of whether inter SFX is enabled or not
        hr = m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_Endpoint_Inter_Gain_Level_SFX, &var);
        if (SUCCEEDED(hr) && (var.vt == VT_UI4))
        {
            if (ValidateGainValue(var.ulVal))
            {
                var.ulVal = CorrectGainValue(var.ulVal);
                hr = m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_Gain_Level_SFX, var);
            }
            *pdwValue = var.ulVal;
        }

        PropVariantClear(&var);
    }

    return(hr);
}


#if 0
// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::SetInterGainSFXValue
//
// Description:
//      Set gain value EQ SFX
//
// Return values:
//      S_OK if successful
// ----------------------------------------------------------------------------
HRESULT CInterPropPage::SetInterGainSFXValue(DWORD dwPosValue)
{
    BOOL fCurrentState = 0;
    DWORD dwCurrentValue = 0;
    HRESULT hr = RetrieveInterSFXState(&fCurrentState);

    IF_FAILED_JUMP(hr, Exit);

    if (m_fEnableInterSFX)
    {
        PROPVARIANT var;
        var.vt = VT_UI4;
        var.ulVal = m_dwInterGainSFX;

        hr = RetrieveInterGainSFXValue(&dwCurrentValue);
        IF_FAILED_JUMP(hr, Exit);

        if (dwCurrentValue != dwPosValue)
        {
            // Set value
            hr = m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_Gain_Level_SFX, var);
        }
    }

Exit:
    return(hr);
}

HRESULT CInterPropPage::RetrieveInterGainMFXValue(DWORD* pdwValue)
{
    UNREFERENCED_PARAMETER(pdwValue);
    return(S_OK);
}

HRESULT CInterPropPage::SetInterGainMFXValue(DWORD dwPosValue)
{
    UNREFERENCED_PARAMETER(dwPosValue);
    return(S_OK);
}
#endif

HRESULT CInterPropPage::RetrieveInterBassSFXValue(DWORD* pdwValue)
{
    HRESULT hr = E_POINTER;

    if ((m_pAudioFXExtParams != NULL) && (m_pAudioFXExtParams->pFxProperties != NULL) && (pdwValue != NULL))
    {
        PROPVARIANT var;
        PropVariantInit(&var);

        // Get the state of whether inter SFX is enabled or not
        hr = m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_Endpoint_Inter_EQ_Low_SFX, &var);
        if (SUCCEEDED(hr) && (var.vt == VT_UI4))
        {
            if (ValidateEqLevel(var.ulVal))
            {
                var.ulVal = CorrectGainValue(var.ulVal);
                hr = m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_EQ_Low_SFX, var);
            }
            *pdwValue = var.ulVal;
        }

        PropVariantClear(&var);
    }

    return(hr);
}

#if 0
HRESULT CInterPropPage::SetInterBassSFXValue(DWORD dwPosValue)
{
    UNREFERENCED_PARAMETER(dwPosValue);
    return(S_OK);
}

HRESULT CInterPropPage::RetrieveInterBassMFXValue(DWORD* pdwValue)
{
    UNREFERENCED_PARAMETER(pdwValue);
    return(S_OK);
}

HRESULT CInterPropPage::SetInterBassMFXValue(DWORD dwPosValue)
{
    UNREFERENCED_PARAMETER(dwPosValue);
    return(S_OK);
}
#endif

HRESULT CInterPropPage::RetrieveInterMidrangeSFXValue(DWORD* pdwValue)
{
    HRESULT hr = E_POINTER;

    if ((m_pAudioFXExtParams != NULL) && (m_pAudioFXExtParams->pFxProperties != NULL) && (pdwValue != NULL))
    {
        PROPVARIANT var;
        PropVariantInit(&var);

        // Get the state of whether inter SFX is enabled or not
        hr = m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_Endpoint_Inter_EQ_Mid_SFX, &var);
        if (SUCCEEDED(hr) && (var.vt == VT_UI4))
        {
            if (ValidateEqLevel(var.ulVal))
            {
                var.ulVal = CorrectGainValue(var.ulVal);
                hr = m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_EQ_Mid_SFX, var);
            }
            *pdwValue = var.ulVal;
        }

        PropVariantClear(&var);
    }

    return(hr);
}

#if 0
HRESULT CInterPropPage::SetInterMidrangeSFXValue(DWORD dwPosValue)
{
    UNREFERENCED_PARAMETER(dwPosValue);
    return(S_OK);
}

HRESULT CInterPropPage::RetrieveInterMidrangeMFXValue(DWORD* pdwValue)
{
    UNREFERENCED_PARAMETER(pdwValue);
    return(S_OK);
}

HRESULT CInterPropPage::SetInterMidrangeMFXValue(DWORD dwPosValue)
{
    UNREFERENCED_PARAMETER(dwPosValue);
    return(S_OK);
}
#endif

HRESULT CInterPropPage::RetrieveInterTrebleSFXValue(DWORD* pdwValue)
{
    HRESULT hr = E_POINTER;

    if ((m_pAudioFXExtParams != NULL) && (m_pAudioFXExtParams->pFxProperties != NULL) && (pdwValue != NULL))
    {
        PROPVARIANT var;
        PropVariantInit(&var);

        // Get the state of whether inter SFX is enabled or not
        hr = m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_Endpoint_Inter_EQ_High_SFX, &var);
        if (SUCCEEDED(hr) && (var.vt == VT_UI4))
        {
            if (ValidateEqLevel(var.ulVal))
            {
                var.ulVal = CorrectGainValue(var.ulVal);
                hr = m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_EQ_High_SFX, var);
            }
            *pdwValue = var.ulVal;
        }

        PropVariantClear(&var);
    }

    return(hr);
}

#if 0
HRESULT CInterPropPage::SetInterTrebleSFXValue(DWORD dwPosValue)
{
    UNREFERENCED_PARAMETER(dwPosValue);
    return(S_OK);
}

HRESULT CInterPropPage::RetrieveInterTrebleMFXValue(DWORD* pdwValue)
{
    UNREFERENCED_PARAMETER(pdwValue);
    return(S_OK);
}

HRESULT CInterPropPage::SetInterTrebleMFXValue(DWORD dwPosValue)
{
    UNREFERENCED_PARAMETER(dwPosValue);
    return(S_OK);
}
#endif

// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::OnInitDialog
//
// Description:
//      Dialog initialization routine
//
// Parameters:
//      hwndDlg - [in] Handle to dialog box
//      wParam - [in] Handle to control to receive the default keyboard focus
//      lParam - [in] Specifies additional message-specific information
//
// Return values:
//      TRUE to direct the system to set the keyboard focus to the control
//      specified by wParam. Otherwise, it should return FALSE to prevent the
//      system from setting the default keyboard focus.
// ----------------------------------------------------------------------------
BOOL CInterPropPage::OnInitDialog
(
    HWND hwndDlg,
    WPARAM wParam,
    LPARAM lParam
)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    HRESULT hr = S_OK;
    LPWSTR pwstrEndpointName = NULL;
    DWORD dwRegValue = 0;

    // Retrieve the endpoint's friendly name, system effects, and inter SFX and MFX states
    hr = GetDeviceFriendlyName(&pwstrEndpointName);
    IF_FAILED_JUMP(hr, Exit);

    hr = RetrieveSysFXState(&m_fDisableSysFX);
    IF_FAILED_JUMP(hr, Exit);

    hr = RetrieveInterSFXState(&m_fEnableInterSFX);
    IF_FAILED_JUMP(hr, Exit);

    hr = RetrieveInterGainSFXValue(&dwRegValue);
    IF_FAILED_JUMP(hr, Exit);
    m_dwInterGainSFX = ConvertGainFromRegToValue(dwRegValue);

    hr = RetrieveInterBassSFXValue(&dwRegValue);
    IF_FAILED_JUMP(hr, Exit);
    m_dwInterBassSFX = ConvertEqLevelFromRegToValue(dwRegValue);

    hr = RetrieveInterMidrangeSFXValue(&dwRegValue);
    IF_FAILED_JUMP(hr, Exit);
    m_dwInterMidrangeSFX = ConvertEqLevelFromRegToValue(dwRegValue);

    hr = RetrieveInterTrebleSFXValue(&dwRegValue);
    IF_FAILED_JUMP(hr, Exit);
    m_dwInterTrebleSFX = ConvertEqLevelFromRegToValue(dwRegValue);

    // Update the property page with retrieved information
    SetWindowText(GetDlgItem(hwndDlg, IDC_SPP_ENDPOINT_NAME), pwstrEndpointName);

    // Based on the retrieved states, toggle the checkboxes to reflect them
    if (m_fDisableSysFX)
    {
        CheckDlgButton(hwndDlg, IDC_DISABLE_SYSFX, BST_CHECKED);

        // Disable APO toggling controls on the page
        EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_INTER_SFX), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_INTER_MFX), FALSE);
    }
    else
    {
        CheckDlgButton(hwndDlg, IDC_DISABLE_SYSFX, BST_UNCHECKED);

        // Enable APO toggling controls on the page
        EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_INTER_SFX), TRUE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_INTER_MFX), TRUE);
    }

    if (m_fEnableInterSFX)
    {
        CheckDlgButton(hwndDlg, IDC_ENABLE_INTER_SFX, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwndDlg, IDC_ENABLE_INTER_SFX, BST_UNCHECKED);
    }

    if (m_fEnableInterMFX)
    {
        CheckDlgButton(hwndDlg, IDC_ENABLE_INTER_MFX, BST_CHECKED);
    }
    else
    {
        CheckDlgButton(hwndDlg, IDC_ENABLE_INTER_MFX, BST_UNCHECKED);
    }

Exit:
    SAFE_COTASKMEMFREE(pwstrEndpointName);
    return(FALSE);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::OnApply
//
// Description:
//      Handle the pressing of the apply button
//
// Parameters:
//      hwndDlg - [in] Handle to the dialog box
//
// Return values:
//      TRUE to set keyboard focus on control
// ----------------------------------------------------------------------------
BOOL CInterPropPage::OnApply
(
    HWND hwndDlg
)
{
    HRESULT hr = S_OK;

    // Commit the settings
    hr = SetSysFXState();
    IF_FAILED_JUMP(hr, Exit);

    hr = SetInterSFXState();
    IF_FAILED_JUMP(hr, Exit);

    hr = SetInterMFXState();
    IF_FAILED_JUMP(hr, Exit);
    
    if (NULL != m_pAudioFXExtParams && NULL != m_pAudioFXExtParams->pFxProperties)
    {
        hr = m_pAudioFXExtParams->pFxProperties->Commit();
        IF_FAILED_JUMP(hr, Exit);
    }
    
Exit:
    if (SUCCEEDED(hr))
    {
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_NOERROR);
    }
    else
    {
        SetWindowLongPtr(hwndDlg, DWLP_MSGRESULT, PSNRET_INVALID);
    }

    return(TRUE);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::OnCheckBoxClickedDisableSysFX
//
// Description:
//      Handle the clicking of the Disable System Effects check box
//
// Parameters:
//      hwndDlg - [in] Handle to the dialog box
//
// Return values:
//      FALSE to not set default keyboard focus
// ----------------------------------------------------------------------------
BOOL CInterPropPage::OnCheckBoxClickedDisableSysFX
(
    HWND hwndDlg
)
{
    // Check the state of the check box and update associated data member
    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_DISABLE_SYSFX))
    {
        m_fDisableSysFX = TRUE;

        // Disable APO toggling controls on the page
        EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_INTER_SFX), FALSE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_INTER_MFX), FALSE);
    }
    else
    {
        m_fDisableSysFX = FALSE;

        // Enable APO toggling controls on the page
        EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_INTER_SFX), TRUE);
        EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_INTER_MFX), TRUE);
    }

    // If the user changes the check box, enable the Apply button
    SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);

    return(FALSE);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::OnCheckBoxClickedEnableInterSFX
//
// Description:
//      Handle the clicking of the Enable Channel Inter SFX check box
//
// Parameters:
//      hwndDlg - [in] Handle to the dialog box
//
// Return values:
//      FALSE to not set default keyboard focus
// ----------------------------------------------------------------------------
BOOL CInterPropPage::OnCheckBoxClickedEnableInterSFX
(
    HWND hwndDlg
)
{
    // Check the state of the check box and update associated data member
    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_ENABLE_INTER_SFX))
    {
        m_fEnableInterSFX = TRUE;
    }
    else
    {
        m_fEnableInterSFX = FALSE;
    }

    // If the user changes the check box, enable the Apply button
    SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);

    return(FALSE);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::OnCheckBoxClickedEnableInterMFX
//
// Description:
//      Handle the clicking of the Enable Channel Inter MFX check box
//
// Parameters:
//      hwndDlg - [in] Handle to the dialog box
//
// Return values:
//      FALSE to not set default keyboard focus
// ----------------------------------------------------------------------------
BOOL CInterPropPage::OnCheckBoxClickedEnableInterMFX
(
    HWND hwndDlg
)
{
    // Check the state of the check box and update associated data member
    if (BST_CHECKED == IsDlgButtonChecked(hwndDlg, IDC_ENABLE_INTER_MFX))
    {
        m_fEnableInterMFX = TRUE;
    }
    else
    {
        m_fEnableInterMFX = FALSE;
    }

    // If the user changes the check box, enable the Apply button
    SendMessage(GetParent(hwndDlg), PSM_CHANGED, (WPARAM)hwndDlg, 0);

    return(FALSE);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::DialogProcPage1
//
// Description:
//      Callback for property page
//
// Parameters:
//      hwndDlg - [in] Handle to the dialog box
//      uMsg - [in] Specifies the message
//      wParam - [in] Specifies additional message-specific information
//      lParam - [in] Specifies additional message-specific information
//
// Return values:
//      TRUE if it processed the message, FALSE if not
// ----------------------------------------------------------------------------
INT_PTR CALLBACK CInterPropPage::DialogProcPage1
(
    HWND    hwndDlg,
    UINT    uMsg,
    WPARAM  wParam,
    LPARAM  lParam
)
{
    CInterPropPage* pthis = (CInterPropPage*)(LONG_PTR)GetWindowLongPtr(
                                hwndDlg, GWLP_USERDATA);
    BOOL fRet = FALSE;
    BOOL fChanged = FALSE;
    HWND hSldGain = GetDlgItem(hwndDlg, IDC_SLD_EQ_GAIN);
    HWND hSldBass = GetDlgItem(hwndDlg, IDC_SLD_EQ_BASS);
    HWND hSldMidrange = GetDlgItem(hwndDlg, IDC_SLD_EQ_MIDRANGE);
    HWND hSldTreble = GetDlgItem(hwndDlg, IDC_SLD_EQ_TREBLE);
    DWORD CurrentReg = 0;
    DWORD AssumeReg = 0;
    DWORD CurrentBar = 0;
    LONG  CurrentGain = 0;
    LONG  CurrentEqLevel = 0;

    HRESULT hr = S_OK;

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // Extract the context data from PROPSHEETPAGE::lParam
            PROPSHEETPAGE*  pSheetDesc = (PROPSHEETPAGE*)lParam;

            // Create the property page factory class
#pragma warning(push)
#pragma warning(disable: 28197)
            pthis = new CComObject<CInterPropPage>();
#pragma warning(pop)
            if (pthis == NULL)
            {
                return(FALSE);
            }

            // Save this object in lParam
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, (LONG_PTR)pthis);

            // Keep audio FX extension parameters passed by the control panel
            pthis->m_pAudioFXExtParams = (AudioFXExtensionParams*)pSheetDesc->lParam;

            fRet = pthis->OnInitDialog(hwndDlg, wParam, lParam);

            CurrentBar = pthis->ConvertGainFromValueToBar(pthis->m_dwInterGainSFX);
            TrackBar_SetPos(hSldGain, CurrentBar);

            CurrentBar = pthis->ConvertEqLevelFromValueToBar(pthis->m_dwInterBassSFX);
            TrackBar_SetPos(hSldBass, CurrentBar);

            CurrentBar = pthis->ConvertEqLevelFromValueToBar(pthis->m_dwInterMidrangeSFX);
            TrackBar_SetPos(hSldMidrange, CurrentBar);

            CurrentBar = pthis->ConvertEqLevelFromValueToBar(pthis->m_dwInterTrebleSFX);
            TrackBar_SetPos(hSldTreble, CurrentBar);

            break;
        }

        case WM_NOTIFY:
        {
            switch (((NMHDR FAR*)lParam)->code)
            {
                case PSN_APPLY:
                    if (pthis)
                    {
                        // Apply button pressed
                        fRet = pthis->OnApply(hwndDlg);
                    }
                    break;
            }
            break;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                // Handle the clicking of the check boxes
                case IDC_DISABLE_SYSFX:
                    if (pthis)
                    {
                        fRet = pthis->OnCheckBoxClickedDisableSysFX(hwndDlg);
                    }
                    break;

                case IDC_ENABLE_INTER_SFX:
                    if (pthis)
                    {
                        fRet = pthis->OnCheckBoxClickedEnableInterSFX(hwndDlg);
                    }
                    break;

                case IDC_ENABLE_INTER_MFX:
                    if (pthis)
                    {
                        fRet = pthis->OnCheckBoxClickedEnableInterMFX(hwndDlg);
                    }
                    break;
            }
            break;
        }

        case WM_HSCROLL:
        {
            PROPVARIANT var;
            PropVariantInit(&var);

            if ((HWND)(lParam) == hSldGain)
            {
                // Handle Gain slider movement
                CurrentBar = TrackBar_GetPos(hSldGain);
                CurrentGain = pthis->CnvertGainFromBarToValue(CurrentBar);
                AssumeReg = pthis->ConvertGainFromValueToReg(CurrentGain);

                // Get the value of inter SFX Gain level from the registry
                hr = pthis->m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_Endpoint_Inter_Gain_Level_SFX, &var);
                if (SUCCEEDED(hr) && (var.vt == VT_UI4))
                {
                    if (!pthis->ValidateGainValue(var.ulVal))
                    {
                        // correct and set the value
                        var.ulVal = pthis->CorrectGainValue(var.ulVal);
                        hr = pthis->m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_Gain_Level_SFX, var);
                    }
                    if (SUCCEEDED(hr))
                    {
                        CurrentReg = var.ulVal;
                    }
                }

                IF_FAILED_JUMP(hr, Exit);
                // check if changed
                if (CurrentReg == AssumeReg)
                {
                    // nothing to do
                    break;
                }
                else if (pthis->m_fEnableInterSFX)
                {
                    pthis->m_dwInterGainSFX = CurrentGain;
                    var.ulVal = AssumeReg;
                    hr = pthis->m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_Gain_Level_SFX, var);
                }

                IF_FAILED_JUMP(hr, Exit);
                fChanged = TRUE;
            }
            else if ((HWND)(lParam) == hSldBass)
            {
                // Handle Bass slider movement
                CurrentBar = TrackBar_GetPos(hSldBass);
                CurrentEqLevel = pthis->CnvertEqLevelFromBarToValue(CurrentBar);
                AssumeReg = pthis->ConvertGainFromValueToReg(CurrentEqLevel);

                // Get the value of inter SFX EQ Low level from the registry
                hr = pthis->m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_Endpoint_Inter_EQ_Low_SFX, &var);
                if (SUCCEEDED(hr) && (var.vt == VT_UI4))
                {
                    if (!pthis->ValidateEqLevel(var.ulVal))
                    {
                        // correct and set the value
                        var.ulVal = pthis->CorrectEqLevel(var.ulVal);
                        hr = pthis->m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_EQ_Low_SFX, var);
                    }
                    if (SUCCEEDED(hr))
                    {
                        CurrentReg = var.ulVal;
                    }
                }

                IF_FAILED_JUMP(hr, Exit);
                // check if changed
                if (CurrentReg == AssumeReg)
                {
                    // nothing to do
                    break;
                }
                else if (pthis->m_fEnableInterSFX)
                {
                    pthis->m_dwInterBassSFX = CurrentEqLevel;
                    var.ulVal = AssumeReg;
                    hr = pthis->m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_EQ_Low_SFX, var);
                }

                IF_FAILED_JUMP(hr, Exit);
                fChanged = TRUE;
            }
            else if ((HWND)(lParam) == hSldMidrange)
            {
                // Handle Midrange slider movement
                CurrentBar = TrackBar_GetPos(hSldMidrange);
                CurrentEqLevel = pthis->CnvertEqLevelFromBarToValue(CurrentBar);
                AssumeReg = pthis->ConvertGainFromValueToReg(CurrentEqLevel);

                // Get the value of inter SFX EQ Mid level from the registry
                hr = pthis->m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_Endpoint_Inter_EQ_Mid_SFX, &var);
                if (SUCCEEDED(hr) && (var.vt == VT_UI4))
                {
                    if (!pthis->ValidateEqLevel(var.ulVal))
                    {
                        // correct and set the value
                        var.ulVal = pthis->CorrectEqLevel(var.ulVal);
                        hr = pthis->m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_EQ_Mid_SFX, var);
                    }
                    if (SUCCEEDED(hr))
                    {
                        CurrentReg = var.ulVal;
                    }
                }

                IF_FAILED_JUMP(hr, Exit);
                // check if changed
                if (CurrentReg == AssumeReg)
                {
                    // nothing to do
                    break;
                }
                else if (pthis->m_fEnableInterSFX)
                {
                    pthis->m_dwInterMidrangeSFX = CurrentEqLevel;
                    var.ulVal = AssumeReg;
                    hr = pthis->m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_EQ_Mid_SFX, var);
                }

                IF_FAILED_JUMP(hr, Exit);
                fChanged = TRUE;
            }
            else if ((HWND)(lParam) == hSldTreble)
            {
                // Handle Treble slider movement
                CurrentBar = TrackBar_GetPos(hSldTreble);
                CurrentEqLevel = pthis->CnvertEqLevelFromBarToValue(CurrentBar);
                AssumeReg = pthis->ConvertGainFromValueToReg(CurrentEqLevel);

                // Get the value of inter SFX EQ High level from the registry
                hr = pthis->m_pAudioFXExtParams->pFxProperties->GetValue(PKEY_Endpoint_Inter_EQ_High_SFX, &var);
                if (SUCCEEDED(hr) && (var.vt == VT_UI4))
                {
                    if (!pthis->ValidateEqLevel(var.ulVal))
                    {
                        // correct and set the value
                        var.ulVal = pthis->CorrectEqLevel(var.ulVal);
                        hr = pthis->m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_EQ_High_SFX, var);
                    }
                    if (SUCCEEDED(hr))
                    {
                        CurrentReg = var.ulVal;
                    }
                }

                IF_FAILED_JUMP(hr, Exit);
                // check if changed
                if (CurrentReg == AssumeReg)
                {
                    // nothing to do
                    break;
                }
                else if (pthis->m_fEnableInterSFX)
                {
                    pthis->m_dwInterTrebleSFX = CurrentEqLevel;
                    var.ulVal = AssumeReg;
                    hr = pthis->m_pAudioFXExtParams->pFxProperties->SetValue(PKEY_Endpoint_Inter_EQ_High_SFX, var);
                }

                IF_FAILED_JUMP(hr, Exit);
                fChanged = TRUE;
            }
            break;
        }
        case WM_DESTROY:
        {
            SAFE_DELETE(pthis);
            SetWindowLongPtr(hwndDlg, GWLP_USERDATA, NULL);
            fRet = TRUE;
            break;
        }
    }

    if (fChanged)
    {
        if (NULL != pthis->m_pAudioFXExtParams && NULL != pthis->m_pAudioFXExtParams->pFxProperties)
        {
            hr = pthis->m_pAudioFXExtParams->pFxProperties->Commit();
            IF_FAILED_JUMP(hr, Exit);
        }
    }

Exit:
    return(fRet);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::PropSheetPageProc
//
// Description:
//      Callback that gets invoked right after page creation or right before
//      before page destruction
//
// Parameters:
//      hwnd - Reserved; must be NULL
//      uMsg - [in] Action flag. PSPCB_ADDREF, PSPCB_CREATE, or PSPCB_RELEASE
//      ppsp - [in, out] Pointer to a PROPSHEETPAGE structure that defines
//             the page being created or destroyed.
//
// Return values:
//      Depends on the value of the uMsg parameter
// ----------------------------------------------------------------------------
UINT CALLBACK CInterPropPage::PropSheetPageProc
(
    HWND            hwnd,
    UINT            uMsg,
    LPPROPSHEETPAGE ppsp
)
{
    UNREFERENCED_PARAMETER(hwnd);
    UNREFERENCED_PARAMETER(uMsg);
    UNREFERENCED_PARAMETER(ppsp);

    // if (uMsg == PSPCB_CREATE) ...
    return(1);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::Initialize
//
// Description:
//      Implementation of IShellExtInit::Initialize. Initializes a property
//      sheet extension, shortcut menu extension, or drag-and-drop handler.
//
// Parameters:
//      pidlFolder - [in] Address of an ITEMIDLIST structure that uniquely
//                   identifies a folder. For property sheet extensions,
//                   this parameter is NULL.
//      pdtobj - [out] Address of an IDataObject interface object that can be
//               used to retrieve the objects being acted upon. 
//      hkeyProgID - [in] Registry key for the file object or folder type.
//
// Return values:
//      Returns NOERROR if successful, or an OLE-defined error value otherwise
// ----------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT CInterPropPage::Initialize
(
    LPCITEMIDLIST   pidlFolder,
    IDataObject*    pdtobj,
    HKEY            hkeyProgID
)
{
    UNREFERENCED_PARAMETER(pidlFolder);
    UNREFERENCED_PARAMETER(pdtobj);
    UNREFERENCED_PARAMETER(hkeyProgID);

    return(S_OK);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::AddPages
//
// Description:
//      Implementation of IShellPropSheetExt::AddPages. Adds one or more pages
//      to a property sheet that the Shell displays for a file object.
//
// Parameters:
//      lpfnAddPage - [in] Address of a function that the property sheet
//                    handler calls to add a page to the property sheet. The
//                    function takes a property sheet handle returned by the
//                    CreatePropertySheetPage function and the lParam parameter
//                    passed to the AddPages method. 
//      lParam - [in] Parameter to pass to the function specified by the
//               lpfnAddPage method.
//
// Return values:
//      Returns S_OK if successful. If the method fails, an OLE-defined error
//      code is returned
// ----------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT STDMETHODCALLTYPE CInterPropPage::AddPages
(
    LPFNADDPROPSHEETPAGE    lpfnAddPage,    // See PrSht.h
    LPARAM                  lParam          // Used by caller, don't modify
)
{
    HRESULT                 hr = S_OK;
    PROPSHEETPAGE           psp;
    HPROPSHEETPAGE          hPage1 = NULL;
    AudioFXExtensionParams* pAudioFXParams = (AudioFXExtensionParams*)lParam;
#pragma warning(push)
#pragma warning(disable: 28197)
    AudioFXExtensionParams* pAudioFXParamsCopy = new AudioFXExtensionParams;
#pragma warning(pop)

    if (pAudioFXParamsCopy == NULL)
    {
        return E_OUTOFMEMORY;
    }

    // Make a copy of the params
    CopyMemory(pAudioFXParamsCopy, pAudioFXParams, sizeof(AudioFXExtensionParams));
    SAFE_ADDREF(pAudioFXParamsCopy->pFxProperties);

    // Initialize property page params and create page
    psp.dwSize        = sizeof(psp);
    psp.dwFlags       = PSP_USEREFPARENT | PSP_USECALLBACK;
    psp.hInstance     = _AtlBaseModule.GetModuleInstance();
    psp.hIcon         = 0;
    psp.pcRefParent   = (UINT*)&m_dwRef;
    psp.lParam        = (LPARAM)pAudioFXParamsCopy;
    psp.pszTemplate   = MAKEINTRESOURCE(IDD_INTER_PROP_PAGE);
    psp.pfnDlgProc    = (DLGPROC)DialogProcPage1;
    psp.pfnCallback   = PropSheetPageProc;

    // Create the property sheet page and add the page
    hPage1 = CreatePropertySheetPage(&psp);
    if (hPage1)
    {
        if (!lpfnAddPage(hPage1, pAudioFXParams->AddPageParam))
        {
            hr = E_FAIL;
            delete pAudioFXParamsCopy;
            DestroyPropertySheetPage(hPage1);
        }
        else
        {
            // Add ref for page
            this->AddRef();
        }
    }
    else
    {
        delete pAudioFXParamsCopy;
        hr = E_OUTOFMEMORY;
    }

    return(hr);
}


// ----------------------------------------------------------------------------
// Function:
//      CInterPropPage::ReplacePage
//
// Description:
//      Implementation of IShellPropSheetExt::ReplacePage. Replaces a page in
//      a property sheet for a Control Panel object.
//
// Parameters:
//      uPageID - [in] Identifier of the page to replace 
//      lpfnReplacePage - [in] Address of a function that the property sheet
//                        handler calls to replace a page to the property
//                        sheet. The function takes a property sheet handle
//                        returned by the CreatePropertySheetPage function and
//                        the lParam parameter passed to the ReplacePage
//                        method.
//      lParam - [in] Parameter to pass to the function specified by the
//               lpfnReplacePage parameter. 
//
// Return values:
//      Returns NOERROR if successful, or an OLE-defined error value otherwise
// ----------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT STDMETHODCALLTYPE CInterPropPage::ReplacePage
(
    UINT                    uPageID,
    LPFNSVADDPROPSHEETPAGE  lpfnReplaceWith,
    LPARAM                  lParam
)
{
    UNREFERENCED_PARAMETER(uPageID);
    UNREFERENCED_PARAMETER(lpfnReplaceWith);
    UNREFERENCED_PARAMETER(lParam);

    return(S_FALSE);
}
