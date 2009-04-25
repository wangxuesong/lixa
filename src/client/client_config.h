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
#ifndef CLIENT_CONFIG_H
# define CLIENT_CONFIG_H



#include <config.h>



#ifdef HAVE_LIBXML_TREE_H
# include <libxml/tree.h>
#endif
#ifdef HAVE_LIBXML_PARSER_H
# include <libxml/parser.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif



/* save old LIXA_TRACE_MODULE and set a new value */
#ifdef LIXA_TRACE_MODULE
# define LIXA_TRACE_MODULE_SAVE LIXA_TRACE_MODULE
# undef LIXA_TRACE_MODULE
#else
# undef LIXA_TRACE_MODULE_SAVE
#endif /* LIXA_TRACE_MODULE */
#define LIXA_TRACE_MODULE      LIXA_TRACE_MOD_CLIENT_CONFIG



/**
 * It contains the configuration of a transaction manager (how to reach and
 * use it)
 */
struct trnmgr_config_s {
    /**
     * Socket domain for the socket connection
     */
    int domain;
    /**
     * Address used to reach the transaction manager
     */
    char *address;
    /**
     * Port used to reach the transaction manager
     */
    in_port_t port;
    /**
     * Transactional profile associated to this entry
     */
    char *profile;
};

/**
 * It contains the configuration of all the transaction managers
 */
struct trnmgr_config_array_s {
    /**
     * Number of elements
     */
    int n;
    /**
     * Elements
     */
    struct trnmgr_config_s *array;
};



/**
 * It contains the configuration for the client
 */
struct client_config_s {
    /**
     * Transaction managers' configuration
     */
    struct trnmgr_config_s       trnmgrs;
};



#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



    /**
     * Read and parse client config file
     * @param cc OUT the object containing the client configuration
     * @param config_filename IN a filename PATH must looked at before
     *                           searching default system config file
     *                           default = NULL
     * @return a standardized return code
     */
    int client_config(struct client_config_s *cc,
                      const char *config_filename);

    

    /**
     * Parse the configuration tree
     * @param cc OUT server configuration structure
     * @param a_node IN the current subtree must be parsed
     * @return a standardized return code
     */
    int client_parse(struct client_config_s *cc,
                     xmlNode *a_node);

    
#ifdef __cplusplus
}
#endif /* __cplusplus */



/* restore old value of LIXA_TRACE_MODULE */
#ifdef LIXA_TRACE_MODULE_SAVE
# undef LIXA_TRACE_MODULE
# define LIXA_TRACE_MODULE LIXA_TRACE_MODULE_SAVE
# undef LIXA_TRACE_MODULE_SAVE
#endif /* LIXA_TRACE_MODULE_SAVE */



#endif /* CLIENT_CONFIG_H */