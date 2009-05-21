/*
 * Copyright (c) 2009, Christian Ferrari
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the names of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include <config.h>



#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#endif



#include <lixa_errors.h>
#include <lixa_trace.h>
#include <server_status.h>



/* set module trace flag */
#ifdef LIXA_TRACE_MODULE
# undef LIXA_TRACE_MODULE
#endif /* LIXA_TRACE_MODULE */
#define LIXA_TRACE_MODULE   LIXA_TRACE_MOD_SERVER_STATUS



int status_record_load(union status_record_u **sr,
                       const char *status_file)
{
    enum Exception { OPEN_ERROR1
                     , OPEN_ERROR2
                     , WRITE_ERROR
                     , FSTAT_ERROR
                     , MMAP_ERROR
                     , CLOSE_ERROR
                     , NONE } excp;
    int ret_cod = LIXA_RC_INTERNAL_ERROR;

    int fd = LIXA_NULL_FD;
    struct stat fd_stat;
    union status_record_u *tmp_sra = NULL;
    
    LIXA_TRACE(("status_record_load\n"));
    TRY {
        LIXA_TRACE(("status_record_load: trying to open '%s' status file...\n",
                    status_file));
        if (-1 == (fd = open(status_file, O_RDWR))) {
            int i;
            
            LIXA_TRACE(("status_record_load: status file '%s' does not "
                        "exist\n", status_file));
            if (ENOENT != errno)
                THROW(OPEN_ERROR1);
            /* the file does not exist and must be created */
            if (-1 == (fd = open(status_file, O_RDWR | O_CREAT | O_EXCL,
                                 S_IRUSR | S_IWUSR | S_IRGRP)))
                THROW(OPEN_ERROR2);
            LIXA_TRACE(("status_record_load: created new status file '%s' "
                        "with file descriptor %d\n",
                        status_file, fd));
            for (i = 0; i < STATUS_FILE_INIT_SIZE; ++i) {
                union status_record_u tmp_sr;

                memset(&tmp_sr, 0, sizeof(tmp_sr));
                if (i == 0) {
                    /* write control record */
                    tmp_sr.ctrl.magic_number = STATUS_FILE_MAGIC_NUMBER;
                    tmp_sr.ctrl.level = STATUS_FILE_LEVEL;
                    tmp_sr.ctrl.first_used_block = 0;
                    tmp_sr.ctrl.first_free_block = 1;
                } else {
                    if (i == STATUS_FILE_INIT_SIZE - 1)
                        tmp_sr.data.next_block = 0;
                    else
                        tmp_sr.data.next_block = i + 1;
                }
                if (sizeof(tmp_sr) != write(fd, &tmp_sr, sizeof(tmp_sr)))
                    THROW(WRITE_ERROR);
            }
        }

        /* retrieve size */
        if (0 != fstat(fd, &fd_stat))
            THROW(FSTAT_ERROR);

        if (NULL == (tmp_sra = mmap(NULL, fd_stat.st_size,
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED, fd, 0)))
            THROW(MMAP_ERROR);
        LIXA_TRACE(("status_record_load: status file '%s' mapped at "
                    "address %p\n", status_file, tmp_sra));
        *sr = tmp_sra;

        if (0 != close(fd))
            THROW(CLOSE_ERROR);
        fd = LIXA_NULL_FD;
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case OPEN_ERROR1:
            case OPEN_ERROR2:
                ret_cod = LIXA_RC_OPEN_ERROR;
                break;
            case WRITE_ERROR:
                ret_cod = LIXA_RC_WRITE_ERROR;
                break;
            case FSTAT_ERROR:
                ret_cod = LIXA_RC_FSTAT_ERROR;
                break;
            case MMAP_ERROR:
                ret_cod = LIXA_RC_MMAP_ERROR;
                break;
            case CLOSE_ERROR:
                ret_cod = LIXA_RC_CLOSE_ERROR;
                break;
            case NONE:
                ret_cod = LIXA_RC_OK;
                break;
            default:
                ret_cod = LIXA_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    /* recovery actions */
    if (NONE != excp) {
        LIXA_TRACE(("status_record_load: values before recovery actions "
                    "excp=%d/ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
        if (LIXA_NULL_FD != fd) {
            LIXA_TRACE(("status_record_load: closing file descriptor %d\n",
                        fd));
            close(fd);
        }
    }
    LIXA_TRACE(("status_record_load/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}



int status_record_get_free(union status_record_u *sr,
                           uint32_t *slot)
{
    enum Exception { NONE } excp;
    int ret_cod = LIXA_RC_INTERNAL_ERROR;
    
    LIXA_TRACE(("status_record_get_free\n"));
    TRY {
        if (sr[0].ctrl.first_free_block != 0) {
            LIXA_TRACE(("status_record_get_free: first_free_block = "
                        UINT32_T_FORMAT ", first_used_block = "
                        UINT32_T_FORMAT "\n",
                        sr[0].ctrl.first_free_block,
                        sr[0].ctrl.first_used_block));
            *slot = sr[0].ctrl.first_free_block;
            sr[0].ctrl.first_free_block = sr[*slot].data.next_block;
            sr[*slot].data.next_block = sr[0].ctrl.first_used_block;
            sr[0].ctrl.first_used_block = *slot;
            LIXA_TRACE(("status_record_get_free: first_free_block = "
                        UINT32_T_FORMAT ", first_used_block = "
                        UINT32_T_FORMAT "\n",
                        sr[0].ctrl.first_free_block,
                        sr[0].ctrl.first_used_block));
        } else {
            /* resize status file @@@ */
            
        }
            
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case NONE:
                ret_cod = LIXA_RC_OK;
                break;
            default:
                ret_cod = LIXA_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    LIXA_TRACE(("status_record_get_free/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}
