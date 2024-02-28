// Microsoft Windows
// Copyright (C) Microsoft Corporation. All rights reserved.
//
#pragma once

// header files for imported files
#include "propidl.h"

#ifdef DEFINE_PROPERTYKEY
#undef DEFINE_PROPERTYKEY
#endif

#ifdef INITGUID
#define DEFINE_PROPERTYKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const PROPERTYKEY name = { { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }, pid }
#else
#define DEFINE_PROPERTYKEY(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8, pid) EXTERN_C const PROPERTYKEY name
#endif // INITGUID

// ----------------------------------------------------------------------
//
// PKEY_Endpoint_Enable_Channel_Swap_SFX: When value is 0x00000001, Channel Swap local effect is enabled
// {A44531EF-5377-4944-AE15-53789A9629C7},2
// vartype = VT_UI4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Enable_Channel_Swap_SFX, 0xa44531ef, 0x5377, 0x4944, 0xae, 0x15, 0x53, 0x78, 0x9a, 0x96, 0x29, 0xc7, 2);

// PKEY_Endpoint_Enable_Channel_Swap_MFX: When value is 0x00000001, Channel Swap global effect is enabled
// {A44531EF-5377-4944-AE15-53789A9629C7},3
// vartype = VT_UI4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Enable_Channel_Swap_MFX, 0xa44531ef, 0x5377, 0x4944, 0xae, 0x15, 0x53, 0x78, 0x9a, 0x96, 0x29, 0xc7, 3);

// PKEY_Endpoint_Enable_Delay_SFX: When value is 0x00000001, Delay local effect is enabled
// {A44531EF-5377-4944-AE15-53789A9629C7},4
// vartype = VT_UI4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Enable_Delay_SFX, 0xa44531ef, 0x5377, 0x4944, 0xae, 0x15, 0x53, 0x78, 0x9a, 0x96, 0x29, 0xc7, 4);

// PKEY_Endpoint_Enable_Delay_MFX: When value is 0x00000001, Delay global effect is enabled
// {A44531EF-5377-4944-AE15-53789A9629C7},5
// vartype = VT_UI4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Enable_Delay_MFX, 0xa44531ef, 0x5377, 0x4944, 0xae, 0x15, 0x53, 0x78, 0x9a, 0x96, 0x29, 0xc7, 5);

// PKEY_Endpoint_Enable_Inter_SFX: When value is 0x00000001, Inter local effect is enabled
// {A44531EF-5377-4944-AE15-53789A9629C7},6
// vartype = VT_UI4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Enable_Interface_SFX, 0xa44531ef, 0x5377, 0x4944, 0xae, 0x15, 0x53, 0x78, 0x9a, 0x96, 0x29, 0xc7, 6);

// PKEY_Endpoint_Enable_Inter_MFX: When value is 0x00000001, Inter global effect is enabled
// {A44531EF-5377-4944-AE15-53789A9629C7},7
// vartype = VT_UI4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Enable_Interface_MFX, 0xa44531ef, 0x5377, 0x4944, 0xae, 0x15, 0x53, 0x78, 0x9a, 0x96, 0x29, 0xc7, 7);


// PKEY_Endpoint_Inter_Gain_Level_SFX: Inter APO gain level times by 100, typically -3000 .. 3000  
// {0F2212E5-3612-459C-BE43-1FF0E576786A},0
// vartype = VT_I4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Inter_Gain_Level_SFX, 0x0f2212e5, 0x3612, 0x459c, 0xbe, 0x43, 0x1f, 0xf0, 0xe5, 0x76, 0x78, 0x6a, 0);

// PKEY_Endpoint_Inter_Gain_Level_MFX: Inter APO gain level times by 100, typically -3000 .. 3000  
// {0F2212E5-3612-459C-BE43-1FF0E576786A},1
// vartype = VT_I4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Inter_Gain_Level_MFX, 0x0f2212e5, 0x3612, 0x459c, 0xbe, 0x43, 0x1f, 0xf0, 0xe5, 0x76, 0x78, 0x6a, 1);


// PKEY_Endpoint_Inter_EQ_Low_SFX: Inter EQ level -100 .. 100  
// {0F2212E5-3612-459C-BE43-1FF0E576786A},2
// vartype = VT_I4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Inter_EQ_Low_SFX, 0x0f2212e5, 0x3612, 0x459c, 0xbe, 0x43, 0x1f, 0xf0, 0xe5, 0x76, 0x78, 0x6a, 2);

// PKEY_Endpoint_Inter_EQ_Low_MFX: Inter EQ level -100 .. 100  
// {0F2212E5-3612-459C-BE43-1FF0E576786A},3
// vartype = VT_I4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Inter_EQ_Low_MFX, 0x0f2212e5, 0x3612, 0x459c, 0xbe, 0x43, 0x1f, 0xf0, 0xe5, 0x76, 0x78, 0x6a, 3);

// PKEY_Endpoint_Inter_EQ_Mid_SFX: Inter EQ level -100 .. 100  
// {0F2212E5-3612-459C-BE43-1FF0E576786A},4
// vartype = VT_I4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Inter_EQ_Mid_SFX, 0x0f2212e5, 0x3612, 0x459c, 0xbe, 0x43, 0x1f, 0xf0, 0xe5, 0x76, 0x78, 0x6a, 4);

// PKEY_Endpoint_Inter_EQ_Mid_MFX: Inter EQ level -100 .. 100  
// {0F2212E5-3612-459C-BE43-1FF0E576786A},5
// vartype = VT_I4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Inter_EQ_Mid_MFX, 0x0f2212e5, 0x3612, 0x459c, 0xbe, 0x43, 0x1f, 0xf0, 0xe5, 0x76, 0x78, 0x6a, 5);
// PKEY_Endpoint_Inter_EQ_High_SFX: Inter EQ level -100 .. 100  
// {0F2212E5-3612-459C-BE43-1FF0E576786A},6
// vartype = VT_I4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Inter_EQ_High_SFX, 0x0f2212e5, 0x3612, 0x459c, 0xbe, 0x43, 0x1f, 0xf0, 0xe5, 0x76, 0x78, 0x6a, 6);

// PKEY_Endpoint_Inter_EQ_High_MFX: Inter EQ level -100 .. 100  
// {0F2212E5-3612-459C-BE43-1FF0E576786A},7
// vartype = VT_I4
DEFINE_PROPERTYKEY(PKEY_Endpoint_Inter_EQ_High_MFX, 0x0f2212e5, 0x3612, 0x459c, 0xbe, 0x43, 0x1f, 0xf0, 0xe5, 0x76, 0x78, 0x6a, 7);
