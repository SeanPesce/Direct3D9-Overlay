// Author: Sean Pesce

#pragma once

#ifndef _SP_IDIRECT3D_SWAP_CHAIN_H_
	#define _SP_IDIRECT3D_SWAP_CHAIN_H_

#include <d3dx9core.h>

class spIDirect3DSwapChain9 : public IDirect3DSwapChain9
{
public:
	IDirect3DSwapChain9 *m_pD3D9_swap_chain;
	spIDirect3DSwapChain9(IDirect3DSwapChain9 **ppIDirect3DSwapChain9, UINT *present_counter, bool *presented_flag);

	unsigned int *present_calls; // Number of times Present() was called thus far in the current second
	bool *overlay_rendered_this_frame; // Indicates whether the current frame has been presented

	// Original DX9 function definitions
	ULONG	__stdcall AddRef();
	HRESULT	__stdcall QueryInterface(REFIID riid, void **ppvObject);
	ULONG	__stdcall Release();
	HRESULT	__stdcall GetBackBuffer(UINT BackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9 **ppBackBuffer);
	HRESULT	__stdcall GetDevice(IDirect3DDevice9 **ppDevice);
	HRESULT	__stdcall GetDisplayMode(D3DDISPLAYMODE *pMode);
	HRESULT	__stdcall GetFrontBufferData(IDirect3DSurface9 *pDestSurface);
	HRESULT	__stdcall GetPresentParameters(D3DPRESENT_PARAMETERS *pPresentationParameters);
	HRESULT	__stdcall GetRasterStatus(D3DRASTER_STATUS *pRasterStatus);
	HRESULT	__stdcall Present(const RECT *pSourceRect, const RECT *pDestRect, HWND hDestWindowOverride, const RGNDATA *pDirtyRegion, DWORD dwFlags);
	// End of original DX9 function definitions
};

#endif // _SP_IDIRECT3D_SWAP_CHAIN_H_