#include "list_dot_log.h"

bool list_dot_log_create_img(const char* input_filename, const char* output_filename) {
    assert(input_filename);
    assert(output_filename);

    static const size_t MAX_COMMAND_LEN = 256;
    char command[MAX_COMMAND_LEN] = {};

#ifdef _WIN32
    snprintf(command, MAX_COMMAND_LEN, "type %s | iconv -f cp1251 -t utf-8 | dot -Tsvg -o %s",
                                             input_filename,                   output_filename);
#else //< #ifndef _WIN32
    snprintf(command, MAX_COMMAND_LEN, "dot -Tsvg %s > %s", input_filename, output_filename);
#endif //< #ifdef _WIN32

    if (system(command) != 0) {
        fprintf(stderr, "Error executing \"%s\"\n", command);
        return false;
    }

    return true;
}
