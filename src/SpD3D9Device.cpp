// Author: Sean Pesce
// Original d3d9.dll wrapper base by Michael Koch

#include "stdafx.h"


SpD3D9Device::SpD3D9Device(SpD3D9Interface *d3d_interface, IDirect3DDevice9* pOriginal, HWND new_focus_window, D3DPRESENT_PARAMETERS *present_params)
{
	m_pIDirect3DDevice9 = pOriginal; // Store the pointer to original object
	
	overlay = new SpD3D9Overlay(d3d_interface, this, new_focus_window, present_params);
}

SpD3D9Device::~SpD3D9Device(void)
{
}

HRESULT SpD3D9Device::QueryInterface(REFIID riid, void** ppvObj)
{
	// Check if original dll can provide interface, then send this address
	*ppvObj = NULL;

	HRESULT hres = m_pIDirect3DDevice9->QueryInterface(riid, ppvObj);

	if (hres == S_OK)
	{
		*ppvObj = this;
	}

	return hres;
}

ULONG SpD3D9Device::AddRef(void)
{
	return(m_pIDirect3DDevice9->AddRef());
}

ULONG SpD3D9Device::Release(void)
{
	// ATTENTION: This is a booby-trap! Watch out!
	// If we create our own sprites, surfaces, etc. (thus increasing the ref counter
	// by external action), we need to delete those objects before calling the
	// original Release() function.

	// Global vars
	extern SpD3D9Device *gl_pSpD3D9Device;
	extern SpD3D9SwapChain *gl_pSpD3D9SwapChain;

	// Call original function
	ULONG count = m_pIDirect3DDevice9->Release();

	if (count == 0)
	{
		overlay->release_tasks();

		if (gl_pSpD3D9SwapChain != NULL)
		{
			gl_pSpD3D9SwapChain->Release();
		}
		gl_pSpD3D9SwapChain = NULL;


		// Delete device wrapper
		m_pIDirect3DDevice9 = NULL;
		gl_pSpD3D9Device = NULL;
		delete(this);  // Destructor will be called automatically
	}
	
	return (count);
}

ULONG SpD3D9Device::ForceRelease()
{
	// ATTENTION: This is a booby-trap! Watch out!
	// If we create our own sprites, surfaces, etc. (thus increasing the ref counter
	// by external action), we need to delete those objects before calling the
	// original Release() function.

	// Global vars
	extern SpD3D9Device *gl_pSpD3D9Device;
	extern SpD3D9SwapChain *gl_pSpD3D9SwapChain;

	// Release overlay resources
	overlay->force_release_tasks();

	ULONG count = 1;
	while (gl_pSpD3D9SwapChain != NULL && count > 0)
	{
		gl_pSpD3D9SwapChain->Release();
	}
	gl_pSpD3D9SwapChain = NULL;


	// Release main device interface
	count = 1;
	while (count > 0)
	{
		count = m_pIDirect3DDevice9->Release();
		Sleep(100);
	}

	// Delete device wrapper
	m_pIDirect3DDevice9 = NULL;
	gl_pSpD3D9Device = NULL;
	delete(this);  // Destructor will be called automatically

	return (count);
}


HRESULT SpD3D9Device::TestCooperativeLevel(void)
{
	HRESULT hres = m_pIDirect3DDevice9->TestCooperativeLevel();

	if (hres == D3DERR_DRIVERINTERNALERROR)
	{
		_SP_D3D9_CHECK_AND_RETURN_FAILED_(hres);
	}

	return hres;
}

UINT SpD3D9Device::GetAvailableTextureMem(void)
{
	return(m_pIDirect3DDevice9->GetAvailableTextureMem());
}

HRESULT SpD3D9Device::EvictManagedResources(void)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->EvictManagedResources());
}

HRESULT SpD3D9Device::GetDirect3D(IDirect3D9** ppD3D9)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetDirect3D(ppD3D9));
}

HRESULT SpD3D9Device::GetDeviceCaps(D3DCAPS9* pCaps)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetDeviceCaps(pCaps));
}

HRESULT SpD3D9Device::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetDisplayMode(iSwapChain, pMode));
}

HRESULT SpD3D9Device::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetCreationParameters(pParameters));
}

HRESULT SpD3D9Device::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap));
}

void    SpD3D9Device::SetCursorPosition(int X, int Y, DWORD Flags)
{
	return(m_pIDirect3DDevice9->SetCursorPosition(X, Y, Flags));
}

BOOL    SpD3D9Device::ShowCursor(BOOL bShow)
{
	return(m_pIDirect3DDevice9->ShowCursor(bShow));
}

HRESULT SpD3D9Device::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain));
}

HRESULT SpD3D9Device::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
	extern SpD3D9SwapChain *gl_pSpD3D9SwapChain;

	HRESULT hres = m_pIDirect3DDevice9->GetSwapChain(iSwapChain, pSwapChain);

	_SP_D3D9_CHECK_FAILED_(hres);
	if (iSwapChain == 0 && !FAILED(hres))
	{
		// Initialize swap chain wrapper object
		if (gl_pSpD3D9SwapChain == NULL)
		{
			gl_pSpD3D9SwapChain = new SpD3D9SwapChain(pSwapChain, this);
		}

		// Return wrapper object to calling program
		*pSwapChain = gl_pSpD3D9SwapChain;
	}
	else if (iSwapChain != 0)
	{
		_SP_D3D9_LOG_EVENT_("WARNING: Multiple swap chains not supported (GetSwapChain called with index %u)", iSwapChain);
		overlay->text_feed->print(std::string("WARNING: Multiple swap chains not supported (GetSwapChain called with index ").append(std::to_string(iSwapChain)).append(")").c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true, SP_D3D9O_TEXT_COLOR_RED);
	}

	return hres;
}

UINT    SpD3D9Device::GetNumberOfSwapChains(void)
{
	return(m_pIDirect3DDevice9->GetNumberOfSwapChains());
}

HRESULT SpD3D9Device::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	_SP_D3D9_LOG_EVENT_("Entering %s (thread %d)", __FUNCTION__, GetCurrentThreadId());

	HRESULT hres;

	// Store presentation parameters
	D3DPRESENT_PARAMETERS present_params;
	memcpy_s(&present_params, sizeof(present_params), pPresentationParameters, sizeof(*pPresentationParameters));

	overlay->reset_tasks();

	// Call original Reset() method
	hres = m_pIDirect3DDevice9->Reset(pPresentationParameters);
	_SP_D3D9_CHECK_FAILED_(hres);

	overlay->post_reset_tasks(&present_params);

	_SP_D3D9_LOG_EVENT_("Exiting %s (thread %d)", __FUNCTION__, GetCurrentThreadId());
	return hres;
}

HRESULT SpD3D9Device::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
	// Draw overlay before presenting frame
	overlay->draw(NULL);

	present_calls++; // Increment Present() call counter for the current second

	// Call original routine
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion));
}

HRESULT SpD3D9Device::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	get_back_buffer_calls++;

	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer));
}

HRESULT SpD3D9Device::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetRasterStatus(iSwapChain, pRasterStatus));
}

HRESULT SpD3D9Device::SetDialogBoxMode(BOOL bEnableDialogs)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetDialogBoxMode(bEnableDialogs));
}

void    SpD3D9Device::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	return(m_pIDirect3DDevice9->SetGammaRamp(iSwapChain, Flags, pRamp));
}

void    SpD3D9Device::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	return(m_pIDirect3DDevice9->GetGammaRamp(iSwapChain, pRamp));
}

HRESULT SpD3D9Device::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle));
}

HRESULT SpD3D9Device::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle));
}

HRESULT SpD3D9Device::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle));
}

HRESULT SpD3D9Device::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle));
}

HRESULT SpD3D9Device::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle));
}

HRESULT SpD3D9Device::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle));
}

HRESULT SpD3D9Device::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle));
}

HRESULT SpD3D9Device::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint));
}

HRESULT SpD3D9Device::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->UpdateTexture(pSourceTexture, pDestinationTexture));
}

HRESULT SpD3D9Device::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetRenderTargetData(pRenderTarget, pDestSurface));
}

HRESULT SpD3D9Device::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetFrontBufferData(iSwapChain, pDestSurface));
}

HRESULT SpD3D9Device::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter));
}

HRESULT SpD3D9Device::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->ColorFill(pSurface, pRect, color));
}

HRESULT SpD3D9Device::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle));
}

HRESULT SpD3D9Device::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetRenderTarget(RenderTargetIndex, pRenderTarget));
}

HRESULT SpD3D9Device::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetRenderTarget(RenderTargetIndex, ppRenderTarget));
}

HRESULT SpD3D9Device::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetDepthStencilSurface(pNewZStencil));
}

HRESULT SpD3D9Device::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	HRESULT hres = m_pIDirect3DDevice9->GetDepthStencilSurface(ppZStencilSurface);

	if (hres != D3D_OK && hres != D3DERR_NOTFOUND)
	{
		_SP_D3D9_CHECK_AND_RETURN_FAILED_(hres);
	}

	return hres;
}

HRESULT SpD3D9Device::BeginScene(void)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->BeginScene());
}

HRESULT SpD3D9Device::EndScene(void)
{
	// Drawing can be done here before the scene is shown to the user

	overlay->end_scene_tasks();

	endscene_calls++; // Increment EndScene call counter for stat-keeping

	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->EndScene());
}

HRESULT SpD3D9Device::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->Clear(Count, pRects, Flags, Color, Z, Stencil));
}

HRESULT SpD3D9Device::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetTransform(State, pMatrix));
}

HRESULT SpD3D9Device::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetTransform(State, pMatrix));
}

HRESULT SpD3D9Device::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->MultiplyTransform(State, pMatrix));
}

HRESULT SpD3D9Device::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetViewport(pViewport));
}

HRESULT SpD3D9Device::GetViewport(D3DVIEWPORT9* pViewport)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetViewport(pViewport));
}

HRESULT SpD3D9Device::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetMaterial(pMaterial));
}

HRESULT SpD3D9Device::GetMaterial(D3DMATERIAL9* pMaterial)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetMaterial(pMaterial));
}

HRESULT SpD3D9Device::SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetLight(Index, pLight));
}

HRESULT SpD3D9Device::GetLight(DWORD Index, D3DLIGHT9* pLight)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetLight(Index, pLight));
}

HRESULT SpD3D9Device::LightEnable(DWORD Index, BOOL Enable)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->LightEnable(Index, Enable));
}

HRESULT SpD3D9Device::GetLightEnable(DWORD Index, BOOL* pEnable)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetLightEnable(Index, pEnable));
}

HRESULT SpD3D9Device::SetClipPlane(DWORD Index, CONST float* pPlane)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetClipPlane(Index, pPlane));
}

HRESULT SpD3D9Device::GetClipPlane(DWORD Index, float* pPlane)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetClipPlane(Index, pPlane));
}

HRESULT SpD3D9Device::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetRenderState(State, Value));
}

HRESULT SpD3D9Device::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetRenderState(State, pValue));
}

HRESULT SpD3D9Device::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateStateBlock(Type, ppSB));
}

HRESULT SpD3D9Device::BeginStateBlock(void)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->BeginStateBlock());
}

HRESULT SpD3D9Device::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->EndStateBlock(ppSB));
}

HRESULT SpD3D9Device::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetClipStatus(pClipStatus));
}

HRESULT SpD3D9Device::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetClipStatus(pClipStatus));
}

HRESULT SpD3D9Device::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetTexture(Stage, ppTexture));
}

HRESULT SpD3D9Device::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetTexture(Stage, pTexture));
}

HRESULT SpD3D9Device::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetTextureStageState(Stage, Type, pValue));
}

HRESULT SpD3D9Device::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetTextureStageState(Stage, Type, Value));
}

HRESULT SpD3D9Device::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetSamplerState(Sampler, Type, pValue));
}

HRESULT SpD3D9Device::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetSamplerState(Sampler, Type, Value));
}

HRESULT SpD3D9Device::ValidateDevice(DWORD* pNumPasses)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->ValidateDevice(pNumPasses));
}

HRESULT SpD3D9Device::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT SpD3D9Device::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT SpD3D9Device::SetCurrentTexturePalette(UINT PaletteNumber)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetCurrentTexturePalette(PaletteNumber));
}

HRESULT SpD3D9Device::GetCurrentTexturePalette(UINT *PaletteNumber)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetCurrentTexturePalette(PaletteNumber));
}

HRESULT SpD3D9Device::SetScissorRect(CONST RECT* pRect)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetScissorRect(pRect));
}

HRESULT SpD3D9Device::GetScissorRect(RECT* pRect)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetScissorRect(pRect));
}

HRESULT SpD3D9Device::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetSoftwareVertexProcessing(bSoftware));
}

BOOL    SpD3D9Device::GetSoftwareVertexProcessing(void)
{
	return(m_pIDirect3DDevice9->GetSoftwareVertexProcessing());
}

HRESULT SpD3D9Device::SetNPatchMode(float nSegments)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetNPatchMode(nSegments));
}

float   SpD3D9Device::GetNPatchMode(void)
{
	return(m_pIDirect3DDevice9->GetNPatchMode());
}

HRESULT SpD3D9Device::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount));
}

HRESULT SpD3D9Device::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount));
}

HRESULT SpD3D9Device::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride));
}

HRESULT SpD3D9Device::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride));
}

HRESULT SpD3D9Device::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags));
}

HRESULT SpD3D9Device::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateVertexDeclaration(pVertexElements, ppDecl));
}

HRESULT SpD3D9Device::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetVertexDeclaration(pDecl));
}

HRESULT SpD3D9Device::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetVertexDeclaration(ppDecl));
}

HRESULT SpD3D9Device::SetFVF(DWORD FVF)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetFVF(FVF));
}

HRESULT SpD3D9Device::GetFVF(DWORD* pFVF)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetFVF(pFVF));
}

HRESULT SpD3D9Device::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateVertexShader(pFunction, ppShader));
}

HRESULT SpD3D9Device::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetVertexShader(pShader));
}

HRESULT SpD3D9Device::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetVertexShader(ppShader));
}

HRESULT SpD3D9Device::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT SpD3D9Device::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT SpD3D9Device::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT SpD3D9Device::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT SpD3D9Device::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT SpD3D9Device::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT SpD3D9Device::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride));
}

HRESULT SpD3D9Device::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride));
}

HRESULT SpD3D9Device::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetStreamSourceFreq(StreamNumber, Divider));
}

HRESULT SpD3D9Device::GetStreamSourceFreq(UINT StreamNumber, UINT* Divider)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetStreamSourceFreq(StreamNumber, Divider));
}

HRESULT SpD3D9Device::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetIndices(pIndexData));
}

HRESULT SpD3D9Device::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetIndices(ppIndexData));
}

HRESULT SpD3D9Device::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreatePixelShader(pFunction, ppShader));
}

HRESULT SpD3D9Device::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetPixelShader(pShader));
}

HRESULT SpD3D9Device::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetPixelShader(ppShader));
}

HRESULT SpD3D9Device::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT SpD3D9Device::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT SpD3D9Device::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT SpD3D9Device::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT SpD3D9Device::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT SpD3D9Device::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT SpD3D9Device::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo));
}

HRESULT SpD3D9Device::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo));
}

HRESULT SpD3D9Device::DeletePatch(UINT Handle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DeletePatch(Handle));
}

HRESULT SpD3D9Device::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateQuery(Type, ppQuery));
}



/////////////////////// Overlay-related functions start here ///////////////////////





/*// Prints various game window data to the overlay text feed
void SpD3D9Device::print_debug_data(unsigned long long duration, bool show_timestamp)
{
	std::string str;
	if (is_windowed)
	{
		print_to_overlay_feed("Windowed mode", duration, show_timestamp);
	}
	else
	{
		print_to_overlay_feed("Fullscreen mode", duration, show_timestamp);
	}

	D3DDISPLAYMODE display_mode;
	HRESULT hres = GetDisplayMode(0, &display_mode);
	_SP_D3D9_CHECK_FAILED_(GetDisplayMode(0, &display_mode));
	if (!FAILED(hres))
	{
		print_to_overlay_feed(std::string("DisplayMode: (0,0)  ").append(std::to_string(display_mode.Width)).append("x").append(std::to_string(display_mode.Height)).c_str(), duration, show_timestamp);
	}
	else
	{
		// Handle error
		print_to_overlay_feed("Device: ERROR", duration, show_timestamp);
	}

	if (game_window != NULL)
	{
		rect_to_string(game_window_rect, "GameWindow", &str);
		print_to_overlay_feed(str.c_str(), duration, show_timestamp);
	}
	else
	{
		print_to_overlay_feed("GameWindow: NULL", duration, show_timestamp);
	}

	// Print focus window data (if not NULL)
	if (focus_window != NULL)
	{
		rect_to_string(&focus_window_rect, "FocusWindow", &str);
		print_to_overlay_feed(str.c_str(), duration, show_timestamp);
	}
	else
	{
		print_to_overlay_feed("FocusWindow: NULL", duration, show_timestamp);
	}

	// Print device window data (if not NULL)
	if (device_window != NULL)
	{
		rect_to_string(&device_window_rect, "DeviceWindow", &str);
		print_to_overlay_feed(str.c_str(), duration, show_timestamp);
	}
	else
	{
		print_to_overlay_feed("DeviceWindow: NULL", duration, show_timestamp);
	}

	rect_to_string(&focus_window_rect, "BackBuffer", &str);
	print_to_overlay_feed(str.c_str(), duration, show_timestamp);
	
	RECT vp_rect;
	get_viewport_as_rect(&vp_rect);
	rect_to_string(&vp_rect, "ViewPort", &str);
	print_to_overlay_feed(str.c_str(), duration, show_timestamp);

	print_to_overlay_feed(std::string("GetBackBufferCallCount: ").append(std::to_string(get_back_buffer_calls)).c_str(), duration, false);
	print_to_overlay_feed(std::string("PresentCallCount: ").append(std::to_string(present_calls)).c_str(), duration, false);
	print_to_overlay_feed(std::string("SwapChainPresentCallCount: ").append(std::to_string(swap_chain_present_calls)).c_str(), duration, false);
	print_to_overlay_feed(std::string("EndSceneCallCount: ").append(std::to_string(endscene_calls)).c_str(), duration, false);
}


// Constructs a string describing the specified RECT struct and stores it in the given std::string
void rect_to_string(RECT *rect, const char *rect_name, std::string *str)
{
	str->clear();
	str->append(rect_name).append(std::string(" (")).append(std::to_string(rect->left)).append(",").append(std::to_string(rect->top)).append(")  ").append(std::to_string(rect->right - rect->left)).append("x").append(std::to_string(rect->bottom - rect->top));
}

// Constructs a RECT struct from the device viewport
RECT *SpD3D9Device::get_viewport_as_rect(RECT *rect)
{
	D3DVIEWPORT9 viewport;
	HRESULT hres = GetViewport(&viewport);

	if (hres == D3DERR_INVALIDCALL)
	{
		// Handle error
	}

	SetRect(rect, viewport.X, viewport.Y, viewport.X + viewport.Width, viewport.Y + viewport.Height);

	return rect;
}

// Constructs a RECT struct from the device viewport (and stores the viewport)
RECT *SpD3D9Device::get_viewport_as_rect(RECT *rect, D3DVIEWPORT9 *viewport)
{
	HRESULT hres = GetViewport(viewport);

	if (hres == D3DERR_INVALIDCALL)
	{
		// Handle error
	}

	SetRect(rect, viewport->X, viewport->Y, viewport->X + viewport->Width, viewport->Y + viewport->Height);

	return rect;
}*/
