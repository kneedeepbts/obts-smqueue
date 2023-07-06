#include "smnet.h"
#include "smsc.h"
#include <time.h>
#include <osipparser2/osip_message.h>
#include <iostream>
#include <fstream>
#include <cstdlib>			// strtol
#include <errno.h>
#include <sys/types.h>          // open
#include <sys/stat.h>           // open
#include <fcntl.h>          // open
#include <ctype.h>		// isdigit
#include <string>
#include <stdlib.h>

#undef WARNING

#include <Globals.h>

#include <Logger.h>
#include <Timeval.h>

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "cpptoml.h"

//#include "QueuedMsgHdrs.h"
//#include "SmqMessageHandler.h"

#include "SmqTest.h"

#include "smqmanager.h"

//using namespace std;

//using namespace SMqueue;


/* The global config table. */
// DAB
//ConfigurationTable gConfig("/etc/OpenBTS/smqueue.db", "smqueue", getConfigurationKeys());

// Global Smqueue object
//SMqueue::SMq smq;  // smq defined

/* The global CDR file. */
//FILE * gCDRFile = NULL;


/* Global variables */
//bool SMqueue::osip_initialized = false;	// Have we called lib's initializer?
//struct osip *SMqueue::osipptr = NULL;	// Ptr to struct sorta used by library
//const char *kneedeepbts::smqueue::ShortMsgPending::smp_my_ipaddress = NULL;	// Accessible copy
//const char *kneedeepbts::smqueue::ShortMsgPending::smp_my_2nd_ipaddress = NULL;	// Accessible copy

//bool print_as_we_validate = false;      // Debugging

/*
 * Associative map between target phone numbers (short codes) and
 * function pointers that implement those numbers.
 */
//short_code_map_t short_code_map;  // *********** Defined here does not change after startup






// C++ wants you to use <string>.  Fuck that.  When we want dynamically
// allocated char *'s, then we'll use them.  strdup uses malloc, tho, so
// we need a "new/delete" oriented strdup.  Ok, simple.
char * new_strdup(const char *orig)
{
	int len = strlen(orig);
	char *result = new char[len+1];
	strncpy(result, orig, len+1);
	return result;
}






/*
 * Check the username in the To: field, perhaps in the From: fiend,
 * perhaps in the URI in the MESSAGE line at the top...
 */
bool check_to_user (char *user)
{
	// For now, don't check -- but port some checks up from the
	// code below in lookup_from_address.  FIXME.
	return true;
}










/* Print net addr in hex.  Returns a static buffer.  */
char * netaddr_fmt(char *srcaddr, unsigned len)
{
	static char buffer[999];
	char *bufp = buffer;

	buffer[0] = 0;
	*bufp++ = '=';
	while (len > 0) {
		snprintf(bufp, 3, "%02x", *(unsigned char *)srcaddr);
		bufp+= 2;
		len--;
	}
	return buffer;
}


/* Parse a string of hex into a net address and length */
bool netaddr_parse(const char *str, char *addr, unsigned int *len)
{
	if (str[0] != '=')
		return false;
	str++;
	int xlen = strlen(str);
	int retlen = xlen/2;
	if (xlen != retlen*2)
		return false;
	*len = retlen;
	char *myaddr = addr;
	const char *strp = str;
	while (retlen > 0) {
		char blah[3];
		unsigned int mybyte;
		blah[0] = strp[0]; blah[1] = strp[1]; blah[2] = '\0';
		sscanf(blah, "%x", &mybyte);
		*myaddr++ = mybyte;
		strp+= 2;
		retlen--;
	}
	return true;
}







ConfigurationKeyMap getConfigurationKeys()
{
	ConfigurationKeyMap map;
	ConfigurationKey *tmp;
	//Don't log in here log lib is not ready

	tmp = new ConfigurationKey("Asterisk.address","127.0.0.1:5060",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::IPANDPORT,
		"",
		false,
		"The Asterisk/SIP PBX IP address and port."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("Bounce.Code","101",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[0-9]{3,6}$",
		false,
		"The short code that bounced messages originate from."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("Bounce.Message.IMSILookupFailed","Cannot determine return address; bouncing message.  Text your phone number to 101 to register and try again.",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[ -~]+$",
		false,
		"The bounce message that is sent when the originating IMSI cannot be verified."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("Bounce.Message.NotRegistered","Phone not registered here.",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[ -~]+$",
		false,
		"Bounce message indicating that the destination phone is not registered."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("CDRFile","/var/lib/OpenBTS/smq.cdr",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::FILEPATH_OPT,// audited
		"",
		false,
		"Log CDRs here.  "
		"To enable, specify an absolute path to where the CDRs should be logged."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("Debug.print_as_we_validate","0",
		"",
		ConfigurationKey::DEVELOPER,
		ConfigurationKey::BOOLEAN,
		"",
		false,
		"Generate lots of output during validation."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("savefile","/tmp/save",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::FILEPATH,
		"",
		false,
		"The file to save SMS messages to when exiting."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.DebugDump.Code","2336",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[0-9]{3,6}$",
		false,
		"Short code to the application which dumps debug information to the log.  Intended for administrator use."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Info.Code","411",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[0-9]{3,6}$",
		false,
		"Short code to the application which tells the sender their own number and registration status."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.QuickChk.Code","2337",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[0-9]{3,6}$",
		false,
		"Short code to the application which tells the sender the how many messages are currently queued.  Intended for administrator use."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Code","101",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[0-9]{3,6}$",
		false,
		"Short code to the application which registers the sender to the system."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Digits.Max","10",
		"digits",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::VALRANGE,
		"7:10",// educated guess
		false,
		"The maximum number of digits a phone number can have."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Digits.Min","7",
		"digits",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::VALRANGE,
		"7:10",// educated guess
		false,
		"The minimum number of digits a phone number must have."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Digits.Override","0",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::BOOLEAN,
		"",
		false,
		"Ignore phone number digit length checks."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Msg.AlreadyA","Your phone is already registered as",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[ -~]+$",
		false,
		"First part of message sent during registration if the handset is already registered, followed by the current handset number."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Msg.AlreadyB",".",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[ -~]+$",
		false,
		"Second part of message sent during registration if the handset is already registered."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Msg.ErrorA","Error in assigning",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[ -~]+$",
		false,
		"First part of message sent during registration if the handset fails to register, followed by the attempted handset number."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Msg.ErrorB","to IMSI",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[ -~]+$",
		false,
		"Second part of message sent during registration if the handset fails to register, followed by the handset IMSI."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Msg.TakenA","The phone number",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[ -~]+$",
		false,
		"First part of message sent during registration if the handset fails to register because the desired number is already taken, followed by the attempted handset number."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Msg.TakenB","is already in use. Try another, then call that one to talk to whoever took yours.",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[ -~]+$",
		false,
		"Second part of message sent during registration if the handset fails to register because the desired number is already taken."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Msg.WelcomeA","Hello",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[ -~]+$",
		false,
		"First part of message sent during registration if the handset registers successfully, followed by the assigned handset number."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.Register.Msg.WelcomeB","! Text to 411 for system status.",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[ -~]+$",
		false,
		"Second part of message sent during registration if the handset registers successfully."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.SMSC.Code","smsc",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[a-zA-Z]+$",
		false,
		"The SMSC entry point. There is where OpenBTS sends SIP MESSAGES to."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.SMSC.Code","smsc",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[a-zA-Z]+$",
		false,
		"The SMSC entry point. There is where OpenBTS sends SIP MESSAGES to."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	// TODO : this should be made optional and default to off
	// TODO : safety check on .defines() vs .length()
	tmp = new ConfigurationKey("SC.WhiplashQuit.Code","314158",
		"",
		ConfigurationKey::DEVELOPER,
		ConfigurationKey::STRING,
		"^[0-9]{3,6}$",
		false,
		"Short code to the application which will make the server quit for valgrind leak checking.  Intended for developer use only."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.WhiplashQuit.Password","Snidely",
		"",
		ConfigurationKey::DEVELOPER,
		ConfigurationKey::STRING,
		"^[a-zA-Z0-9]+$",
		false,
		"Password which must be sent in the message to the application at SC.WhiplashQuit.Code."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.WhiplashQuit.SaveFile","testsave.txt",
		"",
		ConfigurationKey::DEVELOPER,
		ConfigurationKey::FILEPATH,
		"",
		false,
		"Contents of the queue will be dumped to this file when SC.WhiplashQuit.Code is activated."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.ZapQueued.Code","2338",
		"",
		ConfigurationKey::DEVELOPER,
		ConfigurationKey::STRING,
		"^[0-9]{3,6}$",
		false,
		"Short code to the application which will remove a message from the queue, by its tag.  "
			"If first char is \"-\", do not reply, just do it.  "
			"If argument is SC.ZapQueued.Password, then delete any queued message with timeout greater than 5000 seconds."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SC.ZapQueued.Password","6000",
		"",
		ConfigurationKey::DEVELOPER,
		ConfigurationKey::STRING,
		"^[a-zA-Z0-9]+$",
		false,
		"Password which must be sent in the message to the application at SC.ZapQueued.Code."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SIP.Default.BTSPort","5062",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::PORT,
		"",
		false,
		"The default BTS port to try when none is available."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SIP.GlobalRelay.ContentType","application/vnd.3gpp.sms",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::CHOICE,
		"application/vnd.3gpp.sms,"
			"text/plain",
		true,
		"The content type that the global relay expects."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SIP.GlobalRelay.IP","",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::IPADDRESS_OPT,// audited
		"",
		true,
		"IP address of global relay to send unresolvable messages to.  "
			"By default, this is disabled.  "
			"To override, specify an IP address.  "
			"To disable again use \"unconfig SIP.GlobalRelay.IP\"."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SIP.GlobalRelay.Port","",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::PORT_OPT,// audited
		"",
		true,
		"Port of global relay to send unresolvable messages to."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SIP.GlobalRelay.RelaxedVerify","0",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::BOOLEAN,
		"",
		true,
		"Relax relay verification by only using SIP Header."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SIP.Timeout.ACKedMessageResend","60",
		"seconds",
		ConfigurationKey::CUSTOMERTUNE,
		ConfigurationKey::VALRANGE,
		"45:360",// educated guess
		false,
		"Number of seconds to delay resending ACK messages."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SIP.Timeout.MessageBounce","120",
		"seconds",
		ConfigurationKey::CUSTOMERTUNE,
		ConfigurationKey::VALRANGE,
		"45:360",// educated guess
		true,
		"Timeout, in seconds, between bounced message sending tries."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SIP.Timeout.MessageResend","120",
		"seconds",
		ConfigurationKey::CUSTOMERTUNE,
		ConfigurationKey::VALRANGE,
		"45:360",// educated guess
		true,
		"Timeout, in seconds, between message sending tries."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SIP.myIP","127.0.0.1",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::IPADDRESS,
		"",
		false,
		"The internal IP address. Usually 127.0.0.1."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SIP.myIP2","192.168.0.100",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::IPADDRESS,
		"",
		false,
		"The external IP address that is communciated to the SIP endpoints."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SIP.myPort","5063",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::PORT,
		"",
		false,
		"The port that smqueue should bind to."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SMS.FakeSrcSMSC","0000",
		"",
		ConfigurationKey::CUSTOMER,
		ConfigurationKey::STRING,
		"^[0-9]{3,6}$",
		false,
		"Use this to fill in L4 SMSC address in SMS delivery."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SMS.HTTPGateway.Retries","5",
		"retries",
		ConfigurationKey::CUSTOMERTUNE,
		ConfigurationKey::VALRANGE,
		"2:8",// educated guess
		false,
		"Maximum retries for HTTP gateway attempt."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SMS.HTTPGateway.Timeout","5",
		"seconds",
		ConfigurationKey::CUSTOMERTUNE,
		ConfigurationKey::VALRANGE,
		"2:8",// educated guess
		false,
		"Timeout for HTTP gateway attempt in seconds."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SMS.HTTPGateway.URL","",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::STRING_OPT,// audited
		"^(http|https)://[[:alnum:]_.-]",
		false,
		"URL for HTTP API.  "
			"Used directly as a C format string with two \"%s\" substitutions.  "
			"First \"%s\" gets replaced with the destination number.  "
			"Second \"%s\" gets replaced with the URL-endcoded message body."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SMS.MaxRetries","2160",// by default 2160 * 120sec-per-retry = 3 days
		"",
		ConfigurationKey::CUSTOMERTUNE,
		ConfigurationKey::VALRANGE,
		"0:5040",
		false,
		"Messages will only be attempted to be sent this many times before giving up and being dropped. Set to 0 to allow infinite retries."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SMS.RateLimit","0",
		"seconds",
		ConfigurationKey::CUSTOMERTUNE,
		ConfigurationKey::VALRANGE,
		"0:15",
		false,
		"Limit delivery rate to one message every X seconds. Set to 0 to disable rate limiting."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	// TODO : pretty sure this isn't used anywhere...
	tmp = new ConfigurationKey("SubscriberRegistry.A3A8","../comp128",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::FILEPATH,
		"",
		false,
		"Path to the program that implements the A3/A8 algorithm."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SubscriberRegistry.db","/var/lib/asterisk/sqlite3dir/sqlite3.db",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::FILEPATH,
		"",
		false,
		"The location of the sqlite3 database holding the subscriber registry."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SubscriberRegistry.Port","5064",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::PORT,
		"",
		false,
		"Port used by the SIP Authentication Server. NOTE: In some older releases (pre-2.8.1) this is called SIP.myPort."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	tmp = new ConfigurationKey("SubscriberRegistry.UpstreamServer","",
		"",
		ConfigurationKey::CUSTOMERWARN,
		ConfigurationKey::STRING_OPT,// audited
		"",
		false,
		"URL of the subscriber registry HTTP interface on the upstream server.  "
			"By default, this feature is disabled.  "
			"To enable, specify a server URL eg: http://localhost/cgi/subreg.cgi.  "
			"To disable again, execute \"unconfig SubscriberRegistry.UpstreamServer\"."
	);
	map[tmp->getName()] = *tmp;
	delete tmp;

	return map;

}

int main(int argc, char **argv)
{
    /*** Parse CLI Arguments ***/
    // TODO: Properly parse and handle any arguments
    //extern const char *gVersionString;
    const char *gVersionString = "FIXME: Change the way this version stuff works";
    if (argc > 1) {
        for (int argi = 0; argi < argc; argi++) {
            if (!strcmp(argv[argi], "--version") ||
                !strcmp(argv[argi], "-v")) {
                cout << gVersionString << endl;
            }
        }
        return 0;
    }

    /*** Parse Config File ***/
    shared_ptr<cpptoml::table> config = cpptoml::parse_file("/etc/openbts/smqueue.conf"); // FIXME: Get this from a command line arg
    shared_ptr<cpptoml::table> config_smqueue = config->get_table("smqueue");
    shared_ptr<cpptoml::table> config_logging = config->get_table("logging");
    std::string log_level = *config_logging->get_as<std::string>("level");
    std::string log_type = *config_logging->get_as<std::string>("type");
    std::string log_filename = *config_logging->get_as<std::string>("filename");

    /*** Setup Logger ***/
    // create color console logger if enabled
    if(log_type == "console") {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::from_str(log_level));
        //console_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");
        auto console_logger = std::make_shared<spdlog::logger>("console_logger", console_sink);
        console_logger->set_level(spdlog::level::from_str(log_level));
        spdlog::register_logger(console_logger);
        spdlog::set_default_logger(console_logger);
    }
        // create file logger if enabled
    else if(log_type == "file") {
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_filename);
        file_sink->set_level(spdlog::level::from_str(log_level));
        //file_sink->set_pattern("[multi_sink_example] [%^%l%$] %v");
        auto file_logger = std::make_shared<spdlog::logger>("file_logger", file_sink);
        file_logger->set_level(spdlog::level::from_str(log_level));
        spdlog::register_logger(file_logger);
        spdlog::set_default_logger(file_logger);
    }
    spdlog::warn("Log level value from the config: {}", log_level);
    spdlog::debug("debug mode is enabled");

    SPDLOG_INFO("smqueue Starting");
    kneedeepbts::smqueue::SmqManager smq_manager = kneedeepbts::smqueue::SmqManager(config_smqueue);

    smq_manager.run();

    // Start tester threads
    //StartTestThreads();  // Disabled in in function StartTestThreads

    // Don't let thread exit
    // FIXME: C++ equivalent to join thread?
    // FIXME: Also should gracefully handles signals like SIGTERM (ctrl+c).
//    while (!smq_manager.stop_main_loop) {
//        msSleep(2000);
//    }

    return 0;
}
