#include "l2frame.h"

namespace kneedeepbts::gsm {
    std::vector<std::uint8_t> L2Frame::get_octets() {
        std::vector<std::uint8_t> result{};
        // FIXME: Implement this based on the Format
    }

    // Address Methods
    void L2Frame::set_cr(L2_CR cr) {
        m_cr = cr;
    }

    void L2Frame::set_sapi(L2_SAPI sapi) {
        m_sapi = sapi;
    }

    void L2Frame::set_lpd(L2_LPD lpd) {
        m_lpd = lpd;
    }

    // Control Methods
    void L2Frame::set_control_format(L2_ControlFormat cformat) {
        m_control_format = cformat;
    }

    void L2Frame::set_recv_sequence(std::uint8_t rseq) {
        m_recv_sequence = 0b00000111 & rseq;
    }

    void L2Frame::set_send_sequence(std::uint8_t sseq) {
        m_send_sequence = 0b00000111 & sseq;
    }

    void L2Frame::set_pf_poll(L2_PollFinalBit pfbit) {
        m_pf_poll = pfbit;
    }

    void L2Frame::set_control_bits(L2_ControlBits cbits) {
        m_control_bits = cbits;
    }

    std::uint8_t L2Frame::get_address_octet() {
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

    std::uint8_t L2Frame::get_control_octet() {
        // FIXME: Is the L2_ControlFormat needed, or can the L2_ControlBits be used?
        switch (m_format) {
            case (int)L2_ControlFormat::IFormat:
                return get_control_octet_i();
            case (int)L2_ControlFormat::SFormat:
                return get_control_octet_s();
            case (int)L2_ControlFormat::UFormat:
                return get_control_octet_u();
        }
    }

    std::uint8_t L2Frame::get_control_octet_i() {
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

    std::uint8_t L2Frame::get_octet_s() {
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

    std::uint8_t L2Frame::get_octet_u() {
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

    LAPDm_FrameType L2Frame::decodeFrameType() const {
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

    // Length Methods
    void L2Frame::set_el(L2_EL el) {
        m_el = el;
    }

    void L2Frame::set_moredata(L2_MOREDATA m) {
        m_more_data = m;
    }

    void L2Frame::set_length(std::uint8_t l) {
        m_length = 0b00111111 & l;
    }

    // Information Methods
    void L2Frame::set_information(std::vector<std::uint8_t> data) {
        // Check incoming data length
        // Maximum number of octets in an information field (N201)(GSM 04.06 5.8.3)(GSM 04.03 4.1.2,4.1.3)
        // - SACCH (Slow Associated DCCH (Dedicated Control Channel)): 18
        // - FACCH (Fast Associated DCCH) and SDCCH (Stand-alone DCCH): 20
        // - BCCH (Broadcast Control Channel), AGCH (Access Grant Channel), and PCH (Paging Channel): 22
        // FIXME: How can which of these be determined to be the one that need to be used?
        //        Seems like these "channels" come from the lower layer (Layer 1?).

        // Copy bytes into local vector
        m_information.clear();
        for (int i = 0; i < data.size(); i++) {
            m_information.emplace_back(data[i]);
        }
    }
}