/*
 * Copyright (c) 2009-2017, Christian Ferrari <tiian@users.sourceforge.net>
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
#ifndef POSTGRESQL_XA_RESOURCE_H
# define POSTGRESQL_XA_RESOURCE_H



#include <config.h>


/* PostgreSQL front-end */
#include <libpq-fe.h>
/* XTA includes */
#include "xta_acquired_xa_resource.h"



/* save old LIXA_TRACE_MODULE and set a new value */
#ifdef LIXA_TRACE_MODULE
# define LIXA_TRACE_MODULE_SAVE LIXA_TRACE_MODULE
# undef LIXA_TRACE_MODULE
#else
# undef LIXA_TRACE_MODULE_SAVE
#endif /* LIXA_TRACE_MODULE */
#define LIXA_TRACE_MODULE      LIXA_TRACE_MOD_XTA



/**
 * XTA PostgreSQL XA Resource data type
 */
typedef struct {
    union {
        xta_xa_resource_t                    xa_resource;
        xta_acquired_xa_resource_t  acquired_xa_resource;
    };
} xta_postgresql_xa_resource_t;



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Create a new object to represent a PostgreSQL XA resource
     * @param[in] connection to PostgreSQL already opened by the application
     *            program
     * @return a new object or NULL in the event of error
     */
    xta_postgresql_xa_resource_t *xta_postgresql_xa_resource_new(
        PGconn *connection);

    

    /**
     * Delete an object that represent a PostgreSQL XA Resource
     * @param[in] this : PostgreSQL XA Resource
     */
    void xta_postgresql_xa_resource_delete(
        xta_postgresql_xa_resource_t *this);
    
    

#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of LIXA_TRACE_MODULE */
#ifdef LIXA_TRACE_MODULE_SAVE
# undef LIXA_TRACE_MODULE
# define LIXA_TRACE_MODULE LIXA_TRACE_MODULE_SAVE
# undef LIXA_TRACE_MODULE_SAVE
#endif /* LIXA_TRACE_MODULE_SAVE */



#endif /* POSTGRESQL_XA_RESOURCE_H */
