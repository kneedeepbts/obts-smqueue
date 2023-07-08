#include <doctest/doctest.h>

#include <string>
#include <cstdlib>

#include <enet/enet.h>
#include <spdlog/spdlog.h>

class UdpNetworkingTestFixture {
public:
    UdpNetworkingTestFixture() {
        enet_address_set_host(&m_address, m_hostname.c_str());
        m_address.port = m_port;
        m_client = enet_host_create(nullptr, 1, 1, 0, 0);
        m_peer = enet_host_connect(m_client, &m_address, 1, 0);
    }

    ~UdpNetworkingTestFixture() {
        enet_host_destroy(m_client);
        m_client = nullptr;
    }

protected:
    std::string m_hostname = "127.0.0.1";
    int m_port = 5064;
    ENetAddress m_address{};
    ENetHost * m_client;
    ENetPeer * m_peer;
};

// FIXME: Should this test grab the IP Address and Port from the smqueue.conf?

TEST_CASE_FIXTURE(UdpNetworkingTestFixture, "Sending a good short message to the server (reader)") {
    REQUIRE((m_client != nullptr));
    REQUIRE((m_peer != nullptr));

    ENetEvent event;
    ENetPacket * packet;
    if(enet_host_service(m_client, &event, 2000) > 0 && event.type == ENET_EVENT_TYPE_CONNECT) {
        SPDLOG_INFO("Connection to {}:{} succeeded.", m_hostname, m_port);
    }
    else
    {
        // NOTE: Two seconds are up or a disconnect event was received.  Reset the peer.
        enet_peer_reset(m_peer);
        SPDLOG_ERROR("Connection to {}:{} failed.", m_hostname, m_port);
    }

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
    packet = enet_packet_create(form, strlen(form) + 1, ENET_PACKET_FLAG_UNSEQUENCED);
    enet_peer_send(m_peer, 0, packet);
    enet_host_flush(m_client);
    SPDLOG_INFO("Packet Sent");

    int i = 15;
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

