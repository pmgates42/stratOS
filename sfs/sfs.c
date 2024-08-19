/**********************************************************
 * 
 *  sfs.c
 * 
 *  DESCRIPTION:
 *     Simple File System (SFS)
 *
 */

#include "fs.h"
#include "generic.h"

/**********************************************************
 * 
 *  fs_open_file()
 * 
 *  DESCRIPTION:
 *     Open a file. Contracted function.
 *
 */

file_err_t fs_open_file(const char * path, file_handle_t * out_handle, file_open_flags_t open_flags)
{
    return FILE_ERR_NONE;
}

/**********************************************************
 * 
 *  fs_read()
 * 
 *  DESCRIPTION:
 *     Read from file. Contracted function.
 *
 */

file_err_t fs_read(file_handle_t handle, void * buff, size_t size)
{
    return FILE_ERR_NONE;
}

/**********************************************************
 * 
 *  fs_write()
 * 
 *  DESCRIPTION:
 *     Write to a file. Contracted function.
 *
 */

file_err_t fs_write(file_handle_t handle, void * buff, size_t size)
{
    return FILE_ERR_NONE;
}
