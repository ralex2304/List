#ifndef LIST_LOG_H_
#define LIST_LOG_H_

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32

#define WIN(...) __VA_ARGS__

#define UNIX(...)

#include<windows.h>
#else

#define WIN(...)

#define UNIX(...) __VA_ARGS__

#endif

/**
 * @brief Log file data struct
 *
 * @attention You may use LogFileData as global var
 */
struct ListLogFileData {
    // if more than LIFETIME seconds have passed since the last write, a new file will be created
    static const long LIFETIME = 1;

    const char* dir = nullptr;
    FILE* file = nullptr;

    static const size_t MAX_FILENAME_LEN = 256;

    char timestamp_dir[MAX_FILENAME_LEN] = {};
    time_t last_write = 0;
};

/**
 * @brief Prints data in printf format to log file
 *
 * @param log
 * @param format
 * @param ...
 * @return int
 */
int list_log_printf(ListLogFileData* log, const char* format, ...);

/**
 * @brief Opens log file
 *
 * @param log
 * @param mode fopen() mode
 * @return true success
 * @return false failure
 */
bool list_log_open_file(ListLogFileData* log, const char* mode = "ab");

/**
 * @brief Closes log fle
 *
 * @param log
 * @return true success
 * @return false failure
 */
bool list_log_close_file(ListLogFileData* log);

/**
 * @brief Searches for dir by dir_name. If it doesn't exist, creates
 *
 * @param dir_name
 * @return true
 * @return false
 */
bool list_log_create_dir(const char* dir_name);

/**
 * @brief Creates dir woth timestamp in log_file.dir. Saves it's name to log_file.timestamp_dir
 *
 * @param log_file
 * @return true
 * @return false
 */
bool list_log_create_timestamp_dir(ListLogFileData* log_file);

#endif // #ifndef LIST_LOG_H_
