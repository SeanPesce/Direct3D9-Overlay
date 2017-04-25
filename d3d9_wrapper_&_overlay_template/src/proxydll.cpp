// Author: Sean Pesce
// Original d3d9.dll wrapper by Michael Koch

#include "stdafx.h"
#include "proxydll.h"

// global variables
#pragma data_seg (".d3d9_shared")
myIDirect3DDevice9* gl_pmyIDirect3DDevice9;
myIDirect3D9*       gl_pmyIDirect3D9;
HINSTANCE           gl_hOriginalDll;
HINSTANCE           gl_hThisInstance;
#pragma data_seg ()

BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	// To avoid compiler lvl4 warnings 
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
	if (!gl_hOriginalDll) LoadOriginalDll(); // Looking for the "right d3d9.dll"
	
	// Hooking IDirect3D Object from Original Library
	typedef IDirect3D9 *(WINAPI* D3D9_Type)(UINT SDKVersion);
	D3D9_Type D3DCreate9_fn = (D3D9_Type) GetProcAddress( gl_hOriginalDll, "Direct3DCreate9");
    
    // Debug
	if (!D3DCreate9_fn) 
    {
        OutputDebugString("PROXYDLL: Pointer to original D3DCreate9 function not received ERROR ****\r\n");
        ::ExitProcess(0); // exit the hard way
    }
	
	// Request pointer from Original Dll. 
	IDirect3D9 *pIDirect3D9_orig = D3DCreate9_fn(SDKVersion);
	
	// Create my IDirect3D9 object and store pointer to original object there.
	// Note: the object will delete itself once Ref count is zero (similar to COM objects)
	gl_pmyIDirect3D9 = new myIDirect3D9(pIDirect3D9_orig);
	
	// Return pointer to hooking Object instead of "real one"
	return (gl_pmyIDirect3D9);
}

void InitInstance(HANDLE hModule) 
{
	OutputDebugString("PROXYDLL: InitInstance called.\r\n");
	
	// Initialization
	gl_hOriginalDll        = NULL;
	gl_hThisInstance       = NULL;
	gl_pmyIDirect3D9       = NULL;
	gl_pmyIDirect3DDevice9 = NULL;	
	
	// Storing Instance handle into global variable
	gl_hThisInstance = (HINSTANCE)  hModule;
}

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

// Parses settings file for intialization settings
int InitSettings()
{
	// Get keybinds from settings file
	hotkey_toggle_overlay_text = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_KEYBINDS_, _SP_DS_HOTKEY_TOGGLE_OL_TXT_KEY_);
	hotkey_next_overlay_text_pos = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_KEYBINDS_, _SP_DS_HOTKEY_NEXT_OL_TXT_POS_KEY_);
	hotkey_next_overlay_text_style = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_KEYBINDS_, _SP_DS_HOTKEY_NEXT_OL_TXT_STYLE_KEY_);
	hotkey_toggle_audio_feedback = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_TOGGLE_AUDIO_FEEDBACK_KEY_);
	hotkey_toggle_verbose_output = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_TOGGLE_VERBOSE_OUTPUT_KEY_);
	hotkey_print_overlay_test_message = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_PRINT_OL_TXT_TEST_MSG_KEY_);
	hotkey_increase_overlay_text_size = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_INCREASE_TXT_SIZE_KEY_);
	hotkey_decrease_overlay_text_size = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_DECREASE_TXT_SIZE_KEY_);
	hotkey_reset_overlay_text_size = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_DEV_KEYBINDS_, _SP_DS_HOTKEY_RESET_TXT_SIZE_KEY_);

	// Get user preferences from settings file
	get_user_preferences();

	char settings_buffer[128];

	// Check settings file for DLL chain
	settings_buffer[0] = '\0';
	GetPrivateProfileString(_SP_DS_SETTINGS_SECTION_SETTINGS_, _SP_DS_DLL_CHAIN_KEY_, NULL, settings_buffer, 128, _SP_DS_SETTINGS_FILE_);
	if (settings_buffer[0] != '\0') // Found DLL_Chain entry in settings file
	{
		gl_hOriginalDll = LoadLibrary(settings_buffer);
		if (!gl_hOriginalDll)
		{
			// Failed to load next wrapper DLL
			OutputDebugString("PROXYDLL: Failed to load chained DLL; loading original from system directory instead...\r\n");
			return 2; // Return 2 if given DLL could not be loaded
		}
	}
	else
	{
		OutputDebugString("PROXYDLL: No DLL chain specified; loading original from system directory...\r\n");
		return 1; // Return 1 if d3d9.ini or DLL_Chain entry could not be located
	}

	return 0; // Return 0 on success
}


DWORD WINAPI init_mod_thread(LPVOID lpParam)
{

	if (!user_pref_overlay_text_enabled		// @TODO: update this in real implementation
		&& hotkey_toggle_overlay_text == 0
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

	mod_loop();

	return 0;
}

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

void get_user_preferences()
{
	char settings_buffer[128];
	std::string setting_value;

	// Overlay enabled/disabled
	user_pref_overlay_text_enabled = ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_ENABLED_KEY_, _SP_DS_DEFAULT_VAL_OL_TXT_ENABLED_, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED);

	// Audio feedback enabled/disabled
	user_pref_audio_feedback_enabled = ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_AUDIO_ENABLED_KEY_, _SP_DS_DEFAULT_VAL_OL_TXT_AUDIO_ENABLED_, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED);

	// Verbose text output enabled/disabled
	user_pref_verbose_output_enabled = ((int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_DEV_PREFS_, _SP_DS_OL_TXT_VERBOSE_OUTPUT_ENABLED_KEY_, _SP_DS_DEFAULT_VAL_OL_TXT_VERBOSE_OUTPUT_ENABLED_, _SP_DS_SETTINGS_FILE_) != OL_TXT_DISABLED);

	// Overlay text size
	user_pref_overlay_text_size = (int)GetPrivateProfileInt(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_SIZE_KEY_, _SP_DEFAULT_TEXT_HEIGHT_, _SP_DS_SETTINGS_FILE_);

	// Overlay text horizonal position
	GetPrivateProfileString(_SP_DS_SETTINGS_SECTION_PREFS_, _SP_DS_OL_TXT_HORIZONTAL_POS_KEY_, _SP_DS_DEFAULT_VAL_OL_TXT_HORIZONTAL_POS_, settings_buffer, 128, _SP_DS_SETTINGS_FILE_);
	setting_value = settings_buffer;
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

	// Overlay text style
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
}

