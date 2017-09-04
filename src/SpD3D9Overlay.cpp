// Author: Sean Pesce

#include "stdafx.h"
#include "SpD3D9Overlay.h"


// Initialize static member data
bool SpD3D9Overlay::run_plugin_funcs = true;
std::list<SP_D3D9_PLUGIN> SpD3D9Overlay::loaded_libraries;


SpD3D9Overlay::SpD3D9Overlay(SpD3D9Interface *new_interface, SpD3D9Device *new_device, HWND new_focus_window, D3DPRESENT_PARAMETERS *present_params)
{
	// Store pointer to device/interface wrappers
	d3d_interface = new_interface;
	device = new_device;

	is_windowed = present_params->Windowed != 0;

	// Store the focus window attributes
	focus_window = new_focus_window;
	if (focus_window != NULL)
	{
		//if (GetClientRect(focus_window, &focus_window_rect))
		//{
			// Handle error
		//}
	}
	else
	{
		//SetRect(&focus_window_rect, 0, 0, 0, 0);
	}

	// Store the device window attributes
	device_window = present_params->hDeviceWindow;
	if (device_window != NULL)
	{
		//if (GetClientRect(device_window, &device_window_rect))
		//{
			// Handle error
		//}

		// Store the back buffer attributes
		if (present_params->BackBufferWidth && present_params->BackBufferHeight)
		{
			SetRect(&back_buffer, 0, 0, present_params->BackBufferWidth, present_params->BackBufferHeight);
		}
		else
		{
			// BackBuffer Width or Height was zero; use device window attributes
			if (GetClientRect(device_window, &back_buffer))
			{
				// Handle error
			}
		}
	}
	else
	{
		//SetRect(&device_window_rect, 0, 0, 0, 0);

		// Store the back buffer attributes
		if (present_params->BackBufferWidth && present_params->BackBufferHeight)
		{
			SetRect(&back_buffer, 0, 0, present_params->BackBufferWidth, present_params->BackBufferHeight);
		}
		else
		{
			// BackBuffer Width or Height was zero; use focus window attributes
			if (GetClientRect(focus_window, &back_buffer))
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
			//game_window_rect = &device_window_rect;
		}
		else
		{
			game_window = &focus_window;
			//game_window_rect = &focus_window_rect;
		}
	}
	else
	{ // Full-screen mode

		if (focus_window != NULL)
		{
			game_window = &focus_window;
			//game_window_rect = &focus_window_rect;
		}
		else
		{
			game_window = &device_window;
			//game_window_rect = &device_window_rect;
		}
	}

	// Initialize overlay components
	text_feed = new SpD3D9OTextFeed(this);
	console = new SpD3D9OConsole(this);

	// Create state block for drawing the overlay
	create_state_block();

	// Start the FPS count timer
	fps_timer_id = 0;
	fps_timer_id = (unsigned int)SetTimer(NULL, 0, 1000, &update_fps_count);
	if (!fps_timer_id)
	{
		// Handle error
	}
}



SpD3D9Overlay::~SpD3D9Overlay()
{
	delete text_feed;
	text_feed = NULL;

	delete console;
	console = NULL;
}



// Creates a suitable state block for drawing the overlay
void SpD3D9Overlay::create_state_block()
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
	_SP_D3D9_CHECK_FAILED_(device->CreateStateBlock(D3DSBT_ALL, &current_state_block));
	_SP_D3D9_CHECK_FAILED_(current_state_block->Capture());

	// Create and configure new state block for drawing the overlay
	hres = device->CreateStateBlock(D3DSBT_ALL, &overlay_state_block);
	_SP_D3D9_CHECK_FAILED_(hres);
	if (FAILED(hres))
	{
		// Handle error
		current_state_block->Release();
		current_state_block = NULL;
	}

	device->SetVertexShader(NULL);
	device->SetPixelShader(NULL);
	device->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);

	device->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
	device->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
	device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE); // 0x16
	device->SetRenderState(D3DRS_WRAP0, FALSE); // 0x80

	device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	device->SetRenderState(D3DRS_ALPHATESTENABLE, TRUE);
	device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATER);

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	device->SetRenderState(D3DRS_ZFUNC, D3DCMP_ALWAYS);
	device->SetRenderState(D3DRS_COLORVERTEX, FALSE);

	device->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	device->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);

	device->SetRenderState(D3DRS_LIGHTING, FALSE);

	// Store overlay state block
	_SP_D3D9_CHECK_FAILED_(overlay_state_block->Capture());

	// Restore current state block and release temporary resources
	_SP_D3D9_CHECK_FAILED_(current_state_block->Apply());
	current_state_block->Release();
}



void SpD3D9Overlay::update_back_buffer_parameters()
{
	// Store the device window attributes
	RECT device_window_rect;
	if (device_window != NULL)
	{
		// Store the back buffer attributes
		if (!back_buffer.right || !back_buffer.bottom)
		{
			// BackBuffer Width or Height was zero; use device window attributes
			if (GetClientRect(device_window, &device_window_rect))
			{
				// Handle error
			}

			SetRect(&back_buffer, device_window_rect.left, device_window_rect.top, device_window_rect.right, device_window_rect.bottom);
		}
	}
	else
	{
		SetRect(&device_window_rect, 0, 0, 0, 0);

		// Store the back buffer attributes
		if (!back_buffer.right || !back_buffer.bottom)
		{
			// BackBuffer Width or Height was zero; use device window attributes
			if (GetClientRect(focus_window, &back_buffer))
			{
				// Handle error
			}
		}
	}

	needs_update = false;
}



void SpD3D9Overlay::release_tasks()
{
	KillTimer(NULL, fps_timer_id); // Disable the FPS count timer

	// Release resources
	if (overlay_state_block != NULL)
	{
		overlay_state_block->Release();
	}
	overlay_state_block = NULL;


	/*if (text_feed->font != NULL)
	{
		//text_feed->font->Release();
		text_feed->font->DeleteDeviceObjects();
	}*/
	text_feed->font = NULL;

	/*if (console->font != NULL)
	{
		console->font->DeleteDeviceObjects();
	}
	if (console->cursor != NULL)
	{
		console->cursor->DeleteDeviceObjects();
	}*/
	console->font = NULL;
	console->cursor = NULL;
}



void SpD3D9Overlay::force_release_tasks()
{
	KillTimer(NULL, fps_timer_id); // Disable the FPS count timer

	// Release resources
	if (overlay_state_block != NULL)
	{
		overlay_state_block->Release();
	}
	overlay_state_block = NULL;


	// Release overlay resources to avoid memory leaks
	/*if (text_feed->font != NULL)
	{
		//text_feed->font->Release();
		text_feed->font->DeleteDeviceObjects();
	}*/
	text_feed->font = NULL;

	/*if (console->font != NULL)
	{
		console->font->DeleteDeviceObjects();
	}
	if (console->cursor != NULL)
	{
		console->cursor->DeleteDeviceObjects();
	}*/
	console->font = NULL;
	console->cursor = NULL;
}



void SpD3D9Overlay::reset_tasks(D3DPRESENT_PARAMETERS *present_params, bool console_is_open)
{
	// Release the previously-created overlay state block (if it exists)
	if (overlay_state_block != NULL)
	{
		overlay_state_block->Release();
		overlay_state_block = NULL;
	}


	// Free video memory resources used by text feed
	if (text_feed->font != NULL)
	{
		#ifdef _SP_D3D9O_TF_USE_ID3DX_FONT_
			_SP_D3D9_CHECK_FAILED_(text_feed->font->OnLostDevice()); // (Must be called before resetting device)
		#else
			delete text_feed->font;
			text_feed->font = NULL;
		#endif // _SP_D3D9O_TF_USE_ID3DX_FONT_
	}

	// Temporarily close console while resetting device
	if (console_is_open && console->is_open())
		console->toggle();
	
	// Remove any console text selection
	console->clear_selection();

	// Free video memory resources used by console
	if (console->font != NULL)
	{
		delete console->font;
		console->font = NULL;
	}
	if (console->cursor != NULL)
	{
		delete console->cursor;
		console->cursor = NULL;
	}
	if (console->win_cursor_sprite != NULL)
	{
		console->win_cursor_sprite->Release();
		console->win_cursor_sprite = NULL;
	}
	if (console->win_cursor_tex != NULL)
	{
		console->win_cursor_tex->Release();
		console->win_cursor_tex = NULL;
	}

}



void SpD3D9Overlay::post_reset_tasks(D3DPRESENT_PARAMETERS *present_params, bool console_is_open)
{
	// Re-acquire video memory resources for overlay text feed font
	#ifdef _SP_D3D9O_TF_USE_ID3DX_FONT_
		if (text_feed->font != NULL)
		{
			_SP_D3D9_CHECK_FAILED_(text_feed->font->OnResetDevice()); // (Must be called after resetting a device)
		}
	#else
		text_feed->font = new CD3DFont(_SP_D3D9O_TF_DEFAULT_FONT_FAMILY_, text_feed->font_height, D3DFONT_BOLD);
		text_feed->font->InitializeDeviceObjects(device->m_pIDirect3DDevice9);
		text_feed->font->RestoreDeviceObjects();
	#endif // _SP_D3D9O_TF_USE_ID3DX_FONT_

	// Re-acquire video memory resources for overlay console
	console->font = new CD3DFont(console->font_family.c_str(), console->font_height, 0);
	console->font->InitializeDeviceObjects(device->m_pIDirect3DDevice9);
	console->font->RestoreDeviceObjects();
	console->cursor = new CD3DFont(console->cursor_font_family.c_str(), console->cursor_size, 0);
	console->cursor->InitializeDeviceObjects(device->m_pIDirect3DDevice9);
	console->cursor->RestoreDeviceObjects();
	console->init_win_cursor();
	

	// Store window mode (windowed or fullscreen)
	is_windowed = present_params->Windowed != 0;

	// Set device window parameters
	device_window = present_params->hDeviceWindow;
	if (device_window != NULL)
	{
		// Store the back buffer attributes
		if (present_params->BackBufferWidth && present_params->BackBufferHeight)
		{
			SetRect(&back_buffer, 0, 0, present_params->BackBufferWidth, present_params->BackBufferHeight);
		}
		else
		{
			// BackBuffer Width and/or Height was zero
			SetRect(&back_buffer, 0, 0, 0, 0);
		}
	}
	else
	{
		// Store the back buffer attributes
		if (present_params->BackBufferWidth && present_params->BackBufferHeight)
		{
			SetRect(&back_buffer, 0, 0, present_params->BackBufferWidth, present_params->BackBufferHeight);
		}
		else
		{
			// BackBuffer Width and/or Height was zero
			SetRect(&back_buffer, 0, 0, 0, 0);
		}
	}


	// Set main game window
	if (is_windowed)
	{ // Windowed mode

		if (device_window != NULL)
		{
			game_window = &device_window;
		}
		else
		{
			game_window = &focus_window;
		}
	}
	else
	{ // Full-screen mode

		if (focus_window != NULL)
		{
			game_window = &focus_window;
		}
		else
		{
			game_window = &device_window;
		}
	}

	Sleep(100);

	update_back_buffer_parameters();
	create_state_block();

	// Re-open console if it was open before resetting the device
	if (console_is_open && !console->is_open())
		console->toggle();

	needs_update = true;
}



void SpD3D9Overlay::end_scene_tasks()
{
	if (enabled_elements & SP_D3D9O_TEXT_FEED_ENABLED)
	{
		// Set overlay rect sizes and positions
		if (needs_update)
		{
			update_back_buffer_parameters();
			if (is_windowed)
			{
				RECT game_window_rect;
				if (GetClientRect((*game_window), &game_window_rect))
				{
					// Handle error
				}
				text_feed->set_bounds(&game_window_rect);
			}
			else
			{
				D3DDISPLAYMODE display_mode;
				_SP_D3D9_CHECK_FAILED_(device->GetDisplayMode(0, &display_mode));
				RECT display_mode_rect;
				SetRect(&display_mode_rect, 0, 0, display_mode.Width, display_mode.Height);
				text_feed->set_bounds(&display_mode_rect);
			}
		}

		text_feed->clean_feed(); // Remove expired text feed messages
	}
}



// (Called once per second) Records the number of frames that were rendered in the last second.
void CALLBACK update_fps_count(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	extern SpD3D9Device *gl_pSpD3D9Device;

	if (gl_pSpD3D9Device->present_calls >= gl_pSpD3D9Device->swap_chain_present_calls)
	{
		// Program uses IDirect3DDevice9::Present() to render frames
		gl_pSpD3D9Device->overlay->fps_count = gl_pSpD3D9Device->present_calls; // Store the number of frames that were rendered in the last second
	}
	else
	{
		// Program uses IDirect3DSwapChain9::Present() to render frames
		gl_pSpD3D9Device->overlay->fps_count = gl_pSpD3D9Device->swap_chain_present_calls; // Store the number of frames that were rendered in the last second
	}

	// Reset call counters
	gl_pSpD3D9Device->present_calls = 0;
	gl_pSpD3D9Device->swap_chain_present_calls = 0;
	gl_pSpD3D9Device->endscene_calls = 0;
	gl_pSpD3D9Device->get_back_buffer_calls = 0;

	// Restart timer
	if (!(gl_pSpD3D9Device->overlay->fps_timer_id = (unsigned int)SetTimer(NULL, idEvent, 1000, &update_fps_count)))
	{
		// Handle error
	}
}



void SpD3D9Overlay::draw(IDirect3DSwapChain9 *swap_chain)
{

	if (!enabled_elements)
	{
		return;
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

	D3DVIEWPORT9 viewport;
	_SP_D3D9_CHECK_FAILED_(device->GetViewport(&viewport));


	// Adjust viewport to be the correct size
	RECT game_window_rect;
	if (GetClientRect((*game_window), &game_window_rect))
	{
		// Handle error
	}

	if (viewport.Width != (DWORD)game_window_rect.right || viewport.Height != (DWORD)game_window_rect.bottom)
	{
		viewport.X = 0;
		viewport.Y = 0;
		viewport.Width = (DWORD)game_window_rect.right;
		viewport.Height = (DWORD)game_window_rect.bottom;
		viewport.MinZ = 0.0f;
		viewport.MaxZ = 1.0f;
	}


	_SP_D3D9_CHECK_FAILED_(device->BeginScene()); // Begin drawing the overlay


	// Add overlay elements from plugins
	std::string info_bar_plugin_element;
	if (SpD3D9Overlay::run_plugin_funcs)
	{
		for (auto plugin : SpD3D9Overlay::loaded_libraries)
		{
			if (plugin.draw_overlay_func != NULL)
			{
				info_bar_plugin_element.clear();
				plugin.draw_overlay_func(&info_bar_plugin_element);
				text_feed->info_bar_plugin_elements.append(info_bar_plugin_element);
			}
		}
	}

	if (enabled_elements & SP_D3D9O_TEXT_FEED_ENABLED)
	{
		// Draw text feed
		text_feed->draw();
	}
	text_feed->info_bar_plugin_elements.clear();

	if (enabled_elements & SP_D3D9O_CONSOLE_ENABLED)
	{
		// Draw console
		console->draw();
	}

	_SP_D3D9_CHECK_FAILED_(device->EndScene()); // Finished drawing the overlay


	// Restore current state block
	_SP_D3D9_CHECK_FAILED_(current_state_block->Apply());

	// Release temporary resources
	current_state_block->Release();
	render_target->Release();
	back_buffer->Release();
}


// Loads exported DLL plugin functions for execution
void SpD3D9Overlay::load_plugin_funcs(HINSTANCE new_dll_instance, const char* new_dll_name)
{
	if (new_dll_instance == NULL)
	{
		return;
	}

	// Disable plugin funcs while new plugins are being loaded
	SpD3D9Overlay::run_plugin_funcs = false;

	SP_D3D9_PLUGIN plugin;

	plugin.module = new_dll_instance;
	plugin.name = new_dll_name;

	typedef void(__stdcall *initialization_func_T)();
	plugin.init_func = (initialization_func_T)GetProcAddress(new_dll_instance, "initialize_plugin");

	typedef void(__stdcall *main_loop_func_T)();
	plugin.main_loop_func = (main_loop_func_T)GetProcAddress(new_dll_instance, "main_loop");

	typedef void(__stdcall *plugin_draw_ol_func_T)(std::string *);
	plugin.draw_overlay_func = (plugin_draw_ol_func_T)GetProcAddress(new_dll_instance, "draw_overlay"); // Plugin function for drawing overlay elements and adding extra info to the text feed info header

	typedef bool(__stdcall *plugin_get_raw_input_data_func_T)(RAWINPUT *, PUINT);
	plugin.get_raw_input_data_func = (plugin_get_raw_input_data_func_T)GetProcAddress(new_dll_instance, "get_raw_input_data");

	typedef void(__stdcall *present_func_T)(const RECT *, const RECT *, HWND, const RGNDATA *, DWORD);
	plugin.present_func = (present_func_T)GetProcAddress(new_dll_instance, "present");

	typedef void(__stdcall *end_scene_func_T)();
	plugin.end_scene_func = (end_scene_func_T)GetProcAddress(new_dll_instance, "end_scene");


	SpD3D9Overlay::loaded_libraries.push_back(plugin);

	// Re-enable plugin functions
	SpD3D9Overlay::run_plugin_funcs = true;
}
