#ifndef OBTS_SMQUEUE_L2FRAME_H
#define OBTS_SMQUEUE_L2FRAME_H

#include <cstdint>
#include <vector>

#include "BitVector.h"

//#include "l2address.h"
//#include "l2control.h"
//#include "l2length.h"

namespace kneedeepbts::gsm {
    /** Layer 2 (Data Link Layer) frame format types, GSM 04.06 2.1 */
    enum L2_FrameFormat {
        FormatA, // full header, DCCHs no information field
        FormatB, // full header, DCCHs information field
        FormatAbis, // FmtA in unacknowledged mode and SAPI=0 (BCCH, PCH, AGCH)
        FormatBbis, // FmtA in unacknowledged mode and SAPI=0 (BCCH, PCH, AGCH), OLD: no header (actually, a pseudolength header)
        FormatBter, // "short header" (which we don't use)
        FormatB4, // addesss and control only, implied length
        FormatC, // Random Access Signals (RACHs)
    };

    /** Address Field Extension Bit (EA) - GSM 04.06 3.3.1 */
    enum class L2_EA {
        MORE_OCTETS = 0,
        FINAL_OCTET = 1
    };

    /** Command/Response Field Bit (C/R) - GSM 04.06 3.3.2 */
    enum class L2_CR {
        CMD_BS_TO_MS = 1,
        CMD_MS_TO_BS = 0,
        RSP_BS_TO_MS = 0,
        RSP_MS_TO_BS = 1
    };

    /** Sevice Access Point Identifier (SAPI) - GSM 04.06 3.3.3 */
    enum class L2_SAPI {
        CC_MM_RR = 0,
        SMS = 3
    };

    /** Link Protocol Discriminator (LPD) - GSM GSM 04.06 3.2 */
    enum class L2_LPD {
        LAPDM = 0,
        SMSCB = 1
    };

    /** Control field format types, GSM 04.06 3.4 */
    enum class L2_ControlFormat {
        IFormat, // numbered information transfer
        SFormat, // supervisory functions
        UFormat  // unnumbered information transfer and control functions
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

    /** Length Indicator Field Extension Bit (EL) - GSM 04.06 3.7.1 */
    enum class L2_EL {
        MORE_OCTETS = 0,
        FINAL_OCTET = 1
    };

    /** More Data Bit (M) - GSM 04.06 3.7.2 */
    enum class L2_MOREDATA {
        FINAL_DATA = 0,
        MORE_DATA = 1
    };

    /**
    * Interlayer primitives, GSM 04.04 4, GSM 04.06 4, GSM 04.07 10.
    * PH, DL, MDL, etc. is implied by context.
    *  - L1-L2: PH
    *  - L2-L3: DL, MDL
    *           We don't provide the full req-conf-ind-ack handshake because we
    *           don't always need it in such a tighly integrated system, so
    *           our primitive set is simple.
    */
    // FIXME: This is for layer-to-layer translation, probably should move it to another class
    enum Primitive {
        ESTABLISH,		///< channel establihsment
        RELEASE,		///< normal channel release
        DATA,			///< multiframe data transfer
        UNIT_DATA,		///< datagram-type data transfer
        ERROR,			///< channel error
        HARDRELEASE		///< forced release after an assignment
    };

    /** GSM 04.06 2 */
    /** Data Link Layer frames used for peer to peer communication */
    class L2Frame {
    public:
        std::vector<std::uint8_t> get_octets();

        // Address Methods
        void set_cr(L2_CR cr);
        void set_sapi(L2_SAPI sapi);
        void set_lpd(L2_LPD lpd);

        // Control Methods
        void set_control_format(L2_ControlFormat cformat);
        void set_recv_sequence(std::uint8_t rseq);
        void set_send_sequence(std::uint8_t sseq);
        void set_pf_poll(L2_PollFinalBit pfbit);
        void set_control_bits(L2_ControlBits cbits);

        // Length Methods
        void set_el(L2_EL el);
        void set_moredata(L2_MOREDATA m);
        void set_length(std::uint8_t l);

        // Information Methods
        void set_information(std::vector<std::uint8_t> data);

        // FIXME: Should these be public?  Or private with getters/setters?
        L2_FrameFormat m_format = L2_FrameFormat::FormatB;

        // L2Address m_address{};
        L2_EA m_ea = L2_EA::FINAL_OCTET;
        L2_CR m_cr = L2_CR::CMD_MS_TO_BS;
        L2_SAPI m_sapi = L2_SAPI::CC_MM_RR;
        L2_LPD m_lpd = L2_LPD::LAPDM;

        // L2Control m_control{};
        // FIXME: Can the control format or frame format be used to figure out the other (so both aren't needed)?
        L2_ControlFormat m_control_format = L2_ControlFormat::IFormat;
        std::uint8_t m_recv_sequence = 0;
        std::uint8_t m_send_sequence = 0;
        L2_PollFinalBit m_pf_poll = L2_PollFinalBit::P_NO_POLL;
        L2_ControlBits m_control_bits = L2_ControlBits::I_I;
        // FIXME: Should this be switched to L2_ControlBits?
        [[nodiscard]] LAPDm_FrameType decodeFrameType() const;

        // L2Length m_length{};
        L2_EL m_el = L2_EL::FINAL_OCTET;
        L2_MOREDATA m_more_data = L2_MOREDATA::FINAL_DATA;
        std::uint8_t m_length = 0; // FIXME: Should this be gathered from the information field vector

        // L2Information m_information{};
        std::vector<std::uint8_t> m_information{};

        // FIXME: How to handle fill/N201?

    private:
        std::uint8_t get_address_octet();
        std::uint8_t get_control_octet();
        std::uint8_t get_control_octet_i();
        std::uint8_t get_control_octet_s();
        std::uint8_t get_control_octet_u();
    };

    /**
    * The bits of an L2Frame
    * Bit ordering is MSB-first in each octet.
    */
    class L2FrameOld : public BitVector {
    public:

        /** Fill the frame with the GSM idle pattern, GSM 04.06 2.2. */
        void idleFill();

        /** Build an empty frame with a given primitive. */
        L2Frame(Primitive wPrimitive=UNIT_DATA) : BitVector(23*8), mPrimitive(wPrimitive) {
            idleFill();
        }

        /** Make a new L2 frame by copying an existing one. */
        L2Frame(const L2Frame& other) : BitVector((const BitVector&)other), mPrimitive(other.mPrimitive) {}

        /**
            Make an L2Frame from a block of bits.
            BitVector must fit in the L2Frame.
        */
        L2Frame(const BitVector&, Primitive);

        /**
            Make an L2Frame from a payload using a given header.
            The L3Frame must fit in the L2Frame.
            The primitive is DATA.
        */
        L2Frame(const L2Header&, const BitVector&);

        /**
            Make an L2Frame from a header with no payload.
            The primitive is DATA.
        */
        L2Frame(const L2Header&);

        /** Get the LPD from the L2 header.  Assumes address byte is first. */
        unsigned LPD() const;

        /**
            Look into the LAPDm header and get the SAPI, see GSM 04.06 2 and 3.2.
            This method assumes frame format A or B, GSM 04.06 2.1.
        */
        unsigned SAPI() const;


        /**@name Decoding methods that assume A/B header format. */
        //@{

        /** Look into the LAPDm header and get the control format.  */
        L2Control::ControlFormat controlFormat() const;

        /** Look into the LAPDm header and decode the U-frame type. */
        L2Control::FrameType UFrameType() const;

        /** Look into the LAPDm header and decode the S-frame type. */
        L2Control::FrameType SFrameType() const;

        /** Look into the LAPDm header and get the P/F bit. */
        bool PF() const { return mStart[8+3] & 0x01; }

        /** Set/clear the PF bit. */
        void PF(bool wPF) { mStart[8+3]=wPF; }

        /** Look into the header and get the length of the payload. */
        unsigned L() const { return peekField(8*2,6); }

        /** Get the "more data" bit (M). */
        bool M() const { return mStart[8*2+6] & 0x01; }

        /** Return the L3 payload part.  Assumes A or B header format. */
        BitVector L3Part() const { return cloneSegment(8*3,8*L()); }

        /** Return NR sequence number, GSM 04.06 3.5.2.4.  Assumes A or B header. */
        unsigned NR() const { return peekField(8*1+0,3); }

        /** Return NS sequence number, GSM 04.06 3.5.2.5.  Assumes A or B header. */
        unsigned NS() const { return peekField(8*1+4,3); }

        /** Return the CR bit, GSM 04.06 3.3.2.  Assumes A or B header. */
        bool CRm() const { return mStart[6] & 0x01; }

        /** Return truw if this a DCCH idle frame. */
        bool DCCHIdle() const
        {
            return peekField(0,32)==0x0103012B;
        }

        //@}

        Primitive primitive() const { return mPrimitive; }

        /** This is used only for testing. */
        void primitive(Primitive wPrimitive) { mPrimitive=wPrimitive; }

    private:
        GSM::Primitive mPrimitive;

    };
}

#endif //OBTS_SMQUEUE_L2FRAME_H
