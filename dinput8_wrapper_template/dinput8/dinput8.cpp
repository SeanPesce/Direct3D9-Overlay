// Author: Sean Pesce

#include "SP_DS_dinput8_Template.h"

HINSTANCE mHinst = 0, mHinstDLL = 0;
UINT_PTR mProcs[6] = {0};

void LoadOriginalDll();
int InitSettings();
DWORD WINAPI init_mod_thread(LPVOID lpParam);

LPCSTR mImportNames[] = {"DirectInput8Create", "DllCanUnloadNow", "DllGetClassObject", "DllRegisterServer", "DllUnregisterServer", "GetdfDIJoystick"};
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
		for ( int i = 0; i < 6; i++ )
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

extern "C" __declspec(naked) void __stdcall DirectInput8Create_wrapper(){__asm{jmp mProcs[0*4]}}
extern "C" __declspec(naked) void __stdcall DllCanUnloadNow_wrapper(){__asm{jmp mProcs[1*4]}}
extern "C" __declspec(naked) void __stdcall DllGetClassObject_wrapper(){__asm{jmp mProcs[2*4]}}
extern "C" __declspec(naked) void __stdcall DllRegisterServer_wrapper(){__asm{jmp mProcs[3*4]}}
extern "C" __declspec(naked) void __stdcall DllUnregisterServer_wrapper(){__asm{jmp mProcs[4*4]}}
extern "C" __declspec(naked) void __stdcall GetdfDIJoystick_wrapper(){__asm{jmp mProcs[5*4]}}


// Loads the original DLL from the default system directory
//	Function originally written by Michael Koch
void LoadOriginalDll()
{
	char buffer[MAX_PATH];

	// Get path to system dir and to dinput8.dll
	GetSystemDirectory(buffer, MAX_PATH);

	// Append DLL name
	strcat_s(buffer, "\\dinput8.dll");

	// Try to load the system's dinput8.dll, if pointer empty
	if (!mHinstDLL) mHinstDLL = LoadLibrary(buffer);

	// Debug
	if (!mHinstDLL)
	{
		OutputDebugString("PROXYDLL: Original dinput8.dll not loaded ERROR ****\r\n");
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
		return 1; // Return 1 if settings file or DLL_Chain entry could not be located
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
