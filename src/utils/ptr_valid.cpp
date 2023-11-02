#include "ptr_valid.h"

#ifdef LINUX_MANUAL_PTR_VALIDATION

bool is_ptr_valid(void* p) {
    uintptr_t begin = 0;
    uintptr_t end = 0;

    char readable   = '\0';
    char writable   = '\0';
    char executable = '\0';
    char mapped     = '\0';

    FILE* file = fopen("/proc/self/maps", "r");

    if (!file)
        return false;

    while (fscanf(file, "%" SCNxPTR "-%" SCNxPTR " %c%c%c%c",
            &begin, &end, &readable, &writable, &executable, &mapped) == 6) {

        if (begin <= (uintptr_t)p && (uintptr_t)p < end) {
            fclose(file);

            return writable == 'w';
        }

        int c = '\0';
        do {
            c = fgetc(file);
        } while (c != '\n' && c != EOF);
    }

    fclose(file);

    return false;

}
#else // #ifndef LINUX_MANUAL_PTR_VALIDATION

#ifdef _WIN32

bool is_ptr_valid(void* p) {
    // Thanks to God-blessed library "TxLib.h"

    MEMORY_BASIC_INFORMATION mbi = {};
    if (!VirtualQuery (p, &mbi, sizeof (mbi))) return true;

    if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))  return true;  // Guard page -> bad ptr

    DWORD readRights = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY | PAGE_EXECUTE_READ
                                     | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

    return !(mbi.Protect & readRights);
}

#else // #ifndef _WIN32

#if defined(unix) || defined(__APPLE__)


bool is_ptr_valid(void* p) {
    char filename[] = "/tmp/kurwa_ptr.XXXXXX";
    int file = mkstemp(filename);

    if (file == -1) {
        perror("");
        assert(0 && "Error opening tmp file");
        return false;
    }

    bool ret = false;

    ssize_t res = write(file, p, 1);

    if (res == 1) {
        ret = true;
    } else if (errno == EFAULT) {
        ret = false;
    } else {
        assert(0 && "Error writing to tmp file");
    }

    if (close(file) == -1) {
        assert(0 && "Error closing tmp file");
        ret = false;
    }

    return ret;
}

#endif // #ifdef unix || __APPLE__

#endif // #ifdef _WIN32

#endif // #ifdef LINUX_MANUAL_PTR_VALIDATION


