// Author: Sean Pesce

#include "ExampleD3D9OverlayPlugin.h"

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


#define _DEVICE_READY_ _SP_D3D9O_PLUGIN_DEVICE_READY_			// Checks that the D3D9 device, overlay, and text feed are initialized
#define _GET_TEXT_FEED_ (*device)->overlay->text_feed			// Text feed object (prints messages)
#define _SETTINGS_FILE_ _SP_D3D9_SETTINGS_FILE_					// File to load settings from
#define _KEYBINDS_SECTION_ _SP_D3D9_SETTINGS_SECTION_KEYBINDS_	// Section of settings file that holds keybind assignments


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


//Exported functions: 

void __stdcall load_keybinds(std::list<SP_KEY_FUNCTION> *new_keybinds)
{
	keybinds = new_keybinds;

	if (keybinds != NULL)
	{
		settings_file = _SETTINGS_FILE_;
		keybinds_section = _KEYBINDS_SECTION_;

		unsigned int key = 0;

		if (key = get_vk_hotkey(settings_file.c_str(), keybinds_section.c_str(), "PrintExternalTestMessage"))
		{
			add_function_keybind(key, print_test_msg, keybinds);
		}
		key = 0;
	}
}

void __stdcall set_device_wrapper(SpD3D9Device **new_device)
{
	device = new_device;
}



// Other functions:

int print_test_msg()
{
	if (_DEVICE_READY_) // _DEVICE_READY_ should always be called before printing to the overlay from an external DLL
	{
		SpD3D9OTextFeed *text_feed = _GET_TEXT_FEED_;

		text_feed->print("Printing from external DLL", 2000, true);

		Sleep(200);
		return 0;
	}

	return -1; // Text feed is NULL
}