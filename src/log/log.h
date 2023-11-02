#ifndef LOG_H_
#define LOG_H_

#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Log file data struct
 *
 * @attention You may use LogFileData as global var
 */
struct LogFileData {
    const char* filename = nullptr;
    FILE* file = nullptr;
};

/**
 * @brief Prints data in printf format to log file
 *
 * @param log
 * @param format
 * @param ...
 * @return int
 */
int log_printf(LogFileData* log, const char* format, ...);

/**
 * @brief Opens log file
 *
 * @param log
 * @param mode fopen() mode
 * @return true success
 * @return false failure
 */
bool log_open_file(LogFileData* log, const char* mode = "ab");

/**
 * @brief Closes log fle
 *
 * @param log
 * @return true success
 * @return false failure
 */
bool log_close_file(LogFileData* log);

#endif // #ifndef LOG_H_
