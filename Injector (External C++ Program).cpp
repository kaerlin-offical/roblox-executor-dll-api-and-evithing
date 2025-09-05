#include <Windows.h>
#include <TlHelp32.h>

DWORD GetRobloxPID() {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);
    if (Process32First(snapshot, &pe32)) {
        do {
            if (strcmp(pe32.szExeFile, "RobloxPlayerBeta.exe") == 0) {
                CloseHandle(snapshot);
                return pe32.th32ProcessID;
            }
        } while (Process32Next(snapshot, &pe32));
    }
    CloseHandle(snapshot);
    return 0;
}

int main() {
    DWORD robloxPID = GetRobloxPID();
    if (!robloxPID) {
        MessageBoxA(NULL, "Roblox not found!", "Error", MB_ICONERROR);
        return 1;
    }

    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, robloxPID);
    if (!hProcess) {
        MessageBoxA(NULL, "Failed to open Roblox process!", "Error", MB_ICONERROR);
        return 1;
    }

    // Load the DLL into Roblox
    char dllPath[MAX_PATH];
    GetFullPathNameA("YourExecutor.dll", MAX_PATH, dllPath, NULL);
    LPVOID remoteMem = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    WriteProcessMemory(hProcess, remoteMem, dllPath, strlen(dllPath) + 1, NULL);
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, remoteMem, 0, NULL);

    if (!hThread) {
        MessageBoxA(NULL, "Failed to inject DLL!", "Error", MB_ICONERROR);
        return 1;
    }

    CloseHandle(hProcess);
    MessageBoxA(NULL, "Executor injected! Press OK to execute a test script.", "Success", MB_OK);

    // Example: Execute a test script
    typedef void(*ExecuteScriptFunc)(const char*);
    HANDLE hDll = LoadLibraryA("YourExecutor.dll");
    ExecuteScriptFunc execute = (ExecuteScriptFunc)GetProcAddress(hDll, "ExecuteScript");
    execute("game.Players.LocalPlayer.Character.Humanoid.Health = 99999");

    return 0;
}
