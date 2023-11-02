#ifndef PTR_VALID_H_
#define PTR_VALID_H_

#include <assert.h>
#include <stdio.h>

#ifdef LINUX_MANUAL_PTR_VALIDATION

#include <stdio.h>
#include <inttypes.h>

#else // #ifndef LINUX_MANUAL_PTR_VALIDATION

#ifdef _WIN32

#include <io.h>
#include <winnt.h>
#include <memoryapi.h>

#else // #ifndef _WIN32

#if defined(unix) || defined(__APPLE__)

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#endif // #ifdef unix || __APPLE__

#endif // #ifdef _WIN32

#endif // #ifdef LINUX_MANUAL_PTR_VALIDATION


/**
 * @brief Checks if pointer is valid for access in mode
 *
 * @param p pointer
 * @return true is valid
 * @return false is not valid
 */
bool is_ptr_valid(void* p);

#endif /// #ifndef PTR_VALID_H_
