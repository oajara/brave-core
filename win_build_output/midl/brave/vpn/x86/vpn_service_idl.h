

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


 /* File created by MIDL compiler version 8.xx.xxxx */
/* at a redacted point in time
 */
/* Compiler settings for ../../brave/vpn/vpn_service_idl.idl:
    Oicf, W1, Zp8, env=Win32 (32b run), target_arch=X86 8.xx.xxxx 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
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

#ifndef __vpn_service_idl_h__
#define __vpn_service_idl_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#ifndef DECLSPEC_XFGVIRT
#if _CONTROL_FLOW_GUARD_XFG
#define DECLSPEC_XFGVIRT(base, func) __declspec(xfg_virtual(base, func))
#else
#define DECLSPEC_XFGVIRT(base, func)
#endif
#endif

/* Forward Declarations */ 

#ifndef __IVpnService_FWD_DEFINED__
#define __IVpnService_FWD_DEFINED__
typedef interface IVpnService IVpnService;

#endif 	/* __IVpnService_FWD_DEFINED__ */


#ifndef __IVpnService_FWD_DEFINED__
#define __IVpnService_FWD_DEFINED__
typedef interface IVpnService IVpnService;

#endif 	/* __IVpnService_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 


#ifndef __IVpnService_INTERFACE_DEFINED__
#define __IVpnService_INTERFACE_DEFINED__

/* interface IVpnService */
/* [unique][helpstring][uuid][oleautomation][object] */ 


EXTERN_C const IID IID_IVpnService;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("A949CB4E-C4F9-44C4-B213-6BF8AA9AC69A")
    IVpnService : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE EnableDNSFilters( 
            /* [in] */ const BSTR connection_name,
            /* [out] */ DWORD *error_code) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE DisableDNSFilters( 
            /* [in] */ const BSTR connection_name,
            /* [out] */ DWORD *error_code) = 0;
        
    };
    
    
#else 	/* C style interface */

    typedef struct IVpnServiceVtbl
    {
        BEGIN_INTERFACE
        
        DECLSPEC_XFGVIRT(IUnknown, QueryInterface)
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IVpnService * This,
            /* [in] */ REFIID riid,
            /* [annotation][iid_is][out] */ 
            _COM_Outptr_  void **ppvObject);
        
        DECLSPEC_XFGVIRT(IUnknown, AddRef)
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IVpnService * This);
        
        DECLSPEC_XFGVIRT(IUnknown, Release)
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IVpnService * This);
        
        DECLSPEC_XFGVIRT(IVpnService, EnableDNSFilters)
        HRESULT ( STDMETHODCALLTYPE *EnableDNSFilters )( 
            IVpnService * This,
            /* [in] */ const BSTR connection_name,
            /* [out] */ DWORD *error_code);
        
        DECLSPEC_XFGVIRT(IVpnService, DisableDNSFilters)
        HRESULT ( STDMETHODCALLTYPE *DisableDNSFilters )( 
            IVpnService * This,
            /* [in] */ const BSTR connection_name,
            /* [out] */ DWORD *error_code);
        
        END_INTERFACE
    } IVpnServiceVtbl;

    interface IVpnService
    {
        CONST_VTBL struct IVpnServiceVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IVpnService_QueryInterface(This,riid,ppvObject)	\
    ( (This)->lpVtbl -> QueryInterface(This,riid,ppvObject) ) 

#define IVpnService_AddRef(This)	\
    ( (This)->lpVtbl -> AddRef(This) ) 

#define IVpnService_Release(This)	\
    ( (This)->lpVtbl -> Release(This) ) 


#define IVpnService_EnableDNSFilters(This,connection_name,error_code)	\
    ( (This)->lpVtbl -> EnableDNSFilters(This,connection_name,error_code) ) 

#define IVpnService_DisableDNSFilters(This,connection_name,error_code)	\
    ( (This)->lpVtbl -> DisableDNSFilters(This,connection_name,error_code) ) 

#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IVpnService_INTERFACE_DEFINED__ */



#ifndef __VpnServiceLib_LIBRARY_DEFINED__
#define __VpnServiceLib_LIBRARY_DEFINED__

/* library VpnServiceLib */
/* [helpstring][version][uuid] */ 



EXTERN_C const IID LIBID_VpnServiceLib;
#endif /* __VpnServiceLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

unsigned long             __RPC_USER  BSTR_UserSize(     unsigned long *, unsigned long            , BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserMarshal(  unsigned long *, unsigned char *, BSTR * ); 
unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR * ); 
void                      __RPC_USER  BSTR_UserFree(     unsigned long *, BSTR * ); 

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


