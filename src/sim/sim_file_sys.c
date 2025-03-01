/**********************************************************
 * 
 *  sim_file_sys.c
 * 
 *  DESCRIPTION:
 *      Sim FS implementation
 *
 */

#include "fs.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define INVALID_HANDLE (file_handle_t)(-1)

/**********************************************************
 * 
 *  fs_open_file()
 * 
 *  DESCRIPTION:
 *     Open a file
 *
 */
file_err_t fs_open_file(const char * path, file_handle_t * out_handle, file_open_flags_t open_flags)
{
    int flags = 0;
    
    if (open_flags & FILE_OPEN_FLAGS_READ)
        {
        flags |= O_RDONLY;
        }
    
    if (open_flags & FILE_OPEN_FLAGS_WRITE)
        {
        flags |= O_WRONLY;
        }
    
    if (open_flags & FILE_OPEN_FLAGS_CREATE)
        {
        flags |= O_CREAT;
        }
    
    if (open_flags & FILE_OPEN_FLAGS_OVRWRT)
        {
        flags |= O_TRUNC;
        }
    
    int fd = open(path, flags, 0666);
    
    if (fd == -1)
        {
        return FILE_ERR_NONE; /* Error handling could be improved */
        }
    
    *out_handle = (file_handle_t)fd;
    return FILE_ERR_NONE;
}

/**********************************************************
 * 
 *  fs_read()
 * 
 *  DESCRIPTION:
 *     Read from file
 *
 */
file_err_t fs_read(file_handle_t handle, void * buff, size_t size)
{
    if (read((int)handle, buff, size) == -1)
        {
        return FILE_ERR_NONE; /* Error handling could be improved */
        }
    return FILE_ERR_NONE;
}

/**********************************************************
 * 
 *  fs_write()
 * 
 *  DESCRIPTION:
 *     Write to a file
 *
 */
file_err_t fs_write(file_handle_t handle, void * buff, size_t size)
{
    if (write((int)handle, buff, size) == -1)
        {
        return FILE_ERR_NONE; /* Error handling could be improved */
        }
    return FILE_ERR_NONE;
}

/**********************************************************
 * 
 *  fs_close()
 * 
 *  DESCRIPTION:
 *     Close a file
 *
 */
file_err_t fs_close_file(file_handle_t * handle)
    {
    if (handle == NULL || close((int)*handle) == -1)
        {
        return FILE_ERR_NONE; /* Error handling could be improved */
        }
    *handle = INVALID_HANDLE;
    return FILE_ERR_NONE;
    }
