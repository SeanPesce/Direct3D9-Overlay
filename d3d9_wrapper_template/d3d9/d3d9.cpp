// Author: Sean Pesce

#include "SP_DS_d3d9_Template.h"

HINSTANCE mHinst = 0, mHinstDLL = 0;
UINT_PTR mProcs[18] = {0};

void LoadOriginalDll();
int InitSettings();
DWORD WINAPI init_mod_thread(LPVOID lpParam);

LPCSTR mImportNames[] = {"D3DPERF_BeginEvent", "D3DPERF_EndEvent", "D3DPERF_GetStatus", "D3DPERF_QueryRepeatFrame", "D3DPERF_SetMarker", "D3DPERF_SetOptions", "D3DPERF_SetRegion", "DebugSetLevel", "DebugSetMute", "Direct3D9EnableMaximizedWindowedModeShim", "Direct3DCreate9", "Direct3DCreate9Ex", "Direct3DShaderValidatorCreate9", "PSGPError", "PSGPSampleTexture", (LPCSTR)16, (LPCSTR)17, (LPCSTR)18};
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved ) {
	mHinst = hinstDLL;
	if ( fdwReason == DLL_PROCESS_ATTACH ) {
		InitSettings();
		if (!mHinstDLL)
		{
			// No chain was loaded; get original DLL from system directory
			LoadOriginalDll();
		}
		if ( !mHinstDLL )
			return ( FALSE );
		for ( int i = 0; i < 18; i++ )
			mProcs[ i ] = (UINT_PTR)GetProcAddress( mHinstDLL, mImportNames[ i ] );
		// Initialize the thread for the mod:
		mod_thread = CreateThread(
			NULL,				// default security attributes
			0,					// use default stack size  
			init_mod_thread,	// thread function name
			NULL,				// argument to thread function 
			0,					// use default creation flags 
			&mod_thread_id);	// returns the thread identifier 
	} else if ( fdwReason == DLL_PROCESS_DETACH ) {
		mod_loop_enabled = false;
		FreeLibrary( mHinstDLL );
	}
	return ( TRUE );
}

extern "C" __declspec(naked) void __stdcall D3DPERF_BeginEvent_wrapper(){__asm{jmp mProcs[0*4]}}
extern "C" __declspec(naked) void __stdcall D3DPERF_EndEvent_wrapper(){__asm{jmp mProcs[1*4]}}
extern "C" __declspec(naked) void __stdcall D3DPERF_GetStatus_wrapper(){__asm{jmp mProcs[2*4]}}
extern "C" __declspec(naked) void __stdcall D3DPERF_QueryRepeatFrame_wrapper(){__asm{jmp mProcs[3*4]}}
extern "C" __declspec(naked) void __stdcall D3DPERF_SetMarker_wrapper(){__asm{jmp mProcs[4*4]}}
extern "C" __declspec(naked) void __stdcall D3DPERF_SetOptions_wrapper(){__asm{jmp mProcs[5*4]}}
extern "C" __declspec(naked) void __stdcall D3DPERF_SetRegion_wrapper(){__asm{jmp mProcs[6*4]}}
extern "C" __declspec(naked) void __stdcall DebugSetLevel_wrapper(){__asm{jmp mProcs[7*4]}}
extern "C" __declspec(naked) void __stdcall DebugSetMute_wrapper(){__asm{jmp mProcs[8*4]}}
extern "C" __declspec(naked) void __stdcall Direct3D9EnableMaximizedWindowedModeShim_wrapper(){__asm{jmp mProcs[9*4]}}
extern "C" __declspec(naked) void __stdcall Direct3DCreate9_wrapper(){__asm{jmp mProcs[10*4]}}
extern "C" __declspec(naked) void __stdcall Direct3DCreate9Ex_wrapper(){__asm{jmp mProcs[11*4]}}
extern "C" __declspec(naked) void __stdcall Direct3DShaderValidatorCreate9_wrapper(){__asm{jmp mProcs[12*4]}}
extern "C" __declspec(naked) void __stdcall PSGPError_wrapper(){__asm{jmp mProcs[13*4]}}
extern "C" __declspec(naked) void __stdcall PSGPSampleTexture_wrapper(){__asm{jmp mProcs[14*4]}}
extern "C" __declspec(naked) void __stdcall ExportByOrdinal16(){__asm{jmp mProcs[15*4]}}
extern "C" __declspec(naked) void __stdcall ExportByOrdinal17(){__asm{jmp mProcs[16*4]}}
extern "C" __declspec(naked) void __stdcall ExportByOrdinal18(){__asm{jmp mProcs[17*4]}}


// Loads the original DLL from the default system directory
//	Function originally written by Michael Koch
void LoadOriginalDll()
{
	char buffer[MAX_PATH];

	// Get path to system dir and to d3d9.dll
	GetSystemDirectory(buffer, MAX_PATH);

	// Append DLL name
	strcat_s(buffer, "\\d3d9.dll");

	// Try to load the system's d3d9.dll, if pointer empty
	if (!mHinstDLL) mHinstDLL = LoadLibrary(buffer);

	// Debug
	if (!mHinstDLL)
	{
		OutputDebugString("PROXYDLL: Original d3d9.dll not loaded ERROR ****\r\n");
		ExitProcess(0); // Exit the hard way
	}
}


// Parses settings file for intialization settings
int InitSettings()
{
	hotkey1 = get_vk_hotkey(SETTINGS_FILE, SETTINGS_FILE_SUBSECTION, HOTKEY1_KEY);

	char dll_chain_buffer[128];

	// Check settings file for DLL chain
	GetPrivateProfileString(SETTINGS_FILE_SUBSECTION, DLL_CHAIN_KEY, NULL, dll_chain_buffer, 128, SETTINGS_FILE);

	if (dll_chain_buffer[0] != '\0') // Found DLL_Chain entry in settings file
	{
		mHinstDLL = LoadLibrary(dll_chain_buffer);
		if (!mHinstDLL)
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

	if (_ALL_HOTKEYS_EQUAL_ZERO_)
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
