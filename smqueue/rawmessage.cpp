#include "rawmessage.h"

namespace kneedeepbts::smqueue {
    const char * RawMessage::bytes_as_cstr() {
        return (char*)m_bytes.data();
    }
}