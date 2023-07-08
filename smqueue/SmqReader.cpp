#include "SmqReader.h"

#include <string>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

#include <osipparser2/osip_message.h>

//#include "QueuedMsgHdrs.h"
//#include "SmqMessageHandler.h"

namespace kneedeepbts::smqueue {
    SmqReader::SmqReader(std::string listen_address, uint16_t listen_port) {
        if(listen_address == "" || listen_address == "0.0.0.0") {
            m_net_address.host = ENET_HOST_ANY;
        } else {
            enet_address_set_host(&m_net_address, listen_address.c_str());
        }
        m_net_address.port = listen_port;
    }

    std::thread SmqReader::run() {
        return std::thread(&SmqReader::reader_thread, this);
    }

    void SmqReader::stop() {
        SPDLOG_DEBUG("Stopping reader thread.");
        m_stop_thread = true;
    }

    void SmqReader::reader_thread() {
        SPDLOG_DEBUG("Starting the reader thread.");
        char hostname[16] = {0};
        size_t hostlength = 16;
        enet_address_get_host_ip(&m_net_address, hostname, hostlength);
        SPDLOG_DEBUG("Server Host: {}, Port: {}", hostname, m_net_address.port);

        m_net_server = enet_host_create(&m_net_address, m_num_connections, 1, 0, 0);
        if(m_net_server == nullptr) {
            SPDLOG_ERROR("Error occurred trying to open UDP port for SmqReader.");
            return; // Bail out now.
        }

        ENetEvent event;
        while(!m_stop_thread) {
            int32_t result = enet_host_service(m_net_server, &event, m_timeout_ms);
            if(result < 0) {
                SPDLOG_ERROR("Error occurred in UDP Server in SmqReader: {}", result);
                m_stop_thread = true;
            } else {
                // All is well, handle the event.
                switch(event.type) {
                    case ENET_EVENT_TYPE_CONNECT:
                        SPDLOG_INFO("Client Connected {}:{}", event.peer->address.host, event.peer->address.port);
                    case ENET_EVENT_TYPE_DISCONNECT:
                        SPDLOG_INFO("Client Disconnected {}:{}", event.peer->address.host, event.peer->address.port);
                    case ENET_EVENT_TYPE_RECEIVE:
                        SPDLOG_TRACE("Received Packet {}:{}", event.peer->address.host, event.peer->address.port);
                        SPDLOG_TRACE("Number of bytes: {}", event.packet->dataLength);
                        process_event(&event);
                        enet_packet_destroy(event.packet);
                    case ENET_EVENT_TYPE_NONE:
                    default:
                        SPDLOG_TRACE("Nothing happened, loop again.");
                }
            }
        }

        SPDLOG_DEBUG("Reader thread cleaning up.");
        enet_host_destroy(m_net_server);
        m_net_server = nullptr;

        SPDLOG_DEBUG("Reader thread finished.");
    }

    void SmqReader::process_event(ENetEvent * event) {
        SPDLOG_TRACE("Processing event: {}:{}", event->peer->address.host, event->peer->address.port);
        //ShortMsgPending * smp;
        ShortMsgPending smp = ShortMsgPending();
        char hostname[16] = {0};
        size_t hostlength = 16;
        // FIXME: This is copied from the old code.  The ShortMsg and ShortMsgPending
        //        should be combined and made to initialize in a more modern way.
        //smp = new ShortMsgPending();
        smp.initialize(event->packet->dataLength, (char *)event->packet->data, false);
        smp.ms_to_sc = true;

        SPDLOG_TRACE("Getting the IP and copying into short message.");
        enet_address_get_host_ip(&event->peer->address, hostname, hostlength);
        smp.srcaddrlen = hostlength; // Need to find the first null character
        memcpy(smp.srcaddr, hostname, hostlength);

        SPDLOG_TRACE("About to validate short message.");
        uint32_t result = 0;
        //int32_t errcode = validate_short_msg(smp, true); // FIXME: Why is this needed?
        // FIXME: Can this validation be moved into the short message class?
        result = validate_msg(&smp);
        SPDLOG_TRACE("Validation of short message complete.");

        if(result == 0) {
            //insert_new_message(smp); // FIXME: Bring this in to the local queue.
            m_tsq.push(std::move(smp)); // std::move lets go of the object in this scope, so the queue now owns it.
            result = 202;
            // It's OK to reference "smp" here, whether it's in the
            // smpl list, or has been moved into the main time_sorted_list.
            // FIXME: What's the best way to ACK/Reply to a message?
            //m_writer.queue_respond_sip_ack(errcode, smp, smp->srcaddr, smp->srcaddrlen); // Send respond_sip_ack message to writer thread
        } else {
            // Message is bad not inserted in queue
            SPDLOG_WARN("Received bad message, error: {}", result);
            //m_writer.queue_respond_sip_ack(errcode, smp, smp->srcaddr, smp->srcaddrlen); // SVG This might need to be removed for failed messages  Investigate
            // Don't log message data it's invalid and should not be accessed
        }
    }

    uint32_t SmqReader::validate_msg(ShortMsgPending * smp) {
        int32_t result = 0;
        //int32_t errcode = validate_short_msg(smp, true); // FIXME: Why is this needed?
        // FIXME: Can this validation be moved into the short message class?
        if(!smp->parsed_is_valid) {
            if(!smp->parse()) {
                SPDLOG_DEBUG("Failed to Parse");
                result = 400;
            }
        }

        if(!smp->parsed) {
            SPDLOG_DEBUG("Parsed is null");
            result = 400;
        }

        if(!smp->parsed->sip_version || 0 != strcmp("SIP/2.0", smp->parsed->sip_version)) {
            SPDLOG_DEBUG("Invalid SIP version");
            result = 400;
        }

        if(MSG_IS_RESPONSE(smp->parsed)) {
            // ACK
            result = verify_ack(smp);
        } else {
            // MESSAGE
            result = verify_msg(smp);
        }

        if(!smp->parsed->call_id) {
            SPDLOG_DEBUG("No call-id");
            result = 400;
        }

        char * fromtag = nullptr;
        __node_t * plist = (__node_t *) smp->parsed->from->gen_params.node;
        if (plist) {
            do {
                osip_generic_param_t * param = (osip_generic_param_t *) plist->element;
                if (param) {
                    SPDLOG_DEBUG("Param {}={}", param->gname, param->gvalue);
                    if (!strcmp("tag", param->gname)) {
                        SPDLOG_DEBUG("Got from tag");
                        fromtag = param->gvalue;
                        break;  // Only need one
                    }
                    plist = (__node_t *)plist->next;
                } else {
                    SPDLOG_DEBUG("No element in list");  // This is valid
                    break;
                }
            } while (plist); // while
            if (!fromtag) {
                SPDLOG_DEBUG("No from tag");  // This is valid
            }
        } else {
            SPDLOG_DEBUG("No node list element");  // Will not cause a problem. Not sure is this is normal
        }

        if (!smp->parsed->mime_version) {
            ; // No mime version, OK
        } else if (!smp->parsed->mime_version->value // Version 1.0 is OK.
                   || 0 != strcmp("1.0", smp->parsed->mime_version->value)) {
            SPDLOG_DEBUG("Wrong mime version");
            result = 415; // Any other is NOPE.
        }

        // proxy_authenticates - no restrictions
        // proxy_authentication_infos -- no restriction
        // record_routes -- no restriction
        // require -- RFC 3261 sec 20.32: If received, reject this msg!
        //    Unfortunately, the parser doesn't even seem to recognize it!
        // routes -- no restrictions
        if (!smp->parsed->to || !smp->parsed->to->url || !smp->parsed->to->url->scheme
            || 0 != strcmp("sip", smp->parsed->to->url->scheme)
            || !smp->check_host_port(smp->parsed->to->url->host, smp->parsed->to->url->port)
            || !smp->parsed->to->url->username

            // Asterisk returns a tag on the To: line of its SIP REGISTER
            // responses.  This violates the RFC but we'd better allow it.
            // (We don't really care -- we just ignore it anyway.)
            // A from tag is not required
            // || p->to->gen_params.nb_elt != 0

                ) {	// no tag field allowed; see
            SPDLOG_DEBUG("Invalid To header"); // RFC 3261 sec 8.1.1.2
            result = 400;
        }

        char * user = smp->parsed->to->url->username;
        // FIXME: Make this work at some point in the future.
        //if (!check_to_user(user)) {
        //    LOG(DEBUG) << "Invalid To user";
        //    return 400;
        //}

        // We need to see if this is a message form the relay. If it is, we can process a user lookup here
        // BUT ONLY IF the message is not a response (ACK) AND MUST BE a SIP MESSAGE.
        // FIXME: Should all of these items be checked here?
//        if (should_early_check
//            && !MSG_IS_RESPONSE(smp->parsed)
//            && (0 == strcmp("MESSAGE", smp->parsed->sip_method))
//            && (my_network.msg_is_from_relay(smp->srcaddr, smp->srcaddrlen, global_relay_host.c_str(), global_relay_port.c_str())
//                // FIXME: From config:
//                // || (gConfig.getBool("SIP.GlobalRelay.RelaxedVerify") &&
//            || (true && relaxed_verify_relay(&smp->parsed->vias, global_relay_host.c_str(), global_relay_port.c_str())
//            ))) {
//            // We cannot deliver the message since we cannot resolve the TO
//            if (!to_is_deliverable(user)) {
//                SPDLOG_DEBUG("To address not deliverable");
//                result = 404;
//            }
//            // We need to reject the message, because we cannot resolve the FROM
//            // TODO: Don't do this. From coming in through the relay is probably not resolvable.
//            /*if (!manager->from_is_deliverable(p->from->url->username))
//                return 403;*/
//            smp->from_relay = true;
//            LOG(DEBUG) << "In bound message from " << smp->parsed->from->url->username << " to " << user << " is from relay";
//        }

        // www_authenticates - no restrictions
        // headers - ???
        // message_property ??? FIXME
        // message ??? FIXME
        // message_length ??? FIXME
        // application_data - no restrictions

        // Set the qtag from the parsed fields, if it hasn't been set yet.
        result = (smp->set_qtag());
        return result;
    }

    uint32_t SmqReader::verify_ack(kneedeepbts::smqueue::ShortMsgPending *smp) {
        size_t clen = 0;
        char * endptr;

        if(smp->parsed->status_code < 0 || !smp->parsed->reason_phrase) {
            SPDLOG_DEBUG("Status code invalid or no reason");
            return 400;
        }

        // Leave this test in for responses
        if (smp->parsed->content_length && smp->parsed->content_length->value) {
            errno = 0;
            clen = strtol(smp->parsed->content_length->value, &endptr, 10);
            if (*endptr != '\0' || errno != 0) {
                SPDLOG_DEBUG("Invalid Content Length");
                return 400;
            }
        } else {
            SPDLOG_DEBUG("Content Length zero");
        }

        if (clen != 0) {
            SPDLOG_DEBUG("ACKs shouldn't have a content length.");
            return 400;
        }

        if (smp->parsed->bodies.nb_elt != 0) {
            SPDLOG_DEBUG("ACKs shouldn't have a body.");
            return 400;
        }
        return 0;
    }

    uint32_t SmqReader::verify_msg(kneedeepbts::smqueue::ShortMsgPending *smp) {
        char * user;

        if (!smp->parsed->req_uri || !smp->parsed->req_uri->scheme) {
            LOG(DEBUG) << "No scheme or uri";
            return 400;
        }

        if (0 != strcmp("sip", smp->parsed->req_uri->scheme)) {
            LOG(DEBUG) << "Not SIP scheme";
            return 416;
        }

        if (!smp->check_host_port(smp->parsed->req_uri->host, smp->parsed->req_uri->port)) {
            LOG(DEBUG) << "Host port check failed";
            return 484;
        }

        if (!smp->parsed->sip_method) {
            LOG(DEBUG) << "SIP method not set";
            return 405;
        }

        if (0 == strcmp("MESSAGE", smp->parsed->sip_method)) {
            user = smp->parsed->req_uri->username;
            if (!user) {
                return 484;
            }
            // FIXME:
            //    if (!check_to_user(user))  // Check user does nothing
            //        return 484;
            // FIXME, URL versus To: might have different username requirements?
            // FIXME, add support for more Content-Type's.
            if (!smp->parsed->content_type || !smp->parsed->content_type->type || !smp->parsed->content_type->subtype
                || !( (0 == strcmp("text", smp->parsed->content_type->type) && 0 == strcmp("plain", smp->parsed->content_type->subtype))
                || (0 == strcmp("application", smp->parsed->content_type->type) && 0 == strcmp("vnd.3gpp.sms", smp->parsed->content_type->subtype)) )) {
                SPDLOG_DEBUG("Content type not supported");
                return 415;
            }

            int len;
            if (smp->parsed->content_length && smp->parsed->content_length->value && ((len=strtol(smp->parsed->content_length->value, NULL, 10)) > 0) ) {
                //If length is greater than zero check this
                // Bad if any of these are true nb_elt!=1,  node=0,   node->next!=0,   node->element=0
                if (smp->parsed->bodies.nb_elt != 1  // p->bodies.nb_elt should always be one
                    || 0 == smp->parsed->bodies.node
                    || 0 != smp->parsed->bodies.node->next
                    || 0 == smp->parsed->bodies.node->element) {
                    SPDLOG_DEBUG("Message entity-body too large");
                    return 413;
                }
            } else {
                SPDLOG_DEBUG("Content length is zero");
            }

            if (!smp->parsed->cseq || !smp->parsed->cseq->method
                || 0 != strcmp("MESSAGE", smp->parsed->cseq->method)
                || !smp->parsed->cseq->number) {  // FIXME, validate number??
                SPDLOG_DEBUG("Invalid sequence number");
                return 400;
            }
        } else if (0 == strcmp("REGISTER", smp->parsed->sip_method)) {
            // Null username is OK in register message.
            // Null content-type is OK.
            // Null message body also OK.
            if (!smp->parsed->cseq || !smp->parsed->cseq->method
                || 0 != strcmp("REGISTER", smp->parsed->cseq->method)
                || !smp->parsed->cseq->number) {  // FIXME, validate number??
                SPDLOG_DEBUG("Invalid REGISTER");
                return 400;
            }
        } else {
            SPDLOG_DEBUG("Unknown SIP datagram");
            return 405;
        }
        return 0;
    }
}
