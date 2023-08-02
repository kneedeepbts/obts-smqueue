#ifndef OBTS_SMQUEUE_L2HEADER_H
#define OBTS_SMQUEUE_L2HEADER_H

#include <cstdint>

#include "l2address.h"
#include "l2control.h"
#include "l2length.h"

namespace kneedeepbts::gsm {
    /** The total L2 header, as per GSM 04.06 3 */
    class L2Header {
    public:
        /** LAPDm frame format types, GSM 04.06 2.1 */
        enum FrameFormat {
            FmtA, // full header, DCCHs no information field
            FmtB, // full header, DCCHs information field
            FmtAbis, // FmtA in unacknowledged mode and SAPI=0 (BCCH, PCH, AGCH)
            FmtBbis, // FmtA in unacknowledged mode and SAPI=0 (BCCH, PCH, AGCH), OLD: no header (actually, a pseudolength header)
            FmtBter, // "short header" (which we don't use)
            FmtB4, // addesss and control only, implied length
            FmtC, // Random Access Signals (RACHs)
        };

        /** Parse the header from an L2Frame, assuming DCCH uplink. */
        // FIXME: Why does all of this god-forsaken code have so many circular references!?
        L2Header(FrameFormat wFormat, const L2Frame& source);

        /** Format A or B. */
        L2Header(const L2Address& address, const L2Control& control, const L2Length& length, FrameFormat format = FmtB)
            : m_format(format), m_address(address), m_control(control), m_length(length) {}

        /** Format B4. */
        L2Header(const L2Address& address, const L2Control& control)
            : m_format(FmtB4), m_address(address), m_control(control) {}

        /** Pseudolength case, used on non-dedicated control channels. */
        L2Header(const L2Length& length) : m_format(FmtBbis), m_length(length) {}

        /**
            Write the header into an L2Frame at a given offset.
            @param frame The frame to write to.
            @return number of bits written.
        */
        // FIXME: Again with the circular references...
        size_t write(L2Frame& target) const;

        /** Determine the header's LAPDm operation. */
        L2Control::FrameType decodeFrameType() const {
            return m_control.decodeFrameType();
        }

        /**@name Obvious accessors. */
        FrameFormat format() const {
            return m_format;
        }

        void format(FrameFormat format) {
            m_format = format;
        }

        const L2Address& address() const {
            return m_address;
        }

        L2Address& address() {
            return m_address;
        }

        void address(const L2Address& address) {
            m_address = address;
        }

        const L2Control& control() const {
            return m_control;
        }

        L2Control& control() {
            return m_control;
        }

        void control(const L2Control& control) {
            m_control = control;
        }

        const L2Length& length() const {
            return m_length;
        }

        L2Length& length() {
            return m_length;
        }

        void length(const L2Length& length) {
            m_length = length;
        }

        /** Return the number of bits needed to encode the header. */
        std::uint16_t bitsNeeded() const;

    private:
        FrameFormat m_format; // format to use in the L2 frame
        L2Address m_address; // GSM 04.06 2.3
        L2Control m_control; // GSM 04.06 2.4
        L2Length m_length; // GSM 04.06 2.5
        // Information Field
        // Fill bits (number?)

    };
}

#endif //OBTS_SMQUEUE_L2HEADER_H
