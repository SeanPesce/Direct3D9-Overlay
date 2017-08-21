// Author: Sean Pesce
// Source file for exported data (used by overlay plugins)

#include "stdafx.h"
#include "SpD3D9Exports.h"


__declspec(dllexport) IDirect3DDevice9 *get_d3d9_device()
{
	if (gl_pSpD3D9Device != NULL)
	{
		return gl_pSpD3D9Device->m_pIDirect3DDevice9;
	}

	return NULL;
}


__declspec(dllexport) unsigned int register_hotkey_function(unsigned int vk_hotkey, int(*function)())
{
	if (vk_hotkey > 0 && function != NULL)
	{
		extern std::list<SP_KEY_FUNCTION> keybinds; // Stores all function/keybind mappings
		add_function_keybind(vk_hotkey, function, &keybinds);
		return vk_hotkey;
	}
	else
	{
		return 0;
	}
}


__declspec(dllexport) int register_console_command(const char *command, void(*function)(std::vector<std::string>, std::string *), const char *help_message)
{
	return SpD3D9OConsole::register_command(command, function, help_message);
}


__declspec(dllexport) int register_console_alias(const char *new_alias, const char *existing_command)
{
	return SpD3D9OConsole::register_alias(new_alias, existing_command);
}


__declspec(dllexport) int execute_console_command(const char *command, std::string *output)
{
	if (gl_pSpD3D9Device != NULL && gl_pSpD3D9Device->overlay != NULL && gl_pSpD3D9Device->overlay->console != NULL)
	{
		if (command != NULL)
		{
			gl_pSpD3D9Device->overlay->console->execute_command(command, output);
			return 0;
		}
		else
		{
			return ERROR_INVALID_ADDRESS;
		}
	}
	else
	{
		// Console is not initialized
		return PEERDIST_ERROR_NOT_INITIALIZED;
	}
}


__declspec(dllexport) bool print(const char *message, unsigned long long duration, bool include_timestamp, SP_D3D9O_TEXT_COLOR_ENUM text_color)
{
	extern SpD3D9Device* gl_pSpD3D9Device;
	if (gl_pSpD3D9Device != NULL && gl_pSpD3D9Device->overlay != NULL && gl_pSpD3D9Device->overlay->text_feed != NULL)
	{
		gl_pSpD3D9Device->overlay->text_feed->print(message, duration, include_timestamp, text_color);
		return true;
	}

	return false;
}


__declspec(dllexport) bool print_console(const char *message)
{
	if (gl_pSpD3D9Device != NULL && gl_pSpD3D9Device->overlay != NULL && gl_pSpD3D9Device->overlay->console != NULL)
	{
		gl_pSpD3D9Device->overlay->console->print(message);
		return true;
	}

	return false;
}


__declspec(dllexport) bool set_text_feed_title(const char *new_title)
{
	extern SpD3D9Device* gl_pSpD3D9Device;
	if (gl_pSpD3D9Device != NULL && gl_pSpD3D9Device->overlay != NULL && gl_pSpD3D9Device->overlay->text_feed != NULL)
	{
		gl_pSpD3D9Device->overlay->text_feed->set_title(new_title);
		return true;
	}

	return false;
}