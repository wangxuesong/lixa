/*
 * Copyright (c) 2009-2010, Christian Ferrari <tiian@users.sourceforge.net>
 * All rights reserved.
 *
 * This file is part of LIXA.
 *
 * LIXA is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * LIXA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with LIXA.  If not, see <http://www.gnu.org/licenses/>.
 */



/*
 * This is a trivial sample to show how you can use Lixa TX (Transaction
 * Demarcation) API and OCI (Oracle C Interface) API together.
 * To run this program:
 * 0. install Oracle XE
 * 1. compile and install Lixa
 * 2. activate "hr/hr" Oracle internal user
 * 3. check hr schema contains "COUNTRIES" table
 * 4. configure lixac_conf.xml with a valid Oracle xa_open_info string
 * 5. switch to "oracle" UNIX account or equivalent account with a running
 *    Oracle environment
 * 6. execute this sample with:
 *    ...somepath.../lixa_oracle_sample
 *    or
 *    ...somepath.../lixa_oracle_sample delete
 * 7. check with web object browser interface - or the Oracle interface of
 *    your choice - the record is inserted/deleted from the table
 *    ...hope you'll enjoy it ;)
 */



/* standard UNIX headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Oracle C Interface API header */
#include <oci.h>

/* TX (Transaction Demarcation) header */
#include <tx.h>



int main(int argc, char *argv[])
{
    int            txrc, ocirc, delete;
    OCIEnv        *oci_env;
    OCISvcCtx     *oci_svc_ctx;
    OCIStmt       *stmt_hndl;
    OCIError      *err_hndl;
    text          *stmt_i = (text *) "INSERT INTO COUNTRIES "
        "(COUNTRY_ID, COUNTRY_NAME, REGION_ID) "
        "VALUES ('RS', 'Repubblica San Marino', 1)";
    text          *stmt_d = (text *) "DELETE FROM COUNTRIES WHERE "
        "COUNTRY_ID = 'RS'";

    if (argc > 1 && (!strcmp(argv[1], "delete") || !strcmp(argv[1], "DELETE")))
        delete = 1;
    else
        delete = 0;
    
    /* open the resource manager(s) */
    if (TX_OK != (txrc = tx_open())) {
        fprintf(stderr, "tx_open error: %d\n", txrc);
        exit(txrc);
    }

    /* retrieve environment and context */
    if (NULL == (oci_env = xaoEnv(NULL))) {
        fprintf(stderr, "xaoEnv returned a NULL pointer\n");
        exit(1);
    }
    if (NULL == (oci_svc_ctx = xaoSvcCtx(NULL))) {
        fprintf(stderr, "xaoSvcCtx returned a NULL pointer\n");
        exit(1);
    }

    /* allocate statement and error handles */
    if (0 != OCIHandleAlloc( (dvoid *)oci_env, (dvoid **)&stmt_hndl,
                             OCI_HTYPE_STMT, (size_t)0, (dvoid **)0)) {
        fprintf(stderr, "Unable to allocate statement handle\n");
        exit(1);
    }
    if (0 != OCIHandleAlloc( (dvoid *)oci_env, (dvoid **)&err_hndl,
                             OCI_HTYPE_ERROR, (size_t)0, (dvoid **)0)) {
        fprintf(stderr, "Unable to allocate error handle\n");
        exit(1);
    }
    
    if (TX_OK != (txrc = tx_begin())) {
        fprintf(stderr, "tx_begin error: %d\n", txrc);
        exit(txrc);
    }

    if (delete) {
        fprintf(stderr, "First arg is '%s', bypassing INSERT statement...\n",
                argv[1]);
    } else {
        if (OCI_SUCCESS != OCIStmtPrepare(stmt_hndl, err_hndl, stmt_i,
                                          (ub4) strlen((char *)stmt_i),
                                          (ub4) OCI_NTV_SYNTAX,
                                          (ub4) OCI_DEFAULT)) {
            fprintf(stderr,
                    "Unable to prepare INSERT statement for execution\n");
            exit(1);
        }

        ocirc = OCIStmtExecute(oci_svc_ctx, stmt_hndl, err_hndl, (ub4)1, (ub4)0,
                               (CONST OCISnapshot *)NULL,
                               (OCISnapshot *)NULL, OCI_DEFAULT);
        if (OCI_SUCCESS != ocirc && OCI_SUCCESS_WITH_INFO != ocirc) {
            fprintf(stderr, "Error while executing INSERT statement; "
                    "ocirc = %d\n", ocirc);
            exit(ocirc);
        } else
            fprintf(stderr, "INSERT statement executed!\n");
    }

    if (delete) {
        if (OCI_SUCCESS != OCIStmtPrepare(stmt_hndl, err_hndl, stmt_d,
                                          (ub4) strlen((char *)stmt_d),
                                          (ub4) OCI_NTV_SYNTAX,
                                          (ub4) OCI_DEFAULT)) {
            fprintf(stderr, "Unable to prepare DELETE statement for "
                    "execution\n");
            exit(1);
        }

        ocirc = OCIStmtExecute(oci_svc_ctx, stmt_hndl, err_hndl, (ub4)1, (ub4)0,
                               (CONST OCISnapshot *)NULL,
                               (OCISnapshot *)NULL, OCI_DEFAULT);
        if (OCI_SUCCESS != ocirc && OCI_SUCCESS_WITH_INFO != ocirc) {
            fprintf(stderr, "Error while executing DELETE statement; "
                    "ocirc = %d\n",
                    ocirc);
            exit(ocirc);
        } else
            fprintf(stderr, "DELETE statement executed!\n");
    } else
        fprintf(stderr, "First arg is not 'DELETE', bypassing DELETE "
                "statement...\n");

    /*
    if (TX_OK != (txrc = tx_commit())) {
        fprintf(stderr, "tx_commit error: %d\n", txrc);
        exit(txrc);
    }
    */
    if (TX_OK != (txrc = tx_rollback())) {
        fprintf(stderr, "tx_rollback error: %d\n", txrc);
        exit(txrc);
    }

    /* free the allocated handles */
    OCIHandleFree((dvoid *)stmt_hndl, (ub4)OCI_HTYPE_STMT);
    OCIHandleFree((dvoid *)err_hndl, (ub4)OCI_HTYPE_ERROR);
    
    /* close the resource manager(s) */
    if (TX_OK != (txrc = tx_close())) {
        fprintf(stderr, "tx_close error: %d\n", txrc);
        exit(txrc);
    }
    
    return 0;
}