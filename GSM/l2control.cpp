#include "l2control.h"

namespace kneedeepbts::gsm {
    void L2Control::set_format(L2_ControlFormat format) {
        m_format = format;
    }

    void L2Control::set_recv_sequence(std::uint8_t rseq) {
        m_recv_sequence = 0b00000111 & rseq;
    }

    void L2Control::set_send_sequence(std::uint8_t sseq) {
        m_send_sequence = 0b00000111 & sseq;
    }

    void L2Control::set_pf_poll(L2_PollFinalBit pfbit) {
        m_pf_poll = pfbit;
    }

    void L2Control::set_control_bits(L2_ControlBits cbits) {
        m_control_bits = cbits;
    }

    std::uint8_t L2Control::get_octet() {
        // FIXME: Is the L2_ControlFormat needed, or can the L2_ControlBits be used?
        switch (m_format) {
            case L2_ControlFormat::IFormat:
                return get_octet_i();
            case L2_ControlFormat::SFormat:
                return get_octet_s();
            case L2_ControlFormat::UFormat:
                return get_octet_u();
        }
    }

    std::uint8_t L2Control::get_octet_i() {
        /** Per GSM 04.06 3.4 Table 3, the bits get packed as follows:
         * Iformat:
         *  8 7 6 5 4 3 2 1 (really 7-0)
         *  | | | | | | | \
         *  | | | | \ \ \  0 (1 bit)
         *  | | | \  Transmitter Send Sequence Number (N(S))(3 bits)
         *  \ \ \  Poll bit (P)(1 bit)
         *   Transmitter Receive Sequence Number (N(R))(3 bits)
         */
        std::uint8_t value = (std::uint8_t)L2_ControlBits::I_I;
        value = value | (0b00001110 & ((std::uint8_t)m_send_sequence << 1));
        value = value | (std::uint8_t)m_pf_poll;
        value = value | (0b11100000 & ((std::uint8_t)m_recv_sequence << 5));
        return value;
    }

    std::uint8_t L2Control::get_octet_s() {
        /** Per GSM 04.06 3.4 Table 3, the bits get packed as follows:
         * Iformat:
         *  8 7 6 5 4 3 2 1 (really 7-0)
         *  | | | | | | | \
         *  | | | | \ \ \  1 (1 bit)
         *  | | | | \ \  0 (1 bit)
         *  | | | \  Supervisory Function Bits (S)(2 bits)
         *  \ \ \  Poll bit (P)(1 bit)
         *   Transmitter Receive Sequence Number (N(R))(3 bits)
         */
        std::uint8_t value = (std::uint8_t)m_control_bits;
        value = value | (uint8_t)m_pf_poll;
        value = value | (0b11100000 & ((uint8_t)m_recv_sequence << 5));
        return value;
    }

    std::uint8_t L2Control::get_octet_u() {
        /** Per GSM 04.06 3.4 Table 3, the bits get packed as follows:
         * Iformat:
         *  8 7 6 5 4 3 2 1 (really 7-0)
         *  | | | | | | | \
         *  | | | | \ \ \  1 (1 bit)
         *  | | | | \ \  1 (1 bit)
         *  | | | \  Unnumbered Function Bits (U)(2 bits)
         *  \ \ \  Poll bit (P)(1 bit)
         *   Unnumbered Function Bits (U)(3 bits)
         */
        std::uint8_t value = (std::uint8_t)m_control_bits;
        value = value | (uint8_t)m_pf_poll;
        return value;
    }

    LAPDm_FrameType L2Control::decodeFrameType() const {
        // FIXME: Can this just use the L2_ControlBits?
        switch (m_control_bits) {
            case L2_ControlBits::I_I:
                return LAPDm_FrameType::IFrame;
            case L2_ControlBits::S_RR:
                return LAPDm_FrameType::RRFrame;
            case L2_ControlBits::S_RNR:
                return LAPDm_FrameType::RNRFrame;
            case L2_ControlBits::S_REJ:
                return LAPDm_FrameType::REJFrame;
            case L2_ControlBits::U_SABM:
                return LAPDm_FrameType::SABMFrame;
            case L2_ControlBits::U_DM:
                return LAPDm_FrameType::DMFrame;
            case L2_ControlBits::U_UI:
                return LAPDm_FrameType::UIFrame;
            case L2_ControlBits::U_DISC:
                return LAPDm_FrameType::DISCFrame;
            case L2_ControlBits::U_UA:
                return LAPDm_FrameType::UAFrame;
        }
    }
}
