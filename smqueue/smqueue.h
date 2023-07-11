/*
 * SMqueue.h - In-memory queue manager for Short Messages (SMS's) for OpenBTS.
 * Written by John Gilmore, July 2009.
 *
 * Copyright 2009, 2014 Free Software Foundation, Inc.
 * Copyright 2014 Range Networks, Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * See the COPYING file in the main directory for details.
 */


#ifndef SM_QUEUE_H
#define SM_QUEUE_H

//#include "SmqGlobals.h"
//#include <time.h>
////#include <osipparser2/osip_message.h>	/* from osipparser2 */
//#include <stdlib.h>			/* for osipparser2 */
//#include <sys/time.h>			/* for osip_init */
//#include <osip2/osip.h>			/* for osip_init */
//#include <list>
//#include <map>
//#include <string>
//#include <iostream>
//#include <stdio.h>
//
//#include "smnet.h" // My network support
//#include <SubscriberRegistry.h> // My home location register
//
//#include <Logger.h>
//
//#include "shortmsg.h"
//#include "shortmsgpending.h"
//#include "smqmanager.h"

//void ProcessReceivedMsg();

// That's awful OSIP has a CR define.
// It clashes with our innocent L2Address::CR().
// Don't create 2-letter #defines, ever!
//#undef CR
//#include "SMSMessages.h"
//using namespace SMS;


//namespace SMqueue {

///* Maximum text size of an SMS message.  */
//#define SMS_MESSAGE_MAX_LENGTH  160


/* strdup uses malloc, which doesn't play well with new/delete.
   The idiots who defined C++ don't provide one, so we will. */
//char *new_strdup(const char *orig);

///*
// * States that a message can be in, while processed.
// * Messages come in, they need various lookups, then they go out.
// * Then they sit pending acknowledgment that they were successful.
// * The state machine is driven by incoming packets or information,
// * and in the absence of such incoming info, by timeouts.
// *
// * Note: If you change this enum, you MUST CHANGE SMqueue::timeouts and
// *       SMqueue::sm_state_strings.
// */
//
//enum sm_state {				// timeout, next-state-if-timeout
//	NO_STATE,
//	INITIAL_STATE, // 1
//	REQUEST_FROM_ADDRESS_LOOKUP,
//	ASKED_FOR_FROM_ADDRESS_LOOKUP,  //3
//
//	AWAITING_TRY_DESTINATION_IMSI,
//	REQUEST_DESTINATION_IMSI, // 5
//	ASKED_FOR_DESTINATION_IMSI,
//
//	AWAITING_TRY_DESTINATION_SIPURL,
//	REQUEST_DESTINATION_SIPURL,  // 8
//	ASKED_FOR_DESTINATION_SIPURL,
//
//	AWAITING_TRY_MSG_DELIVERY, // 10
//	REQUEST_MSG_DELIVERY,  // 11
//	ASKED_FOR_MSG_DELIVERY, // 12
//
//	DELETE_ME_STATE,
//
//	AWAITING_REGISTER_HANDSET,
//	REGISTER_HANDSET,
//	ASKED_TO_REGISTER_HANDSET,
//
//	STATE_MAX_PLUS_ONE,		/* Keep this one last! */
//};

//#define STATE_MAX  (STATE_MAX_PLUS_ONE - 1)
//
//// How to print a state
//extern std::string sm_state_strings[kneedeepbts::smqueue::STATE_MAX_PLUS_ONE];
//extern string sm_state_string(enum kneedeepbts::smqueue::sm_state astate);

/* Set this once we've called the initializer for the OSIP parser library. */
//extern bool osip_initialized;
//extern struct osip *osipptr;




// I couldn't figure out how to make these static members of the class...
/* Timeouts when going from NO_STATE into each subsequent state */
//    /*static*/ int timeouts_NO_STATE[STATE_MAX_PLUS_ONE];

/* Index to all timeouts */
//extern /*static*/ int (*timeouts[kneedeepbts::smqueue::STATE_MAX_PLUS_ONE])[kneedeepbts::smqueue::STATE_MAX_PLUS_ONE];

//class SMq;



//typedef std::list<kneedeepbts::smqueue::ShortMsgPending> short_msg_p_list;

///*
// * Function parameters and return value for short-code "command" functions that
// * process SMS messages internally rather than sending the SMS message
// * on to somebody else.
// */
//enum short_code_action {
//	SCA_DONE = 0, ///< No further processing is needed. Free message.
//	SCA_INTERNAL_ERROR = 1, //< Just report error and bail out.
//	SCA_REPLY = 2, ///< Free this message and send replay back to the msg sender
//	               ///< with a text from params.scp_reply
//	SCA_RETRY_AFTER_DELAY = 3, ///< HLR is busy. Retry query later.
//	SCA_REPLY_AND_RETRY = 4, ///< UNUSED.
//	SCA_QUIT_SMQUEUE = 5, ///< Self-explanatory. Exit smqueue.
//	SCA_AWAIT_REGISTER = 6, ///< HLR response is delayed. Wait.
//	SCA_REGISTER = 7, ///< HLR record for this phone has been retrieved.
//	                  ///< Proceed to registration with Asterisk.
//	SCA_TREAT_AS_ORDINARY = 8, ///< Continue msg processing as if it were non-shortcode msg.
//	SCA_EXEC_SMQUEUE = 9, ///< Fork new smqueue instance and exit this one.
//	SCA_RESTART_PROCESSING = 10 ///< Return from this short code processing
//	                                 ///< and run another short code.
//};
//
//class short_code_params {
//  public:
//	int scp_retries;		// in: 0 if first call for this msg
//    kneedeepbts::smqueue::SmqManager *scp_smq;			// in: The entire "global" SMq structure
//	kneedeepbts::smqueue::short_msg_p_list::iterator scp_qmsg_it; // in: iterator for our msg
//	char *scp_reply;		// out: Reply msg to sender
//	int scp_delay;			// out: Delay before re-call
//
//	short_code_params() :
//		scp_retries(0), scp_smq (NULL), scp_qmsg_it (),
//		scp_reply (NULL), scp_delay (0)
//	{ }
//  private:
//	// Avoid copy-construction and assignment by making private.
//	short_code_params(const short_code_params &) :
//		scp_retries(0), scp_smq (NULL), scp_qmsg_it (),
//		scp_reply (NULL), scp_delay (0)
//	{ }
//	short_code_params & operator=(const short_code_params &);
//  public:
//
//	~short_code_params() {
//		delete [] scp_reply;
//	}
//};
//
//// Function pointer declaration for short-code action functions
//typedef enum short_code_action (*short_func_t)
//			(const char *imsi, const char *msgtext,
//			 short_code_params *scp);

/*
 * Associative map between target phone numbers (short codes) and
 * function pointers that implement those numbers.
 */
//typedef std::map<std::string,short_func_t> short_code_map_t;  // Maps description (phone number) to function to call

/* What fills in that map */
//void init_smcommands(kneedeepbts::smqueue::short_code_map_t *scm);

/* 
 * Main class for SIP Short Message processing.
 * The daemon is designed to be running one copy of this class.
 */
//class SMq {
//	public:
//
//	// Put all timeouts in one place
//	const static int TIMEOUTMS = 5000;
//	const static int SMSRATELIMITMS = 1000;
//	const static int LONGDELETMS = 5000000;   // 83 minutes  Used by SC.ZapQueued.Password
//	const static int INCREASEACKEDMSGTMOMS = 60000;  // 5 minutes
//
//	void InitBeforeMainLoop();
//	void CleaupAfterMainreaderLoop();
//	void InitInsideReaderLoop();
//
//	/* A list of all messages we know about, sorted by time of next
//	   action (assuming nothing arrives to change our mind before that
//	   time). */
//	short_msg_p_list time_sorted_list;
//
//	std::string savefile; //SMq
//	bool please_re_exec;
//
//	pthread_mutexattr_t mutexSLAttr;
//	pthread_mutex_t sortedListMutex;
//
//	void unlockSortedList() {
//		// LOG(DEBUG) << "UNLOCK";  // debug opnly
//		pthread_mutex_unlock(&sortedListMutex);
//	}
//
//	void lockSortedList() {
//		// LOG(DEBUG) << "LOCK"; // debug only
//		pthread_mutex_lock(&sortedListMutex);
//	}
//
//	/* We may later want other accessors for faster access to various
//	   messages when things DO arrive.  For now, linear search!  */
//
//	/* The network sockets that we're using for I/O */
//	SMnet my_network;
//
//	/* The interface to the Host Location Register for routing
//	   messages and looking up their return and destination addresses.  */
//	SubscriberRegistry my_hlr;
//
//	/* Where to send SMS's that we can't route locally. */
//	std::string global_relay;
//	std::string global_relay_port;
//	kneedeepbts::smqueue::ShortMsg::ContentType global_relay_contenttype;
//
//	/* My IP address (I can't tell how I look to others). */
//	std::string my_ipaddress;
//	/* Idiocy for NAT */
//	std::string my_2nd_ipaddress;
//
//	/* My port number. */
//	std::string my_udp_port;
//
//	/* The IP addr:port of the HLR, where we send SIP REGISTER
//	   messages to associate IMSIs with cell site addr:port numbers. */
//	std::string my_register_hostport;
//
//	/* The call-ID, CSeq, and flag we use in registration requests */
//	std::string register_call_id;
//	int register_call_seq;
//	bool have_register_call_id;
//
//	/* Set this to true when you want main loop to stop.  */
//	bool stop_main_loop;
//
//	/* Set this to true when you want the program to re-exec itself
//	   instead of terminating after the main loop stops.  */
//	bool reexec_smqueue;
//
//	// Input from command line
//	int argc;
//	char **argv;
//
//	/* Constructor */
//	SMq () :
//		time_sorted_list (),
//		my_network (),
//		my_hlr(),
//		global_relay(""),
//		my_ipaddress(""),
//		my_2nd_ipaddress(""),
//		my_udp_port(""),
//		my_register_hostport(""),
//		register_call_id(""),
//		register_call_seq(0),
//		have_register_call_id(false),
//		stop_main_loop (false),
//		reexec_smqueue (false)
//	{
//		// We need recursive attribute set
//		int mStatus;
//		mStatus = pthread_mutexattr_init(&mutexSLAttr);
//		if (mStatus != 0) { LOG(DEBUG) << "Mutex pthread_mutexattr_init error " << mStatus; }
//		pthread_mutexattr_settype(&mutexSLAttr, PTHREAD_MUTEX_RECURSIVE);
//		if (mStatus != 0) { LOG(DEBUG) << "Mutex pthread_mutexattr_settype error " << mStatus; }
//		pthread_mutex_init(&sortedListMutex, &mutexSLAttr);
//		if (mStatus != 0) { LOG(DEBUG) << "Mutex pthread_mutex_init error " << mStatus; }
//
//		my_hlr.init();
//	}
//
//
//	/* Destructor */
//	~SMq() {
//		pthread_mutex_destroy(&sortedListMutex);
//	}
//
//
//	// Override operator= so -Weffc++ doesn't complain
//	// *DISABLE* assignments by making the = operation private.
//	private:
//	SMq & operator= (const SMq &rvalue);
//	public:
//
//	/* Set my own IP address, since I can't tell how I look to others. */
//	void set_my_ipaddress(std::string myip) {
//		my_ipaddress = myip;
//		// Point to it for message validity checking.
//		// NOTE: that copy shares same storage as this one.
//		kneedeepbts::smqueue::ShortMsgPending::smp_my_ipaddress = myip.c_str();
//	}
//
//	/* Set my 2nd IP address, since NAT is widespread among idiots. */
//	void set_my_2nd_ipaddress(std::string myip) {
//		my_2nd_ipaddress = myip;
//		// Point to it for message validity checking.
//		// NOTE: that copy shares same storage as this one.LOG(DEBUG) << "Run once a minute stuff"
//		kneedeepbts::smqueue::ShortMsgPending::smp_my_2nd_ipaddress = myip.c_str();
//	}
//
//	/* Set the global relay address (host:port string) */
//	void set_global_relay(std::string gr, std::string port, std::string contentType) {
//		global_relay = gr;
//		global_relay_port = port;
//		if (contentType.length()) {
//			if (contentType == "text/plain") {
//				global_relay_contenttype = kneedeepbts::smqueue::ShortMsg::TEXT_PLAIN;
//			} else if (contentType == "application/vnd.3gpp.sms") {
//				global_relay_contenttype = kneedeepbts::smqueue::ShortMsg::VND_3GPP_SMS;
//			}
//		} else {
//			global_relay_contenttype = kneedeepbts::smqueue::ShortMsg::VND_3GPP_SMS;
//		}
//	}
//
//	/* Set the register host & port -- where to register handsets */
//	void set_register_hostport(std::string hp) {
//		my_register_hostport = hp;
//	}
//
//	/* Initialize the listener -- sets up and opens my_network. */
//	bool init_listener (std::string port) {
//		my_udp_port = port;
//		return my_network.listen_on_port (port);
//	}
//
//	bool to_is_deliverable(const char *username);
//	bool from_is_deliverable(const char *from);
//
//	bool convert_content_type(short_msg_pending *message, kneedeepbts::smqueue::ShortMsg::ContentType to_type);
//
//	/* Convert a short_msg to a given content type */
//	//void convert_message(short_msg_pending *qmsg, short_msg::ContentType toType);
//
//	// Main loop listening for dgrams and processing them.
//	void main_loop(int tmo);
//
//	/* If nothing happens for a while, handle that.  */
//	void process_timeout();
//
//	/* Send a SIP response to acknowledge reciept of a short msg. */
//	void respond_sip_ack(int errcode, short_msg_pending *smp, char *netaddr, size_t netaddrlen);
//
//	/*
//	 * Originate a short message
//	 * Put it in the queue and start handling it.
//	 * From is a shortcode (currently),
//	 * To is an IMSI (currently),
//	 * msgtext is plain ASCII text.
//	 * Result is 0 for success, negative for error.
//	 */
//	int
//	originate_sm(const char *from, const char *to, const char *msgtext,
//			enum sm_state firststate);
//
//	/*
//	 * Originate half of a short message
//	 * Put it in the queue and start handling it, but don't actually
//	 * finish it or send it; return it to the caller for further mucking.
//	 *
//	 * In particular, the caller must set the:
//	 *    uri
//	 *    From:
//	 *    To:
//	 *    Content-Type and message body, if any.
//	 *
//	 * Method is which type of SIP packet (MESSAGE, RESPONSE, etc).
//	 * Result is a short_msg_p_list containing one short_msg_pending, with
//	 * the half-initialized message in it.
//	 */
//	short_msg_p_list *
//	originate_half_sm(std::string method);
//
//	/*
//	 * Send a bounce message, based on an existing queued message.
//	 * Return the state to set the original bouncing message to.
//	 */
//	enum sm_state
//	bounce_message(short_msg_pending *sent_msg, const char *errstr);
//
//	/*
//	 * See if the handset's imsi and phone number are in the HLR
//	 * database yet, since if it isn't, we can't register the imsi
//	 * at its cell's host:port yet.
//	 */
//	bool
//	ready_to_register (short_msg_p_list::iterator qmsg);
//
//	/*
//	 * Register a handset's IMSI with its cell, by sending Asterisk
//	 * a SIP REGISTER message that we "relay" from the cell.  (We
//	 * actually originate it, but we pretend that the cell sent it to us.
//	 * Actually the cell sent us an incoming shortcode SMS from an
//	 * unregistered phone, which is in the queue in REGISTER_HANDSET state.)
//	 */
//	enum sm_state
//	register_handset (short_msg_p_list::iterator qmsg);
//
//	/* Check if this is a short-code message and handle it.
//	 * Return true if message has been handled, false if you should continue
//	 * message handling as usual. In latter case \p next_state is untouched.
//	 */
//	bool
//	handle_short_code(const short_code_map_t &short_code_map,
//	                  short_msg_p_list::iterator qmsg, enum sm_state &next_state);
//
//	/* When a SIP response arrives, search the queue for its matching
//	   MESSAGE and handle both.  */
//	void
//	handle_response(short_msg_p_list::iterator qmsg);
//
//	/* Search the message queue to find a message whose tag matches.  */
//	bool
//	find_queued_msg_by_tag(short_msg_p_list::iterator &mymsg,
//				    const char *tag, int taghash);
//	/* Same, but without a known taghash. */
//	bool
//	find_queued_msg_by_tag(short_msg_p_list::iterator &mymsg,
//				    const char *tag);
//
//	/*
//	 * Look up the hostname and port number where we should send Short
//	 * Messages for the IMSI in the To address.
//	 *
//	 * This is also where we assign a new Call-ID to the message, so that
//	 * re-sends will use the same Call-ID, but re-locate's (looking up the
//	 * recipient's location again) will use a new one.
//	 */
//	enum sm_state
//	lookup_uri_hostport (short_msg_pending *qmsg);
//
//	/*
//	 * Change the From address username to a valid phone number in format:
//	 *     +countrycodephonenum
//	 */
//	enum sm_state
//	lookup_from_address(short_msg_pending *qmsg);
//
//	/* Change the Request-URI's address to a valid IMSI.  */
//	enum sm_state
//	lookup_uri_imsi (short_msg_pending *qmsg);
//
//	// For memory allocation simplicity, it's easiest to create
//	// new messages as a 1-entry short_msg_p_list and then move
//	// them to the real list.  Note that this moves the message's list
//	// entry itself off the original list (which can then be discarded).
//	// Push_front only does a copy so use splice ??
//	void insert_new_message(short_msg_p_list &smp) {
//		lockSortedList();
//		time_sorted_list.splice (time_sorted_list.begin(), smp);
//		time_sorted_list.begin()->set_state (INITIAL_STATE);  // Note set state can move the entries in the queue
//		// time_sorted_list.begin()->timeout = 0;  // it is already
//		// Low timeout will cause this msg to be at front of queue.
//		unlockSortedList();
//		debug_dump(); //svgfix
//		ProcessReceivedMsg();
//	}
//
//	// This version lets the initial state be set.
//	void insert_new_message(short_msg_p_list &smp, enum sm_state s) {
//		LOG(DEBUG) << "Insert message into queue 2";
//		lockSortedList();
//		time_sorted_list.splice (time_sorted_list.begin(), smp);
//		time_sorted_list.begin()->set_state (s);
//		// time_sorted_list.begin()->timeout = 0;  // it is already
//		// Low timeout will cause this msg to be at front of queue.
//		unlockSortedList();
//		debug_dump(); //svgfix
//		ProcessReceivedMsg();
//	}
//	// This version lets the state and timeout be set.
//	void insert_new_message(short_msg_p_list &smp, enum sm_state s, time_t t) {
//		LOG(DEBUG) << "Insert message into queue 3";
//		lockSortedList();
//		time_sorted_list.splice (time_sorted_list.begin(), smp);
//		time_sorted_list.begin()->set_state (s, t);
//		unlockSortedList();
//		debug_dump(); //svgfix
//		ProcessReceivedMsg();
//	}
//
//	/* Debug dump of the queue and the SMq class in general. */
//	void debug_dump();
//
//	// Set the linktag of "newmsg" to point to oldmsg.
//	void set_linktag(short_msg_p_list::iterator newmsg,
//			 short_msg_p_list::iterator oldmsg);
//
//	// Get the old message that this message links to.
//	bool get_link(short_msg_p_list::iterator &oldmsg,
//		      short_msg_p_list::iterator qmsg);
//
//	/*
// 	 * When we reset the state and timestamp of a message,
//	 * we need to reinsert it into the queue.  C++ doesn't seem
//	 * to have a standard container that lets one do this cleanly.
//	 * Even with lists it's a kludge -- we have to do our own search.
//	 * With multisets you can't splice an element out while keeping the
//	 * element... etc.
//	 */
//	void set_state(short_msg_p_list::iterator sm, enum sm_state newstate) {
//		short_msg_p_list temp;
//		lockSortedList();
//		/* Extract the current sm from the time_sorted_list */
//		temp.splice(temp.begin(), time_sorted_list, sm);
//		sm->set_state(newstate);
//		// Insert it according to the new timestamp.
//		// One would think that "multisets" could do this simply,
//		// but they don't appear to have this capability.
//		// Note: if list is empty, or all are too early, insert at end.
//		for (short_msg_p_list::iterator x = time_sorted_list.begin();
//		     true; x++) {
//			if (x == time_sorted_list.end()
//			    || x->next_action_time >= sm->next_action_time) {
//				time_sorted_list.splice(x, temp);
//				break;
//			}
//		}
//		unlockSortedList();
//	} // set_state
//
//	/*
// 	 * When we reset the state and timestamp of a message,
//	 * we need to reinsert it into the queue.  C++ doesn't seem
//	 * to have a standard container that lets one do this cleanly.
//	 * Even with lists it's a kludge -- we have to do our own search.
//	 * With multisets you can't splice an element out while keeping the
//	 * element... etc.
//	 */
//	void set_state(short_msg_p_list::iterator sm, enum sm_state newstate, time_t timestamp) {
//		lockSortedList();
//		short_msg_p_list temp;
//		/* Extract the current sm from the time_sorted_list */
//		temp.splice(temp.begin(), time_sorted_list, sm);
//		sm->set_state(newstate, timestamp);
//		// Insert it according to the new timestamp.
//		// One would think that "multisets" could do this simply,
//		// but they don't appear to have this capability.
//		// Note: if list is empty, or all are too early, insert at end.
//		for (short_msg_p_list::iterator x = time_sorted_list.begin();
//		     true; x++) {
//			if (x == time_sorted_list.end()
//			    || x->next_action_time >= sm->next_action_time) {
//				time_sorted_list.splice(x, temp);
//				break;
//			}
//		}
//		unlockSortedList();
//	} // set_state
//
//	/* Save the queue to a file; read it back from a file.
//	   Reading a queue file doesn't delete things that might already
// 	   be in the queue; if you want a clean queue, delete anything
//	   already in the queue first.  */
//	bool
//	save_queue_to_file(std::string qfile);
//	bool
//	read_queue_from_file(std::string qfile);
//

//}; // SMq class
//} // namespace SMqueue

//extern SMqueue::SMq smq;

#endif

