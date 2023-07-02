#ifndef OBTS_SMQUEUE_SHORTCODEPARAMS_H
#define OBTS_SMQUEUE_SHORTCODEPARAMS_H

#include <list>
#include <map>
#include <string>

#include "shortmsgpending.h"

namespace kneedeepbts::smqueue {
    /*
     * Function parameters and return value for short-code "command" functions that
     * process SMS messages internally rather than sending the SMS message
     * on to somebody else.
     */
    enum short_code_action {
        SCA_DONE = 0, ///< No further processing is needed. Free message.
        SCA_INTERNAL_ERROR = 1, //< Just report error and bail out.
        SCA_REPLY = 2, ///< Free this message and send replay back to the msg sender
        ///< with a text from params.scp_reply
        SCA_RETRY_AFTER_DELAY = 3, ///< HLR is busy. Retry query later.
        SCA_REPLY_AND_RETRY = 4, ///< UNUSED.
        SCA_QUIT_SMQUEUE = 5, ///< Self-explanatory. Exit smqueue.
        SCA_AWAIT_REGISTER = 6, ///< HLR response is delayed. Wait.
        SCA_REGISTER = 7, ///< HLR record for this phone has been retrieved.
        ///< Proceed to registration with Asterisk.
        SCA_TREAT_AS_ORDINARY = 8, ///< Continue msg processing as if it were non-shortcode msg.
        SCA_EXEC_SMQUEUE = 9, ///< Fork new smqueue instance and exit this one.
        SCA_RESTART_PROCESSING = 10 ///< Return from this short code processing
        ///< and run another short code.
    };

    class short_code_params {
    public:
        int scp_retries; // in: 0 if first call for this msg
        //SmqManager *scp_smq; // in: The entire "global" SMq structure
        std::list<kneedeepbts::smqueue::ShortMsgPending>::iterator scp_qmsg_it; // in: iterator for our msg
        char *scp_reply; // out: Reply msg to sender
        int scp_delay; // out: Delay before re-call

        short_code_params() :
                scp_retries(0),
                //scp_smq(NULL),
                scp_qmsg_it(),
                scp_reply(NULL),
                scp_delay(0) {}

    private:
        // Avoid copy-construction and assignment by making private.
        short_code_params(const short_code_params &) :
                scp_retries(0),
                //scp_smq(NULL),
                scp_qmsg_it(),
                scp_reply(NULL),
                scp_delay(0) {}

        short_code_params &operator=(const short_code_params &);

    public:

        ~short_code_params() {
            delete[] scp_reply;
        }
    };

    // Function pointer declaration for short-code action functions
    typedef enum short_code_action (*short_func_t)(const char *imsi, const char *msgtext, short_code_params *scp);

    // Maps description (phone number) to function to call
    typedef std::map <std::string, short_func_t> short_code_map_t;

}

#endif //OBTS_SMQUEUE_SHORTCODEPARAMS_H
