#include <doctest/doctest.h>

#include <string>

#include <spdlog/spdlog.h>

#include <sys/socket.h>
#include <arpa/inet.h> // This contains inet_addr
#include <unistd.h> // This contains close
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)
typedef int SOCKET;

class UdpNetworkingTestFixture {
public:
    UdpNetworkingTestFixture() {
        // Create the socket handle
        m_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if(m_socket == INVALID_SOCKET) {
            // FIXME: The rest of this code doesn't throw, should this code here?
            throw std::runtime_error("Could not create socket");
        }
        m_addr.sin_family = AF_INET;

        set_address("127.0.0.1");
        set_port(5656);
    }

    ~UdpNetworkingTestFixture() = default;

protected:
    void set_port(std::uint16_t port) {
        m_addr.sin_port = htons(port);
    }
    int set_address(const std::string& ip_address) {
        return inet_pton(AF_INET, ip_address.c_str(), &m_addr.sin_addr);
    }

    SOCKET m_socket;
    sockaddr_in m_addr{};
};

// FIXME: Should this test grab the IP Address and Port from the smqueue.conf?

TEST_CASE_FIXTURE(UdpNetworkingTestFixture, "Sending a good short message to the server (reader)") {
    REQUIRE((m_socket > 0));

    // Send a short message
    static const char form[] =
        "MESSAGE sip:1234@127.0.0.1 SIP/2.0\n"
        "Via: SIP/2.0/UDP 127.0.0.1;branch=ABCD\n"
        "Max-Forwards: 2\n"
        "From: 4321 <sip:4321@127.0.0.1:5064>;tag=DCBA\n"
        "To: sip:1234@127.0.0.1\n"
        "Call-ID: BCD@127.0.0.1:5064\n"
        "CSeq: 1 MESSAGE\n"
        "Content-Type: text/plain\n" \
        "Content-Length: 4\n"
        "\ntest\n";

    // Create the address and peer
    sockaddr_in to_addr{};
    to_addr.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &to_addr.sin_addr);
    to_addr.sin_port = htons(5060);

    // Send the message
    // FIXME: Error handling?
    sendto(m_socket, form, strlen(form), 0, reinterpret_cast<sockaddr*>(&to_addr), sizeof(to_addr));
    SPDLOG_INFO("Packet Sent");

    int i = 5;
    while(i > 0) {
        sleep(1);
        i--;
    }

    // Check the ack (should there be an ack or result?)

    // CHECK STUFF HERE

//    spdlog::set_level(spdlog::level::info);
//
//    kneedeepbts::crypto::KasumiKey key{0x2BD6, 0x459F, 0x82C5, 0xB300, 0x952C, 0x4910, 0x4881, 0xFF48};
//    uint64_t input = 0xEA024714AD5C4D84;
//    uint64_t expected = 0xDF1F9B251C0BF45F;
//
//    kneedeepbts::crypto::Kasumi dut_kasumi = kneedeepbts::crypto::Kasumi(key);
//    uint64_t output = dut_kasumi.run(input);
//
//    CHECK(expected == output);
}


//#define TEST_SUCCESS 1
//#define TEST_FAIL 0
//
//// (pat 8-9-2013) Unfortunately we need a default config to eliminate link errors.
//#include <Configuration.h>
//ConfigurationTable gConfig("/etc/OpenBTS/smqueue.db","smtest");
//
///** Submit an SMS for delivery.
// *  @return The server return code.
// */
//int sendMessage(const char *smqueueIP, int smqueuePort, const char *myIP, int myPort,
//		const char *smscCode, const char *from, const char *to,
//		const char *txtBuf)
//{
//	static UDPSocket sock(myPort, smqueueIP, smqueuePort);
//
//	static const char form[] =
//		"MESSAGE sip:%s@%s SIP/2.0\n"
//		"Via: SIP/2.0/UDP %s;branch=%x\n"
//		"Max-Forwards: 2\n"
//		"From: %s <sip:%s@%s:%d>;tag=%d\n"
//		"To: sip:%s@%s\n"
//		"Call-ID: %x@%s:%d\n"
//		"CSeq: 1 MESSAGE\n"
//		"Content-Type: text/plain\n" \
//		"Content-Length: %u\n"
//		"\n%s\n";
//	static char buffer[1500];
//	snprintf(buffer, 1499, form,
//		smscCode, smqueueIP,
//		myIP, (unsigned)random(),
//		from, from, myIP, myPort, (unsigned)random(),
//		to, smqueueIP,
//		(unsigned)random(), myIP, myPort,
//		strlen(txtBuf), txtBuf);
//	sock.write(buffer);
//
//	int numRead = sock.read(buffer,10000);
//	if (numRead >= 0) {
//		buffer[numRead] = '\0';
//
//		printf("%s\n", buffer);
//	} else {
//		printf("%s\n", "Timed out");
//	}
//
//	return TEST_SUCCESS;
//}
//
//int main(int argc, const char *argv[])
//{
//	if (argc == 8) {
//		const char *smqueueIP = argv[1]; //"127.0.0.1";
//		int smqueuePort = atoi(argv[2]); //5062;
//		const char *myIP = argv[3]; //"127.0.0.1";
//		int myPort = atoi(argv[4]); //5070;
//		const char *smscCode = "smsc";
//		const char *from = argv[5];
//		const char *to = argv[6];
//		const char *msg = argv[7];
//
//#define doTest(x,y,z) sendMessage(smqueueIP, smqueuePort, myIP, myPort, smscCode, x, y, z)
//
//		//doTest("from", "to", "message");
//		doTest(from, to, msg);
//	} else {
//		printf("usage: (All fields are required)\n"
//			"smtest smqueueIP smqueuePort myIP myPort from to message\n\n");
//	}
//
//	return 1;
//}

