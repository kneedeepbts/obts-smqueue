#include "shortmsgpending.h"

namespace kneedeepbts::smqueue {
    std::string sm_state_strings[STATE_MAX_PLUS_ONE] = {
            "No State",
            "Initial State",
            "Request From-Address Lookup",
            "Asked for From-Address",
            "Awaiting Try Destination IMSI",
            "Request Destination IMSI",
            "Asked for Destination IMSI",
            "Awaiting Try Destination SIP URL",
            "Request Destination SIP URL",
            "Asked for Destination SIP URL",
            "Awaiting Try Message Delivery",
            "Request Message Delivery",
            "Asked for Message Delivery",
            "Delete Me",
            "Awaiting Register Handset",
            "Register Handset",
            "Asked to Register Handset",
    };

    std::string sm_state_string(enum sm_state astate)
    {
        if (astate < STATE_MAX_PLUS_ONE)
            return sm_state_strings[astate];
        else	return "Invalid State Number";
    }


    /* We try to centralize most of the timeout values into this table.
       Occasionally the code might do something different where it knows
       better, but most state transitions just use this table to set the
       timeout that will fire if nothing happens in the new state for X
       seconds. */

    /* Timeouts when going from NO_STATE into each subsequent state
     * Timeouts are in MS
     *  */
    #define NT	3000*1000	/* msseconds = 50 minutes - "No Timeout" - for states
                   where we will only time out if something is really
                   broken. Changed to MS */
    #define RT	300*1000	/* msseconds = 5 minutes - "Re Try" - for state
                   transitions where we're starting over from
                   scratch due to some error. Changed to MS */
    #define TT	60*1000      /*  60 seconds the amount of time we add to a transaction
                    when we get a 100 TRYING message  Changed to MS */

    /* Timeout when moving from this state to new state:
     NS  IS  RF  AF   WD  RD  AD   WS  RS  AS   WM  RM  AM   DM   WR  RH  AR  */
        int timeouts_NO_STATE[STATE_MAX_PLUS_ONE] = {
                NT,  0,  0, NT,  NT,  0, NT,  NT,  0, NT,  NT,  0, NT,   0,  NT, NT, NT,};
        int timeouts_INITIAL_STATE[STATE_MAX_PLUS_ONE] = {
                0,  0,  0, NT,  NT, NT, NT,  NT, NT, NT,  NT,  0, NT,   0,  NT, NT, NT,};
        int timeouts_REQUEST_FROM_ADDRESS_LOOKUP[STATE_MAX_PLUS_ONE] = {
                0, NT, 10, 10,  NT,  0, NT,  NT, NT, NT,  NT, NT, NT,   0,   1,  0, NT,};
        int timeouts_ASKED_FOR_FROM_ADDRESS_LOOKUP[STATE_MAX_PLUS_ONE] = {
                0, NT, 60, NT,  NT, NT, NT,  NT, NT, NT,  NT, NT, NT,   0,  NT, NT, NT,};
        int timeouts_AWAITING_TRY_DESTINATION_IMSI[STATE_MAX_PLUS_ONE] = {
                0, NT, RT, NT,  RT, NT, NT,  NT, NT, NT,  NT, NT, NT,   0,  NT, NT, NT,};
        int timeouts_REQUEST_DESTINATION_IMSI[STATE_MAX_PLUS_ONE] = {  // 5
                0, NT, RT, NT,  RT, NT, NT,  NT,  0, NT,  NT, NT, NT,   0,  NT, NT, NT,};
        int timeouts_ASKED_FOR_DESTINATION_IMSI[STATE_MAX_PLUS_ONE] = {
                0, NT, RT, NT,  RT, NT, NT,  NT, NT, NT,  NT, NT, NT,   0,  NT, NT, NT,};
        int timeouts_AWAITING_TRY_DESTINATION_SIPURL[STATE_MAX_PLUS_ONE] = {
                0, NT, RT, NT,  RT, NT, NT,  NT, NT, NT,  NT, NT, NT,   0,  NT, NT, NT,};
        int timeouts_REQUEST_DESTINATION_SIPURL[STATE_MAX_PLUS_ONE] = {
                0, NT, RT, NT,  RT, NT, NT,  NT, NT, NT,  NT, 0, NT,  0,  NT, NT, NT,};
        int timeouts_ASKED_FOR_DESTINATION_SIPURL[STATE_MAX_PLUS_ONE] = {  // 8
                0, NT, RT, NT,  RT, NT, NT,  NT, NT, NT,  NT, NT, NT,   0,  NT, NT, NT,};
        int timeouts_AWAITING_TRY_MSG_DELIVERY[STATE_MAX_PLUS_ONE] = {
                0, NT, RT, NT,  RT, NT, NT,  NT, NT, NT,  75*1000,  0, NT,   0,  NT, NT, NT,};
        int timeouts_REQUEST_MSG_DELIVERY[STATE_MAX_PLUS_ONE] = { // 10
    //  0, NT, RT, NT,  RT, NT, NT,  NT, 75*1000, NT,  75*1000, 75, 15,   0,  NT, NT, NT,};
                0, NT, RT, NT,  RT, NT, NT,  NT,  15*1000, NT,  75*1000, 75*1000, 15*1000,   0,  NT, NT, NT,};
        int timeouts_ASKED_FOR_MSG_DELIVERY[STATE_MAX_PLUS_ONE] = { // 11
                0, NT, RT, NT,  NT, NT, NT,  NT, NT, NT,  60*1000, 10*1000, TT,   0,  NT, NT, NT,};  // changed 11 -> 12 from NT to TT
        int timeouts_DELETE_ME_STATE[STATE_MAX_PLUS_ONE] = { // 12
                0,  0,  0,  0,   0,  0,  0,   0,  0,  0,   0,  0,  0,   0,   0,  0,  0,};
        int timeouts_AWAITING_REGISTER_HANDSET[STATE_MAX_PLUS_ONE] = {
                0, NT,  0, NT,  RT, NT, NT,  NT, NT, NT,  NT, NT, NT,   0,   1*1000,  0, NT,};
        int timeouts_REGISTER_HANDSET[STATE_MAX_PLUS_ONE] = {
                0, NT,  0, NT,  RT, NT, NT,  NT, NT, NT,  NT, NT, NT,   0,   1*1000,  1*1000,  2*1000,};
        int timeouts_ASKED_TO_REGISTER_HANDSET[STATE_MAX_PLUS_ONE] = {
                0, NT,  0, NT,  RT, NT, NT,  NT, NT, NT,  NT, NT, NT,   0,   1*1000,  1*1000, 10*1000,};
    /* Timeout when moving from this state to new state:
     NS  IS  RF  AF   WD  RD  AD   WS  RS  AS   WM  RM  AM   DM   WR  RH  AR  */

    #undef NT	/* No longer needed */
    #undef RT

    /* Index to all timeouts.  Keep in order!  */
    int (*timeouts[STATE_MAX_PLUS_ONE])[STATE_MAX_PLUS_ONE] = {
            &timeouts_NO_STATE,
            &timeouts_INITIAL_STATE,
            &timeouts_REQUEST_FROM_ADDRESS_LOOKUP,
            &timeouts_ASKED_FOR_FROM_ADDRESS_LOOKUP, // 3

            &timeouts_AWAITING_TRY_DESTINATION_IMSI,
            &timeouts_REQUEST_DESTINATION_IMSI,
            &timeouts_ASKED_FOR_DESTINATION_IMSI,

            &timeouts_AWAITING_TRY_DESTINATION_SIPURL, //
            &timeouts_REQUEST_DESTINATION_SIPURL,  // 8
            &timeouts_ASKED_FOR_DESTINATION_SIPURL,

            &timeouts_AWAITING_TRY_MSG_DELIVERY,
            &timeouts_REQUEST_MSG_DELIVERY, //11
            &timeouts_ASKED_FOR_MSG_DELIVERY,

            &timeouts_DELETE_ME_STATE,

            &timeouts_AWAITING_REGISTER_HANDSET,
            &timeouts_REGISTER_HANDSET,
            &timeouts_ASKED_TO_REGISTER_HANDSET,
    };

    void ShortMsgPending::set_state(enum sm_state newstate) {
        next_action_time = msgettime() + (*timeouts[state])[newstate];
        LOG(DEBUG) << "Set state Current: " << sm_state_string(state) << " Newstate: " << sm_state_string(newstate)
                   << " Timeout value " << *timeouts[state][newstate];
        state = newstate;
        /* If we're in a queue, some code in another class is now going
           to have to change our queue position.  */
    }


    /*
     * Validate a short_msg by parsing it and then checking the parse
     * to make sure it has *everything* we need to process and forward it.
     * **ALL** validity checks on incoming short_msg's are centralized here.
     * Thus the rest of the code doesn't need to muck around checking
     * every possible thing -- it can just look at the parts of the message
     * that it needs to.
     *
     * Also, if the qtag hasn't been set yet, set it, based on the message's
     * fields.
     *
     * If we reject a message, return the SIP response code that we should
     * return to the sender, diagnosing the problem.  If the message is OK,
     * result is zero.
     * Return
     * 	0 Is okay
     * 	Error code if failed
     *
     */
    int ShortMsgPending::validate_short_msg(bool should_early_check)
    {
        osip_message_t *p;
        __node_t *plist;
        osip_generic_param_t *param;
        char *user;
        size_t clen;
        char *fromtag;
        char *endptr;

        // FIXME: Need to handle debug logging in a better way.
//        if (print_as_we_validate) {    // debugging
//            make_text_valid();
//            LOG(DEBUG) << "MSG = " << this->text;
//        }

        if (!parsed_is_valid) {
            if (!parse()) {
                LOG(DEBUG) << "Invalid parse";
                return 400;
            }
        }

        /* The message has been parsed.  Now check that we like its
           structure and contents.  */

        p = parsed;
        if (!p) {
            LOG(DEBUG) << "Parse is NULL";
            return 400;
        }

        if (!p->sip_version || 0 != strcmp("SIP/2.0", p->sip_version)) {
            LOG(DEBUG) << "Invalid SIP version";
            return 400;
        }
        // If it's an ack, check some things.  If it's a message,
        // check others.
        if (MSG_IS_RESPONSE(p)) {
            // It's an ack.
            if (p->status_code < 0
                || !p->reason_phrase) {
                LOG(DEBUG) << "Status code invalid or no reason";
                return 400;
            }
            clen = 0;
            // Leave this test in for responses
            if (p->content_length && p->content_length->value) {
                errno = 0;
                clen = strtol(p->content_length->value, &endptr, 10);
                if (*endptr != '\0' || errno != 0) {
                    LOG(DEBUG) << "Invalid Content Length";
                    return 400;
                }
            } else {
                LOG(DEBUG) << "Content Length zero";
            }
            if (clen != 0) {
                LOG(DEBUG) << "ACK has a content length";
                return 400;	// Acks have no content!
            }

            if (p->bodies.nb_elt != 0) {
                LOG(DEBUG) << "ACK has a body";
                return 400;	// Acks have no bodies!
            }
        } else {
            // It's a message.
            if (!p->req_uri || !p->req_uri->scheme) {
                LOG(DEBUG) << "No scheme or uri";
                return 400;
            }
            if (0 != strcmp("sip", p->req_uri->scheme)) {
                LOG(DEBUG) << "Not SIP scheme";
                return 416;
            }
            if (!check_host_port(p->req_uri->host,p->req_uri->port)) {
                LOG(DEBUG) << "Host port check failed";
                return 484;
            }
            if (!p->sip_method) {
                LOG(DEBUG) << "SIP method not set";
                return 405;
            }
            if (0 == strcmp("MESSAGE", p->sip_method)) {
                user = p->req_uri->username;
                if (!user)
                    return 484;
                // FIXME:
//                if (!check_to_user(user))  // Check user does nothing
//                    return 484;
                // FIXME, URL versus To: might have different username
                // requirements?

                // FIXME, add support for more Content-Type's.
                if (!p->content_type || !p->content_type->type
                    || !p->content_type->subtype
                    || !( (0 == strcmp("text", p->content_type->type)
                           && 0 == strcmp("plain", p->content_type->subtype))
                          ||(0 == strcmp("application", p->content_type->type)
                             && 0 == strcmp("vnd.3gpp.sms", p->content_type->subtype))
                )
                        ) {
                    LOG(DEBUG) << "Content type not supported";
                    return 415;
                }

                int len;
                if (p->content_length && p->content_length->value && ((len=strtol(p->content_length->value, NULL, 10)) > 0) ) {
                    //If length is greater than zero check this
                    // Bad if any of these are true nb_elt!=1,  node=0,   node->next!=0,   node->element=0
                    if (p->bodies.nb_elt != 1  // p->bodies.nb_elt should always be one
                        || 0 == p->bodies.node
                        || 0 != p->bodies.node->next
                        || 0 == p->bodies.node->element) {
                        LOG(DEBUG) << "Message entity-body too large";
                        return 413;	// Request entity-body too large
                    }
                } else {
                    LOG(DEBUG) << "Content length is zero";
                }

                if (!p->cseq || !p->cseq->method
                    || 0 != strcmp("MESSAGE", p->cseq->method)
                    || !p->cseq->number) {  // FIXME, validate number??
                    LOG(DEBUG) << "Invalid sequence number";
                    return 400;
                }

            } else if (0 == strcmp("REGISTER", p->sip_method)) {
                // Null username is OK in register message.
                // Null content-type is OK.
                // Null message body also OK.
                if (!p->cseq || !p->cseq->method
                    || 0 != strcmp("REGISTER", p->cseq->method)
                    || !p->cseq->number) {  // FIXME, validate number??
                    LOG(DEBUG) << "Invalid REGISTER";
                    return 400;
                }

            } else {
                LOG(DEBUG) << "Unknown SIP datagram";
                return 405;	// Unknown SIP datagram type
            }
        }

        // accepts - no restrictions
        // accept_encodings - no restrictions?
        // accept_langauges - no restrictions?
        // alert_infos - no restrictions?
        // allows - no restrictions?
        // authentication_infos - no restrictions?
        // authorizations - no restrictions
        if (!p->call_id) {
            LOG(DEBUG) << "No call-id";
            return 400;
        }

        // Remove content length check
        // According to rfc3261 length can be 0 or greater  svg 03/20/2014

        // From address needs to exist but a tag in the from address is optional
        if (!p->from || !p->from->url
            //|| p->from->gen_params.nb_elt < 1  // Message tag is not required
            //|| !p->from->gen_params.node		 // Message tag is not required SVG 03/12/14
                )
        {
            LOG(DEBUG) << "Invalid from address in header from " << p->from << " fromurl " << p->from->url
                       << " nb.elt " << p->from->gen_params.nb_elt << " params.node " << p->from->gen_params.node;
            return 400;
        }

        fromtag = nullptr;
        plist = (__node_t *) p->from->gen_params.node;
        if (plist) {
            do {
                param = (osip_generic_param_t *) plist->element;
                if (param) {
                    LOG(DEBUG) << "Param " << param->gname << "=" << param->gvalue;
                    if (!strcmp("tag", param->gname)) {
                        LOG(DEBUG) << "Got from tag";
                        fromtag = param->gvalue;
                        break;  // Only need one
                    }
                    plist = (__node_t *)plist->next;
                } else {
                    LOG(DEBUG) << "No element in list";  // This is valid
                    break;
                }
            } while (plist); // while
            if (!fromtag) {
                LOG(DEBUG) << "No from tag";  // This is valid
            }
        }
        else {
            LOG(DEBUG) << "No node list element";  // Will not cause a problem. Not sure is this is normal
        }

        if (!p->mime_version) {
            ; // No mime version, OK
        } else if (!p->mime_version->value // Version 1.0 is OK.
                   || 0 != strcmp("1.0", p->mime_version->value)) {
            LOG(DEBUG) << "Wrong mime version";
            return 415; // Any other is NOPE.
        }
        // proxy_authenticates - no restrictions
        // proxy_authentication_infos -- no restriction
        // record_routes -- no restriction
        // require -- RFC 3261 sec 20.32: If received, reject this msg!
        //    Unfortunately, the parser doesn't even seem to recognize it!
        // routes -- no restrictions
        if (!p->to || !p->to->url || !p->to->url->scheme
            || 0 != strcmp("sip", p->to->url->scheme)
            || !check_host_port(p->to->url->host, p->to->url->port)
            || !p->to->url->username
//#if 0
//            // Asterisk returns a tag on the To: line of its SIP REGISTER
//            // responses.  This violates the RFC but we'd better allow it.
//            // (We don't really care -- we just ignore it anyway.)
//            // A from tag is not required
//            // || p->to->gen_params.nb_elt != 0
//#endif
                ) {	// no tag field allowed; see
            LOG(DEBUG) << "Invalid To header";	// RFC 3261 sec 8.1.1.2
            return 400;
        }
        user = p->to->url->username;

        // FIXME:
//        if (!check_to_user(user)) {
//            LOG(DEBUG) << "Invalid To user";
//            return 400;
//        }

        // FIXME: This logic should be moved somewhere else.  The message shouldn't check itself
        //        against the manager.
//        // We need to see if this is a message from the relay. If it is, we can process a user lookup here
//        // BUT ONLY IF the message is not a response (ACK) AND MUST BE a SIP MESSAGE.
//        if (should_early_check && !MSG_IS_RESPONSE(p) && (0 == strcmp("MESSAGE", p->sip_method))
//            && (manager->my_network.msg_is_from_relay(srcaddr, srcaddrlen,
//                                                      manager->global_relay.c_str(), manager->global_relay_port.c_str()) ||
//                (gConfig.getBool("SIP.GlobalRelay.RelaxedVerify") &&
//                 relaxed_verify_relay(&p->vias, manager->global_relay.c_str(), manager->global_relay_port.c_str())
//                ))) {
//            // We cannot deliver the message since we cannot resolve the TO
//            if (!manager->to_is_deliverable(user)) {
//                LOG(DEBUG) << "To address not deliverable";
//                return 404;
//            }
//            // We need to reject the message, because we cannot resolve the FROM
//            // TODO: Don't do this. From coming in through the relay is probably not resolvable.
//            /*if (!manager->from_is_deliverable(p->from->url->username))
//                return 403;*/
//            from_relay = true;
//            LOG(DEBUG) << "In bound message from " << p->from->url->username << " to " << user << " is from relay";
//        }

        // www_authenticates - no restrictions
        // headers - ???
        // message_property ??? FIXME
        // message ??? FIXME
        // message_length ??? FIXME
        // application_data - no restrictions

        // Set the qtag from the parsed fields, if it hasn't been set yet.
        return (set_qtag());
    }


    // Set the qtag from the parsed fields, if it hasn't been set yet.
    // Whenever we change any of these fields, we have to recalculate
    // the qtag.
    // FIXME, we assume that the CSEQ, Call-ID, and From tag all are
    // components that, in combination, identify the message uniquely.
    // (pat) There should not be a from-tag in a MESSAGE; the from- and to-tags are used only for INVITE.
    // FIXME!  The spec is unpleasantly unclear about this.
    // (pat) RFC3261 17.1.3 is clear: we are supposed to use either the via-branch or the call-id+cseq to identify the message.
    // Result is 0 for success, or 3-digit integer error code if error.
    // FIXME!  The Call-ID changes each time we re-send an SMS.
    //  When we get a 200 "Delivered" message for ANY of those
    //  different Call-ID's, we should accept that the msg was
    //  delivered and delete it.  Therefore, remove the Call-ID
    //  from the qtag.
    int ShortMsgPending::set_qtag() {
        osip_message_t *p;
        __node_t *plist;
        osip_generic_param_t *param;
        char *fromtag;
        int error;

        //LOG(DEBUG) << "Enter parse";
        if (!parsed_is_valid) {
            if (!parse()) {
                LOG(DEBUG) << "Parse failed in setqtag";
                return 400;
            }
        }
        p = parsed;

        if (!p->from) {
            LOG(DEBUG) << "No from address in setqtag";
            return 400;
        }

//#if 0
//        plist = (__node_t *) p->from->gen_params.node;
//        if (!plist) {
//            LOG(DEBUG) << "No node in from param list";
//            return 400;
//        }
//#endif

        // Handle from tag which is optional
        fromtag = nullptr;
        plist = (__node_t *) p->from->gen_params.node;
        if (plist) {
            do {
                param = (osip_generic_param_t *) plist->element;
                if (param) {
                    LOG(DEBUG) << "Param " << param->gname << "=" << param->gvalue;
                    if (!strcmp("tag", param->gname)) {
                        LOG(DEBUG) << "Got from tag";
                        fromtag = param->gvalue;
                        break;  // Only need one
                    }
                    plist = (__node_t *)plist->next;
                } else {
                    LOG(DEBUG) << "No element in list";  // This is valid
                    break;
                }
            } while (plist); // while
            if (!fromtag) {
                LOG(DEBUG) << "No from tag";  // This is valid
            }
        }
        else {
            LOG(DEBUG) << "No node list element";  // Will not cause a problem. Not sure is this is normal
        }

        if (!p->call_id) {
            LOG(DEBUG) << "No callID";
            return 401;
        }

        if (!p->cseq) {
            LOG(DEBUG) << "No sequence number";
            return 402;
        }

        delete [] qtag; // slag the old one, if any.

        int len = strlen(p->cseq->number) + 2 // crlf or --
#ifdef USE_CALL_ID_TAG
                  + strlen(p->call_id->number) + 1 // @
                  + strlen(p->call_id->host) + 2 // crlf or --
#endif
                  + strlen(fromtag) + 1; // null at end
        qtag = new char[len];
        // There's probably some fancy C++ way to do this.  FIXME.
        strcpy(qtag, p->cseq->number);
        strcat(qtag, "--");
#ifdef USE_CALL_ID_TAG
        strcat(qtag, p->call_id->number);
        strcat(qtag, "@");
        strcat(qtag, p->call_id->host);
        strcat(qtag, "--");
#endif
        strcat(qtag, fromtag);
        // Check the length calculation, abort if bad.
        if (qtag[len-1] != '\0'
            || qtag[len-2] == '\0') {
            LOG(DEBUG) << "qtag length check failed"; // Not sure why this is being done svg
            return 400;
        }

        // Set the taghash too.
        // FIXME, if we set this with a good hash function,
        // our linear searches will run much faster, avoiding
        // almost all strcmp's.  For now, punt easy.
        qtaghash = taghash_of(qtag);

        return 0;
    }

    /*
     * Hash a tag value for fast searches.
     */
    // FIXME, if we set this with a good hash function,
    // our linear searches will run much faster, avoiding
    // almost all strcmp's.  For now, punt easy.
    int ShortMsgPending::taghash_of (const char *fromtag)
    {
        return fromtag[0];
    }

    /* Check the host and port number specified.
     * Currently, be conservative and only take localhost refs.
     * We know FIXME that this will have to be expanded...
     */
    bool ShortMsgPending::check_host_port (char *host, char *port) {
        static int warn_once;

        if (!host)
            return false;
        if (!strcmp ("localhost", host) ||
            !strcmp ("127.0.0.1", host) /*||
            * FIXME: The message shouldn't check itself.
            (smp_my_ipaddress && !strcmp (smp_my_ipaddress, host))*/ /*||
            (smp_my_2nd_ipaddress && !strcmp (smp_my_2nd_ipaddress, host))*/) {
            ;
        } else {
            if (0 == warn_once++) {
                LOG(NOTICE) << "Accepting SIP Message from " << host <<
                            " for SMS delivery, even though it's not "
                            "from localhost.";
            }
            // In theory we should be *routing* it to where it
            // specifies -- but the address is *probably* us, and
            // we have no way to tell what our own address is,
            // since (in a real configuration) we're behind a NAT
            // firewall at an address that never appears in ifconfig,
            // for example.  Must assume it's OK -- for now.  FIXME.
            return true;
        }
        // port can either be specified or not.
        // we could check whether it's all digits...  FIXME

        return true;
    }

    //void ShortMsgPending::write_cdr(FILE * cdrfile, SubscriberRegistry& hlr) const {
//        char * from = parsed->from->url->username;
//        char * dest = parsed->to->url->username;
//        time_t now = time(NULL);  // Need real time for CDR
//
//        if (cdrfile) {
//            char * user = hlr.getIMSI2(from); // I am not a fan of this hlr call here. Probably a decent performance hit...
//            // source, sourceIMSI, dest, date
//            fprintf(cdrfile,"%s,%s,%s,%s", from, user, dest, ctime(&now));
//            fflush(cdrfile);
//        } else {
//            LOG(ALERT) << "CDR file at " << gConfig.getStr("CDRFile").c_str() << " could not be created or opened!";
//        }
    //}
}
