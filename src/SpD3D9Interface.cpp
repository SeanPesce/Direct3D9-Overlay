// Author: Sean Pesce
// Original d3d9.dll wrapper base by Michael Koch

#include "stdafx.h"


SpD3D9Interface::SpD3D9Interface(IDirect3D9 *pOriginal)
{
    m_pIDirect3D9 = pOriginal;
}

SpD3D9Interface::~SpD3D9Interface(void)
{
}

HRESULT  __stdcall SpD3D9Interface::QueryInterface(REFIID riid, void** ppvObj)
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

ULONG    __stdcall SpD3D9Interface::AddRef(void)
{
    return(m_pIDirect3D9->AddRef());
}

ULONG    __stdcall SpD3D9Interface::Release(void)
{
    extern SpD3D9Interface* gl_pSpD3D9Interface;

	// Call original routine
	ULONG count = m_pIDirect3D9->Release();
	
    // In case no further Ref is there, the Original Object has deleted itself,
	// and so do we here
	if (count == 0) 
	{
		gl_pSpD3D9Interface = NULL;
  	    delete(this); 
	}

	return(count);
}

HRESULT __stdcall SpD3D9Interface::RegisterSoftwareDevice(void* pInitializeFunction)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->RegisterSoftwareDevice(pInitializeFunction));
}

UINT __stdcall SpD3D9Interface::GetAdapterCount(void)
{
    return(m_pIDirect3D9->GetAdapterCount());
}

HRESULT __stdcall SpD3D9Interface::GetAdapterIdentifier(UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->GetAdapterIdentifier(Adapter,Flags,pIdentifier));
}

UINT __stdcall SpD3D9Interface::GetAdapterModeCount(UINT Adapter, D3DFORMAT Format)
{
    return(m_pIDirect3D9->GetAdapterModeCount(Adapter, Format));
}

HRESULT __stdcall SpD3D9Interface::EnumAdapterModes(UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->EnumAdapterModes(Adapter,Format,Mode,pMode));
}

HRESULT __stdcall SpD3D9Interface::GetAdapterDisplayMode( UINT Adapter,D3DDISPLAYMODE* pMode)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->GetAdapterDisplayMode(Adapter,pMode));
}

HRESULT __stdcall SpD3D9Interface::CheckDeviceType(UINT iAdapter,D3DDEVTYPE DevType,D3DFORMAT DisplayFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->CheckDeviceType(iAdapter,DevType,DisplayFormat,BackBufferFormat,bWindowed));
}

HRESULT __stdcall SpD3D9Interface::CheckDeviceFormat(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat)
{
	HRESULT hres = m_pIDirect3D9->CheckDeviceFormat(Adapter,DeviceType,AdapterFormat,Usage,RType,CheckFormat);

	if (hres != D3DERR_NOTAVAILABLE)
	{
		_SP_D3D9_CHECK_AND_RETURN_FAILED_(hres);
	}

	return hres;
}

HRESULT __stdcall SpD3D9Interface::CheckDeviceMultiSampleType(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels)
{
	HRESULT hres = m_pIDirect3D9->CheckDeviceMultiSampleType(Adapter,DeviceType,SurfaceFormat,Windowed,MultiSampleType,pQualityLevels);

	if (hres != D3DERR_NOTAVAILABLE)
	{
		_SP_D3D9_CHECK_AND_RETURN_FAILED_(hres);
	}

	return hres;
}

HRESULT __stdcall SpD3D9Interface::CheckDepthStencilMatch(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->CheckDepthStencilMatch(Adapter,DeviceType,AdapterFormat,RenderTargetFormat,DepthStencilFormat));
}

HRESULT __stdcall SpD3D9Interface::CheckDeviceFormatConversion(UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->CheckDeviceFormatConversion(Adapter,DeviceType,SourceFormat,TargetFormat));
}

HRESULT __stdcall SpD3D9Interface::GetDeviceCaps(UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3D9->GetDeviceCaps(Adapter,DeviceType,pCaps));
}

HMONITOR __stdcall SpD3D9Interface::GetAdapterMonitor(UINT Adapter)
{
    return(m_pIDirect3D9->GetAdapterMonitor(Adapter));
}

HRESULT __stdcall SpD3D9Interface::CreateDevice(UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface)
{
	extern SpD3D9Device* gl_pSpD3D9Device; // Global var
	extern bool mod_loop_paused;

	mod_loop_paused = true;

	//while (gl_pSpD3D9Device != NULL)
	if(gl_pSpD3D9Device != NULL)
	{
		_SP_D3D9_LOG_EVENT_("Attempting to create device in thread %d with adapter id=%u   (Warning: device wrapper not NULL)", GetCurrentThreadId(), Adapter);
		gl_pSpD3D9Device->ForceRelease();
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
	gl_pSpD3D9Device = new SpD3D9Device(this, *ppReturnedDeviceInterface, hFocusWindow, pPresentationParameters);

	// Store our pointer (the fake one) for returning it to the calling progam
	*ppReturnedDeviceInterface = gl_pSpD3D9Device;

	mod_loop_paused = false;
	return(hres); 
}
  
