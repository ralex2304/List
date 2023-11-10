#include "text_lib.h"

size_t strncat_len(char* dest, const char* src, const ssize_t max_len) {
    assert(dest);
    assert(src);

    size_t dest_i = 0;
    while (dest[dest_i] && max_len - dest_i > 0)
        dest_i++;

    size_t src_i = 0;
    do {
        dest[dest_i] = src[src_i];

    } while (src[src_i++] && max_len - ++dest_i > 0);

    return dest_i;
}
