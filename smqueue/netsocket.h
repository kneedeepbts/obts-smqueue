#ifndef OBTS_SMQUEUE_NETSOCKET_H
#define OBTS_SMQUEUE_NETSOCKET_H

#include <string>
#include <iostream>

#include <sys/socket.h>
#include <arpa/inet.h> // This contains inet_addr
#include <unistd.h> // This contains close
#define INVALID_SOCKET (SOCKET)(~0)
#define SOCKET_ERROR (-1)
typedef int SOCKET;

namespace kneedeepbts::smqueue {
    class NetSocket {
    public:
        enum class SocketType {
            TYPE_STREAM = SOCK_STREAM,
            TYPE_DGRAM = SOCK_DGRAM
        };

        enum class SocketError {
            NO_ERROR = 0,
            SOCKET_BIND_ERROR = 3,
            SOCKET_ACCEPT_ERROR = 4,
            CONNECTION_ERROR = 5,
            MESSAGE_SEND_ERROR = 6,
            RECEIVE_ERROR = 7
        };

    protected:
        explicit NetSocket(SocketType socket_type);
        ~NetSocket() = default;

        void set_port(std::uint16_t port);
        int set_address(const std::string& ip_address);

        SOCKET m_socket;
        sockaddr_in m_addr;

    private:
    };
}

#endif //OBTS_SMQUEUE_NETSOCKET_H
