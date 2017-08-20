// Author: Sean Pesce


/*
	Authors of significant portions code for this class:

	-----------------------------------------
	* Game hacking QTS ( Quickie Tip Series )
	* no. 16 - Callback based keyboard and mouse input
	-----------------------------------------
	* Author: SEGnosis      - GHAnon.net
	* Thanks to:
	* bitterbanana          - No known site
	* Drunken Cheetah       - No known site
	* fatboy88              - No known site
	* Geek4Ever             - No known site
	* learn_more            - www.uc-forum.com
	* Novocaine             - http://ilsken.net/blog/?page_id=64
	* Philly0494            - No known site
	* Roverturbo            - www.uc-forum.com
	* SilentKarma           - www.halocoders.com - offline
	* Strife                - www.uc-forum.com
	* Wieter20              - No known site
	* evolution536
*/

#include "stdafx.h"
#include "SpD3D9OInputHandler.h"


#ifdef _SP_USING_DETOURS_
	#include <detours.h>
#endif // _SP_USING_DETOURS_

SpD3D9OInputHandler * SpD3D9OInputHandler::instance = NULL;

#ifdef _SP_DISABLE_DINPUT8_INPUT_
	// Create the original function call definition
	extern "C"
	{
		HRESULT(__fastcall *oGetDeviceData)(void* pThis, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) = NULL;
	}
#endif // _SP_DISABLE_DINPUT8_INPUT_


SpD3D9OInputHandler *SpD3D9OInputHandler::get()
{
	if (instance == NULL)
	{
		instance = new SpD3D9OInputHandler();
	}

	return instance;
}


SpD3D9OInputHandler::SpD3D9OInputHandler()
{
	#ifdef _SP_USE_DETOUR_DISPATCH_MSG_INPUT_

	#ifdef _SP_USE_DETOUR_DISPATCH_MSG_A_INPUT_
		while (!(oDispatchMessage = (tDispatchMessage)GetProcAddress(GetModuleHandle("User32.dll"), "DispatchMessageA")))
	#else
		while (!(oDispatchMessage = (tDispatchMessage)GetProcAddress(GetModuleHandle("User32.dll"), "DispatchMessageW")))
	#endif // _SP_USE_DETOUR_DISPATCH_MSG_A_INPUT_
		{
			Sleep(200);
		}

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)oDispatchMessage, hkDispatchMessage);
		DetourTransactionCommit();

		if (oDispatchMessage == NULL)
		{
			// Handle error?
			Beep(300, 200);
			Beep(300, 200);
		}
	#endif // _SP_USE_DETOUR_DISPATCH_MSG_INPUT_


	#ifdef _SP_USE_WIN_HOOK_EX_INPUT_
		extern HINSTANCE gl_hThisInstance;
		extern DWORD lasterr;
		lasterr = 0;
		if (!(input_hook = SetWindowsHookEx(WH_CALLWNDPROC, handle_message, gl_hThisInstance, 0)))
		//if (!(input_hook = SetWindowsHookEx(WH_CALLWNDPROC, handle_message, GetModuleHandle(NULL), 0)))
		{
			// Handle error

			lasterr = GetLastError();

			Beep(300, 200);
			Beep(300, 200);
		}
	#endif // _SP_USE_WIN_HOOK_EX_INPUT_


	#ifdef _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
		initialize_vk_to_char_conversion_array();

		while (!(oGetRawInputData = (tGetRawInputData)GetProcAddress(GetModuleHandle("User32.dll"), "GetRawInputData")))
		{
			Sleep(200);
		}

		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)oGetRawInputData, hkGetRawInputData);
		DetourTransactionCommit();

		if (oGetRawInputData == NULL)
		{
			// Handle error?
			Beep(300, 200);
			Beep(300, 200);
		}
	#endif // _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_


	#ifdef _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
		initialize_vk_to_char_conversion_array();

		DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&dinput8, NULL);

		if (dinput8 == NULL)
		{
			// Handle error
			Beep(800, 200);
			Beep(800, 200);
		}

		dinput8->CreateDevice(GUID_SysKeyboard, &di8_keyboard, NULL);

		if (di8_keyboard != NULL)
		{
			di8_keyboard->SetDataFormat(&c_dfDIKeyboard);
			if (!SUCCEEDED(di8_keyboard->SetCooperativeLevel(*(gl_pSpD3D9Device->overlay->game_window), DISCL_FOREGROUND | DISCL_NONEXCLUSIVE)))
			{
				// Handle error
				Beep(400, 100);
				Beep(300, 100);
				Beep(200, 100);
			}

			DIPROPDWORD dipdw;
			dipdw.diph.dwSize = sizeof(DIPROPDWORD);
			dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
			dipdw.diph.dwObj = 0;
			dipdw.diph.dwHow = DIPH_DEVICE;
			dipdw.dwData = _SP_DINPUT8_DEVICE_BUFFER_SIZE_;

			if (di8_keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph) != DI_OK)
			{
				// Handle error
				Beep(300, 200);
				Beep(800, 200);
			}

			extern DWORD lasterr;
			lasterr = 0;
			HRESULT hres = di8_keyboard->Acquire();
			switch (hres)
			{
				case DI_OK:
					break;
				case S_FALSE:
					// Handle this case?
					break;
				default:
					// Handle error
					lasterr = GetLastError();
					Beep(800, 200);
					Beep(300, 400);
					break;
			}
		}
		else
		{
			// Handle error
			Beep(300, 200);
			Beep(300, 200);
		}


	#ifdef _SP_DISABLE_DINPUT8_INPUT_
		// Set the original function to the address of the start of our member function
		oGetDeviceData = (HRESULT(__fastcall *)(void*, DWORD, LPDIDEVICEOBJECTDATA, LPDWORD, DWORD))_SP_GETDEVICEDATA_ADDR_;

		// Apply the detour
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)oGetDeviceData, hkGetDeviceData);
		DetourTransactionCommit();
	#endif // _SP_DISABLE_DINPUT8_INPUT_


	#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_


	#ifdef _SP_USE_GET_RAW_INPUT_DATA_INPUT_
		extern DWORD lasterr;
		lasterr = 0;
		if (!initialize_devices())
		{
			lasterr = GetLastError();
			// Handle error
			Beep(800, 200);
			Beep(300, 200);
		}
	#endif // _SP_USE_GET_RAW_INPUT_DATA_INPUT_

	// instance = this;
}


SpD3D9OInputHandler::~SpD3D9OInputHandler()
{
	#ifdef _SP_USE_WIN_HOOK_EX_INPUT_
		while (!UnhookWindowsHookEx(input_hook)) // @TODO: Fix this (unhooking crashes other programs)
		{
			Sleep(100);
		}
	#endif // _SP_USE_WIN_HOOK_EX_INPUT_

	#ifdef _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
			di8_keyboard->Unacquire();
			di8_keyboard->Release();
			dinput8->Release();
	#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_

	instance = NULL;
}


#ifdef _SP_USE_GET_RAW_INPUT_DATA_INPUT_
bool SpD3D9OInputHandler::initialize_devices()
{
	RAWINPUTDEVICE raw_input[SP_RAW_DEVICE_COUNT];

	raw_input[SP_RAW_INPUT_KEYBOARD].usUsagePage = 0x01;
	raw_input[SP_RAW_INPUT_KEYBOARD].usUsage = 0x06; // Keyboard
	raw_input[SP_RAW_INPUT_KEYBOARD].dwFlags = 0;
	//raw_input[SP_RAW_INPUT_KEYBOARD].hwndTarget = *gl_pSpD3D9Device->overlay->game_window;
	raw_input[SP_RAW_INPUT_KEYBOARD].hwndTarget = 0;

	/*raw_input[SP_RAW_INPUT_MOUSE].usUsagePage = 0x01;
	raw_input[SP_RAW_INPUT_MOUSE].usUsage = 0x02; // Mouse
	raw_input[SP_RAW_INPUT_MOUSE].dwFlags = 0;
	//raw_input[SP_RAW_INPUT_MOUSE].hwndTarget = *gl_pSpD3D9Device->overlay->game_window;
	raw_input[SP_RAW_INPUT_MOUSE].hwndTarget = 0;

	return RegisterRawInputDevices(raw_input, SP_RAW_DEVICE_COUNT, sizeof(raw_input[0]) * SP_RAW_DEVICE_COUNT);*/

	return RegisterRawInputDevices(raw_input, 1, sizeof(raw_input[0]));
}

void SpD3D9OInputHandler::get_device_input()
{
	
	UINT dwSize;
	
	// @TODO: this func needs to be a callback function 
	
	

	static POINT ptCursor; // Current mouse cursor position
	const DWORD	dwLButtonTime = 13379,
		dwRButtonTime = 13378;

	handled = false;

	RAWINPUT *raw_input = (RAWINPUT *)pData;

	switch (raw_input->header.dwType)
	{
		case RIM_TYPEHID:
			break; // case RIM_TYPEHID

		case RIM_TYPEMOUSE:

			switch (raw_input->data.mouse.usButtonFlags)
			{
				case WM_LBUTTONDOWN:
					//SetTimer(hWnd, dwLButtonTime, 50, 0);
					break;

				case WM_LBUTTONUP:
					//KillTimer(hWnd, dwLButtonTime);
					break;

				case WM_RBUTTONDOWN:
					//SetTimer(hWnd, dwRButtonTime, 50, 0);
					break;

				case WM_RBUTTONUP:
					//KillTimer(hWnd, dwRButtonTime);
					break;

				case WM_MOUSEWHEEL:
					if (raw_input->data.mouse.usButtonData == 120)
					{

					}

					if (raw_input->data.mouse.usButtonData == -120)
					{

					}
					break;
		} // switch (raw_input->data.mouse.usButtonFlags)

			break; // case RIM_TYPEMOUSE

		case RIM_TYPEKEYBOARD:
			Beep(600, 100);

			switch (raw_input->data.keyboard.Message)
			{
			case WM_KEYDOWN: // A nonsystem key is a key that is pressed when the ALT key is not pressed
				switch (raw_input->data.keyboard.VKey)
				{
					case VK_SHIFT:
						shift = true;
						break;
					case VK_CONTROL:
						ctrl = true;
						break;
					case VK_MENU: // ALT key
						alt = true;
						break;
				}

				if (gl_pSpD3D9Device != NULL)
				{
					gl_pSpD3D9Device->overlay->console->handle_key_press(raw_input->data.keyboard.VKey); // Send keypress to the console
				}


			case WM_SYSKEYDOWN: // A system key is a key that is pressed when the ALT key is pressed
				switch (raw_input->data.keyboard.VKey)
				{
					case VK_LEFT:
						break;
					case VK_RIGHT:
						break;
					case VK_DOWN:
						break;
					case VK_UP:
						break;
				}
				break; // case  WM_KEYDOWN || WM_SYSKEYDOWN

			case WM_CHAR:
				if (gl_pSpD3D9Device != NULL)
				{
					gl_pSpD3D9Device->overlay->console->handle_text_input(raw_input->data.keyboard.VKey);
				}
				break;

			case WM_KEYUP:
				switch (raw_input->data.keyboard.VKey)
				{
					case VK_SHIFT:
						shift = false;
						break;
					case VK_CONTROL:
						ctrl = false;
						break;
					case VK_MENU: // ALT key
						alt = false;
						break;
				}
				break; // case WM_KEYUP

			} // switch (raw_input->data.keyboard.Message)
			break; // case RIM_TYPEKEYBOARD

	} // switch (raw_input->header.dwType)

}
#endif // _SP_USE_GET_RAW_INPUT_DATA_INPUT_


#ifdef _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
void SpD3D9OInputHandler::get_dinput_data()
{
	HRESULT hres = NULL;

	DWORD event_count = _SP_DINPUT8_DEVICE_BUFFER_SIZE_; // Maximum number of events to pull from the buffer

	hres = di8_keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), kb_data_buffer, &event_count, 0);
	if (SUCCEEDED(hres))
	{
		// event_count = number of events read from buffer
		for (unsigned int i = 0; i < event_count && gl_pSpD3D9Device->overlay->console->is_open(); i++)
		{
			gl_pSpD3D9Device->overlay->console->handle_key_event(&kb_data_buffer[i]);
		}
	}
	else
	{
		// Handle error
		Beep(200, 200);
	}
}

void SpD3D9OInputHandler::flush_keyboard_input_buffer()
{
	if (!SUCCEEDED(di8_keyboard->Acquire()))
	{
		// Handle error
	}

	DWORD event_count = INFINITE;
	HRESULT hres = di8_keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), NULL, &event_count, 0);
	if (!SUCCEEDED(hres))
	{
		// Handle error
	}
}


void SpD3D9OInputHandler::initialize_vk_to_char_conversion_array()
{
	memset(convert_char, 0, (sizeof(char) * 257));
	memset(convert_shift_char, 0, (sizeof(char) * 257));

	convert_char[DIK_0] = '0';
	convert_char[DIK_1] = '1';
	convert_char[DIK_2] = '2';
	convert_char[DIK_3] = '3';
	convert_char[DIK_4] = '4';
	convert_char[DIK_5] = '5';
	convert_char[DIK_6] = '6';
	convert_char[DIK_7] = '7';
	convert_char[DIK_8] = '8';
	convert_char[DIK_9] = '9';
	convert_char[DIK_A] = 'a';
	convert_char[DIK_B] = 'b';
	convert_char[DIK_C] = 'c';
	convert_char[DIK_D] = 'd';
	convert_char[DIK_E] = 'e';
	convert_char[DIK_F] = 'f';
	convert_char[DIK_G] = 'g';
	convert_char[DIK_H] = 'h';
	convert_char[DIK_I] = 'i';
	convert_char[DIK_J] = 'j';
	convert_char[DIK_K] = 'k';
	convert_char[DIK_L] = 'l';
	convert_char[DIK_M] = 'm';
	convert_char[DIK_N] = 'n';
	convert_char[DIK_O] = 'o';
	convert_char[DIK_P] = 'p';
	convert_char[DIK_Q] = 'q';
	convert_char[DIK_R] = 'r';
	convert_char[DIK_S] = 's';
	convert_char[DIK_T] = 't';
	convert_char[DIK_U] = 'u';
	convert_char[DIK_V] = 'v';
	convert_char[DIK_W] = 'w';
	convert_char[DIK_X] = 'x';
	convert_char[DIK_Y] = 'y';
	convert_char[DIK_Z] = 'z';
	convert_char[DIK_ABNT_C1] = '/'; // Brazilian keyboards
	convert_char[DIK_APOSTROPHE] = '\'';
	convert_char[DIK_BACKSLASH] = '\\';
	convert_char[DIK_COMMA] = ',';
	convert_char[DIK_EQUALS] = '=';
	convert_char[DIK_GRAVE] = '`'; // Grave accent (`)
	convert_char[DIK_LBRACKET] = '[';
	convert_char[DIK_MINUS] = '-';
	convert_char[DIK_OEM_102] = '>'; // German keyboard
	convert_char[DIK_PERIOD] = '.';
	convert_char[DIK_RBRACKET] = ']';
	convert_char[DIK_SEMICOLON] = ';';
	convert_char[DIK_SLASH] = '/';

	convert_char[DIK_ABNT_C2] = '.'; // Numpad '.' on Brazilian keyboards
	convert_char[DIK_ADD] = '+';
	convert_char[DIK_COLON] = ':';
	convert_char[DIK_DECIMAL] = '.';
	convert_char[DIK_DIVIDE] = '/';
	convert_char[DIK_MULTIPLY] = '*';
	convert_char[DIK_NUMPAD0] = '0';
	convert_char[DIK_NUMPAD1] = '1';
	convert_char[DIK_NUMPAD2] = '2';
	convert_char[DIK_NUMPAD3] = '3';
	convert_char[DIK_NUMPAD4] = '4';
	convert_char[DIK_NUMPAD5] = '5';
	convert_char[DIK_NUMPAD6] = '6';
	convert_char[DIK_NUMPAD7] = '7';
	convert_char[DIK_NUMPAD8] = '8';
	convert_char[DIK_NUMPAD9] = '9';
	convert_char[DIK_NUMPADCOMMA] = ',';
	convert_char[DIK_NUMPADEQUALS] = '=';
	convert_char[DIK_SPACE] = ' ';
	convert_char[DIK_SUBTRACT] = '-';
	convert_char[DIK_YEN] = (unsigned char)157; // '¥' (code might be wrong, some references use 190)


	convert_shift_char[DIK_0] = ')';
	convert_shift_char[DIK_1] = '!';
	convert_shift_char[DIK_2] = '@';
	convert_shift_char[DIK_3] = '#';
	convert_shift_char[DIK_4] = '$';
	convert_shift_char[DIK_5] = '%';
	convert_shift_char[DIK_6] = '^';
	convert_shift_char[DIK_7] = '&';
	convert_shift_char[DIK_8] = '*';
	convert_shift_char[DIK_9] = '(';
	convert_shift_char[DIK_A] = 'A';
	convert_shift_char[DIK_B] = 'B';
	convert_shift_char[DIK_C] = 'C';
	convert_shift_char[DIK_D] = 'D';
	convert_shift_char[DIK_E] = 'E';
	convert_shift_char[DIK_F] = 'F';
	convert_shift_char[DIK_G] = 'G';
	convert_shift_char[DIK_H] = 'H';
	convert_shift_char[DIK_I] = 'I';
	convert_shift_char[DIK_J] = 'J';
	convert_shift_char[DIK_K] = 'K';
	convert_shift_char[DIK_L] = 'L';
	convert_shift_char[DIK_M] = 'M';
	convert_shift_char[DIK_N] = 'N';
	convert_shift_char[DIK_O] = 'O';
	convert_shift_char[DIK_P] = 'P';
	convert_shift_char[DIK_Q] = 'Q';
	convert_shift_char[DIK_R] = 'R';
	convert_shift_char[DIK_S] = 'S';
	convert_shift_char[DIK_T] = 'T';
	convert_shift_char[DIK_U] = 'U';
	convert_shift_char[DIK_V] = 'V';
	convert_shift_char[DIK_W] = 'W';
	convert_shift_char[DIK_X] = 'X';
	convert_shift_char[DIK_Y] = 'Y';
	convert_shift_char[DIK_Z] = 'Z';
	convert_shift_char[DIK_ABNT_C1] = '?'; // Brazilian keyboards
	convert_shift_char[DIK_APOSTROPHE] = '"';
	convert_shift_char[DIK_BACKSLASH] = '|';
	convert_shift_char[DIK_COMMA] = '<';
	convert_shift_char[DIK_EQUALS] = '+';
	convert_shift_char[DIK_GRAVE] = '~';
	convert_shift_char[DIK_LBRACKET] = '{';
	convert_shift_char[DIK_MINUS] = '_';
	convert_shift_char[DIK_OEM_102] = '<'; // German keyboard
	convert_shift_char[DIK_PERIOD] = '>';
	convert_shift_char[DIK_RBRACKET] = '}';
	convert_shift_char[DIK_SEMICOLON] = ':';
	convert_shift_char[DIK_SLASH] = '?';

	convert_shift_char[DIK_ABNT_C2] = '.'; // Numpad '.' on Brazilian keyboards
	convert_shift_char[DIK_ADD] = '+';
	convert_shift_char[DIK_COLON] = ':';
	convert_shift_char[DIK_DECIMAL] = '.';
	convert_shift_char[DIK_DIVIDE] = '/';
	convert_shift_char[DIK_MULTIPLY] = '*';
	convert_shift_char[DIK_NUMPAD0] = '0';
	convert_shift_char[DIK_NUMPAD1] = '1';
	convert_shift_char[DIK_NUMPAD2] = '2';
	convert_shift_char[DIK_NUMPAD3] = '3';
	convert_shift_char[DIK_NUMPAD4] = '4';
	convert_shift_char[DIK_NUMPAD5] = '5';
	convert_shift_char[DIK_NUMPAD6] = '6';
	convert_shift_char[DIK_NUMPAD7] = '7';
	convert_shift_char[DIK_NUMPAD8] = '8';
	convert_shift_char[DIK_NUMPAD9] = '9';
	convert_shift_char[DIK_NUMPADCOMMA] = ',';
	convert_shift_char[DIK_NUMPADEQUALS] = '=';
	convert_shift_char[DIK_SPACE] = ' ';
	convert_shift_char[DIK_SUBTRACT] = '-';
	convert_char[DIK_YEN] = (unsigned char)157; // '¥' (code might be wrong, some references use 190)

}


#ifdef _SP_DISABLE_DINPUT8_INPUT_
HRESULT __fastcall hkGetDeviceData(void* pThis, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags)
{
	// @TODO: Disable dinput input

	return oGetDeviceData(pThis, cbObjectData, rgdod, pdwInOut, dwFlags); // Call original function
}
#endif // _SP_DISABLE_DINPUT8_INPUT_

#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_


#ifdef _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
UINT WINAPI hkGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader)
{
	// Get return value of original function:
	UINT return_value = SpD3D9OInputHandler::get()->oGetRawInputData(hRawInput, uiCommand, pData, pcbSize, cbSizeHeader);

	if (pData == NULL || return_value == (UINT)-1)
	{
		return return_value;
	}

	static POINT ptCursor; // Current mouse cursor position
	const DWORD	dwLButtonTime = 13379,
		dwRButtonTime = 13378;

	SpD3D9OInputHandler::get()->handled = false;

	RAWINPUT *raw_input = (RAWINPUT *)pData;

	// Send raw input data to overlay plugins
	if (SpD3D9Overlay::run_plugin_funcs)
	{
		for (auto plugin : SpD3D9Overlay::loaded_libraries)
		{
			if (plugin.get_raw_input_data_func != NULL)
			{
				plugin.get_raw_input_data_func(raw_input, pcbSize);
			}
		}
	}
	
	switch (raw_input->header.dwType)
	{
		case RIM_TYPEHID:
			break; // case RIM_TYPEHID

		case RIM_TYPEMOUSE:

			switch (raw_input->data.mouse.usButtonFlags)
			{
				case WM_LBUTTONDOWN:
					//SetTimer(hWnd, dwLButtonTime, 50, 0);
					break;

				case WM_LBUTTONUP:
					//KillTimer(hWnd, dwLButtonTime);
					break;

				case WM_RBUTTONDOWN:
					//SetTimer(hWnd, dwRButtonTime, 50, 0);
					break;

				case WM_RBUTTONUP:
					//KillTimer(hWnd, dwRButtonTime);
					break;

				case WM_MOUSEWHEEL:
					if (raw_input->data.mouse.usButtonData == 120)
					{

					}

					if (raw_input->data.mouse.usButtonData == -120)
					{

					}
					break;
			} // switch (raw_input->data.mouse.usButtonFlags)

			break; // case RIM_TYPEMOUSE

		case RIM_TYPEKEYBOARD:

			switch (raw_input->data.keyboard.Message)
			{
				case WM_KEYDOWN: // A nonsystem key is a key that is pressed when the ALT key is not pressed
					switch (raw_input->data.keyboard.VKey)
					{
						case VK_SHIFT:
							SpD3D9OInputHandler::get()->shift = true;
							break;
						case VK_CONTROL:
							SpD3D9OInputHandler::get()->ctrl = true;
							break;
						case VK_MENU: // ALT key
							SpD3D9OInputHandler::get()->alt = true;
							break;
						case VK_LWIN:
						case VK_RWIN:
							SpD3D9OInputHandler::get()->win = true;
							break;
					}

					if (gl_pSpD3D9Device != NULL)
					{
						gl_pSpD3D9Device->overlay->console->handle_key_press(raw_input->data.keyboard.VKey); // Send keypress to the console
					}


				case WM_SYSKEYDOWN: // A system key is a key that is pressed when the ALT key is pressed
					switch (raw_input->data.keyboard.VKey)
					{
						case VK_LEFT:
							break;
						case VK_RIGHT:
							break;
						case VK_DOWN:
							break;
						case VK_UP:
							break;
					}
					break; // case  WM_KEYDOWN || WM_SYSKEYDOWN

				case WM_CHAR:
					if (gl_pSpD3D9Device != NULL)
					{
						gl_pSpD3D9Device->overlay->console->handle_text_input(raw_input->data.keyboard.VKey);
					}
					break;

				case WM_KEYUP:
					switch (raw_input->data.keyboard.VKey)
					{
						case VK_SHIFT:
							SpD3D9OInputHandler::get()->shift = false;
							break;
						case VK_CONTROL:
							SpD3D9OInputHandler::get()->ctrl = false;
							break;
						case VK_MENU: // ALT key
							SpD3D9OInputHandler::get()->alt = false;
							break;
						case VK_LWIN:
						case VK_RWIN:
							SpD3D9OInputHandler::get()->win = false;
							break;
					}
					break; // case WM_KEYUP

			} // switch (raw_input->data.keyboard.Message)
			break; // case RIM_TYPEKEYBOARD

	} // switch (raw_input->header.dwType)



	// Check if any overlay plugins want to disable in-game player input
	bool disable_input = false;
	if (SpD3D9Overlay::run_plugin_funcs)
	{
		for (auto plugin : SpD3D9Overlay::loaded_libraries)
		{
			if (plugin.disable_player_input_func != NULL)
			{
				disable_input = plugin.disable_player_input_func();
			}
			if (disable_input)
			{
				// Found a plugin that disables input; no need to check the others
				break;
			}
		}
	}


	if (SpD3D9OInputHandler::get()->handled || disable_input)
	{
		// Skip calling original function if message was handled
		*pcbSize = 0;
		return 0;
	}
	else
	{
		return return_value; 
	}
}


void SpD3D9OInputHandler::initialize_vk_to_char_conversion_array()
{
	memset(convert_char, 0, (sizeof(char) * 257));
	memset(convert_shift_char, 0, (sizeof(char) * 257));

	convert_char[0x30] = '0';
	convert_char[0x31] = '1';
	convert_char[0x32] = '2';
	convert_char[0x33] = '3';
	convert_char[0x34] = '4';
	convert_char[0x35] = '5';
	convert_char[0x36] = '6';
	convert_char[0x37] = '7';
	convert_char[0x38] = '8';
	convert_char[0x39] = '9';
	convert_char[0x41] = 'a';
	convert_char[0x42] = 'b';
	convert_char[0x43] = 'c';
	convert_char[0x44] = 'd';
	convert_char[0x45] = 'e';
	convert_char[0x46] = 'f';
	convert_char[0x47] = 'g';
	convert_char[0x48] = 'h';
	convert_char[0x49] = 'i';
	convert_char[0x4A] = 'j';
	convert_char[0x4B] = 'k';
	convert_char[0x4C] = 'l';
	convert_char[0x4D] = 'm';
	convert_char[0x4E] = 'n';
	convert_char[0x4F] = 'o';
	convert_char[0x50] = 'p';
	convert_char[0x51] = 'q';
	convert_char[0x52] = 'r';
	convert_char[0x53] = 's';
	convert_char[0x54] = 't';
	convert_char[0x55] = 'u';
	convert_char[0x56] = 'v';
	convert_char[0x57] = 'w';
	convert_char[0x58] = 'x';
	convert_char[0x59] = 'y';
	convert_char[0x5A] = 'z';
	convert_char[VK_OEM_7] = '\'';
	convert_char[VK_OEM_5] = '\\';
	convert_char[VK_OEM_COMMA] = ',';
	convert_char[VK_OEM_PLUS] = '=';
	convert_char[VK_OEM_3] = '`'; // Grave accent (`)
	convert_char[VK_OEM_4] = '[';
	convert_char[VK_OEM_MINUS] = '-';
	convert_char[VK_OEM_102] = '>'; // German keyboard
	convert_char[VK_OEM_PERIOD] = '.';
	convert_char[VK_OEM_6] = ']';
	convert_char[VK_OEM_1] = ';';
	convert_char[VK_OEM_2] = '/';

	convert_char[VK_ADD] = '+';
	convert_char[VK_DECIMAL] = '.';
	convert_char[VK_DIVIDE] = '/';
	convert_char[VK_MULTIPLY] = '*';
	convert_char[VK_NUMPAD0] = '0';
	convert_char[VK_NUMPAD1] = '1';
	convert_char[VK_NUMPAD2] = '2';
	convert_char[VK_NUMPAD3] = '3';
	convert_char[VK_NUMPAD4] = '4';
	convert_char[VK_NUMPAD5] = '5';
	convert_char[VK_NUMPAD6] = '6';
	convert_char[VK_NUMPAD7] = '7';
	convert_char[VK_NUMPAD8] = '8';
	convert_char[VK_NUMPAD9] = '9';
	convert_char[VK_SPACE] = ' ';
	convert_char[VK_SUBTRACT] = '-';


	convert_shift_char[0x30] = ')';
	convert_shift_char[0x31] = '!';
	convert_shift_char[0x32] = '@';
	convert_shift_char[0x33] = '#';
	convert_shift_char[0x34] = '$';
	convert_shift_char[0x35] = '%';
	convert_shift_char[0x36] = '^';
	convert_shift_char[0x37] = '&';
	convert_shift_char[0x38] = '*';
	convert_shift_char[0x39] = '(';
	convert_shift_char[0x41] = 'A';
	convert_shift_char[0x42] = 'B';
	convert_shift_char[0x43] = 'C';
	convert_shift_char[0x44] = 'D';
	convert_shift_char[0x45] = 'E';
	convert_shift_char[0x46] = 'F';
	convert_shift_char[0x47] = 'G';
	convert_shift_char[0x48] = 'H';
	convert_shift_char[0x49] = 'I';
	convert_shift_char[0x4A] = 'J';
	convert_shift_char[0x4B] = 'K';
	convert_shift_char[0x4C] = 'L';
	convert_shift_char[0x4D] = 'M';
	convert_shift_char[0x4E] = 'N';
	convert_shift_char[0x4F] = 'O';
	convert_shift_char[0x50] = 'P';
	convert_shift_char[0x51] = 'Q';
	convert_shift_char[0x52] = 'R';
	convert_shift_char[0x53] = 'S';
	convert_shift_char[0x54] = 'T';
	convert_shift_char[0x55] = 'U';
	convert_shift_char[0x56] = 'V';
	convert_shift_char[0x57] = 'W';
	convert_shift_char[0x58] = 'X';
	convert_shift_char[0x59] = 'Y';
	convert_shift_char[0x5A] = 'Z';
	convert_shift_char[VK_OEM_7] = '"';
	convert_shift_char[VK_OEM_5] = '|';
	convert_shift_char[VK_OEM_COMMA] = '<';
	convert_shift_char[VK_OEM_PLUS] = '+';
	convert_shift_char[VK_OEM_3] = '~';
	convert_shift_char[VK_OEM_4] = '{';
	convert_shift_char[VK_OEM_MINUS] = '_';
	convert_shift_char[VK_OEM_102] = '<'; // German keyboard
	convert_shift_char[VK_OEM_PERIOD] = '>';
	convert_shift_char[VK_OEM_6] = '}';
	convert_shift_char[VK_OEM_1] = ':';
	convert_shift_char[VK_OEM_2] = '?';

	convert_shift_char[VK_ADD] = '+';
	convert_shift_char[VK_DECIMAL] = '.';
	convert_shift_char[VK_DIVIDE] = '/';
	convert_shift_char[VK_MULTIPLY] = '*';
	convert_shift_char[VK_NUMPAD0] = '0';
	convert_shift_char[VK_NUMPAD1] = '1';
	convert_shift_char[VK_NUMPAD2] = '2';
	convert_shift_char[VK_NUMPAD3] = '3';
	convert_shift_char[VK_NUMPAD4] = '4';
	convert_shift_char[VK_NUMPAD5] = '5';
	convert_shift_char[VK_NUMPAD6] = '6';
	convert_shift_char[VK_NUMPAD7] = '7';
	convert_shift_char[VK_NUMPAD8] = '8';
	convert_shift_char[VK_NUMPAD9] = '9';
	convert_shift_char[VK_SPACE] = ' ';
	convert_shift_char[VK_SUBTRACT] = '-';

}
#endif // _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_



#ifdef _SP_USE_WIN_HOOK_EX_INPUT_
LRESULT __stdcall handle_message(int nCode, WPARAM wParam, LPARAM lParam)
{
	//Beep(500, 20);

	if (wParam) // Specifies whether the message was sent by the current thread. If the message was sent by the current thread, it is nonzero; otherwise, it is zero.
	{
		Beep(500, 70);
	}

	SpD3D9OInputHandler::get()->handled = false;

	WPARAM msg_wParam = ((CWPSTRUCT *)lParam)->wParam;
	WPARAM msg_lParam = ((CWPSTRUCT *)lParam)->lParam;
	HWND hWnd = ((CWPSTRUCT *)lParam)->hwnd;

	static POINT ptCursor; // Current mouse cursor position

	const DWORD	dwLButtonTime = 13379,
		dwRButtonTime = 13378;

	switch (((CWPSTRUCT *)lParam)->message)
	{
		case WM_TIMER:
			switch (msg_wParam)
			{
				case dwLButtonTime:
					break;

				case dwRButtonTime:
					break;
			}
			break;

		case WM_LBUTTONDOWN:
			SetTimer(hWnd, dwLButtonTime, 50, 0);
			break;

		case WM_LBUTTONUP:
			KillTimer(hWnd, dwLButtonTime);
			break;

		case WM_RBUTTONDOWN:
			SetTimer(hWnd, dwRButtonTime, 50, 0);
			break;

		case WM_RBUTTONUP:
			KillTimer(hWnd, dwRButtonTime);
			break;

		case WM_MOUSEMOVE:
			ptCursor.x = LOWORD(msg_lParam);
			ptCursor.y = HIWORD(msg_lParam);
			break;


		case WM_MOUSEWHEEL:
			if (GET_WHEEL_DELTA_WPARAM(msg_wParam) == 120)
			{

			}

			if (GET_WHEEL_DELTA_WPARAM(msg_wParam) == -120)
			{

			}
			break;


		case WM_KEYDOWN: // A nonsystem key is a key that is pressed when the ALT key is not pressed
			switch (msg_wParam)
			{
				case VK_SHIFT:
					SpD3D9OInputHandler::get()->shift = true;
					break;
				case VK_CONTROL:
					SpD3D9OInputHandler::get()->ctrl = true;
					break;
			}

			if (gl_pSpD3D9Device != NULL)
			{
				gl_pSpD3D9Device->overlay->console->handle_key_press(msg_wParam); // Send keypress to the console
			}



		case WM_SYSKEYDOWN: // A system key is a key that is pressed when the ALT key is pressed
			switch (msg_wParam)
			{
				case VK_LEFT:

					break;

				case VK_RIGHT:

					break;

				case VK_DOWN:

					break;

				case VK_UP:

					break;
			}
			break;

		case WM_CHAR:
			if (gl_pSpD3D9Device != NULL)
			{
				gl_pSpD3D9Device->overlay->console->handle_text_input(msg_wParam);
			}
			break;

		case WM_KEYUP:
			switch (msg_wParam)
			{
				case VK_SHIFT:
					SpD3D9OInputHandler::get()->shift = false;
					break;
				case VK_CONTROL:
					SpD3D9OInputHandler::get()->ctrl = false;
					break;
			}
			break;


		case WM_SIZE:
			LOWORD(msg_lParam); // Client width
			HIWORD(msg_lParam); // Client height
			break;
	}

	return CallNextHookEx(input_hook, nCode, wParam, lParam);
}
#endif // _SP_USE_WIN_HOOK_EX_INPUT_


#ifdef _SP_USE_DETOUR_DISPATCH_MSG_INPUT_
LRESULT WINAPI hkDispatchMessage(MSG* lpmsg)
{
	Beep(500, 10);
	SpD3D9OInputHandler::get()->handled = false;

	WPARAM wParam = lpmsg->wParam;
	WPARAM lParam = lpmsg->lParam;
	HWND hWnd = lpmsg->hwnd;

	static POINT ptCursor; // Current mouse cursor position

	const DWORD	dwLButtonTime = 13379,
				dwRButtonTime = 13378;

	switch (lpmsg->message)
	{
		case WM_TIMER:
			switch (wParam)
			{
				case dwLButtonTime:
					break;

				case dwRButtonTime:
					break;
			}
			break;

		case WM_LBUTTONDOWN:
			//SetTimer(hWnd, dwLButtonTime, 50, 0);
			break;

		case WM_LBUTTONUP:
			//KillTimer(hWnd, dwLButtonTime);
			break;

		case WM_RBUTTONDOWN:
			//SetTimer(hWnd, dwRButtonTime, 50, 0);
			break;

		case WM_RBUTTONUP:
			//KillTimer(hWnd, dwRButtonTime);
			break;

		case WM_MOUSEMOVE:
			ptCursor.x = LOWORD(lParam);
			ptCursor.y = HIWORD(lParam);
			break;


		case WM_MOUSEWHEEL:
			if (GET_WHEEL_DELTA_WPARAM(wParam) == 120)
			{

			}

			if (GET_WHEEL_DELTA_WPARAM(wParam) == -120)
			{

			}
			break;


		case WM_KEYDOWN: // A nonsystem key is a key that is pressed when the ALT key is not pressed
			switch(wParam)
			{
				case VK_SHIFT:
					SpD3D9OInputHandler::get()->shift = true;
					break;
				case VK_CONTROL:
					SpD3D9OInputHandler::get()->ctrl = true;
					break;
				case VK_MENU: // ALT key
					SpD3D9OInputHandler::get()->alt = true;
					break;
				case VK_LWIN:
				case VK_RWIN:
					SpD3D9OInputHandler::get()->win = true;
					break;
			}
			
			if (gl_pSpD3D9Device != NULL)
			{
				gl_pSpD3D9Device->overlay->console->handle_key_press(wParam); // Send keypress to the console
			}
			


		case WM_SYSKEYDOWN: // A system key is a key that is pressed when the ALT key is pressed
			switch (wParam)
			{
				case VK_LEFT:

					break;

				case VK_RIGHT:

					break;

				case VK_DOWN:

					break;

				case VK_UP:

					break;
			}
			break;

		case WM_CHAR:
			if (gl_pSpD3D9Device != NULL)
			{
				gl_pSpD3D9Device->overlay->console->handle_text_input(wParam);
			}
			break;

		case WM_KEYUP:
			switch (wParam)
			{
				case VK_SHIFT:
					SpD3D9OInputHandler::get()->shift = false;
					break;
				case VK_CONTROL:
					SpD3D9OInputHandler::get()->ctrl = false;
					break;
				case VK_MENU: // ALT key
					SpD3D9OInputHandler::get()->alt = false;
					break;
				case VK_LWIN:
				case VK_RWIN:
					SpD3D9OInputHandler::get()->win = false;
					break;
			}
			break;


		case WM_SIZE:
			LOWORD(lParam); // Client width
			HIWORD(lParam); // Client height
			break;
		}

	return SpD3D9OInputHandler::get()->oDispatchMessage(lpmsg); // @TODO: Skip calling original function if message was handled?
}



/*DWORD WINAPI HookThread(LPVOID)
{
	DWORD dwDispatchMessageAddress = 0;
	#ifdef _SP_USE_DETOUR_DISPATCH_MSG_A_INPUT_
	while (!(dwDispatchMessageAddress = (DWORD)GetProcAddress(GetModuleHandle("User32.dll"), "DispatchMessageA")))
	#else
	while (!(dwDispatchMessageAddress = (DWORD)GetProcAddress(GetModuleHandle("User32.dll"), "DispatchMessageW")))
	#endif // _SP_USE_DETOUR_DISPATCH_MSG_A_INPUT_
	{
		Sleep(250);
	}

	_SP_HOOK_(DispatchMessage, dwDispatchMessageAddress);

	return 0;
}*/
#endif // _SP_USE_DETOUR_DISPATCH_MSG_INPUT_