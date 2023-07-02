#ifndef OBTS_SMQUEUE_SHORTMSG_H
#define OBTS_SMQUEUE_SHORTMSG_H

#include <string>

#include <osip2/osip.h>

// From SMS Library
#include <SMSMessages.h>

namespace kneedeepbts::smqueue {
    /* In-memory object representing a Short Message.  These are kept as
    text strings (as we received them) and only parsed when we need to
    process them.  This keeps memory usage way down for medium to long
    term storage in the queue.  */
    class ShortMsg {
    public:

        enum ContentType {
            UNSUPPORTED_CONTENT,
            TEXT_PLAIN,
            VND_3GPP_SMS
        };

        /* First just the text string.   A SIP message including body. */
        unsigned short text_length;
        char *text /* [text_length] */;  // C++ doesn't make it simple

        /* Now a flag for whether it's been parsed, and a parsed copy. */
        bool parsed_is_valid;
        /* If the parsed message has been modified, such that the string
           copy is no longer valid, this will be true.  */
        bool parsed_is_better;
        osip_message_t *parsed;
        // from;
        // to;
        // time_t date;
        // expiration;
        ContentType content_type; // Content-Type of the message
        ContentType convert_content_type; // Content type to convert to, or UNSUPPORTED_CONTENT

        SMS::RPData *rp_data; // Parsed RP-DATA of an SMS. It's read from MESSAGE body if
        // it has application/vnd.3gpp.sms MIME-type. Note, that
        // it's parsed on request and may be NULL at any point.
        SMS::TLMessage *tl_message; // Parsed RPDU of an SMS. It's read from rp_data. Note,
        // that it's parsed on request and may be NULL at
        // any point.
        bool ms_to_sc; // Direction of the message. True is this is MS->SC SMS, false
        // otherwise.
        bool need_repack; // Message should be packed into TPDU for delivery. E.g.
        // SIP MESSAGE sent to MS should be packed, while SIP
        // REGISTER should not.

        bool from_relay;

        ShortMsg () :
                text_length (0),
                text (NULL),
                parsed_is_valid (false),
                parsed_is_better (false),
                parsed (NULL),
                content_type(UNSUPPORTED_CONTENT),
                convert_content_type(UNSUPPORTED_CONTENT),
                rp_data(NULL),
                tl_message(NULL),
                ms_to_sc(false),
                need_repack(true),
                from_relay(false)
        {
        }
        // Make a short message, perhaps taking responsibility for deleting
        // the "new"-allocated memory passed in.
        ShortMsg (int len, char * const cstr, bool use_my_memory) :
                text_length (len),
                text (cstr),
                parsed_is_valid (false),
                parsed_is_better (false),
                parsed (NULL),
                content_type(UNSUPPORTED_CONTENT),
                convert_content_type(UNSUPPORTED_CONTENT),
                rp_data(NULL),
                tl_message(NULL),
                ms_to_sc(false),
                need_repack(true),
                from_relay(false)
        {
            if (!use_my_memory) {
                text = new char [text_length+1];
                strncpy(text, cstr, text_length);
                text[text_length] = '\0';
            }
        };

        /* When created from another short_msg, must copy the string. */
        // We would've liked to make this private, so that we can check at
        // compile time that nobody's inadvertently making copies (malloc's)
        // of short_msg classes -- but unfortunately G++'s <list> doesn't
        // allow lists of classes that have no copy-constructors!
        //private:
        ShortMsg (const ShortMsg &sm) :
                text_length (sm.text_length),
                text (0),
                parsed_is_valid (false),
                parsed_is_better (false),
                parsed (NULL),
                content_type(UNSUPPORTED_CONTENT),
                convert_content_type(UNSUPPORTED_CONTENT),
                rp_data(NULL),
                tl_message(NULL),
                ms_to_sc(false),
                need_repack(true),
                from_relay(sm.from_relay)
        {
            if (text_length) {
                text = new char [text_length+1];
                strncpy(text, sm.text, text_length);
                text[text_length] = '\0';
            }
        };

//#if 0
//        short_msg (std::string str) :
//		text_length (str.length()),
//		text (0),
//		parsed_is_valid (false),
//		parsed_is_better (false),
//		parsed (NULL),
//		content_type(UNSUPPORTED_CONTENT),
//		convert_content_type(UNSUPPORTED_CONTENT),
//		rp_data(NULL),
//		tl_message(NULL),
//		ms_to_sc(false),
//		need_repack(false)
//	{
//		text = new char [text_length+1];
//		strncpy(text, str.data(), text_length);
//		text[text_length] = '\0';
//	};
//#endif

        /* Disable operator= to avoid pointer-sharing problems */
    private:
        ShortMsg & operator= (const ShortMsg &rvalue);

    public:

        /* Destructor */
        virtual ~ShortMsg ()
        {
            if (parsed)
                osip_message_free(parsed);
            delete [] text;
            delete rp_data;
            delete tl_message;
        }

        // Pseudo-constructor due to inability to run constructors on
        // members of lists.
        // Initialize a newly-default-constructed short message,
        // perhaps taking responsibility for deleting
        // the "new"-allocated memory passed in.
        void
        initialize (int len, char * const cstr, bool use_my_memory)
        {
            // default constructor needs these things revised to
            // initialize with a message in a string.
            text_length = len;
            text = cstr;
            if (!use_my_memory) {
                text = new char [text_length+1];
                strncpy(text, cstr, text_length);
                text[text_length] = '\0';
            }
        }

        string scanWord(char *start) {
            char *ep = start;
            while (isalpha(*ep) || isdigit(*ep) || *ep == '_') ep++;
            return string(start,ep-start);
        }

        /* Parsing, validating, and unparsing messages.
         * Return
         * 	False if failed
        */
        bool parse() {
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

            unparse();	// Free any previous one.

            // Parse SIP message
            //LOG(DEBUG) << "Calling osip_message_init";
            i = osip_message_init(&sip);
            if (i != 0)  {
                LOG(DEBUG) << "osip_message_init failed error " << i;
                return false;
            }

            //LOG(DEBUG) << "Calling osip_message_parse";
            i = osip_message_parse(sip, text, text_length);
            if (i != 0) {
                LOG(DEBUG) << "osip_message_parse failed error " << i;
                return false;
            }

            parsed = sip;
            parsed_is_valid = true;
            parsed_is_better = false;

            // Now parse SMS if needed
            if (parsed->content_type == NULL)
            {
                LOG(DEBUG) << "SMS content not set continue anyway??";
                // Most likely this is SIP response.
                content_type = UNSUPPORTED_CONTENT;
                // SVG Consider not continuing
            } else if (  strcmp(parsed->content_type->type, "text") == 0
                         && strcmp(parsed->content_type->subtype, "plain") == 0)
            {
                // If Content-Type is text/plain, then no decoding is needed.
                content_type = TEXT_PLAIN;
                //LOG(DEBUG) << "SMS message encoded text";
            } else if (  strcmp(parsed->content_type->type, "application") == 0
                         && strcmp(parsed->content_type->subtype, "vnd.3gpp.sms") == 0)
            {
                // This is an encoded SMS' TPDU.
                LOG(DEBUG) << "SMS message encoded 3GPP";
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
                    LOG(INFO) << "RP-DATA length is zero";  // This is okay
                    return true;
                }

                // Decode RPDU
                tl_message = parseTPDU(rp_data->TPDU());
                if (tl_message == NULL) {
                    LOG(INFO) << "TPDU parsing failed";
                    return false;
                }
            }

            return true;
        } // parse

        /* Anytime a caller CHANGES the values in the parsed tree of the
           message, they MUST call this, to let the caching system
           know that the cached copy of the text-string message is no
           longer valid.  Actually, we have TWO such cached copies!  FIXME
           so we have to invalidate both of them. */
        void
        parsed_was_changed() {
            //LOG(DEBUG) << "Calling osip_message_force_update";
            parsed_is_better = true;
            osip_message_force_update(parsed);   // Tell osip library too
        }

        /* Make the text string valid, if the parsed copy is better.
           (It gets "better" by being modified, and parsed_was_changed()
           got called, but we deferred fixing up the text string till now.) */
        void
        make_text_valid() {
            if (parsed_is_better) {
                /* Make or remake text string from parsed version. */
                char *dest = NULL;
                size_t length = 0;

                if (!parsed_is_valid) {
                    LOG(DEBUG) << "Parsed is not valid";
                    return;
                }

                //LOG(DEBUG) << "Calling osip_message_to_str";
                int i = osip_message_to_str(parsed, &dest, &length);
                if (i != 0) {
                    LOG(DEBUG) << "osip_message_to_str failed";  // Is this fatal
                }
                delete [] text;
                /* Because "osip_free" != "delete", we have to recopy
                   the string!!!  Don't you love C++?  */
                text_length = length;
                text = new char [text_length+1];
                strncpy(text, dest, text_length);
                text[text_length] = '\0';
                osip_free(dest);
                parsed_is_valid = true;
                parsed_is_better = false;
            }
            if (text == NULL) {
                LOG(DEBUG) << "text is null";
                return;
            }
        }

        /* Free up all memory used by parsed version of message. */
        void unparse() {
            // Free parsed TDPU.
            // FIXME -- We should check if it has been changed and update MESSAGE body.
            delete rp_data;
            rp_data = NULL;
            delete tl_message;
            tl_message = NULL;
            content_type = UNSUPPORTED_CONTENT;

            // Now unparse SIP
            if (parsed_is_better)
                make_text_valid();
            if (parsed)
                osip_message_free(parsed);
            parsed = NULL;
            parsed_is_valid = false;
            parsed_is_better = false;
        } //unparse


        // Get text for short message
        std::string get_text() const
        {
            switch (content_type) {
                case TEXT_PLAIN: {
                    if (parsed->bodies.node != 0) {
                        osip_body_t *bod1 = (osip_body_t *)parsed->bodies.node->element;
                        return bod1->body;
                    } else {
                        return "";
                    }
                }
                    break;

                case VND_3GPP_SMS: {
                    if (tl_message == NULL) {
                        return "";
                    }

                    LOG(DEBUG) << "Trying to decode message " << (ms_to_sc?"MS->SC":"SC->MS")
                               << " MTI=" << tl_message->MTI() << " tl_message: " << *tl_message;

                    try {
                        std::string decoded_text;
                        // MS->SC and SC-MS messages have to be handled separately
                        if (ms_to_sc) {
                            // Only SUBMIT messages have text data.
                            if (tl_message->MTI() != SMS::TLMessage::SUBMIT) {
                                LOG(NOTICE) << "Can't decode MS->SC message with MTI=" << tl_message->MTI();
                            } else {
                                const SMS::TLSubmit *submit = (SMS::TLSubmit*)tl_message;
                                decoded_text = submit->UD().decode();
                            }
                        } else {
                            // Only DELIVER messages have text data.
                            if (tl_message->MTI() != SMS::TLMessage::DELIVER) {
                                LOG(NOTICE) << "Can't decode SC->MS message with MTI=" << tl_message->MTI();
                            } else {
                                const SMS::TLDeliver *deliver = (SMS::TLDeliver*)tl_message;
                                decoded_text = deliver->UD().decode();
                            }
                        }
                        LOG(NOTICE) << "Decoded text: " << decoded_text;
                        return decoded_text;
                    }
                    catch (SMS::SMSReadError) {
                        LOG(DEBUG) << "SMS parsing failed (above L3)";
                        // TODO:: Should we send error back to the phone?
                        return "";
                    }

                }
                    break;

                case UNSUPPORTED_CONTENT:
                default:
                    return "";
            } // switch
        } // get_text


        /* Kind of a nasty hack to convert a message as it is going out. Generally from rpdu to text. */
        void convert_message(ContentType to) {
            convert_content_type = to;
        }


        void do_not_convert_message() {
            convert_content_type = UNSUPPORTED_CONTENT;
        }
    };
}


#endif //OBTS_SMQUEUE_SHORTMSG_H
