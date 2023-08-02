#include "shortmsg.h"

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE
#include "spdlog/spdlog.h"

namespace kneedeepbts::smqueue {
    /* Parsing, validating, and unparsing messages.
         * Return
         * 	False if failed
        */
    bool ShortMsg::parse() {
        SPDLOG_DEBUG("ShortMsg::parse()");
        int i;
        osip_message_t *sip;

        if (parsed_is_valid)
            return true;

        // FIXME: Verify that osip initialization happens before parsing a message
        //        is even attempted.
//            if (!osip_initialized) {
//                //LOG(DEBUG) << "Calling osip_init";
//                i = osip_init(&osipptr);
//                if (i != 0) {
//                    LOG(DEBUG) << "osip_init failed error " << i;
//                    return false;
//                }
//                osip_initialized = true;
//            }

        SPDLOG_DEBUG("unparse?");
        unparse();	// Free any previous one.

        // Parse SIP message
        SPDLOG_DEBUG("Calling osip_message_init");
        i = osip_message_init(&sip);
        if (i != 0)  {
            SPDLOG_DEBUG("osip_message_init failed error {}", i);
            return false;
        }

        SPDLOG_DEBUG("Calling osip_message_parse");
        i = osip_message_parse(sip, text, text_length);
        if (i != 0) {
            SPDLOG_DEBUG("osip_message_parse failed error ", i);
            return false;
        }

        parsed = sip;
        parsed_is_valid = true;
        parsed_is_better = false;
        SPDLOG_DEBUG("Parsing done.");

        // Now parse SMS if needed
        if (parsed->content_type == nullptr)
        {
            SPDLOG_DEBUG("SMS content not set continue anyway??");
            // Most likely this is SIP response.
            content_type = UNSUPPORTED_CONTENT;
            // SVG Consider not continuing
        } else if ( strcmp(parsed->content_type->type, "text") == 0 && strcmp(parsed->content_type->subtype, "plain") == 0)
        {
            // If Content-Type is text/plain, then no decoding is needed.
            content_type = TEXT_PLAIN;
            //LOG(DEBUG) << "SMS message encoded text";
        } else if ( strcmp(parsed->content_type->type, "application") == 0 && strcmp(parsed->content_type->subtype, "vnd.3gpp.sms") == 0)
        {
            // This is an encoded SMS' TPDU.
            SPDLOG_DEBUG("SMS message encoded 3GPP");
            content_type = VND_3GPP_SMS;

#if 1
            // (pat 10-2014) Originally we sent the RPDUs encoded as hex, which was incorrect.
            // OpenBTS now optionally sends the RPDU in base64 as per 3GPP 24.341, and adds the official "Content-Transfer-Encoding: base64" header.
            // Unfortunately this SIP library does not have any obvious way to get that information out, so just
            // grunge though the text of the SIP message.  This sucks.
            string encoding = "hex";	// (pat) Use hex if other encoding not explicitly specified for backward compatibility.
            static const char *cteHeader = "\r\nContent-Transfer-Encoding";
            char *contentTransferEncoding = strcasestr(this->text,cteHeader);
            if (contentTransferEncoding) {
                char *cp = contentTransferEncoding + strlen(cteHeader);
                while (isspace(*cp) || *cp == ':') { cp++; }
                encoding = scanWord(cp);
            }

            char *endp = strstr(this->text,"\r\n\r\n");	// Find the beginning of the message body.
            int content_len = parsed->content_length && parsed->content_length->value ? atoi(parsed->content_length->value) : 0;
            rp_data = SMS::decodeRPData(endp + 4, content_len, encoding);
#else
            // (pat 10-2014) This is the original code for hex encoded message body.
				// Decode it RP-DATA
				// (pat 10-2014) Add some checks so we dont crash.
				if (parsed->bodies.node == NULL || parsed->bodies.node->element == NULL) {
					LOG(ERR)<< "SIP Parse error 1";
					return true;
				}
				osip_body_t *bod1 = (osip_body_t *)parsed->bodies.node->element;
				const char *bods = bod1->body;
				if (bods == NULL) { // (pat 10-2014) More checks so we dont crash.
					LOG(ERR)<< "SIP Parse error 2";
					return true;
				}
				//LOG(DEBUG) << "Calling hex2rpdata";
				rp_data = hex2rpdata(bods,true);
#endif
            if (rp_data == NULL) {
                SPDLOG_DEBUG("RP-DATA length is zero");  // This is okay
                return true;
            }

            // Decode RPDU
            tl_message = parseTPDU(rp_data->TPDU());
            if (tl_message == NULL) {
                SPDLOG_INFO("TPDU parsing failed");
                return false;
            }
        }

        return true;
    }
}
