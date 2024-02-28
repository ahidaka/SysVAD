

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* Compiler settings for InterAPODll.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.01.0628 
    protocol : all , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */



/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 500
#endif

/* verify that the <rpcsal.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCSAL_H_VERSION__
#define __REQUIRED_RPCSAL_H_VERSION__ 100
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif /* __RPCNDR_H_VERSION__ */


#ifndef __InterAPODll_h__
#define __InterAPODll_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if defined(_CONTROL_FLOW_GUARD_XFG)
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __InterAPOMFX_FWD_DEFINED__
#define __InterAPOMFX_FWD_DEFINED__

#ifdef __cplusplus
typedef class InterAPOMFX InterAPOMFX;
#else
typedef struct InterAPOMFX InterAPOMFX;
#endif /* __cplusplus */

#endif 	/* __InterAPOMFX_FWD_DEFINED__ */


#ifndef __InterAPOSFX_FWD_DEFINED__
#define __InterAPOSFX_FWD_DEFINED__

#ifdef __cplusplus
typedef class InterAPOSFX InterAPOSFX;
#else
typedef struct InterAPOSFX InterAPOSFX;
#endif /* __cplusplus */

#endif 	/* __InterAPOSFX_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "InterAPOInterface.h"

#ifdef __cplusplus
extern "C"{
#endif 



#ifndef __InterAPODlllib_LIBRARY_DEFINED__
#define __InterAPODlllib_LIBRARY_DEFINED__

/* library InterAPODlllib */
/* [version][uuid] */ 


EXTERN_C const IID LIBID_InterAPODlllib;

EXTERN_C const CLSID CLSID_InterAPOMFX;

#ifdef __cplusplus

class DECLSPEC_UUID("2CB5CD90-6D7B-4ABC-AB85-8CB066C849A2")
InterAPOMFX;
#endif

EXTERN_C const CLSID CLSID_InterAPOSFX;

#ifdef __cplusplus

class DECLSPEC_UUID("01A74FEF-9251-498B-9668-4FDF50261465")
InterAPOSFX;
#endif
#endif /* __InterAPODlllib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


