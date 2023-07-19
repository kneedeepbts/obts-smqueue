#ifndef OBTS_SMQUEUE_SMQMANAGER_H
#define OBTS_SMQUEUE_SMQMANAGER_H

#include <list>
#include <string>

#include <osip2/osip.h>
#include <osipparser2/osip_message.h>
#include <cpptoml.h>

#include <Timeval.h>
#include <NodeManager.h>

#include "shortcodeparams.h"
#include "shortmsgpending.h"
#include "smnet.h"
#include "SmqMessageHandler.h"

#include "SmqReader.h"
#include "SmqWriter.h"
#include "smqacker.h"

/* Maximum text size of an SMS message.  */
#define SMS_MESSAGE_MAX_LENGTH  160

namespace kneedeepbts::smqueue {
    /*
     * Main class for SIP Short Message processing.
     * The daemon is designed to be running one copy of this class.
     */
    class SmqManager {
    public:
        // Put all timeouts in one place
        const static int TIMEOUTMS = 5000;
        const static int SMSRATELIMITMS = 1000;
        const static int LONGDELETMS = 5000000;   // 83 minutes  Used by SC.ZapQueued.Password
        const static int INCREASEACKEDMSGTMOMS = 60000;  // 5 minutes

        explicit SmqManager(std::shared_ptr<cpptoml::table> config);

        /* Destructor */
        ~SmqManager() { pthread_mutex_destroy(&sortedListMutex); }

        //void InitBeforeMainLoop();
        //void CleaupAfterMainreaderLoop();
        //void InitInsideReaderLoop();

        void unlockSortedList() { pthread_mutex_unlock(&sortedListMutex); }
        void lockSortedList() { pthread_mutex_lock(&sortedListMutex); }

        // This is the method to initialize and then run the main_loop
        void run();

//        /* Set my own IP address, since I can't tell how I look to others. */
//        void set_my_ipaddress(std::string myip) {
//            my_ipaddress = myip;
//            // Point to it for message validity checking.
//            // NOTE: that copy shares same storage as this one.
//            // FIXME: What't this for? And do we need it going forward?
//            kneedeepbts::smqueue::ShortMsgPending::smp_my_ipaddress = myip.c_str();
//        }

//        /* Set the global relay address (host:port string) */
//        void set_global_relay(std::string gr, std::string port, std::string contentType) {
//            global_relay = gr;
//            global_relay_port = port;
//            if (contentType.length()) {
//                if (contentType == "text/plain") {
//                    global_relay_contenttype = kneedeepbts::smqueue::ShortMsg::TEXT_PLAIN;
//                } else if (contentType == "application/vnd.3gpp.sms") {
//                    global_relay_contenttype = kneedeepbts::smqueue::ShortMsg::VND_3GPP_SMS;
//                }
//            } else {
//                global_relay_contenttype = kneedeepbts::smqueue::ShortMsg::VND_3GPP_SMS;
//            }
//        }

        // NOTE: Now get from the config.
//        /* Set the register host & port -- where to register handsets */
//        void set_register_hostport(std::string hp) {
//            my_register_hostport = hp;
//        }

        // FIXME: This should be pulled out into a "tcp/udp server" class.
        /* Initialize the listener -- sets up and opens my_network. */
//        bool init_listener (std::string port) {
//            //my_udp_port = port;
//            return my_network.listen_on_port (port);
//        }

        bool to_is_deliverable(const char *username);
        bool from_is_deliverable(const char *from);

        //bool convert_content_type(ShortMsgPending *message, kneedeepbts::smqueue::ShortMsg::ContentType to_type);

        /* Convert a short_msg to a given content type */
        //void convert_message(short_msg_pending *qmsg, short_msg::ContentType toType);

        // Main loop listening for dgrams and processing them.
        void main_loop(int tmo);

        /* If nothing happens for a while, handle that.  */
        void process_timeout();

        /* Send a SIP response to acknowledge reciept of a short msg. */
        void respond_sip_ack(int errcode, ShortMsgPending *smp, char *netaddr, size_t netaddrlen);

        /*
         * Originate a short message
         * Put it in the queue and start handling it.
         * From is a shortcode (currently),
         * To is an IMSI (currently),
         * msgtext is plain ASCII text.
         * Result is 0 for success, negative for error.
         */
        int originate_sm(const char *from, const char *to, const char *msgtext, enum sm_state firststate);

        /*
         * Originate half of a short message
         * Put it in the queue and start handling it, but don't actually
         * finish it or send it; return it to the caller for further mucking.
         *
         * In particular, the caller must set the:
         *    uri
         *    From:
         *    To:
         *    Content-Type and message body, if any.
         *
         * Method is which type of SIP packet (MESSAGE, RESPONSE, etc).
         * Result is a short_msg_p_list containing one short_msg_pending, with
         * the half-initialized message in it.
         */
        short_msg_p_list * originate_half_sm(std::string method);

        /*
         * Send a bounce message, based on an existing queued message.
         * Return the state to set the original bouncing message to.
         */
        enum sm_state bounce_message(ShortMsgPending *sent_msg, const char *errstr);

        /*
         * See if the handset's imsi and phone number are in the HLR
         * database yet, since if it isn't, we can't register the imsi
         * at its cell's host:port yet.
         */
        bool ready_to_register (short_msg_p_list::iterator qmsg);

        /*
         * Register a handset's IMSI with its cell, by sending Asterisk
         * a SIP REGISTER message that we "relay" from the cell.  (We
         * actually originate it, but we pretend that the cell sent it to us.
         * Actually the cell sent us an incoming shortcode SMS from an
         * unregistered phone, which is in the queue in REGISTER_HANDSET state.)
         */
        enum sm_state register_handset (short_msg_p_list::iterator qmsg);

        /* Check if this is a short-code message and handle it.
         * Return true if message has been handled, false if you should continue
         * message handling as usual. In latter case \p next_state is untouched.
         */
        bool handle_short_code(const short_code_map_t &short_code_map, short_msg_p_list::iterator qmsg, enum sm_state &next_state);

        /* When a SIP response arrives, search the queue for its matching
           MESSAGE and handle both.  */
        void handle_response(short_msg_p_list::iterator qmsg);

        /* Search the message queue to find a message whose tag matches.  */
        bool find_queued_msg_by_tag(short_msg_p_list::iterator &mymsg, const char *tag, int taghash);

        /* Same, but without a known taghash. */
        bool find_queued_msg_by_tag(short_msg_p_list::iterator &mymsg, const char *tag);

        /*
         * Look up the hostname and port number where we should send Short
         * Messages for the IMSI in the To address.
         *
         * This is also where we assign a new Call-ID to the message, so that
         * re-sends will use the same Call-ID, but re-locate's (looking up the
         * recipient's location again) will use a new one.
         */
        enum sm_state lookup_uri_hostport (ShortMsgPending *qmsg);

        /*
         * Change the From address username to a valid phone number in format:
         *     +countrycodephonenum
         */
        enum sm_state lookup_from_address(ShortMsgPending *qmsg);

        /* Change the Request-URI's address to a valid IMSI.  */
        enum sm_state lookup_uri_imsi (ShortMsgPending *qmsg);

        // For memory allocation simplicity, it's easiest to create
        // new messages as a 1-entry short_msg_p_list and then move
        // them to the real list.  Note that this moves the message's list
        // entry itself off the original list (which can then be discarded).
        // Push_front only does a copy so use splice ??
        void insert_new_message(short_msg_p_list &smp) {
            lockSortedList();
            time_sorted_list.splice (time_sorted_list.begin(), smp);
            time_sorted_list.begin()->set_state (INITIAL_STATE);  // Note set state can move the entries in the queue
            // time_sorted_list.begin()->timeout = 0;  // it is already
            // Low timeout will cause this msg to be at front of queue.
            unlockSortedList();
            debug_dump(); //svgfix
            //m_writer.ProcessReceivedMsg(); // FIXME: Create a RawMessage and send to the writer?
        }

        // This version lets the initial state be set.
        void insert_new_message(short_msg_p_list &smp, enum sm_state s) {
            LOG(DEBUG) << "Insert message into queue 2";
            lockSortedList();
            time_sorted_list.splice (time_sorted_list.begin(), smp);
            time_sorted_list.begin()->set_state (s);
            // time_sorted_list.begin()->timeout = 0;  // it is already
            // Low timeout will cause this msg to be at front of queue.
            unlockSortedList();
            debug_dump(); //svgfix
            //m_writer.ProcessReceivedMsg(); // FIXME: Create a RawMessage and send to the writer?
        }
        // This version lets the state and timeout be set.
        void insert_new_message(short_msg_p_list &smp, enum sm_state s, time_t t) {
            LOG(DEBUG) << "Insert message into queue 3";
            lockSortedList();
            time_sorted_list.splice (time_sorted_list.begin(), smp);
            time_sorted_list.begin()->set_state (s, t);
            unlockSortedList();
            debug_dump(); //svgfix
            //ProcessReceivedMsg();
            QueuedMsgHdrs* pMsg = new ProcessIncommingMsg();
            SimpleWrapper* sWrap = new SimpleWrapper(pMsg);
            //SendWriterMsg(sWrap);
            //m_writer.getqueHan()->SmqSendMessage(sWrap); // FIXME: Create a RawMessage and send to the writer?
        }

        /* Debug dump of the queue and the SMq class in general. */
        void debug_dump();

        // Set the linktag of "newmsg" to point to oldmsg.
        void set_linktag(short_msg_p_list::iterator newmsg,
                         short_msg_p_list::iterator oldmsg);

        // Get the old message that this message links to.
        bool get_link(short_msg_p_list::iterator &oldmsg,
                      short_msg_p_list::iterator qmsg);

        /*
          * When we reset the state and timestamp of a message,
         * we need to reinsert it into the queue.  C++ doesn't seem
         * to have a standard container that lets one do this cleanly.
         * Even with lists it's a kludge -- we have to do our own search.
         * With multisets you can't splice an element out while keeping the
         * element... etc.
         */
        void set_state(short_msg_p_list::iterator sm, enum sm_state newstate) {
            short_msg_p_list temp;
            lockSortedList();
            /* Extract the current sm from the time_sorted_list */
            temp.splice(temp.begin(), time_sorted_list, sm);
            sm->set_state(newstate);
            // Insert it according to the new timestamp.
            // One would think that "multisets" could do this simply,
            // but they don't appear to have this capability.
            // Note: if list is empty, or all are too early, insert at end.
            for (short_msg_p_list::iterator x = time_sorted_list.begin();
                 true; x++) {
                if (x == time_sorted_list.end()
                    || x->next_action_time >= sm->next_action_time) {
                    time_sorted_list.splice(x, temp);
                    break;
                }
            }
            unlockSortedList();
        } // set_state

        /*
          * When we reset the state and timestamp of a message,
         * we need to reinsert it into the queue.  C++ doesn't seem
         * to have a standard container that lets one do this cleanly.
         * Even with lists it's a kludge -- we have to do our own search.
         * With multisets you can't splice an element out while keeping the
         * element... etc.
         */
        void set_state(short_msg_p_list::iterator sm, enum sm_state newstate, time_t timestamp) {
            lockSortedList();
            short_msg_p_list temp;
            /* Extract the current sm from the time_sorted_list */
            temp.splice(temp.begin(), time_sorted_list, sm);
            sm->set_state(newstate, timestamp);
            // Insert it according to the new timestamp.
            // One would think that "multisets" could do this simply,
            // but they don't appear to have this capability.
            // Note: if list is empty, or all are too early, insert at end.
            for (short_msg_p_list::iterator x = time_sorted_list.begin();
                 true; x++) {
                if (x == time_sorted_list.end()
                    || x->next_action_time >= sm->next_action_time) {
                    time_sorted_list.splice(x, temp);
                    break;
                }
            }
            unlockSortedList();
        } // set_state

        /* Save the queue to a file; read it back from a file.
           Reading a queue file doesn't delete things that might already
            be in the queue; if you want a clean queue, delete anything
           already in the queue first.  */
        //bool save_queue_to_file(std::string qfile);
        bool save_queue_to_file();
        //bool read_queue_from_file(std::string qfile);
        bool read_queue_from_file();

        /* Check that the message is valid, and set the qtag and qtaghash
           from the message's contents.   Result is 0 for valid, or
           SIP response error code (e.g. 405).  */
        // FIXME: This causes a circular dependency between SmqManager and this class
        //int validate_short_msg(ShortMsgPending *smp, bool should_early_check);

    private:
        // Override operator= so -Weffc++ doesn't complain
        // *DISABLE* assignments by making the = operation private.
        SmqManager & operator= (const SmqManager &rvalue);

        std::shared_ptr<cpptoml::table> m_config;

        /* A list of all messages we know about, sorted by time of next action (assuming nothing arrives to change our
         * mind before that time). */
        std::list<ShortMsgPending> time_sorted_list{};

        //std::string savefile;
        bool please_re_exec = false;

        pthread_mutexattr_t mutexSLAttr{};
        pthread_mutex_t sortedListMutex{};

        /* The network sockets that we're using for I/O */
        SMqueue::SMnet my_network{};

        /* The interface to the Host Location Register for routing
           messages and looking up their return and destination addresses.  */
        SubscriberRegistry my_hlr{};

        /* Where to send SMS's that we can't route locally. */
        //std::string global_relay;
        //std::string global_relay_port;
        kneedeepbts::smqueue::ShortMsg::ContentType global_relay_contenttype = kneedeepbts::smqueue::ShortMsg::VND_3GPP_SMS;

        /* The call-ID, CSeq, and flag we use in registration requests */
        std::string register_call_id;
        int register_call_seq = 0;
        bool have_register_call_id = false; // FIXME: Trigger off "" value of call_id?

        /* Set this to true when you want main loop to stop.  */
        bool m_stop_main_loop = false;

        /* Set this to true when you want the program to re-exec itself
           instead of terminating after the main loop stops.  */
        //bool reexec_smqueue = false; // FIXME: Is this necessary?

        // Bringing here from smqueue "global"
        //bool print_as_we_validate = false;
        //bool osip_initialized = false;
        osip_t * osip = nullptr;
        //struct osip *osipptr = nullptr; // Ptr to struct sorta used by library
        FILE * m_cdrfile = nullptr;
        short_code_map_t short_code_map{};

        /** The remote node manager. */
        //NodeManager m_NodeManager;

        /** rate limiting timer */
        Timeval spacingTimer;

        SmqReader m_reader;
        SmqWriter m_writer{};
        SmqAcker m_acker;
    };
}

#endif //OBTS_SMQUEUE_SMQMANAGER_H
