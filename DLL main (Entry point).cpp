#include <Windows.h>
#include <MinHook.h>
#include <lua.hpp>
#include <lauxlib.h>
#include <lualib.h>

// Global Lua state pointer
lua_State* g_LuaState = nullptr;

// Original luaL_dostring function
typedef int(__cdecl* luaL_dostring_t)(lua_State*, const char*);
luaL_dostring_t OriginalLuaLDostring = nullptr;

// Detour for luaL_dostring
int __cdecl HookedLuaLDostring(lua_State* L, const char* script) {
    if (strstr(script, "script injection check")) {
        // Modify or block scripts here
        MessageBoxA(NULL, "Script intercepted!", "Executor", MB_OK);
    }
    return OriginalLuaLDostring(L, script);
}

// Find Roblox's Lua state
bool FindLuaState() {
    // Signature scan for lua_State* (example pattern for x64)
    // Use Cheat Engine to find the correct offset for your Roblox version
    uintptr_t luaStateAddr = 0;
    MEMORY_BASIC_INFORMATION mbi;
    for (uintptr_t addr = 0; addr < 0x7FFFFFFFFFFF; addr = (uintptr_t)mbi.BaseAddress + mbi.RegionSize) {
        if (!VirtualQuery((LPCVOID)addr, &mbi, sizeof(mbi))) continue;
        if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS) continue;

        // Example: Scan for "lua_newthread" or "luaL_newstate"
        // Replace with actual signature for your Roblox version
        if (memcmp((void*)addr, "\x48\x8D\x05\xCC\xCC\xCC\xCC\x48\x8B\xD9", 10) == 0) {
            luaStateAddr = addr + *(int32_t*)(addr + 3) + 7;
            g_LuaState = *(lua_State**)(luaStateAddr);
            return true;
        }
    }
    return false;
}

// Execute a Lua script in Roblox
__declspec(dllexport) void ExecuteScript(const char* script) {
    if (!g_LuaState) return;
    OriginalLuaLDostring(g_LuaState, script);
}

// DLL Entry
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        MH_Initialize();
        uintptr_t luaLDostringAddr = (uintptr_t)GetProcAddress(GetModuleHandleA("lua5.1.dll"), "luaL_dostring");
        MH_CreateHook((LPVOID)luaLDostringAddr, &HookedLuaLDostring, (LPVOID*)&OriginalLuaLDostring);
        MH_EnableHook((LPVOID)luaLDostringAddr);

        if (!FindLuaState()) {
            MessageBoxA(NULL, "Failed to find Lua state!", "Error", MB_ICONERROR);
            return FALSE;
        }
    }
    return TRUE;
}
