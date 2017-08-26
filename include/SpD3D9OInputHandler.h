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
*/


#pragma once

#ifndef _SP_D3D9O_INPUT_HANDLER_H_
	#define _SP_D3D9O_INPUT_HANDLER_H_

// #define _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
#define _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
// #define _SP_USE_DETOUR_DISPATCH_MSG_INPUT_

// #define _SP_USE_ASYNC_KEY_STATE_INPUT_
// #define _SP_USE_DETOUR_DINPUT8_CREATE_DEVICE_INPUT_
// #define _SP_USE_GET_RAW_INPUT_DATA_INPUT_
// #define _SP_USE_WIN_HOOK_EX_INPUT_



#ifdef _SP_USE_DETOUR_DISPATCH_MSG_INPUT_
	#define _SP_USE_DETOUR_DISPATCH_MSG_A_INPUT_
	// #define _SP_USE_DETOUR_DISPATCH_MSG_W_INPUT_
#endif // _SP_USE_DETOUR_DISPATCH_MSG_INPUT_

#include "stdafx.h"

#ifdef _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
	#include "dinput.h"

	//#define _SP_DISABLE_DINPUT8_INPUT_ // If defined, normal dinput8 input will be disabled if the console is open


	#ifdef _SP_DISABLE_DINPUT8_INPUT_
		#define _SP_GETDEVICEDATA_ADDR_ NULL // Set this to the address of GetDeviceData
	#endif // _SP_DISABLE_DINPUT8_INPUT_ 

	#define _SP_DI8_KEY_DOWN_(kbe) (kbe->dwData & 0x80)
	#define _SP_DI8_KEY_UP_(kbe) (!(kbe->dwData & 0x80))
	#define _SP_DINPUT8_DEVICE_BUFFER_SIZE_ 10
#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_

#if (defined _SP_USE_DETOUR_DISPATCH_MSG_INPUT_ || defined _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_ || defined _SP_USE_DETOUR_DINPUT8_CREATE_DEVICE_INPUT_ || defined _SP_DISABLE_DINPUT8_INPUT_)
	#define _SP_USING_DETOURS_
#endif // _SP_USE_DETOUR_DISPATCH_MSG_INPUT_ || _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_ || _SP_USE_DETOUR_DINPUT8_CREATE_DEVICE_INPUT_ || _SP_DISABLE_DINPUT8_INPUT_


#ifdef _SP_USING_DETOURS_
	// #define _SP_HOOK_(func,addy) o##func = (t##func)DetourFunction((PBYTE)addy,(PBYTE)hk##func)
	#define _SP_HOOK_(func,addy) o##func = (t##func)DetourAttach((PVOID *)addy,(PBYTE)hk##func)
#endif // _SP_USING_DETOURS_


#ifdef _SP_DISABLE_DINPUT8_INPUT_
	HRESULT __fastcall hkGetDeviceData(void* pThis, DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags);
#endif // _SP_DISABLE_DINPUT8_INPUT_


#ifdef _SP_USE_DETOUR_DISPATCH_MSG_INPUT_
	typedef LRESULT(WINAPI *tDispatchMessage)(MSG* lpmsg);
#endif // _SP_USE_DETOUR_DISPATCH_MSG_INPUT_


#ifdef _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
	typedef UINT(WINAPI *tGetRawInputData)(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);
#endif // _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_


#ifdef _SP_USE_GET_RAW_INPUT_DATA_INPUT_
	#define SP_RAW_DEVICE_COUNT 2
	enum SP_RAW_INPUT_DEVICE_ENUM {
		SP_RAW_INPUT_KEYBOARD = 0,
		SP_RAW_INPUT_MOUSE = 1
	};
#endif // _SP_USE_GET_RAW_INPUT_DATA_INPUT_


extern SpD3D9Device *gl_pSpD3D9Device;



class SpD3D9OInputHandler
{
	public:
		static SpD3D9OInputHandler* get();
		

#ifdef _SP_USE_DETOUR_DISPATCH_MSG_INPUT_
		tDispatchMessage oDispatchMessage; // Original DispatchMessage function
#endif // _SP_USE_DETOUR_DISPATCH_MSG_INPUT_


#ifdef _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
		tGetRawInputData oGetRawInputData; // Original GetRawInputData function
#endif // _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_


#ifdef _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
		IDirectInput8 *dinput8 = NULL; // DirectInput Interface
		IDirectInputDevice8 *di8_keyboard = NULL; // Keyboard device
		DIDEVICEOBJECTDATA kb_data_buffer[_SP_DINPUT8_DEVICE_BUFFER_SIZE_]; // For getting keyboard buffered data
		void SpD3D9OInputHandler::get_dinput_data();
		void SpD3D9OInputHandler::flush_keyboard_input_buffer();
#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_


#if (defined _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_ || defined _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_)
		char convert_char[257]; // For getting character value of each keyboard button (when shift is not down)
		char convert_shift_char[257]; // For getting character value of each keyboard button (when shift is down)
		void SpD3D9OInputHandler::initialize_vk_to_char_conversion_array();
#endif // _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_ || _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_


#ifdef _SP_USE_GET_RAW_INPUT_DATA_INPUT_
		bool SpD3D9OInputHandler::initialize_devices();
		void SpD3D9OInputHandler::get_device_input();
#endif // _SP_USE_GET_RAW_INPUT_DATA_INPUT_

		bool disable_game_input = false; // If true, blocks player input from game
		bool handled = false; // Current input message has been handled
		
		// Keyboard
		bool shift = false; // Shift is currently held down
		bool ctrl = false; // Ctrl is currently held down
		bool alt = false; // alt is currently held down
		bool win = false; // Windows key is currently held down
		bool capslock = false; // Caplock is currently on

		// Mouse
		POINT cursor_position = { 100, 100 }; // Mouse cursor position
		bool mouse_button_down[5] = { false, false, false, false, false }; // Mouse button is currently down { Left click, right click, middle click, button 4, button 5 }

		// Destructor
		~SpD3D9OInputHandler();
	private:
		static SpD3D9OInputHandler *instance; // Sole instance of this singleton class
		SpD3D9OInputHandler(); // Private constructor because this is a singleton class
};


#ifdef _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_
	UINT WINAPI hkGetRawInputData(HRAWINPUT hRawInput, UINT uiCommand, LPVOID pData, PUINT pcbSize, UINT cbSizeHeader);
#endif // _SP_USE_DETOUR_GET_RAW_INPUT_DATA_INPUT_


#ifdef _SP_USE_WIN_HOOK_EX_INPUT_
	extern HHOOK input_hook;
	LRESULT __stdcall handle_message(int nCode, WPARAM wParam, LPARAM lParam); // Callback function
#endif // _SP_USE_WIN_HOOK_EX_INPUT_



#ifdef _SP_USE_DETOUR_DISPATCH_MSG_INPUT_
	LRESULT WINAPI hkDispatchMessage(MSG* lpmsg);
	//DWORD WINAPI HookThread(LPVOID);
#endif // _SP_USE_DETOUR_DISPATCH_MSG_INPUT_




#endif // _SP_D3D9O_INPUT_HANDLER_H_