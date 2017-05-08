// Author: Sean Pesce

#include "stdafx.h"
#include "spIDirect3DSwapChain9.h"


spIDirect3DSwapChain9::spIDirect3DSwapChain9(IDirect3DSwapChain9 **ppIDirect3DSwapChain9, UINT *present_counter, bool *presented_flag) {
	m_pD3D9_swap_chain = *ppIDirect3DSwapChain9;
	present_calls = present_counter;
	overlay_rendered_this_frame = presented_flag;
	*ppIDirect3DSwapChain9 = this;
}

ULONG spIDirect3DSwapChain9::AddRef()
{
	return m_pD3D9_swap_chain->AddRef();
}


HRESULT spIDirect3DSwapChain9::QueryInterface(REFIID riid, void **ppvObject)
{
	return m_pD3D9_swap_chain->QueryInterface(riid, ppvObject);
}


ULONG spIDirect3DSwapChain9::Release()
{
	return m_pD3D9_swap_chain->Release();
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
	(*present_calls)++;
	HRESULT hres = m_pD3D9_swap_chain->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
	(*overlay_rendered_this_frame) = false;
	return hres;
}