// Author: Sean Pesce

#pragma once



#define WIN32_LEAN_AND_MEAN	// Exclude rarely-used stuff from Windows headers


// Windows Header Files:
#include <Windows.h>

// Other Header Files:
#include "SP_IO.hpp"

class SpD3D9Device;
class SpD3D9Overlay;

#include "SpD3D9Interface.h"
#include "SpD3D9Device.h"
#include "SpD3D9.h"


// Constants
#define _SP_D3D9O_PLUGIN_DEVICE_READY_ (device != NULL && (*device) != NULL && (*device)->overlay != NULL && (*device)->overlay->text_feed != NULL)


// Global variables & data:
SpD3D9Device **device;
std::list<SP_KEY_FUNCTION> *keybinds;
std::string settings_file;
std::string keybinds_section;


// Exported functions:
void __stdcall initialize_plugin();
void __stdcall load_keybinds(std::list<SP_KEY_FUNCTION> *new_keybinds);
void __stdcall set_device_wrapper(SpD3D9Device **new_device);


// Other functions:
int print_test_msg();
