#include "l2length.h"

namespace kneedeepbts::gsm {
    void L2Length::set_m(L2_M m) {
        m_m = m;
    }

    void L2Length::set_l(std::uint8_t l) {
        m_l = 0b00111111 & l;
    }

    std::uint8_t L2Length::get_octet() {
        /** Per GSM 04.06 3.6, the bits get packed as follows:
         *  8 7 6 5 4 3 2 1 (really 7-0)
         *  | | | | | | | \
         *  | | | | | | \  Length Indicator Field Extention Bit (EL)(1 bit)
         *  \ \ \ \ \ \  More Data Bit (M)(1 bit)
         *   Length Indicator (L)(6 bits)
         */
        std::uint8_t value = 0;
        value = value | (0b00000001 & (uint8_t)m_el);
        value = value | (0b00000010 & ((uint8_t)m_m << 1));
        value = value | (0b11111100 & ((uint8_t)m_l << 2));
        return value;
    }
}
