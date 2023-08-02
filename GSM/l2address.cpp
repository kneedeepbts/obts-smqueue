#include "l2address.h"

namespace kneedeepbts::gsm {
    void L2Address::set_cr(L2_CR cr) {
        m_cr = cr;
    }

    void L2Address::set_sapi(L2_SAPI sapi) {
        m_sapi = sapi;
    }

    void L2Address::set_lpd(L2_LPD lpd) {
        m_lpd = lpd;
    }

    std::uint8_t L2Address::get_octet() {
        /** Per GSM 04.06 3.2, the bits get packed as follows:
         *  8 7 6 5 4 3 2 1 (really 7-0)
         *  | | | | | | | \
         *  | | | | | | \  Address Field Extention Bit (EA)(1 bit)
         *  | | | \ \ \  Command/Response Field Bit (CR)(1 bit)
         *  | \ \  Service Access Point Identifier (SAPI)(3 bits)
         *  \  Link Protocol Discriminator (LPD)(2 bits)
         *   Spare Bit (SPARE)(1 bit) Always 0
         */
        std::uint8_t value = 0;
        value = value | (0b00000001 & (uint8_t)m_ea);
        value = value | (0b00000010 & ((uint8_t)m_cr << 1));
        value = value | (0b00011100 & ((uint8_t)m_sapi << 2));
        value = value | (0b01100000 & ((uint8_t)m_lpd << 5));
        return value;
    }
}
