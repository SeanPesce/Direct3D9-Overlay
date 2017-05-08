// Author: Sean Pesce
// Original d3d9.dll wrapper base by Michael Koch

#include "stdafx.h"
#include "proxydll.h"

// Global variables
#pragma data_seg (".d3d9_shared")
myIDirect3DDevice9*		gl_pmyIDirect3DDevice9;
myIDirect3D9*			gl_pmyIDirect3D9;
spIDirect3DSwapChain9*	gl_pspIDirect3DSwapChain9;
HINSTANCE				gl_hOriginalDll;
HINSTANCE				gl_hThisInstance;
HINSTANCE				dinput8_inst;
#pragma data_seg ()

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	// To avoid compiler level 4 warnings 
    LPVOID lpDummy = lpReserved;
    lpDummy = NULL;
    
    switch (ul_reason_for_call)
	{
	    case DLL_PROCESS_ATTACH:
			InitInstance(hModule);
			InitSettings();
			if (!gl_hOriginalDll)
			{
				// No chain was loaded; get original DLL from system directory
				LoadOriginalDll();
			}
			// Initialize the thread for the mod:
			mod_thread = CreateThread(
				NULL,				// Default security attributes
				0,					// Use default stack size
				init_mod_thread,	// Thread function name
				NULL,				// Argument to thread function
				0,					// Use default creation flags
				&mod_thread_id);	// Returns the thread identifier
			break;
	    case DLL_PROCESS_DETACH:
			mod_loop_enabled = false;
			ExitInstance();
			break;
        case DLL_THREAD_ATTACH:
			break;
	    case DLL_THREAD_DETACH:
			break;
		default:
			break;
	}
    return TRUE;
}

// Exported function (faking d3d9.dll's one-and-only export)
IDirect3D9* WINAPI Direct3DCreate9(UINT SDKVersion)
{
	if (!gl_hOriginalDll) LoadOriginalDll(); // Looking for the real d3d9.dll
	
	// Hooking IDirect3D Object from Original Library
	typedef IDirect3D9 *(WINAPI* D3D9_Type)(UINT SDKVersion);
	D3D9_Type D3DCreate9_fn = (D3D9_Type) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9");
    
    // Debug
	if (!D3DCreate9_fn) 
    {
        OutputDebugString("PROXYDLL: Pointer to original D3DCreate9 function not received ERROR ****\r\n");
        ::ExitProcess(0); // Exit the hard way
    }
	
	// Request pointer from Original Dll 
	IDirect3D9 *pIDirect3D9_orig = D3DCreate9_fn(SDKVersion);

	// Create myIDirect3D9 object and store pointer to original object there.
	// Note: The object will delete itself once the Ref count is zero (similar to COM objects)
	gl_pmyIDirect3D9 = new myIDirect3D9(pIDirect3D9_orig);

	// Return pointer to hooking Object instead of the real one
	return (gl_pmyIDirect3D9);
}

void InitInstance(HANDLE hModule) 
{
	OutputDebugString("PROXYDLL: InitInstance called.\r\n");
	
	// Initialization
	gl_hOriginalDll				= NULL;
	gl_hThisInstance			= NULL;
	gl_pmyIDirect3D9			= NULL;
	gl_pmyIDirect3DDevice9		= NULL;
	gl_pspIDirect3DSwapChain9	= NULL;
	dinput8_inst				= NULL;
	
	// Storing Instance handle into global variable
	gl_hThisInstance = (HINSTANCE)  hModule;
}

// Loads the original d3d9.dll from the system directory
void LoadOriginalDll(void)
{
    char buffer[MAX_PATH];
    
    // Getting path to system dir and to d3d9.dll
	::GetSystemDirectory(buffer,MAX_PATH);

	// Append dll name
	strcat_s(buffer,"\\d3d9.dll");
	
	// Try to load the system's d3d9.dll, if pointer empty
	if (!gl_hOriginalDll) gl_hOriginalDll = ::LoadLibrary(buffer);

	// Debug
	if (!gl_hOriginalDll)
	{
		OutputDebugString("PROXYDLL: Original d3d9.dll not loaded ERROR ****\r\n");
		::ExitProcess(0); // Exit the hard way
	}
}

// Load dinput8.dll
void load_dinput8()
{
	char *dinput8_filename = "dinput8.dll";

	if (!dinput8_inst)
	{
		dinput8_inst = LoadLibrary(dinput8_filename);
		if (!dinput8_inst)
		{
			// Failed to load dinput8.dll
			// Handle error
		}
	}

}

// Parses settings file (.ini) for intialization settings
int InitSettings()
{
	// Get keybinds from settings file
	hotkey_toggle_overlay_text_feed = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_KEYBINDS_, _SP_DS_HOTKEY_TOGGLE_OL_TXT_KEY_);
	hotkey_toggle_info_watermark = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_KEYBINDS_, _SP_DS_HOTKEY_TOGGLE_TEXT_WATERMARK_KEY_);
	hotkey_next_overlay_text_pos = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_KEYBINDS_, _SP_DS_HOTKEY_NEXT_OL_TXT_POS_KEY_);
	hotkey_next_overlay_text_style = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_KEYBINDS_, _SP_DS_HOTKEY_NEXT_OL_TXT_STYLE_KEY_);
	hotkey_toggle_audio_feedback = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_TOGGLE_AUDIO_FEEDBACK_KEY_);
	hotkey_toggle_verbose_output = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_TOGGLE_VERBOSE_OUTPUT_KEY_);
	hotkey_print_overlay_test_message = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_PRINT_OL_TXT_TEST_MSG_KEY_);
	hotkey_increase_overlay_text_size = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_INCREASE_TXT_SIZE_KEY_);
	hotkey_decrease_overlay_text_size = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_DECREASE_TXT_SIZE_KEY_);
	hotkey_reset_overlay_text_size = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_RESET_TXT_SIZE_KEY_);
	hotkey_toggle_multicolor_feed = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_TOGGLE_MULTICOLOR_FEED_KEY_);

	// Get user preferences from settings file
	get_user_preferences();

	if (user_pref_load_dinput8_early)
	{
		// Load dinput8.dll early (normally d3d9.dll is loaded first)
		load_dinput8();
	}

	char settings_buffer[128];

	// Check settings file for DLL chain
	settings_buffer[0] = '\0';
	GetPrivateProfileString(_SP_DS_SETTINGS_SECTION_ADV_SETTINGS_, _SP_DS_DLL_CHAIN_KEY_, NULL, settings_buffer, 128, _SP_DS_SETTINGS_FILE_);
	if (settings_buffer[0] != '\0') // Found DLL_Chain entry in settings file
	{
		gl_hOriginalDll = LoadLibrary(settings_buffer);
		if (!gl_hOriginalDll)
		{
			// Failed to load next wrapper DLL
			OutputDebugString("PROXYDLL: Failed to load chained DLL; loading original from system directory instead...\r\n");
			return 2; // Return 2 if specified DLL could not be loaded
		}
	}
	else
	{
		OutputDebugString("PROXYDLL: No DLL chain specified; loading original from system directory...\r\n");
		return 1; // Return 1 if d3d9.ini or DLL_Chain entry could not be located
	}

	return 0; // Return 0 on success
}

// Determines whether mod is enabled and calls the main loop for the mod
DWORD WINAPI init_mod_thread(LPVOID lpParam)
{

	if (!user_pref_overlay_text_feed_enabled	// @TODO: update this in real implementation
		&& hotkey_toggle_overlay_text_feed == 0
		&& hotkey_next_overlay_text_style == 0
		&& hotkey_next_overlay_text_pos == 0
		&& hotkey_print_overlay_test_message == 0)
	{
		// Disable mod
		mod_loop_enabled = false;
	}
	else
	{
		mod_loop_enabled = true;
	}

	extern void mod_loop();	// Main loop for the mod thread
	mod_loop();

	return 0;
}

// Unloads DLL when exiting
void ExitInstance() 
{    
    OutputDebugString("PROXYDLL: ExitInstance called.\r\n");
	
	// Release the system's d3d9.dll
	if (gl_hOriginalDll)
	{
		::FreeLibrary(gl_hOriginalDll);
	    gl_hOriginalDll = NULL;  
	}
}

// Reads in user preferences as specified in the settings file (.ini)
void get_user_preferences()
{
	// Overlay enabled/disabled
	user_pref_overlay_text_feed_enabled = ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_ENABLED_KEY_, _SP_DS_DEFAULT_VAL_OL_TXT_ENABLED_, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED);

	// Audio feedback enabled/disabled
	user_pref_audio_feedback_enabled = ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_AUDIO_ENABLED_KEY_, _SP_DS_DEFAULT_VAL_OL_TXT_AUDIO_ENABLED_, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED);

	// Verbose text output enabled/disabled
	user_pref_verbose_output_enabled = ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_DEV_PREFS_, _SP_DS_OL_TXT_VERBOSE_OUTPUT_ENABLED_KEY_, _SP_DS_DEFAULT_VAL_OL_TXT_VERBOSE_OUTPUT_ENABLED_, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED);

	// Multicolor text feed enabled/disabled
	user_pref_multicolor_feed_enabled = ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_DEV_PREFS_, _SP_DS_OL_TXT_MULTICOLOR_FEED_ENABLED_KEY_, _SP_DS_DEFAULT_VAL_OL_TXT_MULTICOLOR_FEED_ENABLED_, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED);

	// Check if dinput8.dll should be loaded before d3d9.dll
	user_pref_load_dinput8_early = ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_DEV_PREFS_, _SP_DS_OL_LOAD_DINPUT8_EARLY_KEY_, _SP_DS_DEFAULT_VAL_OL_LOAD_DINPUT8_EARLY_, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED);

	// Some games might need frames to be counted from the Present() method instead of the normal EndScene() method to accurately calculate FPS
	user_pref_use_alt_fps_counter = ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_DEV_PREFS_, _SP_DS_OL_USE_ALT_FPS_COUNTER_KEY_, OL_TXT_DISABLED, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED);

	// PvP Watchdog overlay adjustment (Dark Souls)
	user_pref_dspw_ol_offset = ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_ADV_SETTINGS_, _SP_DS_DSPW_ADJUSTMENT_KEY_, OL_TXT_DISABLED, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED);

	// FPS counter enabled/disabled
	if ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_ENABLE_FPS_KEY_, OL_TXT_ENABLED, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED)
	{
		user_pref_show_text_watermark = SP_DX9_WATERMARK_TITLE;
		user_pref_show_text_watermark += SP_DX9_WATERMARK_FPS;
	}
	else
	{
		user_pref_show_text_watermark = 0;
	}

	// Display date enabled/disabled
	if ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_ENABLE_DATE_KEY_, OL_TXT_ENABLED, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED)
	{
		user_pref_show_text_watermark |= SP_DX9_WATERMARK_TITLE;
		user_pref_show_text_watermark += SP_DX9_WATERMARK_DATE;
	}

	// Display time enabled/disabled
	if ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_ENABLE_TIME_KEY_, OL_TXT_ENABLED, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED)
	{
		user_pref_show_text_watermark |= SP_DX9_WATERMARK_TITLE;
		user_pref_show_text_watermark += SP_DX9_WATERMARK_TIME;
	}

	// Overlay text size
	user_pref_overlay_text_size = (int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_SIZE_KEY_, _SP_DEFAULT_TEXT_HEIGHT_, _SP_DS_SETTINGS_FILE_);
	if (user_pref_overlay_text_size < 1)
	{
		// Invalid font size specified; set to default
		user_pref_overlay_text_size = _SP_DEFAULT_TEXT_HEIGHT_;
	}

	char settings_buffer[128];
	
	// Overlay text horizonal position
	GetPrivateProfileString(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_HORIZONTAL_POS_KEY_, _SP_DS_DEFAULT_VAL_OL_TXT_HORIZONTAL_POS_, settings_buffer, 128, _SP_DS_SETTINGS_FILE_);
	std::string setting_value = settings_buffer;
	std::transform(setting_value.begin(), setting_value.end(), setting_value.begin(), ::toupper);
	if (strcmp(setting_value.c_str(), SP_OL_TXT_POS_VALS[OL_TXT_POS_LEFT]) == 0)
	{
		// Overlay text horizontal position will be on the left
		user_pref_overlay_text_pos = (DT_NOCLIP | DT_LEFT);
	}
	else if (strcmp(setting_value.c_str(), SP_OL_TXT_POS_VALS[OL_TXT_POS_RIGHT]) == 0)
	{
		// Overlay text horizontal position will be on the right
		user_pref_overlay_text_pos = (DT_NOCLIP | DT_RIGHT);
	}
	else
	{
		// Overlay text horizontal position will be in the center
		user_pref_overlay_text_pos = (DT_NOCLIP | DT_CENTER);
	}

	// Overlay text vertical position
	GetPrivateProfileString(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_VERTICAL_POS_KEY_, _SP_DS_DEFAULT_VAL_OL_TXT_VERTICAL_POS_, settings_buffer, 128, _SP_DS_SETTINGS_FILE_);
	setting_value = settings_buffer;
	std::transform(setting_value.begin(), setting_value.end(), setting_value.begin(), ::toupper);
	if (strcmp(setting_value.c_str(), SP_OL_TXT_POS_VALS[OL_TXT_POS_TOP]) == 0)
	{
		// Overlay text vertical position will be on top
		user_pref_overlay_text_pos |= (DT_NOCLIP | DT_TOP);
	}
	else if (strcmp(setting_value.c_str(), SP_OL_TXT_POS_VALS[OL_TXT_POS_VCENTER]) == 0)
	{
		// Overlay text vertical position will be in the middle
		user_pref_overlay_text_pos |= (DT_NOCLIP | DT_VCENTER);
	}
	else
	{
		// Overlay text vertical position will be on the bottom
		user_pref_overlay_text_pos |= (DT_NOCLIP | DT_BOTTOM);
	}

	// Overlay text style (outlined, shadowed, or plain)
	GetPrivateProfileString(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_STYLE_KEY_, _SP_DS_DEFAULT_VAL_OL_TXT_STYLE_, settings_buffer, 128, _SP_DS_SETTINGS_FILE_);
	setting_value = settings_buffer;
	std::transform(setting_value.begin(), setting_value.end(), setting_value.begin(), ::toupper);
	if (strcmp(setting_value.c_str(), SP_OL_TXT_STYLE_VALS[SP_DX9_SHADOWED_TEXT]) == 0)
	{
		// Overlay text style will be shadowed
		user_pref_overlay_text_style = SP_DX9_SHADOWED_TEXT;
	}
	else if (strcmp(setting_value.c_str(), SP_OL_TXT_STYLE_VALS[SP_DX9_PLAIN_TEXT]) == 0)
	{
		// Overlay text style will be plain
		user_pref_overlay_text_style = SP_DX9_PLAIN_TEXT;
	}
	else
	{
		// Overlay text style will be outlined
		user_pref_overlay_text_style = SP_DX9_BORDERED_TEXT;
	}

	// Get DSPW font size in case user wants to adjust this overlay to avoid clipping with the PvP Watchdog overlay
	dspw_pref_font_size = get_dspw_font_size();

	if (user_pref_dspw_ol_offset && dspw_pref_font_size)
	{
		// Set overlay offset to adjust for PvP Watchdog overlay
		user_pref_dspw_ol_offset = dspw_pref_font_size + 5;
	}
}

// Reads the PvP Watchdog settings file (DSPWSteam.ini) to obtain the DSPW font size in
//  case the user wants to adjust this overlay to avoid clipping with the PvP Watchdog
//  overlay
int get_dspw_font_size()
{
	std::ifstream in("DSPWSteam.ini"); // Open DSPW settings file stream

	if (!in)
	{
		// Failed to open PvP Watchdog settings file; DSPW is probably not installed
		return 0;
	}

	// The DSPW setting we're looking for (font size)
	std::string dspw_ol_font_size_key = "FontSize ";

	std::string line;

	while (std::getline(in, line)) {
		
		if (line.compare(0, dspw_ol_font_size_key.length(), dspw_ol_font_size_key.c_str()) == 0)
		{
			// Found the DSPW font size key
			std::stringstream string_to_int(&line.c_str()[dspw_ol_font_size_key.length()]);
			int dspw_font_size = 0;
			if (!(string_to_int >> dspw_font_size))
			{
				// Failed to parse the DSPW font size; return default font size
				return 15;
			}
			return dspw_font_size;
		}
	}

	// Couldn't find the FontSize key in the DSPW settings file
	return 0;
}

