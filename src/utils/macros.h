#ifndef MACROS_H_
#define MACROS_H_

#include <stdlib.h>
#include <memory.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define FREE(ptr) do {                \
                      free(ptr);      \
                      ptr = nullptr;  \
                  } while (0)

inline void* recalloc(void* ptr, size_t old_size, size_t new_size) {
    ptr = realloc(ptr, new_size);

    if (!ptr)
        return ptr;

    if (new_size > old_size)
        memset((char*)ptr + old_size, 0, new_size - old_size);

    return ptr;
}

inline void fill(void* array, size_t len, const void* elem, const size_t elem_size) {
    while (len--) {
        memcpy(array, elem, elem_size);
        array = (char*)array + elem_size;
    }
}

struct VarCodeData {
    const char* name = "Not specified";
    const char* file = "Not specified";
          int   line = -1;
    const char* func = "Not specified";
};

#define VAR_CODE_DATA(name) {# name, __FILE__, __LINE__, __func__}
#define VAR_CODE_DATA_PTR(name) {(# name) + 1, __FILE__, __LINE__, __func__}

#endif // #ifndef MACROS_H_
