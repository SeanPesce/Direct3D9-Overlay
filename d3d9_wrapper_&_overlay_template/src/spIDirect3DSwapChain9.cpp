// Author: Sean Pesce

#include "stdafx.h"

SpIDirect3DSwapChain9::SpIDirect3DSwapChain9(IDirect3DSwapChain9 **ppIDirect3DSwapChain9, SpIDirect3DDevice9 *device)
{
	m_pD3D9_swap_chain = *ppIDirect3DSwapChain9;
	this->device = device;
	present_calls = &(device->swap_chain_present_calls);
	overlay_rendered_this_frame = &(device->overlay_rendered_this_frame);
	*ppIDirect3DSwapChain9 = this;
}


ULONG SpIDirect3DSwapChain9::AddRef()
{
	return m_pD3D9_swap_chain->AddRef();
}


HRESULT SpIDirect3DSwapChain9::QueryInterface(REFIID riid, void **ppvObject)
{
	// Check if original dll can provide interface, then send this address
	*ppvObject = NULL;

	HRESULT hres = m_pD3D9_swap_chain->QueryInterface(riid, ppvObject);

	if (hres == S_OK)
	{
		*ppvObject = this;
	}

	return hres;
}


ULONG SpIDirect3DSwapChain9::Release()
{
	extern SpIDirect3DSwapChain9 *gl_pSpIDirect3DSwapChain9;

	ULONG count = m_pD3D9_swap_chain->Release();

	if (count == 0)
	{
		//m_pD3D9_swap_chain = NULL;
		gl_pSpIDirect3DSwapChain9 = NULL;
		delete this;
	}

	return count;
}


HRESULT SpIDirect3DSwapChain9::GetBackBuffer(UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 **ppBackBuffer)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pD3D9_swap_chain->GetBackBuffer(BackBuffer, Type, ppBackBuffer));
}


HRESULT SpIDirect3DSwapChain9::GetDevice(IDirect3DDevice9 **ppDevice)
{
	HRESULT hres = m_pD3D9_swap_chain->GetDevice(ppDevice);

	_SP_D3D9_CHECK_FAILED_(hres);
	if(!FAILED(hres))
	{
		// Return address of device wrapper instead
		(*ppDevice) = device;
	}

	return hres;
}


HRESULT SpIDirect3DSwapChain9::GetDisplayMode(D3DDISPLAYMODE *pMode)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pD3D9_swap_chain->GetDisplayMode(pMode));
}


HRESULT SpIDirect3DSwapChain9::GetFrontBufferData(IDirect3DSurface9 *pDestSurface)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pD3D9_swap_chain->GetFrontBufferData(pDestSurface));
}


HRESULT SpIDirect3DSwapChain9::GetPresentParameters(D3DPRESENT_PARAMETERS *pPresentationParameters)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pD3D9_swap_chain->GetPresentParameters(pPresentationParameters));
}


HRESULT SpIDirect3DSwapChain9::GetRasterStatus(D3DRASTER_STATUS *pRasterStatus)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pD3D9_swap_chain->GetRasterStatus(pRasterStatus));
}


HRESULT SpIDirect3DSwapChain9::Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion, DWORD dwFlags)
{
	// Draw overlay before calling the real Present() method:
	IDirect3DDevice9 *real_device = NULL;
	_SP_D3D9_CHECK_FAILED_(m_pD3D9_swap_chain->GetDevice(&real_device));

	
	if (real_device != NULL)
	{
		device->draw_overlay(real_device, m_pD3D9_swap_chain); // Draw overlay
		device->Release();
		real_device = NULL;
	}


	(*present_calls)++;
	HRESULT hres = m_pD3D9_swap_chain->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
	(*overlay_rendered_this_frame) = false;
	return hres;
}