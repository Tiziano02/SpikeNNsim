#include "Utility.hpp"

#include <iostream>
#include <algorithm> // per std::min e std::max

// Includi gli header di sistema corretti in base all'OS
#if defined(_WIN32)
#include <windows.h>
#elif defined(__APPLE__)
#include <unistd.h>
#include <mach/mach.h>
#elif defined(__linux__)
#include <unistd.h>
#endif

/*
 * Restituisce la RAM attualmente DISPONIBILE in byte.
 */
size_t getAvailableRAM() {
#if defined(_WIN32)
    // Implementazione per WINDOWS
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    return static_cast<size_t>(status.ullAvailPhys);

#elif defined(__APPLE__)
    // Implementazione per macOS
    mach_msg_type_number_t count = HOST_VM_INFO_COUNT;
    vm_statistics_data_t vmstat;
    if (host_statistics(mach_host_self(), HOST_VM_INFO, (host_info_t)&vmstat, &count) == KERN_SUCCESS) {
        size_t page_size = sysconf(_SC_PAGE_SIZE);
        // RAM libera + inattiva (disponibile per nuove allocazioni)
        return static_cast<size_t>(vmstat.free_count + vmstat.inactive_count) * page_size;
    }
    return 512ULL * 1024 * 1024; // Fallback di sicurezza in caso di errore

#elif defined(__linux__)
    // Implementazione per LINUX
    long pages = sysconf(_SC_AVPHYS_PAGES);
    long page_size = sysconf(_SC_PAGE_SIZE);
    if (pages > 0 && page_size > 0) {
        return static_cast<size_t>(pages) * static_cast<size_t>(page_size);
    }
    return 512ULL * 1024 * 1024; // Fallback di sicurezza

#else
    // Fallback per altri OS sconosciuti
    return 512ULL * 1024 * 1024; // 512 MB
#endif
}