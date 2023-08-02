#ifndef OBTS_SMQUEUE_L3FRAME_H
#define OBTS_SMQUEUE_L3FRAME_H

#include "BitVector.h"

#include "l2frame.h"

namespace kneedeepbts::gsm {
    /**
    * L3 Protocol Discriminator, GSM 04.08 10.2, GSM 04.07 11.2.1
    */
    enum L3PD {
        L3GroupCallControlPD=0x00,
        L3BroadcastCallControlPD=0x01,
        L3PDSS1PD=0x02,
        L3CallControlPD=0x03,
        L3PDSS2PD=0x04,
        L3MobilityManagementPD=0x05,
        L3RadioResourcePD=0x06,
        L3MobilityManagementGPRSPD=0x08,
        L3SMSPD=0x09,
        L3GPRSSessionManagementPD=0x0a,
        L3NonCallSSPD=0x0b,
        L3LocationPD=0x0c,
        L3ExtendedPD=0x0e,
        L3TestProcedurePD=0x0f,
        L3UndefinedPD=-1
    };

    /**
    * Representation of a GSM L3 message in a bit vector.
    * Bit ordering is MSB-first in each octet.
    * NOTE: This is for the GSM message bits, not the message content.  See L3Message.
    */
    class L3Frame : public BitVector {
    public:
        /** Empty frame with a primitive. */
        L3Frame(Primitive wPrimitive=DATA, size_t len=0) : BitVector(len), mPrimitive(wPrimitive), mL2Length(len) {}

        /** Put raw bits into the frame. */
        L3Frame(const BitVector& source, Primitive wPrimitive=DATA) : BitVector(source), mPrimitive(wPrimitive), mL2Length(source.size()/8) {
            if (source.size() % 8) {
                mL2Length++;
            }
        }

        /** Concatenate 2 L3Frames */
        L3Frame(const L3Frame& f1, const L3Frame& f2) : BitVector(f1,f2), mPrimitive(DATA), mL2Length(f1.mL2Length + f2.mL2Length) {}

        /** Build from an L2Frame. */
        L3Frame(const L2Frame& source) : BitVector(source.L3Part()), mPrimitive(DATA), mL2Length(source.L()) {}

        /** Serialize a message into the frame. */
        // FIXME: This is circular.  Should be a method that returns the bytes.
        L3Frame(const L3Message& msg, Primitive wPrimitive=DATA);

        /** Get a frame from a hex string. */
        L3Frame(const char*);

        /** Get a frame from raw binary. */
        L3Frame(const char*, size_t len);

        /** Protocol Discriminator, GSM 04.08 10.2. */
        L3PD PD() const { return (L3PD)peekField(4,4); }

        /** Message Type Indicator, GSM 04.08 10.4.  */
        unsigned MTI() const { return peekField(8,8); }

        /** TI value, GSM 04.07 11.2.3.1.3.  */
        unsigned TI() const { return peekField(0,4); }

        /** Return the associated primitive. */
        Primitive primitive() const { return mPrimitive; }

        /** Return frame length in BYTES. */
        size_t length() const { return size()/8; }

        /** Length, or L2 pseudolength, as appropriate */
        size_t L2Length() const { return mL2Length; }

        void L2Length(size_t wL2Length) { mL2Length=wL2Length; }

        // Methods for writing H/L bits into rest octets.
        void writeH(size_t& wp);
        void writeL(size_t& wp);

    private:
        Primitive mPrimitive;
        size_t mL2Length;		///< length, or L2 pseudo-length, as appropriate
    };
}

#endif //OBTS_SMQUEUE_L3FRAME_H
