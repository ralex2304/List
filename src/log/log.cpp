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

    char filename[LogFileData::MAX_FILENAME_LEN] = {};

    if (snprintf(filename, LogFileData::MAX_FILENAME_LEN, "%slog.html", log_file->timestamp_dir) <= 0)
        return false;

    log_file->file = fopen(filename, mode);

    if (log_file->file == nullptr) {
        perror("Error opening log_file file");
        return false;
    }

    return true;
}

bool log_create_timestamp_dir(LogFileData* log_file) {
    assert(log_file);

    time_t ltime = time(NULL);

    if (log_file->timestamp_dir[0] == '\0' || ltime - log_file->last_write > log_file->LIFETIME) {

        tm* tm = localtime(&ltime);

        log_file->timestamp_dir[0] = '\0';

        if (snprintf(log_file->timestamp_dir, log_file->MAX_FILENAME_LEN,
                 "%s" UNIX("/") WIN("\\") "%02d-%02d-%04d_%02d-%02d-%02d" UNIX("/") WIN("\\"),
                 log_file->dir, tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900,
                                tm->tm_hour, tm->tm_min,     tm->tm_sec) <= 0)
            return false;
    }

    log_file->last_write = ltime;

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
    assert(dir_name);

    DIR* dir = opendir(dir_name);

    if (dir) {
        closedir(dir);
    } else if (ENOENT == errno) {
#ifdef _WIN32
        if (CreateDirectoryA(dir_name, NULL) == 0) {
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
