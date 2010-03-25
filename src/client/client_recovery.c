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
#include <config.h>



#ifdef HAVE_SYSLOG_H
# include <syslog.h>
#endif



#include <lixa_errors.h>
#include <lixa_xml_msg_deserialize.h>
#include <lixa_xml_msg_serialize.h>
#include <lixa_xml_msg_trace.h>
#include <lixa_syslog.h>
#include <client_recovery.h>



/* set module trace flag */
#ifdef LIXA_TRACE_MODULE
# undef LIXA_TRACE_MODULE
#endif /* LIXA_TRACE_MODULE */
#define LIXA_TRACE_MODULE   LIXA_TRACE_MOD_CLIENT_RECOVERY



int client_recovery(client_status_t *cs,
                    const struct lixa_msg_body_open_8_client_s *client)
{
    enum Exception { XML_STRDUP_ERROR
                     , MSG_SERIALIZE_ERROR
                     , SEND_ERROR
                     , MSG_RETRIEVE_ERROR
                     , MSG_DESERIALIZE_ERROR
                     , ABORTED_RECOVERY
                     , NONE } excp;
    int ret_cod = LIXA_RC_INTERNAL_ERROR;
    
    LIXA_TRACE(("client_recovery\n"));
    TRY {
        int fd;
        struct lixa_msg_s rqst, rpl;
        size_t buffer_size = 0;
        char buffer[LIXA_MSG_XML_BUFFER_SIZE];
        ssize_t read_bytes;

        /* retrieve the socket */
        fd = client_status_get_sockfd(cs);

        /* build the message */
        rqst.header.level = LIXA_MSG_LEVEL;
        rqst.header.pvs.verb = LIXA_MSG_VERB_QRCVR;
        rqst.header.pvs.step = LIXA_MSG_STEP_INCR;

        if (NULL == (rqst.body.qrcvr_8.client.job = xmlStrdup(client->job)))
            THROW(XML_STRDUP_ERROR);
        strncpy(rqst.body.qrcvr_8.client.config_digest, client->config_digest,
                sizeof(md5_digest_hex_t));
        rqst.body.qrcvr_8.client.config_digest[MD5_DIGEST_LENGTH * 2] = '\0';
        
        if (LIXA_RC_OK != (ret_cod = lixa_msg_serialize(
                               &rqst, buffer, sizeof(buffer)-1, &buffer_size)))
            THROW(MSG_SERIALIZE_ERROR);

        LIXA_TRACE(("client_recovery: sending " SIZE_T_FORMAT
                    " bytes ('%s') to the server for step 8\n",
                    buffer_size, buffer));
        if (buffer_size != send(fd, buffer, buffer_size, 0))
            THROW(SEND_ERROR);

        if (LIXA_RC_OK != (ret_cod = lixa_msg_retrieve(
                               fd, buffer, sizeof(buffer)-1, &read_bytes)))
            THROW(MSG_RETRIEVE_ERROR);
        LIXA_TRACE(("client_recovery: receiving %d"
                    " bytes from the server |%*.*s|\n",
                    read_bytes, read_bytes, read_bytes, buffer));
        
        if (LIXA_RC_OK != (ret_cod = lixa_msg_deserialize(
                               buffer, read_bytes, &rpl)))
            THROW(MSG_DESERIALIZE_ERROR);
#ifdef _TRACE
        lixa_msg_trace(&rpl);
#endif

        /* check config digest */
        if (strncmp(rqst.body.qrcvr_8.client.config_digest,
                    rpl.body.qrcvr_16.client.config_digest,
                    sizeof(md5_digest_hex_t))) {
            char *ser_xid = xid_serialize(
                &rpl.body.qrcvr_16.client.state.xid);
            
            LIXA_TRACE(("client_recovery: current config digest ('%s') does "
                        "not match server stored config digest ('%s'); "
                        "this transaction can NOT be recovered by this "
                        "client\n",
                        rpl.body.qrcvr_16.client.config_digest,
                        rqst.body.qrcvr_8.client.config_digest));
            syslog(LOG_ERR, LIXA_SYSLOG_LXC001E, ser_xid ? ser_xid : "",
                   rpl.body.qrcvr_16.client.config_digest,
                   rqst.body.qrcvr_8.client.config_digest);
            if (ser_xid) free(ser_xid);
            THROW(ABORTED_RECOVERY);
        }
        
        /* @@@ check the answer is arrived from the server */
        
        THROW(NONE);
    } CATCH {
        switch (excp) {
            case XML_STRDUP_ERROR:
                ret_cod = LIXA_RC_XML_STRDUP_ERROR;
                break;
            case MSG_SERIALIZE_ERROR:
                break;
            case SEND_ERROR:
                ret_cod = LIXA_RC_SEND_ERROR;
                break;
            case MSG_RETRIEVE_ERROR:
            case MSG_DESERIALIZE_ERROR:
                break;
            case ABORTED_RECOVERY:
                ret_cod = LIXA_RC_ABORTED_RECOVERY;
                break;
            case NONE:
                ret_cod = LIXA_RC_OK;
                break;
            default:
                ret_cod = LIXA_RC_INTERNAL_ERROR;
        } /* switch (excp) */
    } /* TRY-CATCH */
    LIXA_TRACE(("client_recovery/excp=%d/"
                "ret_cod=%d/errno=%d\n", excp, ret_cod, errno));
    return ret_cod;
}




