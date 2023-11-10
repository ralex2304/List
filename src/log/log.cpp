#include "log.h"

int log_printf(LogFileData* log_file, const char* format, ...) {
    assert(log_file);
    assert(format);

    bool file_is_opened_here = false;
    if (log_file->file == nullptr) {
        if (!log_open_file(log_file))
            return -1;
        file_is_opened_here = true;
    }

    va_list arg_list = {};
    va_start(arg_list, format);

    int ret = vfprintf(log_file->file, format, arg_list);

    if (file_is_opened_here)
        if (!log_close_file(log_file))
            return -1;

    return ret;
}

bool log_open_file(LogFileData* log_file, const char* mode) {
    assert(log_file);

    if (!log_create_dir(log_file->dir))
        return false;

    if (!log_create_timestamp_dir(log_file))
        return false;

    char filename[log_file->MAX_FILENAME_LEN] = {};

    strncat_len(filename, log_file->timestamp_dir, log_file->MAX_FILENAME_LEN);
    strncat_len(filename, "log.html", log_file->MAX_FILENAME_LEN);

    log_file->file = fopen(filename, mode);

    if (log_file->file == nullptr) {
        perror("Error opening log_file file");
        return false;
    }

    return true;
}

bool log_create_timestamp_dir(LogFileData* log_file) {
    assert(log_file);

    if (log_file->timestamp_dir[0] == '\0') {
        time_t ltime = time(NULL);
        tm* tm = localtime(&ltime);

        size_t str_len = strncat_len(log_file->timestamp_dir, log_file->dir, log_file->MAX_FILENAME_LEN);
        str_len = strncat_len(log_file->timestamp_dir, "/", log_file->MAX_FILENAME_LEN);

        snprintf(log_file->timestamp_dir + str_len, log_file->MAX_FILENAME_LEN - str_len,
                "%02d-%02d-%04d_%02d-%02d-%02d", tm->tm_mday, tm->tm_mon, tm->tm_year + 1900,
                                                 tm->tm_hour, tm->tm_min, tm->tm_sec);
        str_len = strncat_len(log_file->timestamp_dir, "/", log_file->MAX_FILENAME_LEN);
    }

    if (!log_create_dir(log_file->timestamp_dir))
        return false;

    return true;
}

bool log_close_file(LogFileData* log_file) {
    assert(log_file);

    if (log_file->file == nullptr || fclose(log_file->file) != 0) {
        perror("Error closing log_file file");
        return false;
    }

    log_file->file = nullptr;
    return true;
}

bool log_create_dir(const char* dir_name) {
    DIR* dir = opendir(dir_name);
    if (dir) {
        closedir(dir);
    } else if (ENOENT == errno) {
#ifdef _WIN32
        if (CreateDirectory(dir_name, NULL) != 0) {
            perror("Error creating dir");
            return false;
        }
#else //< #ifndef _WIN32
        if (mkdir(dir_name, 0744) != 0) {
            perror("Error creating dir");
            return false;
        }
#endif //< #ifdef _WIN32
    } else {
        perror("Error detecting directory");
        return false;
    }
    return true;
}
