#ifndef OBTS_SMQUEUE_SHORTMSGPENDING_H
#define OBTS_SMQUEUE_SHORTMSGPENDING_H

#include <list>

// From SR Library
#include <SubscriberRegistry.h>

#include "shortmsg.h"

namespace kneedeepbts::smqueue {

    /*
     * States that a message can be in, while processed.
     * Messages come in, they need various lookups, then they go out.
     * Then they sit pending acknowledgment that they were successful.
     * The state machine is driven by incoming packets or information,
     * and in the absence of such incoming info, by timeouts.
     *
     * Note: If you change this enum, you MUST CHANGE SMqueue::timeouts and
     *       SMqueue::sm_state_strings.
     */
    enum sm_state {				// timeout, next-state-if-timeout
        NO_STATE,
        INITIAL_STATE, // 1
        REQUEST_FROM_ADDRESS_LOOKUP,
        ASKED_FOR_FROM_ADDRESS_LOOKUP,  //3

        AWAITING_TRY_DESTINATION_IMSI,
        REQUEST_DESTINATION_IMSI, // 5
        ASKED_FOR_DESTINATION_IMSI,

        AWAITING_TRY_DESTINATION_SIPURL,
        REQUEST_DESTINATION_SIPURL,  // 8
        ASKED_FOR_DESTINATION_SIPURL,

        AWAITING_TRY_MSG_DELIVERY, // 10
        REQUEST_MSG_DELIVERY,  // 11
        ASKED_FOR_MSG_DELIVERY, // 12

        DELETE_ME_STATE,

        AWAITING_REGISTER_HANDSET,
        REGISTER_HANDSET,
        ASKED_TO_REGISTER_HANDSET,

        STATE_MAX_PLUS_ONE,		/* Keep this one last! */
    };

    // How to print a state
    //std::string sm_state_strings[kneedeepbts::smqueue::STATE_MAX_PLUS_ONE];
    string sm_state_string(enum kneedeepbts::smqueue::sm_state astate);

    class ShortMsgPending : public ShortMsg {
    public:
        enum sm_state state;        // State of processing
        time_t next_action_time;    // When to do something different
        int retries;            // How many times we've retried
        // this message.
        char srcaddr[16]{};        // Source address (ipv4 or 6 or ...)
        socklen_t srcaddrlen;        // Valid length of src address.
        char *qtag;            // Tag that identifies this msg
        // uniquely in the queue.
        // (It is set 1st time msg is parsed.)
        int qtaghash;            // Simple hash of the qtag.
        char *linktag;            // Tag of a message that this message
        // is related to.  (We use this in
        // handset register messages, to find
        // the original SMS message that
        // prompted us to send the register.)

        static const char *smp_my_ipaddress;    // Static copy of my IP address
        // for validity checking of msgs.
        // (We get our own copy because
        // it's the only thing we need to
        // inherit from SMq.)

        /* Constructors */
        ShortMsgPending() :
                state(NO_STATE),
                next_action_time(0),
                retries(0),
                // srcaddr({0}),  // can't seem to initialize an array?
                srcaddrlen(0),
                qtag(NULL),
                qtaghash(0),
                linktag(NULL) {
        }

        // Make a pending short message, perhaps taking responsibility for
        // deleting the "new"-allocated memory passed in.
        ShortMsgPending(int len, char *const cstr, bool use_my_memory)
                : ShortMsg(len, cstr, use_my_memory),
                  state(NO_STATE),
                  next_action_time(0),
                  retries(0),
                // srcaddr({0}),  // can't seem to initialize an array?
                  srcaddrlen(0),
                  qtag(NULL),
                  qtaghash(0),
                  linktag(NULL) {
        }

//#if 0
//        short_msg_pending (std::string str) :
//        short_msg (str),
//        state (NO_STATE),
//        next_action_time (0),
//        retries (0),
//        // srcaddr({0}),  // can't seem to initialize an array?
//        srcaddrlen(0),
//        qtag (NULL),
//        qtaghash (0),
//        linktag (NULL)
//    {
//    }
//#endif

        //
        // We would've liked to declare this next function PRIVATE,
        // since that would prevent anyone from carelessly duplicating
        // short_msg_pending's in constructors.
        // Unfortunately, libstdc++'s <list> doesn't allow lists to be
        // created from classes that lack a copy constructor.
        //private:
        ShortMsgPending(const ShortMsgPending &smp) :
                ShortMsg(static_cast<const ShortMsg &>(smp)),
                state(smp.state),
                next_action_time(smp.next_action_time),
                retries(smp.retries),
                // srcaddr({0}),  // can't seem to initialize an array?
                srcaddrlen(smp.srcaddrlen),
                qtag(NULL),
                qtaghash(smp.qtaghash),
                linktag(NULL) {
            if (smp.srcaddrlen) {
                if (smp.srcaddrlen > sizeof(srcaddr)) {
                    LOG(DEBUG) << "Srcaddr data too large";
                    return;  // Can't continue
                } else {
                    memcpy(srcaddr, smp.srcaddr, smp.srcaddrlen);
                }
            }

            if (smp.qtag) {
                int len = strlen(smp.qtag);
                this->qtag = new char[len + 1];
                strncpy(this->qtag, smp.qtag, len);
                this->qtag[len] = '\0';
            }

            if (smp.linktag) {
                int len = strlen(smp.linktag);
                this->linktag = new char[len + 1];
                strncpy(this->linktag, smp.linktag, len);
                this->linktag[len] = '\0';
            }
        }

        /* Override operator= to avoid pointer-sharing problems */
    private:
        ShortMsgPending &operator=(const ShortMsgPending &rvalue);

    public:

        /* Destructor */
        virtual ~ShortMsgPending() {
            delete[] qtag;
            delete[] linktag;
        }

        /* Methods */

        /*
         * Most of the time we can't use a constructor, because we
         * want to create our short_msg_pending and put it in a list, without
         * copying it (which would involve lots of useless new/delete's,
         * particularly if we've parsed it into a huge nested struct).
         * So we usually make a list with one element (which uses the
         * default constructor) and then run initialize() on that element
         * with the same arguments we would've used for the constructor.
         * This 'constructs' the new short_msg_pending in a temporary list,
         * and we can then trivially move it into the real message queue,
         * removing it from the temporary list in the process.
         */
        // Make a pending short message, perhaps taking responsibility for
        // deleting the "new"-allocated memory passed in.
        void
        initialize(int len, char *const cstr, bool use_my_memory) {
            ShortMsg::initialize(len, cstr, use_my_memory);
            // initguts();
        }

//#if 0
//        short_msg_pending (std::string str) :
//        short_msg (str),
//        state (NO_STATE),
//        next_action_time (0),
//        retries (0),
//        // srcaddr({0}),  // can't seem to initialize an array?
//        srcaddrlen(0),
//        qtag (NULL),
//        qtaghash (0),
//        linktag (NULL)
//    {
//    }
//#endif


        time_t msgettime() {
            struct timespec tv;
            clock_gettime(CLOCK_REALTIME, &tv);
            unsigned long time_in_mill =
                    (tv.tv_sec * 1000UL) + (tv.tv_nsec / 1000000UL); // convert tv_sec & tv_usec to millisecond
            return time_in_mill;
        }


        /* Reset the message's state and timeout.  Timeout is set based
           on the current state and the new state.  */
        void set_state(enum sm_state newstate);

        /* Reset the message's state and timeout.  Timeout is argument.  */
        void set_state(enum sm_state newstate, time_t timeout) {
            next_action_time = timeout;
            state = newstate;
            /* If we're in a queue, some code in another class is now going
               to have to change our queue position.  */
        }

//        /* Check that the message is valid, and set the qtag and qtaghash
//           from the message's contents.   Result is 0 for valid, or
//           SIP response error code (e.g. 405).  */
//        // FIXME: This causes a circular dependency between SmqManager and this class
//        int validate_short_msg(SmqManager *manager, bool should_early_check);

        // Set the qtag and qtaghash from the parsed fields.
        // Whenever we change any of these fields, we have to recalculate
        // the qtag.
        // FIXME, we assume that the CSEQ, Call-ID, and From tag all are
        // components that, in combination, identify the message uniquely.
        // FIXME!  The spec is unpleasantly unclear about this.
        // Result is 0 for success, or 3-digit integer error code if error.
        int set_qtag();

        // Hash the tag to an int, for speedier searching.
        int taghash_of(const char *tag);

        /* Check host and port for validity.  */
        bool
        check_host_port(char *host, char *port);

        /* Generate a billing record. */
        void write_cdr(FILE * cdrfile, SubscriberRegistry &hlr) const;
    };

    typedef std::list<ShortMsgPending> short_msg_p_list;
}

#endif //OBTS_SMQUEUE_SHORTMSGPENDING_H
