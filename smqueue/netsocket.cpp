#include "netsocket.h"

namespace kneedeepbts::smqueue {
    NetSocket::NetSocket(NetSocket::SocketType socket_type) : m_socket(), m_addr() {
        // Create the socket handle
        m_socket = socket(AF_INET, static_cast<int>(socket_type), 0);
        if(m_socket == INVALID_SOCKET) {
            // FIXME: The rest of this code doesn't throw, should this code here?
            throw std::runtime_error("Could not create socket");
        }
        m_addr.sin_family = AF_INET;
    }

    void NetSocket::set_port(std::uint16_t port) {
        m_addr.sin_port = htons(port);
    }

    int NetSocket::set_address(const std::string &ip_address) {
        return inet_pton(AF_INET, ip_address.c_str(), &m_addr.sin_addr);
    }
}