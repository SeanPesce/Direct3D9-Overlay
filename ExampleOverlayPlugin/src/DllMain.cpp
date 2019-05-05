// Entry point for the DLL plugin

#include "ExampleOverlayPlugin.h"


// Functions from ExampleOverlayPlugin.cpp
DWORD WINAPI on_process_attach_async(LPVOID lpParam);
void on_process_attach();
void on_process_detach();


DWORD  plugin_thread_id;
HANDLE plugin_thread_handle;


BOOL APIENTRY DllMain(HMODULE h_module,
                      DWORD   ul_reason_for_call,
                      LPVOID  lp_reserved)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            // Create a new thread to run code without halting the library-loading thread:
            plugin_thread_handle = CreateThread(
                NULL,				// Default security attributes
                0,					// Use default stack size
                on_process_attach_async, // Thread function name
                NULL,				// Argument to thread function
                0,					// Use default creation flags
                &plugin_thread_id);	// Returns the thread identifier

            // Run important startup code that must be executed before the game loads:
            on_process_attach();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;

        case DLL_PROCESS_DETACH:
            on_process_detach();
            break;
    }
    return TRUE;
}

