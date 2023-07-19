#ifndef OBTS_SMQUEUE_RAWMESSAGE_H
#define OBTS_SMQUEUE_RAWMESSAGE_H

#include <string>
#include <vector>

namespace kneedeepbts::smqueue {
    class RawMessage {
    public:
        const char* bytes_as_cstr();

        std::string m_from_address{};
        std::uint16_t m_from_port = 0;
        std::string m_to_address{};
        std::uint16_t m_to_port = 0;
        std::vector<std::uint8_t> m_bytes{};
    };
}

#endif //OBTS_SMQUEUE_RAWMESSAGE_H
