// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <windows.h>
#include <psapi.h> // For GetModuleInformation
#include "MinHook.h" // Include MinHook header

// Declare the original function pointer
typedef void(*NoPmethod_t)();
NoPmethod_t originalNoPmethod = nullptr;

// Our custom function that will replace NoPmethod
void HookedNoPmethod()
{
    // Custom functionality or NOP out the method
    MessageBoxA(NULL, "NoPmethod has been called and patched!", "Info", MB_OK);

    // Optionally call the original function
    // originalNoPmethod(); // Uncomment if you need to call the original function
}

// Function to get the base address of GameAssembly.dll
HMODULE GetGameAssemblyModule()
{
    HMODULE hMods[1024];
    HANDLE hProcess = GetCurrentProcess();
    DWORD cbNeeded;

    if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
    {
        for (unsigned int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
        {
            char szModName[MAX_PATH];
            if (GetModuleBaseNameA(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(char)))
            {
                if (strcmp(szModName, "GameAssembly.dll") == 0)
                {
                    return hMods[i]; // Return the base address of GameAssembly.dll
                }
            }
        }
    }
    return NULL; // GameAssembly.dll not found
}

// DLL entry point
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        // Get the base address of GameAssembly.dll
        HMODULE hGameAssembly = GetGameAssemblyModule();
        if (!hGameAssembly)
        {
            MessageBoxA(NULL, "Failed to find GameAssembly.dll", "Error", MB_OK);
            return FALSE;
        }

        // Calculate the target function's address using the base address + offset
        // Replace 0xOFFSET with the actual offset from GameAssembly.dll to NoPmethod
        uintptr_t targetAddress = (uintptr_t)hGameAssembly + 0x123456; // Use the actual offset here

        // Initialize MinHook
        if (MH_Initialize() != MH_OK)
        {
            return FALSE;
        }

        // Create a hook for NoPmethod
        if (MH_CreateHook((LPVOID)targetAddress, &HookedNoPmethod, reinterpret_cast<LPVOID*>(&originalNoPmethod)) != MH_OK)
        {
            return FALSE;
        }

        // Enable the hook
        if (MH_EnableHook((LPVOID)targetAddress) != MH_OK)
        {
            return FALSE;
        }

        break;
    }
    case DLL_PROCESS_DETACH:
    {
        // Disable and uninitialize MinHook when the DLL is unloaded
        HMODULE hGameAssembly = GetGameAssemblyModule();
        if (hGameAssembly)
        {
            uintptr_t targetAddress = (uintptr_t)hGameAssembly + 0x123456; // Use the same offset here
            MH_DisableHook((LPVOID)targetAddress);
        }
        MH_Uninitialize();
        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }
    return TRUE;
}
