// Author: Sean Pesce

#include "stdafx.h"

spIDirect3DSwapChain9::spIDirect3DSwapChain9(IDirect3DSwapChain9 **ppIDirect3DSwapChain9, myIDirect3DDevice9 *device)
{
	m_pD3D9_swap_chain = *ppIDirect3DSwapChain9;
	this->device = device;
	present_calls = &(device->present_calls);
	overlay_rendered_this_frame = &(device->overlay_rendered_this_frame);
	*ppIDirect3DSwapChain9 = this;
}


ULONG spIDirect3DSwapChain9::AddRef()
{
	return m_pD3D9_swap_chain->AddRef();
}


HRESULT spIDirect3DSwapChain9::QueryInterface(REFIID riid, void **ppvObject)
{
	// Check if original dll can provide interface, then send this address
	*ppvObject = NULL;

	HRESULT hRes = m_pD3D9_swap_chain->QueryInterface(riid, ppvObject);

	if (hRes == NOERROR)
	{
		*ppvObject = this;
	}

	return hRes;
}


ULONG spIDirect3DSwapChain9::Release()
{
	extern spIDirect3DSwapChain9 *gl_pspIDirect3DSwapChain9;

	ULONG count = m_pD3D9_swap_chain->Release();

	if (count == 0)
	{
		m_pD3D9_swap_chain = NULL;
		gl_pspIDirect3DSwapChain9 = NULL;
		delete this;
	}

	return count;
}


HRESULT spIDirect3DSwapChain9::GetBackBuffer(UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 **ppBackBuffer)
{
	return m_pD3D9_swap_chain->GetBackBuffer(BackBuffer, Type, ppBackBuffer);
}


HRESULT spIDirect3DSwapChain9::GetDevice(IDirect3DDevice9 **ppDevice)
{
	return m_pD3D9_swap_chain->GetDevice(ppDevice);
}


HRESULT spIDirect3DSwapChain9::GetDisplayMode(D3DDISPLAYMODE *pMode)
{
	return m_pD3D9_swap_chain->GetDisplayMode(pMode);
}


HRESULT spIDirect3DSwapChain9::GetFrontBufferData(IDirect3DSurface9 *pDestSurface)
{
	return m_pD3D9_swap_chain->GetFrontBufferData(pDestSurface);
}


HRESULT spIDirect3DSwapChain9::GetPresentParameters(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	return m_pD3D9_swap_chain->GetPresentParameters(pPresentationParameters);
}


HRESULT spIDirect3DSwapChain9::GetRasterStatus(D3DRASTER_STATUS *pRasterStatus)
{
	return m_pD3D9_swap_chain->GetRasterStatus(pRasterStatus);
}


HRESULT spIDirect3DSwapChain9::Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion, DWORD dwFlags)
{
	// Draw overlay before calling the real Present() method:

	IDirect3DDevice9 *real_device = NULL;
	if (GetDevice(&real_device) != D3D_OK)
	{
		// Handle error
	}

	
	if (real_device != NULL)
	{
		device->draw_overlay(real_device, m_pD3D9_swap_chain); // Draw overlay
		real_device->Release();
		real_device = NULL;
	}


	(*present_calls)++;
	HRESULT hres = m_pD3D9_swap_chain->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
	(*overlay_rendered_this_frame) = false;
	return hres;
}