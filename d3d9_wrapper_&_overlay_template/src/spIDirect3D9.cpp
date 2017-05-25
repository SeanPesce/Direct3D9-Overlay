// Author: Sean Pesce
// Original d3d9.dll wrapper base by Michael Koch

#include "stdafx.h"


SpIDirect3D9::SpIDirect3D9(IDirect3D9 *pOriginal)
{
    m_pIDirect3D9 = pOriginal;
}

SpIDirect3D9::~SpIDirect3D9(void)
{
}

HRESULT  __stdcall SpIDirect3D9::QueryInterface(REFIID riid, void** ppvObj)
{
    *ppvObj = NULL;

	// Call this to increase AddRef at original object
	// and to check if such an interface is there

	HRESULT hres = m_pIDirect3D9->QueryInterface(riid, ppvObj); 

	if (hres == S_OK) // If OK, send our "fake" address
	{
		*ppvObj = this;
	}
	
	return hres;
}

ULONG    __stdcall SpIDirect3D9::AddRef(void)
{
    return(m_pIDirect3D9->AddRef());
}

ULONG    __stdcall SpIDirect3D9::Release(void)
{
    extern SpIDirect3D9* gl_pSpIDirect3D9;

	// Call original routine
	ULONG count = m_pIDirect3D9->Release();
	
    // In case no further Ref is there, the Original Object has deleted itself,
	// and so do we here
	if (count == 0) 
	{
		gl_pSpIDirect3D9 = NULL;
  	    delete(this); 
	}

	return(count);
}

HRESULT __stdcall SpIDirect3D9::RegisterSoftwareDevice(void* pInitializeFunction)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->RegisterSoftwareDevice(pInitializeFunction));
}

UINT __stdcall SpIDirect3D9::GetAdapterCount(void)
{
    return(m_pIDirect3D9->GetAdapterCount());
}

HRESULT __stdcall SpIDirect3D9::GetAdapterIdentifier(UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->GetAdapterIdentifier(Adapter,Flags,pIdentifier));
}

UINT __stdcall SpIDirect3D9::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
    return(m_pIDirect3D9->GetAdapterModeCount(Adapter, Format));
}

HRESULT __stdcall SpIDirect3D9::EnumAdapterModes(UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->EnumAdapterModes(Adapter,Format,Mode,pMode));
}

HRESULT __stdcall SpIDirect3D9::GetAdapterDisplayMode( UINT Adapter,D3DDISPLAYMODE* pMode)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->GetAdapterDisplayMode(Adapter,pMode));
}

HRESULT __stdcall SpIDirect3D9::CheckDeviceType(UINT iAdapter,D3DDEVTYPE DevType,D3DFORMAT DisplayFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->CheckDeviceType(iAdapter,DevType,DisplayFormat,BackBufferFormat,bWindowed));
}

HRESULT __stdcall SpIDirect3D9::CheckDeviceFormat(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat)
{
	HRESULT hres = m_pIDirect3D9->CheckDeviceFormat(Adapter,DeviceType,AdapterFormat,Usage,RType,CheckFormat);

	if (hres != D3DERR_NOTAVAILABLE)
	{
		_SP_D3D9_CHECK_AND_RETURN_FAILED_(hres);
	}

	return hres;
}

HRESULT __stdcall SpIDirect3D9::CheckDeviceMultiSampleType(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels)
{
	HRESULT hres = m_pIDirect3D9->CheckDeviceMultiSampleType(Adapter,DeviceType,SurfaceFormat,Windowed,MultiSampleType,pQualityLevels);

	if (hres != D3DERR_NOTAVAILABLE)
	{
		_SP_D3D9_CHECK_AND_RETURN_FAILED_(hres);
	}

	return hres;
}

HRESULT __stdcall SpIDirect3D9::CheckDepthStencilMatch(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->CheckDepthStencilMatch(Adapter,DeviceType,AdapterFormat,RenderTargetFormat,DepthStencilFormat));
}

HRESULT __stdcall SpIDirect3D9::CheckDeviceFormatConversion(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->CheckDeviceFormatConversion(Adapter,DeviceType,SourceFormat,TargetFormat));
}

HRESULT __stdcall SpIDirect3D9::GetDeviceCaps(UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->GetDeviceCaps(Adapter,DeviceType,pCaps));
}

HMONITOR __stdcall SpIDirect3D9::GetAdapterMonitor(UINT Adapter)
{
    return(m_pIDirect3D9->GetAdapterMonitor(Adapter));
}

HRESULT __stdcall SpIDirect3D9::CreateDevice(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface)
{
	extern SpIDirect3DDevice9* gl_pSpIDirect3DDevice9; // Global var
	extern bool mod_loop_paused;

	mod_loop_paused = true;

	//while (gl_pSpIDirect3DDevice9 != NULL)
	if(gl_pSpIDirect3DDevice9 != NULL)
	{
		_SP_D3D9_LOG_EVENT_("Attempting to create device in thread %d with adapter id=%u   (Warning: device wrapper not NULL)", GetCurrentThreadId(), Adapter);
		gl_pSpIDirect3DDevice9->ForceRelease();
		Sleep(100);
	}
	else
	{
		_SP_D3D9_LOG_EVENT_("Attempting to create device in thread %d with adapter id=%u", GetCurrentThreadId(), Adapter);
	}
	
	// We intercept this call and provide our own "fake" Device Object
	HRESULT hres = m_pIDirect3D9->CreateDevice( Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);
	_SP_D3D9_CHECK_FAILED_(hres);

	if (!FAILED(hres))
	{
		// Check initial reference count for d3d9 device interface
		(*ppReturnedDeviceInterface)->AddRef();
		_SP_D3D9_LOG_EVENT_("Device created in thread %d  (adapter id=%u, ref count=%lu)", GetCurrentThreadId(), Adapter, (*ppReturnedDeviceInterface)->Release());
	}

	// Create our own Device object and store it in global pointer
	// Note: The object will delete itself once Ref count is zero (similar to COM objects)
	gl_pSpIDirect3DDevice9 = new SpIDirect3DDevice9(Adapter, *ppReturnedDeviceInterface, hFocusWindow, pPresentationParameters);

	// Store our pointer (the fake one) for returning it to the calling progam
	*ppReturnedDeviceInterface = gl_pSpIDirect3DDevice9;

	mod_loop_paused = false;
	return(hres); 
}
  
