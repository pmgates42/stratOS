/**********************************************************
 * 
 *  fs.h
 * 
 *  DESCRIPTION:
 *      File system contract header
 *
 */

#pragma once

#include "generic.h"

typedef uint32_t file_handle_t;

typedef uint8_t file_err_t;
enum
    {
    FILE_ERR_NONE
    };

/**********************************************************
 * 
 *  File Open Flags
 * 
 *  FILE_OPEN_FLAGS_READ
 *      Open file for reading.
 *
 *  FILE_OPEN_FLAGS_WRITE
 *      Open file for writing.
 *
 *  FILE_OPEN_FLAGS_CREATE
 *      Create file if it does not exist.
 *
 *  FILE_OPEN_FLAGS_OVRWRT
 *      Don't amend to the file, overwrite existing contents.
 *
 */

typedef uint8_t file_open_flags_t;
enum
    {
    FILE_OPEN_FLAGS_READ      = BIT(0),
    FILE_OPEN_FLAGS_WRITE     = BIT(1),
    FILE_OPEN_FLAGS_CREATE    = BIT(2),
    FILE_OPEN_FLAGS_OVRWRT    = BIT(3),
    };

/**********************************************************
 * 
 *  fs_open_file()
 * 
 *  DESCRIPTION:
 *     Open a file
 *
 */

file_err_t fs_open_file(const char * path, file_handle_t * out_handle, file_open_flags_t open_flags);

/**********************************************************
 * 
 *  fs_close_file()
 * 
 *  DESCRIPTION:
 *     Open a file
 *
 */

file_err_t fs_close_file(file_handle_t * out_handle);

/**********************************************************
 * 
 *  fs_read()
 * 
 *  DESCRIPTION:
 *     Read from file
 *
 */

file_err_t fs_read(file_handle_t handle, void * buff, size_t size);

/**********************************************************
 * 
 *  fs_write()
 * 
 *  DESCRIPTION:
 *     Write to a file
 *
 */

file_err_t fs_write(file_handle_t handle, void * buff, size_t size);

/**********************************************************
 * 
 *  fs_write()
 * 
 *  DESCRIPTION:
 *     Write formatted text to a file, like printf.
 *
 */

file_err_t fs_writef(file_handle_t handle, const char * format, ...);

/**********************************************************
 * 
 *  fs_readf()
 * 
 *  DESCRIPTION:
 *     Read formatted text from a file, like sscanf.
 *
 */

file_err_t fs_readf(file_handle_t handle, const char * format, ...);
