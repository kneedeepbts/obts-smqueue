#ifndef OBTS_SMQUEUE_L2CONTROL_H
#define OBTS_SMQUEUE_L2CONTROL_H

#include <cstdint>

namespace kneedeepbts::gsm {
    /** Control field format types, GSM 04.06 3.4 */
    enum class L2_ControlFormat {
        IFormat, // numbered information transfer
        SFormat, // supervisory functions
        UFormat  // unnumberd information transfer and control functions
    };

    /** GSM 04.06 3.5.1 */
    enum class L2_PollFinalBit {
        P_NO_POLL = 0b00000000, // Don't poll (request a response from peer)
        P_POLL = 0b00010000, // Poll (request a response)
        F_NO_POLL_RESPONSE = 0b00000000, // This response is not due to a P_POLL
        F_POLL_RESPONSE = 0b00010000, // This is a response to a P_POLL request
    };

    /** GSM 04.06 3.4.3 Table 3, 3.8.1 Table 4 */
    enum class L2_ControlBits {
        I_I = 0b00000000,
        S_RR = 0b0000001,
        S_RNR = 0b00000101,
        S_REJ = 0b00001001,
        U_SABM = 0b00101111,
        U_DM = 0b00001111,
        U_UI = 0b00000011,
        U_DISC = 0b01000011,
        U_UA = 0b01100011
    };

    /** LAPDm frame types, GSM 04.06 3.8.1. */
    enum LAPDm_FrameType {
        UIFrame,
        SABMFrame,
        UAFrame,
        DMFrame,
        DISCFrame,
        RRFrame,
        RNRFrame,
        REJFrame,
        IFrame,
        BogusFrame // a return code used when parsing fails
    };

    /** The L2 header control field, as per GSM 04.06 3.4 */
    class L2Control {
    public:
        void set_format(L2_ControlFormat format);
        void set_recv_sequence(std::uint8_t rseq);
        void set_send_sequence(std::uint8_t sseq);
        void set_pf_poll(L2_PollFinalBit pfbit);
        void set_control_bits(L2_ControlBits cbits);

        std::uint8_t get_octet();

        // FIXME: Should this be switched to L2_ControlBits?
        LAPDm_FrameType decodeFrameType() const;

    private:
        std::uint8_t get_octet_i();
        std::uint8_t get_octet_s();
        std::uint8_t get_octet_u();

        L2_ControlFormat m_format = L2_ControlFormat::IFormat;
        std::uint8_t m_recv_sequence = 0;
        std::uint8_t m_send_sequence = 0;
        L2_PollFinalBit m_pf_poll = L2_PollFinalBit::P_NO_POLL;
        L2_ControlBits m_control_bits = L2_ControlBits::I_I;
    };

//    class L2ControlOld {
//    public:
//        /** Initialize a U or S frame. */
//        L2Control(ControlFormat wFormat=UFormat, unsigned wPF=0, unsigned bits=0) : mFormat(wFormat), mPF(wPF), mSBits(bits), mUBits(bits) {
//            assert(mFormat!=IFormat);
//            assert(mPF<2);
//            if (mFormat==UFormat) assert(mUBits<0x20);
//            if (mFormat==SFormat) assert(mSBits<0x04);
//        }
//
//        /** Initialize an I frame. */
//        L2Control(unsigned wNR, unsigned wNS, unsigned wPF) : mFormat(IFormat),mNR(wNR),mNS(wNS),mPF(wPF) {
//            assert(mNR<8);
//            assert(mNS<8);
//            assert(mPF<2);
//        }
//
//
//        ControlFormat format() const {
//            return mFormat;
//        }
//
//        unsigned NR() const {
//            assert(mFormat != UFormat);
//            return mNR;
//        }
//
//        void NR(unsigned wNR) {
//            assert(mFormat != UFormat);
//            mNR = wNR;
//        }
//
//        unsigned NS() const {
//            assert(mFormat == IFormat);
//            return mNS;
//        }
//
//        void NS(unsigned wNS) {
//            assert(mFormat == IFormat);
//            mNS = wNS;
//        }
//
//        unsigned PF() const {
//            assert(mFormat != IFormat);
//            return mPF;
//        }
//
//        unsigned P() const {
//            assert(mFormat == IFormat);
//            return mPF;
//        }
//
//        unsigned SBits() const {
//            assert(mFormat == SFormat);
//            return mSBits;
//        }
//
//        unsigned UBits() const {
//            assert(mFormat == UFormat);
//            return mUBits;
//        }
//
//        // FIXME: Sooooo many circular references :(
//        void write(L2Frame& target, size_t& writeIndex) const;
//
//        /** decode frame type */
//        FrameType decodeFrameType() const;
//
//    private:
//        ControlFormat mFormat; // control field format
//        unsigned mNR; // receive sequence number
//        unsigned mNS; // transmit sequence number
//        unsigned mPF; // poll/final bit
//        unsigned mSBits; // supervisory bits
//        unsigned mUBits; // unnumbered function bits
//
//    };
}

#endif //OBTS_SMQUEUE_L2CONTROL_H
