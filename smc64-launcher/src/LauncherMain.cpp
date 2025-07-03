#include <Windows.h>
#include <tlhelp32.h>
#include <iostream>
#include <string>
#include <filesystem>

#define PROCESS_NAME L"MCC-Win64-Shipping.exe"
#define MIN_UPTIME 2000

DWORD findProcessId(const wchar_t* processName) {
    DWORD pid = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe;
        pe.dwSize = sizeof(pe);
        if (Process32First(hSnap, &pe)) {
            do {
                if (wcscmp(pe.szExeFile, processName) == 0) {
                    pid = pe.th32ProcessID;
                    break;
                }
            } while (Process32Next(hSnap, &pe));
        }
        CloseHandle(hSnap);
    }
    return pid;
}

void injectDll(DWORD pid, const char* dllPath) {
    SIZE_T dllPathSize = strlen(dllPath) + 1;
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    if (hProcess) {
        LPVOID dllPathAddress = VirtualAllocEx(hProcess, NULL, dllPathSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (dllPathAddress) {
            if (WriteProcessMemory(hProcess, dllPathAddress, dllPath, dllPathSize, NULL)) {
                HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, dllPathAddress, 0, NULL);
                if (hThread) {
                    WaitForSingleObject(hThread, INFINITE);
                    CloseHandle(hThread);
                } else {
                    std::cerr << "Failed to create remote thread. Error code: " << GetLastError() << std::endl;
                }
                VirtualFreeEx(hProcess, dllPathAddress, 0, MEM_RELEASE);
            } else {
                std::cerr << "Failed to write process memory. Error code: " << GetLastError() << std::endl;
            }
        } else {
            std::cerr << "Failed to allocate memory in the remote process. Error code: " << GetLastError() << std::endl;
        }
        CloseHandle(hProcess);
    } else {
        std::cerr << "Failed to open the process. Error code: " << GetLastError() << std::endl;
    }
}

uint64_t getProcessUpTime(DWORD pid) {
    HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
    if (!hProc) {
        CloseHandle(hProc);
        return 0;
    }

    FILETIME creationTime, exitTime, kernelTime, userTime;
    if (!GetProcessTimes(hProc, &creationTime, &exitTime, &kernelTime, &userTime)) {
        CloseHandle(hProc);
        return 0;
    }

    CloseHandle(hProc);

    SYSTEMTIME creationSysTime;
    FileTimeToSystemTime(&creationTime, &creationSysTime);

    SYSTEMTIME nowSysTime;
    GetSystemTime(&nowSysTime);

    FILETIME creationFileTime, nowFileTime;
    SystemTimeToFileTime(&creationSysTime, &creationFileTime);
    SystemTimeToFileTime(&nowSysTime, &nowFileTime);

    ULARGE_INTEGER creationULarge, nowULarge;
    creationULarge.LowPart = creationFileTime.dwLowDateTime;
    creationULarge.HighPart = creationFileTime.dwHighDateTime;
    nowULarge.LowPart = nowFileTime.dwLowDateTime;
    nowULarge.HighPart = nowFileTime.dwHighDateTime;

    ULARGE_INTEGER diff;
    diff.QuadPart = nowULarge.QuadPart - creationULarge.QuadPart;

    return diff.QuadPart / 10000;
}

std::filesystem::path getModDllPath() {
    char modulePath[MAX_PATH];
    GetModuleFileNameA(NULL, modulePath, MAX_PATH);
    std::filesystem::path moduleDir = std::filesystem::path(modulePath);
    std::filesystem::path binPath = moduleDir.parent_path().parent_path();
    std::filesystem::path modDllPath = binPath / "smc64" / "smc64.dll";
    return modDllPath;
}

int main() {

    DWORD pid = findProcessId(PROCESS_NAME);
    if (pid == 0) {
        std::cerr << "Failed to find process id." << std::endl;
        return 1;
    }

    std::filesystem::path dllPath = getModDllPath();
    std::cout << "Injecting mod dll: " << dllPath << std::endl;
    if (!std::filesystem::exists(dllPath)) {
        std::cerr << "Mod dll not found." << std::endl;
        return 1;
    }

    uint64_t uptime = getProcessUpTime(pid);
    if (uptime < MIN_UPTIME) {
        std::cout << "Waiting for process to start up..." << std::endl;
        Sleep(MIN_UPTIME - (DWORD)uptime);
    }

    injectDll(pid, dllPath.string().c_str());
    
    return 0;
}