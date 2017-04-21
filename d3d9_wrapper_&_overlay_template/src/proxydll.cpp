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
	hotkey_next_overlay_text_pos = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_KEYBINDS_, _SP_DS_HOTKEY_STR_NEXT_OL_TXT_POS_KEY_);
	hotkey_next_overlay_text_style = get_vk_hotkey(_SP_DS_SETTINGS_FILE_, _SP_DS_SETTINGS_SECTION_KEYBINDS_, _SP_DS_HOTKEY_STR_NEXT_OL_TXT_STYLE_KEY_);

	char dll_chain_buffer[128];

	// Check settings file for DLL chain
	GetPrivateProfileString(_SP_DS_SETTINGS_SECTION_SETTINGS_, _SP_DS_DLL_CHAIN_KEY_, NULL, dll_chain_buffer, 128, _SP_DS_SETTINGS_FILE_);

	if (dll_chain_buffer[0] != '\0') // Found DLL_Chain entry in settings file
	{
		gl_hOriginalDll = LoadLibrary(dll_chain_buffer);
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

	if (hotkey_next_overlay_text_pos == 0 && hotkey_next_overlay_text_style == 0) // @TODO: update this in real implementation
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

