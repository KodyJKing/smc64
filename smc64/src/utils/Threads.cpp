// #include <Windows.h>
// #include <TlHelp32.h>
// #include <vector>
// #include <mutex>
// #include "Threads.hpp"

// namespace Threads {

//     std::vector<DWORD> frozenThreads;
//     std::mutex freezeMutex;

//     // Freeze other threads, but lets them run until they exit the specified range.
//     void freezeOthersEx(uint64_t start, uint64_t end) {
//         std::lock_guard<std::mutex> lock( freezeMutex );

//         if (frozenThreads.size() > 0) 
//             return; // Already frozen.

//         DWORD currentThreadId = GetCurrentThreadId();
//         frozenThreads.clear();
//         DWORD processId = GetCurrentProcessId();
//         HANDLE snapshot = CreateToolhelp32Snapshot( TH32CS_SNAPTHREAD, processId );
//         if (snapshot == INVALID_HANDLE_VALUE) return;
//         THREADENTRY32 threadEntry;
//         threadEntry.dwSize = sizeof(THREADENTRY32);
//         if (Thread32First( snapshot, &threadEntry )) {
//             do {
//                 if (threadEntry.th32OwnerProcessID == processId && threadEntry.th32ThreadID != currentThreadId) {
//                     HANDLE thread = OpenThread( THREAD_SUSPEND_RESUME, FALSE, threadEntry.th32ThreadID );
//                     if (thread) {
//                         while (true) {
//                             SuspendThread( thread );
//                             CONTEXT context;
//                             context.ContextFlags = CONTEXT_CONTROL;
//                             GetThreadContext( thread, &context );
//                             if (context.Rip < start || context.Rip >= end)
//                                 break;
//                             ResumeThread( thread );
//                             Sleep(1);
//                         }
//                         CloseHandle( thread );
//                         frozenThreads.push_back( threadEntry.th32ThreadID );
//                     }
//                 }
//             } while (Thread32Next( snapshot, &threadEntry ));
//         }
//         CloseHandle( snapshot );
//     }


//     void freezeOthers() {
//         freezeOthersEx(0, 0);
//     }

//     void unfreeze() {
//         std::lock_guard<std::mutex> lock( freezeMutex );

//         for (DWORD threadId : frozenThreads) {
//             HANDLE thread = OpenThread( THREAD_SUSPEND_RESUME, FALSE, threadId );
//             if (thread) {
//                 ResumeThread( thread );
//                 CloseHandle( thread );
//             }
//         }
//         frozenThreads.clear();
//     }

// }