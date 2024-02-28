

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.01.0628 */
/* Compiler settings for InterAPOInterface.idl:
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

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __InterAPOInterface_h__
#define __InterAPOInterface_h__

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

#ifndef __IInterAPOMFX_FWD_DEFINED__
#define __IInterAPOMFX_FWD_DEFINED__
typedef interface IInterAPOMFX IInterAPOMFX;

#endif 	/* __IInterAPOMFX_FWD_DEFINED__ */


#ifndef __IInterAPOSFX_FWD_DEFINED__
#define __IInterAPOSFX_FWD_DEFINED__
typedef interface IInterAPOSFX IInterAPOSFX;

#endif 	/* __IInterAPOSFX_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"
#include "audioenginebaseapo.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IInterAPOMFX_INTERFACE_DEFINED__
#define __IInterAPOMFX_INTERFACE_DEFINED__

/* interface IInterAPOMFX */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IInterAPOMFX;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("700118CA-F9FC-4C04-BF7A-49B549F7F0E7")
    IInterAPOMFX : public IUnknown
    {
    public:
    };
    
    
#else 	/* C style interface */

    typedef struct IInterAPOMFXVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in IInterAPOMFX * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in IInterAPOMFX * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in IInterAPOMFX * This);
        
        END_INTERFACE
    } IInterAPOMFXVtbl;

    interface IInterAPOMFX
    {
        CONST_VTBL struct IInterAPOMFXVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInterAPOMFX_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IInterAPOMFX_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IInterAPOMFX_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IInterAPOMFX_INTERFACE_DEFINED__ */


#ifndef __IInterAPOSFX_INTERFACE_DEFINED__
#define __IInterAPOSFX_INTERFACE_DEFINED__

/* interface IInterAPOSFX */
/* [unique][uuid][object] */ 


EXTERN_C const IID IID_IInterAPOSFX;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("80CD02D2-8A3F-4250-9E47-A6795020AC79")
    IInterAPOSFX : public IUnknown
    {
    public:
    };
    
    
#else 	/* C style interface */

    typedef struct IInterAPOSFXVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            __RPC__in IInterAPOSFX * This,
            /* [in] */ __RPC__in REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            __RPC__in IInterAPOSFX * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            __RPC__in IInterAPOSFX * This);
        
        END_INTERFACE
    } IInterAPOSFXVtbl;

    interface IInterAPOSFX
    {
        CONST_VTBL struct IInterAPOSFXVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IInterAPOSFX_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IInterAPOSFX_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IInterAPOSFX_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IInterAPOSFX_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


