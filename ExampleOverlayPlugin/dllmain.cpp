// Author: Sean Pesce

#include "ExampleOverlayPlugin.h"

#ifdef _DEVICE_READY_
	#undef _DEVICE_READY_
#endif
#ifdef _GET_TEXT_FEED_
	#undef GET_TEXT_FEED_
#endif
#ifdef _SETTINGS_FILE_
	#undef _SETTINGS_FILE_
#endif
#ifdef _KEYBINDS_SECTION_
	#undef _KEYBINDS_SECTION_
#endif
#ifdef _PRINT_OVERLAY_VERBOSE_
	#undef _PRINT_OVERLAY_VERBOSE_
#endif
#ifdef _PRINT_OVERLAY_
	#undef _PRINT_OVERLAY_
#endif


#define _DEVICE_READY_ _SP_D3D9O_PLUGIN_DEVICE_READY_		// Checks that the D3D9 device, overlay, and text feed are initialized
#define _GET_TEXT_FEED_ (*device)->overlay->text_feed		// Text feed object (prints messages)
#define _SETTINGS_FILE_ ".//d3d9_Mod.ini"					// File to load settings from
#define _KEYBINDS_SECTION_ "Keybinds"						// Section of settings file that holds keybind assignments
#define _PRINT_OVERLAY_VERBOSE_(...) {if(_DEVICE_READY_ && (*verbose_output_enabled)){_GET_TEXT_FEED_->print(##__VA_ARGS__);}} // Performs necessary checks and prints message to overlay text feed, but only if verbose output is enabled
#define _PRINT_OVERLAY_(...) {if(_DEVICE_READY_){_GET_TEXT_FEED_->print(##__VA_ARGS__);}} // Performs necessary checks and prints message to overlay text feed


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
			break;
		case DLL_THREAD_ATTACH:
			break;
		case DLL_THREAD_DETACH:
			break;
		case DLL_PROCESS_DETACH:
		default:
			break;
	}
	return TRUE;
}


// Exported functions: 

void __stdcall initialize_plugin()
{
	// @TODO: Replace this with real initialization tasks
	Beep(500, 200);
	Beep(600, 200);
	Beep(700, 200);
}


void __stdcall load_keybinds(std::list<SP_KEY_FUNCTION> *new_keybinds, bool *audio_feedback)
{
	audio_feedback_enabled = audio_feedback;
	keybinds = new_keybinds;

	if (keybinds != NULL)
	{
		settings_file = _SETTINGS_FILE_;
		keybinds_section = _KEYBINDS_SECTION_;

		unsigned int key = 0;

		// @TODO: Replace this with real keybound functions
		if (key = get_vk_hotkey(settings_file.c_str(), keybinds_section.c_str(), "PrintExternalTestMessage"))
		{
			add_function_keybind(key, print_test_msg, keybinds);
		}
		key = 0;
	}
}

void __stdcall set_device_wrapper(SpD3D9Device **new_device, bool *verbose_output)
{
	verbose_output_enabled = verbose_output;
	device = new_device;
}



// Other functions:

int print_test_msg()
{
	if (*verbose_output_enabled)
	{
		_PRINT_OVERLAY_("Printing from external DLL (Verbose output is enabled)", 2000, true);
	}
	else
	{
		_PRINT_OVERLAY_("Printing from external DLL", 2000, true);
	}

	if (*audio_feedback_enabled)
	{
		Beep(500, 100);
		Sleep(100);
	}
	else
	{
		Sleep(200);
	}
	
	return 0;
}