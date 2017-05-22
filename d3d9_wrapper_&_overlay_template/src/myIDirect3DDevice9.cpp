// Author: Sean Pesce
// Original d3d9.dll wrapper base by Michael Koch

#include "stdafx.h"


myIDirect3DDevice9::myIDirect3DDevice9(UINT Adapter, IDirect3DDevice9* pOriginal, HWND new_focus_window, D3DPRESENT_PARAMETERS *present_params)
{
	m_pIDirect3DDevice9 = pOriginal; // Store the pointer to original object
	
	id = Adapter;
	is_windowed = present_params->Windowed != 0;

	// Store the focus window attributes
	focus_window = new_focus_window;
	if (focus_window != NULL)
	{
		if (GetClientRect(focus_window, &focus_window_rect))
		{
			// Handle error
		}
	}
	else
	{
		SetRect(&focus_window_rect, 0, 0, 0, 0);
	}

	// Store the device window attributes
	device_window = present_params->hDeviceWindow;
	if (device_window != NULL)
	{
		if (GetClientRect(device_window, &device_window_rect))
		{
			// Handle error
		}

		// Store the back buffer attributes
		if (present_params->BackBufferWidth && present_params->BackBufferHeight)
		{
			SetRect(&back_buffer_rect, 0, 0, present_params->BackBufferWidth, present_params->BackBufferHeight);
		}
		else
		{
			// BackBuffer Width or Height was zero; use device window attributes
			if (GetClientRect(device_window, &back_buffer_rect))
			{
				// Handle error
			}
		}
	}
	else
	{
		SetRect(&device_window_rect, 0, 0, 0, 0);

		// Store the back buffer attributes
		if (present_params->BackBufferWidth && present_params->BackBufferHeight)
		{
			SetRect(&back_buffer_rect, 0, 0, present_params->BackBufferWidth, present_params->BackBufferHeight);
		}
		else
		{
			// BackBuffer Width or Height was zero; use focus window attributes
			if (GetClientRect(focus_window, &back_buffer_rect))
			{
				// Handle error
			}
		}
	}

	// Set main game window
	if (is_windowed)
	{ // Windowed mode
		
		if (device_window != NULL)
		{
			game_window = &device_window;
			game_window_rect = &device_window_rect;
		}
		else
		{
			game_window = &focus_window;
			game_window_rect = &focus_window_rect;
		}
	}
	else
	{ // Full-screen mode
		
		if (focus_window != NULL)
		{
			game_window = &focus_window;
			game_window_rect = &focus_window_rect;
		}
		else
		{
			game_window = &device_window;
			game_window_rect = &device_window_rect;
		}
	}

	// Initialize overlay text feed
	SP_DX9_init_text_overlay(pOriginal,
		_SP_DEFAULT_TEXT_HEIGHT_,
		_SP_DEFAULT_TEXT_OUTLINE_THICKNESS_,
		_SP_DEFAULT_TEXT_SHADOW_X_OFFSET_,
		_SP_DEFAULT_TEXT_SHADOW_Y_OFFSET_,
		_SP_DEFAULT_TEXT_COLOR_,
		_SP_DEFAULT_TEXT_OUTLINE_COLOR_,
		_SP_DEFAULT_TEXT_SHADOW_COLOR_,
		_SP_DEFAULT_TEXT_FORMAT_,
		_SP_DEFAULT_TEXT_STYLE_);

	// Start the FPS count timer
	fps_timer_id = 0;
	fps_timer_id = SetTimer(NULL, 0, 1000, &update_fps);
	if (!fps_timer_id)
	{
		// Handle error
	}
}

myIDirect3DDevice9::~myIDirect3DDevice9(void)
{
}

HRESULT myIDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj)
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

ULONG myIDirect3DDevice9::AddRef(void)
{
	return(m_pIDirect3DDevice9->AddRef());
}

ULONG myIDirect3DDevice9::Release(void)
{
	// ATTENTION: This is a booby-trap! Watch out!
	// If we create our own sprites, surfaces, etc. (thus increasing the ref counter
	// by external action), we need to delete those objects before calling the
	// original Release() function.

	// Global vars
	extern myIDirect3DDevice9 *gl_pmyIDirect3DDevice9;
	extern spIDirect3DSwapChain9 *gl_pspIDirect3DSwapChain9;

	// Call original function
	ULONG count = m_pIDirect3DDevice9->Release();

	if (count == 0)
	{
		KillTimer(NULL, fps_timer_id); // Disable the FPS count timer


		// Release overlay resources
		if (overlay_state_block != NULL)
		{
			overlay_state_block->Release();
		}
		overlay_state_block = NULL;


		/*if (gl_pmyIDirect3DDevice9->text_overlay.font != NULL)
		{
			gl_pmyIDirect3DDevice9->text_overlay.font->Release()
		}*/
		gl_pmyIDirect3DDevice9->text_overlay.font = NULL;


		if (gl_pspIDirect3DSwapChain9 != NULL)
		{
			gl_pspIDirect3DSwapChain9->Release();
		}
		gl_pspIDirect3DSwapChain9 = NULL;


		// Delete device wrapper
		m_pIDirect3DDevice9 = NULL;
		gl_pmyIDirect3DDevice9 = NULL;
		delete(this);  // Destructor will be called automatically
	}
	
	return (count);
}

ULONG myIDirect3DDevice9::ForceRelease()
{
	// ATTENTION: This is a booby-trap! Watch out!
	// If we create our own sprites, surfaces, etc. (thus increasing the ref counter
	// by external action), we need to delete those objects before calling the
	// original Release() function.

	// Global vars
	extern myIDirect3DDevice9 *gl_pmyIDirect3DDevice9;
	extern spIDirect3DSwapChain9 *gl_pspIDirect3DSwapChain9;

	KillTimer(NULL, fps_timer_id); // Disable the FPS count timer

	// Release overlay resources
	if (gl_pmyIDirect3DDevice9 != NULL && overlay_state_block != NULL)
	{
		overlay_state_block->Release();
	}
	overlay_state_block = NULL;


	// Release overlay resources to avoid memory leaks
	/*if (gl_pmyIDirect3DDevice9 != NULL && gl_pmyIDirect3DDevice9->text_overlay.font != NULL)
	{
		_SP_D3D9_LOG_EVENT_("Attempting to release font in thread %d", GetCurrentThreadId());
		//gl_pmyIDirect3DDevice9->text_overlay.font->Release();
		_SP_D3D9_LOG_EVENT_("Font released; ref count=%u", gl_pmyIDirect3DDevice9->text_overlay.font->Release());
	}*/
	gl_pmyIDirect3DDevice9->text_overlay.font = NULL;


	ULONG count = 1;
	while (gl_pspIDirect3DSwapChain9 != NULL && count > 0)
	{
		gl_pspIDirect3DSwapChain9->Release();
	}
	gl_pspIDirect3DSwapChain9 = NULL;


	// Release main device interface
	count = 1;
	while (count > 0)
	{
		count = m_pIDirect3DDevice9->Release();
		Sleep(100);
	}

	// Delete device wrapper
	m_pIDirect3DDevice9 = NULL;
	gl_pmyIDirect3DDevice9 = NULL;
	delete(this);  // Destructor will be called automatically

	return (count);
}


HRESULT myIDirect3DDevice9::TestCooperativeLevel(void)
{
	HRESULT hres = m_pIDirect3DDevice9->TestCooperativeLevel();

	if (hres == D3DERR_DRIVERINTERNALERROR)
	{
		_SP_D3D9_CHECK_AND_RETURN_FAILED_(hres);
	}

	return hres;
}

UINT myIDirect3DDevice9::GetAvailableTextureMem(void)
{
	return(m_pIDirect3DDevice9->GetAvailableTextureMem());
}

HRESULT myIDirect3DDevice9::EvictManagedResources(void)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->EvictManagedResources());
}

HRESULT myIDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetDirect3D(ppD3D9));
}

HRESULT myIDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetDeviceCaps(pCaps));
}

HRESULT myIDirect3DDevice9::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetDisplayMode(iSwapChain, pMode));
}

HRESULT myIDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetCreationParameters(pParameters));
}

HRESULT myIDirect3DDevice9::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap));
}

void    myIDirect3DDevice9::SetCursorPosition(int X, int Y, DWORD Flags)
{
	return(m_pIDirect3DDevice9->SetCursorPosition(X, Y, Flags));
}

BOOL    myIDirect3DDevice9::ShowCursor(BOOL bShow)
{
	return(m_pIDirect3DDevice9->ShowCursor(bShow));
}

HRESULT myIDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain));
}

HRESULT myIDirect3DDevice9::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain)
{
	extern spIDirect3DSwapChain9 *gl_pspIDirect3DSwapChain9;

	HRESULT hres = m_pIDirect3DDevice9->GetSwapChain(iSwapChain, pSwapChain);

	_SP_D3D9_CHECK_FAILED_(hres);
	if (iSwapChain == 0 && !FAILED(hres))
	{
		// Initialize swap chain wrapper object
		if (gl_pspIDirect3DSwapChain9 == NULL)
		{
			gl_pspIDirect3DSwapChain9 = new spIDirect3DSwapChain9(pSwapChain, this);
		}

		// Return wrapper object to calling program
		*pSwapChain = gl_pspIDirect3DSwapChain9;
	}
	else if (iSwapChain != 0)
	{
		_SP_D3D9_LOG_EVENT_("WARNING: Multiple swap chains not supported (GetSwapChain called with index %u)", iSwapChain);
		print_to_overlay_feed(std::string("WARNING: Multiple swap chains not supported (GetSwapChain called with index ").append(std::to_string(iSwapChain)).append(")").c_str(), _SP_D3D9_OL_TEXT_FEED_MSG_LIFESPAN_, true, SP_DX9_TEXT_COLOR_RED);
	}

	return hres;
}

UINT    myIDirect3DDevice9::GetNumberOfSwapChains(void)
{
	return(m_pIDirect3DDevice9->GetNumberOfSwapChains());
}

HRESULT myIDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	_SP_D3D9_LOG_EVENT_("Entering %s (thread %d)", __FUNCTION__, GetCurrentThreadId());

	HRESULT hres;

	// Store presentation parameters
	D3DPRESENT_PARAMETERS present_params;
	memcpy_s(&present_params, sizeof(present_params), pPresentationParameters, sizeof(*pPresentationParameters));
	
	// Release the previously-created overlay state block (if it exists)
	if (overlay_state_block != NULL)
	{
		overlay_state_block->Release();
		overlay_state_block = NULL;
	}


	// Release video memory resources used by overlay text feed font
	if (text_overlay.font != NULL)
	{
		_SP_D3D9_CHECK_FAILED_(text_overlay.font->OnLostDevice()); // (Must be called before resetting a device)
	}


	// Call original Reset() method
	hres = m_pIDirect3DDevice9->Reset(pPresentationParameters);
	_SP_D3D9_CHECK_FAILED_(hres);

	// Re-acquire video memory resources for overlay text feed font
	if (text_overlay.font != NULL)
	{
		_SP_D3D9_CHECK_FAILED_(text_overlay.font->OnResetDevice()); // (Must be called after resetting a device)
	}
	


	// Store window mode (windowed or fullscreen)
	is_windowed = present_params.Windowed != 0;

	//print_to_overlay_feed(std::string("Reset() - BackBuffer: ").append(std::to_string(present_params.BackBufferWidth)).append("x").append(std::to_string(present_params.BackBufferHeight)).c_str(), 30000, false);
	
	// Set device window parameters
	device_window = present_params.hDeviceWindow;
	if (device_window != NULL)
	{
		// Store the back buffer attributes
		if (present_params.BackBufferWidth && present_params.BackBufferHeight)
		{
			SetRect(&back_buffer_rect, 0, 0, present_params.BackBufferWidth, present_params.BackBufferHeight);
		}
		else
		{
			// BackBuffer Width and/or Height was zero
			SetRect(&back_buffer_rect, 0, 0, 0, 0);
		}
	}
	else
	{
		// Store the back buffer attributes
		if (present_params.BackBufferWidth && present_params.BackBufferHeight)
		{
			SetRect(&back_buffer_rect, 0, 0, present_params.BackBufferWidth, present_params.BackBufferHeight);
		}
		else
		{
			// BackBuffer Width and/or Height was zero
			SetRect(&back_buffer_rect, 0, 0, 0, 0);
		}
	}


	// Set main game window
	if (is_windowed)
	{ // Windowed mode

		if (device_window != NULL)
		{
			game_window = &device_window;
			game_window_rect = &device_window_rect;
		}
		else
		{
			game_window = &focus_window;
			game_window_rect = &focus_window_rect;
		}
	}
	else
	{ // Full-screen mode

		if (focus_window != NULL)
		{
			game_window = &focus_window;
			game_window_rect = &focus_window_rect;
		}
		else
		{
			game_window = &device_window;
			game_window_rect = &device_window_rect;
		}
	}


	Sleep(100);


	update_overlay_parameters();
	create_overlay_state_block();
	
	overlay_needs_reset = true;

	_SP_D3D9_LOG_EVENT_("Exiting %s (thread %d)", __FUNCTION__, GetCurrentThreadId());
	return hres;
}

HRESULT myIDirect3DDevice9::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
	// Draw overlay before presenting frame
	draw_overlay(m_pIDirect3DDevice9, NULL);

	present_calls++; // Increment Present() call counter for the current second
	overlay_rendered_this_frame = false;

	// Call original routine
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion));
}

HRESULT myIDirect3DDevice9::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer)
{
	get_back_buffer_calls++;

	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer));
}

HRESULT myIDirect3DDevice9::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetRasterStatus(iSwapChain, pRasterStatus));
}

HRESULT myIDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetDialogBoxMode(bEnableDialogs));
}

void    myIDirect3DDevice9::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp)
{
	return(m_pIDirect3DDevice9->SetGammaRamp(iSwapChain, Flags, pRamp));
}

void    myIDirect3DDevice9::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp)
{
	return(m_pIDirect3DDevice9->GetGammaRamp(iSwapChain, pRamp));
}

HRESULT myIDirect3DDevice9::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle));
}

HRESULT myIDirect3DDevice9::CreateDepthStencilSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle));
}

HRESULT myIDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint));
}

HRESULT myIDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->UpdateTexture(pSourceTexture, pDestinationTexture));
}

HRESULT myIDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetRenderTargetData(pRenderTarget, pDestSurface));
}

HRESULT myIDirect3DDevice9::GetFrontBufferData(UINT iSwapChain, IDirect3DSurface9* pDestSurface)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetFrontBufferData(iSwapChain, pDestSurface));
}

HRESULT myIDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter));
}

HRESULT myIDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->ColorFill(pSurface, pRect, color));
}

HRESULT myIDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle));
}

HRESULT myIDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetRenderTarget(RenderTargetIndex, pRenderTarget));
}

HRESULT myIDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetRenderTarget(RenderTargetIndex, ppRenderTarget));
}

HRESULT myIDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetDepthStencilSurface(pNewZStencil));
}

HRESULT myIDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
	HRESULT hres = m_pIDirect3DDevice9->GetDepthStencilSurface(ppZStencilSurface);

	if (hres != D3D_OK && hres != D3DERR_NOTFOUND)
	{
		_SP_D3D9_CHECK_AND_RETURN_FAILED_(hres);
	}

	return hres;
}

HRESULT myIDirect3DDevice9::BeginScene(void)
{
	in_scene = true;
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->BeginScene());
}

HRESULT myIDirect3DDevice9::EndScene(void)
{
	// Draw overlay before the scene is shown to the user:

	if (text_overlay.enabled)
	{
		// Set overlay rect sizes and positions
		if (overlay_needs_reset)
		{
			update_overlay_parameters();
			if (is_windowed)
			{
				init_text_overlay_rects(game_window_rect);
			}
			else
			{
				D3DDISPLAYMODE display_mode;
				_SP_D3D9_CHECK_FAILED_(GetDisplayMode(0, &display_mode));
				RECT display_mode_rect;
				SetRect(&display_mode_rect, 0, 0, display_mode.Width, display_mode.Height);
				init_text_overlay_rects(&display_mode_rect);
			}
		}

		clean_text_overlay_feed(); // Remove expired text feed messages
	}

	endscene_calls++;  // Increment EndScene call counter for the current second to determine FPS later
	in_scene = false;

	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->EndScene());
}

HRESULT myIDirect3DDevice9::Clear(DWORD Count, CONST D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->Clear(Count, pRects, Flags, Color, Z, Stencil));
}

HRESULT myIDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetTransform(State, pMatrix));
}

HRESULT myIDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State, D3DMATRIX* pMatrix)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetTransform(State, pMatrix));
}

HRESULT myIDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State, CONST D3DMATRIX* pMatrix)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->MultiplyTransform(State, pMatrix));
}

HRESULT myIDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetViewport(pViewport));
}

HRESULT myIDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetViewport(pViewport));
}

HRESULT myIDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetMaterial(pMaterial));
}

HRESULT myIDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetMaterial(pMaterial));
}

HRESULT myIDirect3DDevice9::SetLight(DWORD Index, CONST D3DLIGHT9* pLight)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetLight(Index, pLight));
}

HRESULT myIDirect3DDevice9::GetLight(DWORD Index, D3DLIGHT9* pLight)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetLight(Index, pLight));
}

HRESULT myIDirect3DDevice9::LightEnable(DWORD Index, BOOL Enable)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->LightEnable(Index, Enable));
}

HRESULT myIDirect3DDevice9::GetLightEnable(DWORD Index, BOOL* pEnable)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetLightEnable(Index, pEnable));
}

HRESULT myIDirect3DDevice9::SetClipPlane(DWORD Index, CONST float* pPlane)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetClipPlane(Index, pPlane));
}

HRESULT myIDirect3DDevice9::GetClipPlane(DWORD Index, float* pPlane)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetClipPlane(Index, pPlane));
}

HRESULT myIDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State, DWORD Value)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetRenderState(State, Value));
}

HRESULT myIDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State, DWORD* pValue)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetRenderState(State, pValue));
}

HRESULT myIDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateStateBlock(Type, ppSB));
}

HRESULT myIDirect3DDevice9::BeginStateBlock(void)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->BeginStateBlock());
}

HRESULT myIDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->EndStateBlock(ppSB));
}

HRESULT myIDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetClipStatus(pClipStatus));
}

HRESULT myIDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetClipStatus(pClipStatus));
}

HRESULT myIDirect3DDevice9::GetTexture(DWORD Stage, IDirect3DBaseTexture9** ppTexture)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetTexture(Stage, ppTexture));
}

HRESULT myIDirect3DDevice9::SetTexture(DWORD Stage, IDirect3DBaseTexture9* pTexture)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetTexture(Stage, pTexture));
}

HRESULT myIDirect3DDevice9::GetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD* pValue)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetTextureStageState(Stage, Type, pValue));
}

HRESULT myIDirect3DDevice9::SetTextureStageState(DWORD Stage, D3DTEXTURESTAGESTATETYPE Type, DWORD Value)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetTextureStageState(Stage, Type, Value));
}

HRESULT myIDirect3DDevice9::GetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD* pValue)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetSamplerState(Sampler, Type, pValue));
}

HRESULT myIDirect3DDevice9::SetSamplerState(DWORD Sampler, D3DSAMPLERSTATETYPE Type, DWORD Value)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetSamplerState(Sampler, Type, Value));
}

HRESULT myIDirect3DDevice9::ValidateDevice(DWORD* pNumPasses)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->ValidateDevice(pNumPasses));
}

HRESULT myIDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber, CONST PALETTEENTRY* pEntries)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT myIDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber, PALETTEENTRY* pEntries)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT myIDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetCurrentTexturePalette(PaletteNumber));
}

HRESULT myIDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetCurrentTexturePalette(PaletteNumber));
}

HRESULT myIDirect3DDevice9::SetScissorRect(CONST RECT* pRect)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetScissorRect(pRect));
}

HRESULT myIDirect3DDevice9::GetScissorRect(RECT* pRect)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetScissorRect(pRect));
}

HRESULT myIDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetSoftwareVertexProcessing(bSoftware));
}

BOOL    myIDirect3DDevice9::GetSoftwareVertexProcessing(void)
{
	return(m_pIDirect3DDevice9->GetSoftwareVertexProcessing());
}

HRESULT myIDirect3DDevice9::SetNPatchMode(float nSegments)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetNPatchMode(nSegments));
}

float   myIDirect3DDevice9::GetNPatchMode(void)
{
	return(m_pIDirect3DDevice9->GetNPatchMode());
}

HRESULT myIDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType, UINT StartVertex, UINT PrimitiveCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawPrimitive(PrimitiveType, StartVertex, PrimitiveCount));
}

HRESULT myIDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType, INT BaseVertexIndex, UINT MinVertexIndex, UINT NumVertices, UINT startIndex, UINT primCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawIndexedPrimitive(PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount));
}

HRESULT myIDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT PrimitiveCount, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawPrimitiveUP(PrimitiveType, PrimitiveCount, pVertexStreamZeroData, VertexStreamZeroStride));
}

HRESULT myIDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType, UINT MinVertexIndex, UINT NumVertices, UINT PrimitiveCount, CONST void* pIndexData, D3DFORMAT IndexDataFormat, CONST void* pVertexStreamZeroData, UINT VertexStreamZeroStride)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType, MinVertexIndex, NumVertices, PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData, VertexStreamZeroStride));
}

HRESULT myIDirect3DDevice9::ProcessVertices(UINT SrcStartIndex, UINT DestIndex, UINT VertexCount, IDirect3DVertexBuffer9* pDestBuffer, IDirect3DVertexDeclaration9* pVertexDecl, DWORD Flags)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->ProcessVertices(SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags));
}

HRESULT myIDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateVertexDeclaration(pVertexElements, ppDecl));
}

HRESULT myIDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetVertexDeclaration(pDecl));
}

HRESULT myIDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetVertexDeclaration(ppDecl));
}

HRESULT myIDirect3DDevice9::SetFVF(DWORD FVF)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetFVF(FVF));
}

HRESULT myIDirect3DDevice9::GetFVF(DWORD* pFVF)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetFVF(pFVF));
}

HRESULT myIDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateVertexShader(pFunction, ppShader));
}

HRESULT myIDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetVertexShader(pShader));
}

HRESULT myIDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetVertexShader(ppShader));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetVertexShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetVertexShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT myIDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetVertexShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT myIDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetVertexShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT myIDirect3DDevice9::SetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride));
}

HRESULT myIDirect3DDevice9::GetStreamSource(UINT StreamNumber, IDirect3DVertexBuffer9** ppStreamData, UINT* OffsetInBytes, UINT* pStride)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetStreamSource(StreamNumber, ppStreamData, OffsetInBytes, pStride));
}

HRESULT myIDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber, UINT Divider)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetStreamSourceFreq(StreamNumber, Divider));
}

HRESULT myIDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber, UINT* Divider)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetStreamSourceFreq(StreamNumber, Divider));
}

HRESULT myIDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetIndices(pIndexData));
}

HRESULT myIDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetIndices(ppIndexData));
}

HRESULT myIDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreatePixelShader(pFunction, ppShader));
}

HRESULT myIDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetPixelShader(pShader));
}

HRESULT myIDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetPixelShader(ppShader));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister, CONST float* pConstantData, UINT Vector4fCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister, float* pConstantData, UINT Vector4fCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetPixelShaderConstantF(StartRegister, pConstantData, Vector4fCount));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister, CONST int* pConstantData, UINT Vector4iCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister, int* pConstantData, UINT Vector4iCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetPixelShaderConstantI(StartRegister, pConstantData, Vector4iCount));
}

HRESULT myIDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister, CONST BOOL* pConstantData, UINT  BoolCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->SetPixelShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT myIDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister, BOOL* pConstantData, UINT BoolCount)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->GetPixelShaderConstantB(StartRegister, pConstantData, BoolCount));
}

HRESULT myIDirect3DDevice9::DrawRectPatch(UINT Handle, CONST float* pNumSegs, CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawRectPatch(Handle, pNumSegs, pRectPatchInfo));
}

HRESULT myIDirect3DDevice9::DrawTriPatch(UINT Handle, CONST float* pNumSegs, CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo));
}

HRESULT myIDirect3DDevice9::DeletePatch(UINT Handle)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->DeletePatch(Handle));
}

HRESULT myIDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery)
{
	_SP_D3D9_CHECK_AND_RETURN_FAILED_(m_pIDirect3DDevice9->CreateQuery(Type, ppQuery));
}



/////////////////////// Overlay-related functions start here ///////////////////////



// Renders the overlay text feed (monochromatic)
void myIDirect3DDevice9::SP_DX9_draw_overlay_text_feed()
{
	if (!overlay_rendered_this_frame)
	{
		build_text_overlay_feed_string(); // Build text feed string

		switch (text_overlay.text_style) {
			case SP_DX9_SHADOWED_TEXT:
				text_overlay.font->DrawText(NULL, text_overlay.feed_full_text.c_str(), -1, &text_overlay.text_shadow_rect[1], text_overlay.text_format, text_overlay.text_shadow_color); // Draw text shadow
				text_overlay.font->DrawText(NULL, text_overlay.feed_full_text.c_str(), -1, &text_overlay.text_shadow_rect[0], text_overlay.text_format, text_overlay.text_color); // Draw text
				break;
			case SP_DX9_PLAIN_TEXT:
				text_overlay.font->DrawText(NULL, text_overlay.feed_full_text.c_str(), -1, &text_overlay.text_plain_rect, text_overlay.text_format, text_overlay.text_color); // Draw text
				break;
			case SP_DX9_OUTLINED_TEXT:
			default:
				// Draw outlined text
				for (int r = 1; r <= 8; r++)
				{
					text_overlay.font->DrawText(NULL, text_overlay.feed_full_text.c_str(), -1, &text_overlay.text_outline_rect[r], text_overlay.text_format, text_overlay.text_outline_color); // Draw text outline
				}
				text_overlay.font->DrawText(NULL, text_overlay.feed_full_text.c_str(), -1, &text_overlay.text_outline_rect[0], text_overlay.text_format, text_overlay.text_color); // Draw text
				break;
		}
		overlay_rendered_this_frame = true;
	}
}



// Renders the overlay text feed (multicolor)
void myIDirect3DDevice9::SP_DX9_draw_overlay_text_feed_multicolor()
{
	if (!overlay_rendered_this_frame)
	{
		cycle_text_colors(); // Calculate the next ARGB color value for text whose color cycles through all colors

		build_text_overlay_feed_string_multicolor(); // Build text feed string

		switch (text_overlay.text_style) {
			case SP_DX9_SHADOWED_TEXT:
				text_overlay.font->DrawText(NULL, text_overlay.feed_full_text.c_str(), -1, &text_overlay.text_shadow_rect[1], text_overlay.text_format, text_overlay.text_shadow_color); // Draw text shadow
				for (int c = 0; c < _SP_DX9_TEXT_COLOR_COUNT_; c++)
				{
					text_overlay.font->DrawText(NULL, text_overlay.feed_text[c].c_str(), -1, &text_overlay.text_shadow_rect[0], text_overlay.text_format, dx9_text_colors[c]); // Draw text
				}
				break;
			case SP_DX9_PLAIN_TEXT:
				for (int c = 0; c < _SP_DX9_TEXT_COLOR_COUNT_; c++)
				{
					text_overlay.font->DrawText(NULL, text_overlay.feed_text[c].c_str(), -1, &text_overlay.text_plain_rect, text_overlay.text_format, dx9_text_colors[c]); // Draw text
				}
				break;
			case SP_DX9_OUTLINED_TEXT:
			default:
				// Draw outlined text
				for (int r = 1; r <= 8 && text_overlay.font != NULL; r++)
				{
					text_overlay.font->DrawText(NULL, text_overlay.feed_full_text.c_str(), -1, &text_overlay.text_outline_rect[r], text_overlay.text_format, text_overlay.text_outline_color); // Draw text outline
				}
				for (int c = 0; c < _SP_DX9_TEXT_COLOR_COUNT_; c++)
				{
					text_overlay.font->DrawText(NULL, text_overlay.feed_text[c].c_str(), -1, &text_overlay.text_outline_rect[0], text_overlay.text_format, dx9_text_colors[c]); // Draw text
				}
				break;
		}
		overlay_rendered_this_frame = true;
	}
}



// Initializes overlay text feed data structure
void myIDirect3DDevice9::SP_DX9_init_text_overlay(IDirect3DDevice9 *device,
	int text_height,
	unsigned int outline_thickness,
	int shadow_x_offset,
	int shadow_y_offset,
	D3DXCOLOR text_color,
	D3DXCOLOR text_outline_color,
	D3DXCOLOR text_shadow_color,
	DWORD text_format,
	int text_style)
{
	text_overlay.enabled = false;
	if (strcpy_s(text_overlay.font_name, _SP_DEFAULT_TEXT_FONT_) != 0)
	{
		// Handle error
	}
	
	if (text_overlay.font != NULL)
	{
		text_overlay.font->Release();
		text_overlay.font = NULL;
	}

	// Initialize overlay font
	HRESULT font_hr = D3DXCreateFont(
		device,					// D3D device
		text_height,			// Height
		0,						// Width
		FW_BOLD,				// Weight
		1,						// MipLevels, 0 = autogen mipmaps
		FALSE,					// Italic
		DEFAULT_CHARSET,		// CharSet
		OUT_DEFAULT_PRECIS,		// OutputPrecision
		ANTIALIASED_QUALITY,	// Quality
		DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
		text_overlay.font_name,	// pFaceName
		&text_overlay.font);	// ppFont
	_SP_D3D9_CHECK_FAILED_(font_hr);

	thread = GetCurrentThreadId(); // Store the creator thread

	// Set text colors
	text_overlay.text_color = text_color;
	text_overlay.text_outline_color = text_outline_color;
	text_overlay.text_shadow_color = text_shadow_color;

	// Set text style and format
	text_overlay.text_format = text_format;
	text_overlay.text_style = text_style;

	// Initialize starting ARGB value for text whose color cycles through all colors
	cycle_all_colors_current_rgb_vals[0] = 0x00FF0000;
	cycle_all_colors_current_rgb_vals[1] = 0x00000000;
	cycle_all_colors_current_rgb_vals[2] = 0x00000000;

	// Enable multicolor text feed by default
	multicolor_overlay_text_feed_enabled = true;

	// Set text shadow offsets and outline thickness
	text_overlay.text_shadow_x_offset = shadow_x_offset;
	text_overlay.text_shadow_y_offset = shadow_y_offset;
	text_overlay.text_outline_thickness = outline_thickness;

	// Initialize the RECT structures that denote the usable screenspace for the overlay text feed
	init_text_overlay_rects();

	// Initialize all colored text feed strings to empty strings
	for (int c = 0; c < _SP_DX9_TEXT_COLOR_COUNT_; c++)
	{
		text_overlay.feed_text[c] = "";
	}
	
	// Set the default-colored text string to the default message
	text_overlay.feed_text[_SP_DEFAULT_TEXT_COLOR_INDEX_] = std::string(_SP_DEFAULT_OVERLAY_TEXT_FEED_TITLE_);
	//text_overlay.feed_full_text = std::string(_SP_DEFAULT_OVERLAY_TEXT_FEED_TITLE_);
	text_overlay.feed_full_text = std::string("");

	create_overlay_state_block();

	initialized = true;
}


// Initializes the RECT structures that denote the usable screenspace for the overlay text feed
void myIDirect3DDevice9::init_text_overlay_rects()
{
	init_text_overlay_rects(game_window_rect);
}

// Initializes the RECT structures that denote the usable screenspace for the overlay text feed
void myIDirect3DDevice9::init_text_overlay_rects(RECT *window_rect)
{
	extern int user_pref_dspw_ol_offset; // Used to adjust the overlay to avoid clipping with the DSPW overlay

	// Initialize plain text rect
	SetRect(&text_overlay.text_plain_rect,
		window_rect->left,
		window_rect->top + user_pref_dspw_ol_offset,
		window_rect->right,
		window_rect->bottom);

	// Inititialize main shadowed text rect
	if (text_overlay.text_shadow_x_offset >= 0 && text_overlay.text_shadow_y_offset >= 0)
	{
		// Case: x and y offsets are both positive
		SetRect(&text_overlay.text_shadow_rect[0],
			window_rect->left,
			window_rect->top + user_pref_dspw_ol_offset,
			window_rect->right - text_overlay.text_shadow_x_offset,
			window_rect->bottom - text_overlay.text_shadow_y_offset);
	}
	else if (text_overlay.text_shadow_x_offset <= 0 && text_overlay.text_shadow_y_offset >= 0)
	{
		// Case: x offset is negative; y offset is positive
		SetRect(&text_overlay.text_shadow_rect[0],
			window_rect->left - text_overlay.text_shadow_x_offset,
			window_rect->top,
			window_rect->right,
			window_rect->bottom - text_overlay.text_shadow_y_offset);
	}
	else if (text_overlay.text_shadow_x_offset >= 0 && text_overlay.text_shadow_y_offset <= 0)
	{
		// Case: x offset is positive; y offset is negative
		SetRect(&text_overlay.text_shadow_rect[0],
			window_rect->left,
			window_rect->top - text_overlay.text_shadow_y_offset,
			window_rect->right - text_overlay.text_shadow_x_offset,
			window_rect->bottom);
	}
	else
	{
		// Case: x and y offsets are both negative
		SetRect(&text_overlay.text_shadow_rect[0],
			window_rect->left - text_overlay.text_shadow_x_offset,
			window_rect->top - text_overlay.text_shadow_y_offset,
			window_rect->right,
			window_rect->bottom);
	}


	// Initialize text shadow rect
	SetRect(&text_overlay.text_shadow_rect[1],
		text_overlay.text_shadow_rect[0].left + text_overlay.text_shadow_x_offset,
		text_overlay.text_shadow_rect[0].top + text_overlay.text_shadow_y_offset,
		text_overlay.text_shadow_rect[0].right + text_overlay.text_shadow_x_offset,
		text_overlay.text_shadow_rect[0].bottom + text_overlay.text_shadow_y_offset);


	// Inititialize main outlined text rect
	SetRect(&text_overlay.text_outline_rect[0],
		window_rect->left + text_overlay.text_outline_thickness,
		window_rect->top + text_overlay.text_outline_thickness + user_pref_dspw_ol_offset,
		window_rect->right - text_overlay.text_outline_thickness,
		window_rect->bottom - text_overlay.text_outline_thickness);

	// Initialize text outline rects:
	//		Top left outline extrusion
	SetRect(&text_overlay.text_outline_rect[1],
		text_overlay.text_outline_rect[0].left - text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].top - text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].right - text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].bottom - text_overlay.text_outline_thickness);
	//		Top right outline extrusion
	SetRect(&text_overlay.text_outline_rect[2],
		text_overlay.text_outline_rect[0].left + text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].top - text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].right + text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].bottom - text_overlay.text_outline_thickness);
	//		Bottom left outline extrusion
	SetRect(&text_overlay.text_outline_rect[3],
		text_overlay.text_outline_rect[0].left - text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].top + text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].right - text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].bottom + text_overlay.text_outline_thickness);
	//		Bottom right outline extrusion
	SetRect(&text_overlay.text_outline_rect[4],
		text_overlay.text_outline_rect[0].left + text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].top + text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].right + text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].bottom + text_overlay.text_outline_thickness);
	//		Left-side outline extrusion
	SetRect(&text_overlay.text_outline_rect[5],
		text_overlay.text_outline_rect[0].left - text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].top,
		text_overlay.text_outline_rect[0].right - text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].bottom);
	//		Upward outline extrusion
	SetRect(&text_overlay.text_outline_rect[6],
		text_overlay.text_outline_rect[0].left,
		text_overlay.text_outline_rect[0].top - text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].right,
		text_overlay.text_outline_rect[0].bottom - text_overlay.text_outline_thickness);
	//		Right-side outline extrusion
	SetRect(&text_overlay.text_outline_rect[7],
		text_overlay.text_outline_rect[0].left + text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].top,
		text_overlay.text_outline_rect[0].right + text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].bottom);
	//		Downward outline extrusion
	SetRect(&text_overlay.text_outline_rect[8],
		text_overlay.text_outline_rect[0].left,
		text_overlay.text_outline_rect[0].top + text_overlay.text_outline_thickness,
		text_overlay.text_outline_rect[0].right,
		text_overlay.text_outline_rect[0].bottom + text_overlay.text_outline_thickness);
}



// Changes the font height of the overlay text feed
void myIDirect3DDevice9::SP_DX9_set_text_height(IDirect3DDevice9 *device, int new_text_height)
{
	// Store the current font attributes
	D3DXFONT_DESC font_desc;
	HRESULT font_desc_hr = text_overlay.font->GetDesc(&font_desc);
	_SP_D3D9_CHECK_FAILED_(font_desc_hr);

	// Release pre-existing font resources
	if (text_overlay.font != NULL)
	{
		text_overlay.font->Release(); // Decrement reference count for ID3DXFont interface
		text_overlay.font = NULL;
	}

	// Check that the new font height is valid
	if (new_text_height > 0 && new_text_height != font_desc.Height)
	{

		// Re-initialize overlay font with identical attributes other than the new font height
		HRESULT font_hr = D3DXCreateFont(
			device,	// D3D device
			new_text_height,		// Height
			font_desc.Width,		// Width
			font_desc.Weight,		// Weight
			font_desc.MipLevels,	// MipLevels; 0 = autogen mipmaps
			font_desc.Italic,		// Italic
			font_desc.CharSet,		// CharSet
			font_desc.OutputPrecision, // OutputPrecision
			font_desc.Quality,		// Quality
			font_desc.PitchAndFamily, // PitchAndFamily
			font_desc.FaceName,		// pFaceName
			&text_overlay.font);	// ppFont

		_SP_D3D9_CHECK_FAILED_(font_hr);
	}

	text_overlay_new_font_size = 0;

	overlay_needs_reset = true;
}



// Adds a message to the text overlay feed; the message expires in a number of
//	milliseconds denoted by the duration parameter.
// NOTE: Overlay text feed currently does NOT support multi-line messages. Print each line as a separate message instead.
void myIDirect3DDevice9::print_to_overlay_feed(const char *message, unsigned long long duration, bool include_timestamp, int text_color)
{
	// Overlay must be temporarily disabled to avoid race conditions because this function can be called from other threads
	bool reenable_overlay;
	if (text_overlay.enabled)
	{
		// Overlay will need to be re-enabled
		reenable_overlay = true;
		text_overlay.enabled = false;
	}
	else
	{
		// Overlay won't need to be re-enabled
		reenable_overlay = false;
	}

	// Get current time (in milliseconds since epoch)
	unsigned long long ms_since_epoch =	std::chrono::system_clock::now().time_since_epoch() /
										std::chrono::milliseconds(1);

	// Create new overlay text feed message data structure
	SP_DX9_TEXT_OVERLAY_FEED_ENTRY new_message;
	// Store the new message in the text feed message structure
	new_message.message.append(message);

	// Calculate the expiration time for the message
	if (duration == 0)
	{
		// Messages with an expire time of 0 never expire
		new_message.expire_time = 0;
	}
	else
	{
		new_message.expire_time = (ms_since_epoch + duration);
	}

	// Set the text color for the message
	if (text_color < _SP_DX9_TEXT_COLOR_COUNT_ && text_color >= 0)
	{
		new_message.text_color = text_color;
	}
	else
	{
		// Invalid color specified, set to default color
		new_message.text_color = 0;
	}

	// Enable or disable timestamp for the message
	new_message.show_timestamp = include_timestamp;

	// Build timestamp string
	if (get_current_timestamp_string(new_message.timestamp, true))
	{
		// Handle error
	}

	// Add a space to the end of the timestamp string
	new_message.timestamp[10] = ' ';
	new_message.timestamp[11] = '\0';

	// Add the constructed message to the overlay text feed message queue
	text_overlay.feed.push_back(new_message);

	// Re-enable the overlay, if it was enabled
	if (reenable_overlay)
	{
		text_overlay.enabled = true;
	}
}



// Adds a message to the text overlay feed; the message expires in a number of
//	milliseconds denoted by the duration parameter. (Monochromatic)
// NOTE: Overlay text feed currently does NOT support multi-line messages. Print each line as a separate message instead.
void myIDirect3DDevice9::print_to_overlay_feed(const char *message, unsigned long long duration, bool include_timestamp)
{
	// Call overloaded function with default text color specified
	print_to_overlay_feed(message, duration, include_timestamp, 0);
}



// Removes expired messages from the overlay text feed
void myIDirect3DDevice9::clean_text_overlay_feed()
{
	// Get current time (in milliseconds since epoch)
	unsigned long long ms_since_epoch = std::chrono::system_clock::now().time_since_epoch() /
										std::chrono::milliseconds(1);

	// Iterate through overlay text feed message list
	std::list<SP_DX9_TEXT_OVERLAY_FEED_ENTRY>::const_iterator iterator = text_overlay.feed.begin();
	while (iterator != text_overlay.feed.end())
	{
		if ((*iterator).expire_time != 0 && ms_since_epoch >= (*iterator).expire_time)
		{
			// Remove expired message
			iterator = text_overlay.feed.erase(iterator);
		}
		else
		{
			iterator++;
		}
	}
}



// Constructs the overlay text feed from the current list of messages (monochromatic)
void myIDirect3DDevice9::build_text_overlay_feed_string()
{
	// Erase text feed string from last-rendered frame
	text_overlay.feed_full_text.clear();

	if (show_text_feed_info_bar)
	{
		update_overlay_text_feed_info_string();
		text_overlay.feed_full_text.append(text_feed_info_string);
		text_overlay.feed_full_text.append("\n");
	}

	// Iterate through overlay text feed message list
	std::list<SP_DX9_TEXT_OVERLAY_FEED_ENTRY>::const_iterator iterator;
	for (iterator = text_overlay.feed.begin(); iterator != text_overlay.feed.end(); iterator++)
	{
		if (iterator != text_overlay.feed.begin())
		{
			// Each message starts on a new line
			text_overlay.feed_full_text.append("\n");
		}
		if ((*iterator).show_timestamp)
		{
			// Prepend timestamp to message
			text_overlay.feed_full_text.append(iterator->timestamp);
		}
		// Append message text
		text_overlay.feed_full_text.append((*iterator).message);
	}
}



// Constructs the overlay text feed from the current list of messages (multicolor)
void myIDirect3DDevice9::build_text_overlay_feed_string_multicolor()
{
	// Iterate through overlay text feed message list for each color
	std::list<SP_DX9_TEXT_OVERLAY_FEED_ENTRY>::const_iterator iterator;
	for (iterator = text_overlay.feed.begin(); iterator != text_overlay.feed.end(); iterator++)
	{
		if (iterator == text_overlay.feed.begin())
		{
			text_overlay.feed_full_text.clear(); // Erase text feed shadow/outline strings from last-rendered frame
			if (show_text_feed_info_bar)
			{
				text_overlay.feed_text[0].clear();
				update_overlay_text_feed_info_string();
				text_overlay.feed_text[0].append(text_feed_info_string).append("\n");
				text_overlay.feed_full_text.append(text_feed_info_string).append("\n");
				for (int c = 1; c < _SP_DX9_TEXT_COLOR_COUNT_; c++)
				{
					text_overlay.feed_text[c].clear(); // Erase text feed strings from last-rendered frame
					text_overlay.feed_text[c].append(" \r\n");
				}
			}
			else
			{
				for (int c = 0; c < _SP_DX9_TEXT_COLOR_COUNT_; c++)
				{
					text_overlay.feed_text[c].clear(); // Erase text feed strings from last-rendered frame
				}
			}
		}

		for (int c = 0; c < _SP_DX9_TEXT_COLOR_COUNT_; c++)
		{
			if (c == (*iterator).text_color)
			{
				// Construct message for the specified color
				if ((*iterator).show_timestamp)
				{
					// Prepend timestamp to message
					text_overlay.feed_text[c].append(iterator->timestamp);
					text_overlay.feed_full_text.append(iterator->timestamp);
				}
				// Append message text for the specified color
				text_overlay.feed_text[c].append((*iterator).message).append("\r\n");
				text_overlay.feed_full_text.append((*iterator).message).append("\r\n");
			}
			else
			{
				// Create a blank message for all other colors at this line
				text_overlay.feed_text[c].append(" \r\n");
			}
		}
	}
}



// Updates the various overlay text feed info line attributes
void myIDirect3DDevice9::update_overlay_text_feed_info_string()
{
	text_feed_info_string.clear();

	if ((show_text_feed_info_bar & SP_DX9_INFO_BAR_DATE)
		|| (show_text_feed_info_bar & SP_DX9_INFO_BAR_TIME))
	{
		text_feed_info_string.append("[");
	}
	
	if (show_text_feed_info_bar & SP_DX9_INFO_BAR_DATE)
	{
		// Insert current date to text feed info string
		append_current_date_string(&text_feed_info_string, false, SP_DATE_MMDDYYYY);
		if (show_text_feed_info_bar & SP_DX9_INFO_BAR_TIME)
		{
			text_feed_info_string.append("  ");
		}
		else
		{
			text_feed_info_string.append("]  ");
		}
	}

	if (show_text_feed_info_bar & SP_DX9_INFO_BAR_TIME)
	{
		// Append current timestamp to text feed info string
		append_current_timestamp_string(&text_feed_info_string, false);
		text_feed_info_string.append("]  ");
	}

	if (show_text_feed_info_bar & SP_DX9_INFO_BAR_FPS)
	{
		// Insert FPS counter into text feed info string
		text_feed_info_string.append("[");

		int current_fps = fps; // Get FPS

		if (current_fps < 999)
		{
			text_feed_info_string.append(std::to_string(current_fps));
		}
		else
		{
			text_feed_info_string.append("999");
		}

		text_feed_info_string.append(" FPS]  ");
	}

	if (show_text_feed_info_bar & SP_DX9_INFO_BAR_TITLE)
	{
		// Insert title into text feed info string
		text_feed_info_string.append(_SP_DEFAULT_OVERLAY_TEXT_FEED_TITLE_);
	}
}


// (Called once per second) Records the number of frames that were rendered in the last second.
void CALLBACK update_fps(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	extern myIDirect3DDevice9 *gl_pmyIDirect3DDevice9;

	if (gl_pmyIDirect3DDevice9->present_calls >= gl_pmyIDirect3DDevice9->swap_chain_present_calls)
	{
		// Program uses IDirect3DDevice9::Present() to render frames
		gl_pmyIDirect3DDevice9->fps = gl_pmyIDirect3DDevice9->present_calls; // Store the number of frames that were rendered in the last second
	}
	else
	{
		// Program uses IDirect3DSwapChain9::Present() to render frames
		gl_pmyIDirect3DDevice9->fps = gl_pmyIDirect3DDevice9->swap_chain_present_calls; // Store the number of frames that were rendered in the last second
	}

	// Reset call counters
	gl_pmyIDirect3DDevice9->present_calls = 0;
	gl_pmyIDirect3DDevice9->swap_chain_present_calls = 0;
	gl_pmyIDirect3DDevice9->endscene_calls = 0;
	gl_pmyIDirect3DDevice9->get_back_buffer_calls = 0;

	// Restart timer
	if (!(gl_pmyIDirect3DDevice9->fps_timer_id = SetTimer(NULL, idEvent, 1000, &update_fps)))
	{
		// Handle error
	}
}

// Prints various game window data to the overlay text feed
void myIDirect3DDevice9::print_debug_data(unsigned long long duration, bool show_timestamp)
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


// Calculates the next ARGB color value for text whose color cycles through all colors
void myIDirect3DDevice9::cycle_text_colors()
{
	if (cycle_all_colors_current_rgb_vals[0] == 0x00FF0000 && cycle_all_colors_current_rgb_vals[1] != 0x0000FF00 && cycle_all_colors_current_rgb_vals[2] == 0x00000000)
	{
		cycle_all_colors_current_rgb_vals[1] += 0x00000100;
		if (cycle_all_colors_current_rgb_vals[1] == 0x0000FF00)
		{
			cycle_all_colors_current_rgb_vals[0] = 0x00FE0000;
		}
	}
	else if (cycle_all_colors_current_rgb_vals[0] != 0x00FF0000 && cycle_all_colors_current_rgb_vals[1] == 0x0000FF00 && cycle_all_colors_current_rgb_vals[2] == 0x00000000)
	{
		cycle_all_colors_current_rgb_vals[0] -= 0x00010000;
		if (cycle_all_colors_current_rgb_vals[0] == 0x00000000)
		{
			cycle_all_colors_current_rgb_vals[2] = 0x00000001;
		}
	}
	else if (cycle_all_colors_current_rgb_vals[0] == 0x00000000 && cycle_all_colors_current_rgb_vals[1] == 0x0000FF00 && cycle_all_colors_current_rgb_vals[2] != 0x000000FF)
	{
		cycle_all_colors_current_rgb_vals[2]++;
		if (cycle_all_colors_current_rgb_vals[2] == 0x000000FF)
		{
			cycle_all_colors_current_rgb_vals[1] = 0x0000FE00;
		}
	}
	else if (cycle_all_colors_current_rgb_vals[0] == 0x00000000 && cycle_all_colors_current_rgb_vals[1] != 0x0000FF00 && cycle_all_colors_current_rgb_vals[2] == 0x000000FF)
	{
		cycle_all_colors_current_rgb_vals[1] -= 0x00000100;
		if (cycle_all_colors_current_rgb_vals[1] == 0x00000000)
		{
			cycle_all_colors_current_rgb_vals[0] = 0x00010000;
		}
	}
	else if (cycle_all_colors_current_rgb_vals[0] != 0x00000000 && cycle_all_colors_current_rgb_vals[1] == 0x00000000 && cycle_all_colors_current_rgb_vals[2] == 0x000000FF)
	{
		cycle_all_colors_current_rgb_vals[0] += 0x00010000;
		if (cycle_all_colors_current_rgb_vals[0] == 0x00FF0000)
		{
			cycle_all_colors_current_rgb_vals[2] = 0x000000FE;
		}
	}
	else if (cycle_all_colors_current_rgb_vals[0] == 0x00FF0000 && cycle_all_colors_current_rgb_vals[1] == 0x00000000 && cycle_all_colors_current_rgb_vals[2] != 0x00000000)
	{
		cycle_all_colors_current_rgb_vals[2]--;
	}
	dx9_text_colors[SP_DX9_TEXT_COLOR_CYCLE_ALL] = D3DXCOLOR(0xFF000000 + cycle_all_colors_current_rgb_vals[0] + cycle_all_colors_current_rgb_vals[1] + cycle_all_colors_current_rgb_vals[2]);
}


// Constructs a string describing the specified RECT struct and stores it in the given std::string
void rect_to_string(RECT *rect, const char *rect_name, std::string *str)
{
	str->clear();
	str->append(rect_name).append(std::string(" (")).append(std::to_string(rect->left)).append(",").append(std::to_string(rect->top)).append(")  ").append(std::to_string(rect->right - rect->left)).append("x").append(std::to_string(rect->bottom - rect->top));
}

// Constructs a RECT struct from the device viewport
RECT *myIDirect3DDevice9::get_viewport_as_rect(RECT *rect)
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
RECT *myIDirect3DDevice9::get_viewport_as_rect(RECT *rect, D3DVIEWPORT9 *viewport)
{
	HRESULT hres = GetViewport(viewport);

	if (hres == D3DERR_INVALIDCALL)
	{
		// Handle error
	}

	SetRect(rect, viewport->X, viewport->Y, viewport->X + viewport->Width, viewport->Y + viewport->Height);

	return rect;
}

void myIDirect3DDevice9::update_overlay_parameters()
{
	// Store the device window attributes
	if (device_window != NULL)
	{
		if (GetClientRect(device_window, &device_window_rect))
		{
			// Handle error
		}

		// Store the back buffer attributes
		if (!back_buffer_rect.right || !back_buffer_rect.bottom)
		{
			// BackBuffer Width or Height was zero; use device window attributes
			SetRect(&back_buffer_rect, device_window_rect.left, device_window_rect.top, device_window_rect.right, device_window_rect.bottom);
		}
	}
	else
	{
		SetRect(&device_window_rect, 0, 0, 0, 0);

		// Store the back buffer attributes
		if (!back_buffer_rect.right || !back_buffer_rect.bottom)
		{
			// BackBuffer Width or Height was zero; use device window attributes
			if (GetClientRect(focus_window, &back_buffer_rect))
			{
				// Handle error
			}
		}
	}

	overlay_needs_reset = false;
}


// Creates a suitable state block for drawing the overlay.
//	This method was created with the help of the Mumble source code, found here:
//		https://github.com/mumble-voip/mumble/blob/master/overlay/d3d9.cpp
void myIDirect3DDevice9::create_overlay_state_block()
{
	HRESULT hres;

	// Release the previously-created overlay state block, if it exists
	if (overlay_state_block != NULL)
	{
		overlay_state_block->Release();
		overlay_state_block = NULL;
	}


	// Capture current state block
	IDirect3DStateBlock9 *current_state_block = NULL;
	_SP_D3D9_CHECK_FAILED_(CreateStateBlock(D3DSBT_ALL, &current_state_block));
	_SP_D3D9_CHECK_FAILED_(current_state_block->Capture());

	// Create and configure new state block for drawing the overlay
	hres = CreateStateBlock(D3DSBT_ALL, &overlay_state_block);
	_SP_D3D9_CHECK_FAILED_(hres);
	if (FAILED(hres))
	{
		// Handle error
		current_state_block->Release();
		current_state_block = NULL;
	}

	SetVertexShader(NULL);
	SetPixelShader(NULL);
	SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

	SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); // 0x16
	SetRenderState(D3DRS_WRAP0, FALSE); // 0x80

	SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	SetRenderState(D3DRS_ZENABLE, FALSE);
	SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	SetRenderState(D3DRS_COLORVERTEX, FALSE);

	SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	SetRenderState(D3DRS_LIGHTING, FALSE);

	// Store overlay state block
	_SP_D3D9_CHECK_FAILED_(overlay_state_block->Capture());

	// Restore current state block and release temporary resources
	_SP_D3D9_CHECK_FAILED_(current_state_block->Apply());
	current_state_block->Release();
}


// Draws the overlay.
//	This method was created with the help of the Mumble source code, found here:
//		https://github.com/mumble-voip/mumble/blob/master/overlay/d3d9.cpp
void myIDirect3DDevice9::draw_overlay(IDirect3DDevice9 *device, IDirect3DSwapChain9 *swap_chain)
{
	if (!text_overlay.enabled)
	{
		return;
	}

	if (text_overlay_new_font_size)
	{
		SP_DX9_set_text_height(device, text_overlay_new_font_size);
	}

	HRESULT hres = NULL;
	
	// Get back buffer
	IDirect3DSurface9 *back_buffer = NULL;
	if (swap_chain != NULL)
	{
		// Present() was called from swap chain
		_SP_D3D9_CHECK_FAILED_(swap_chain->GetBackBuffer(0, D3DBACKBUFFER_TYPE_MONO, &back_buffer));
	}
	else
	{
		// Present() was called from device
		_SP_D3D9_CHECK_FAILED_(device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &back_buffer));
	}


	// Get current render target
	IDirect3DSurface9 *render_target = NULL;
	hres = device->GetRenderTarget(0, &render_target);
	_SP_D3D9_CHECK_FAILED_(hres);

	
	// Capture current state block
	IDirect3DStateBlock9 *current_state_block = NULL;
	_SP_D3D9_CHECK_FAILED_(device->CreateStateBlock(D3DSBT_ALL, &current_state_block));
	_SP_D3D9_CHECK_FAILED_(current_state_block->Capture());


	// Apply state block for writing the overlay
	_SP_D3D9_CHECK_FAILED_(overlay_state_block->Apply());


	// Set render target to back buffer (if it isn't already)
	if (back_buffer != render_target)
	{
		_SP_D3D9_CHECK_FAILED_(device->SetRenderTarget(0, back_buffer));
	}

	//print_debug_data(10, false);

	D3DVIEWPORT9 viewport;
	_SP_D3D9_CHECK_FAILED_(device->GetViewport(&viewport));


	// Adjust viewport to be the correct size
	if (viewport.Width != (DWORD)game_window_rect->right || viewport.Height != (DWORD)game_window_rect->bottom)
	{
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = (DWORD)game_window_rect->right;
		viewport.Height = (DWORD)game_window_rect->bottom;
		viewport.MinZ = 0.0f;
		viewport.MaxZ = 1.0f;
	}

	
	_SP_D3D9_CHECK_FAILED_(device->BeginScene()); // Begin drawing the overlay
	
	//	Draw text feed
	if (multicolor_overlay_text_feed_enabled)
	{
		SP_DX9_draw_overlay_text_feed_multicolor();
	}
	else
	{
		SP_DX9_draw_overlay_text_feed();
	}

	_SP_D3D9_CHECK_FAILED_(device->EndScene()); // Finished drawing the overlay
	

	// Restore current state block
	_SP_D3D9_CHECK_FAILED_(current_state_block->Apply());

	// Release temporary resources
	current_state_block->Release();
	render_target->Release();
	back_buffer->Release();
}