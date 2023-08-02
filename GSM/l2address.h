#ifndef OBTS_SMQUEUE_L2ADDRESS_H
#define OBTS_SMQUEUE_L2ADDRESS_H

#include <cstdint>

namespace kneedeepbts::gsm {
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

    /** L2 Address - GSM 04.06 3.2, 3.3 */
    class L2Address {
    public:
        void set_cr(L2_CR cr);
        void set_sapi(L2_SAPI sapi);
        void set_lpd(L2_LPD lpd);

        std::uint8_t get_octet();

    private:
        L2_EA m_ea = L2_EA::FINAL_OCTET;
        L2_CR m_cr = L2_CR::CMD_MS_TO_BS;
        L2_SAPI m_sapi = L2_SAPI::CC_MM_RR;
        L2_LPD m_lpd = L2_LPD::LAPDM;
    };

//    class L2AddressOld {
//    public:
//        explicit L2Address(unsigned wCR = 0, unsigned wSAPI = 0, unsigned wLPD = 0) : mSAPI(wSAPI), mCR(wCR), mLPD(wLPD) {
//            // FIXME: This assert isn't valid.  It should be "assert(wSAPI == 0 || wSAPI == 3);", but shouldn't
//            //        be needed anymore as we're going to use enums.
//            assert(wSAPI < 4);
//        }
//
//        [[nodiscard]] unsigned SAPI() const {
//            return mSAPI;
//        }
//
//        [[nodiscard]] unsigned CRm() const {
//            return mCR;
//        }
//
//        [[nodiscard]] unsigned LPD() const {
//            return mLPD;
//        }
//
//        /** Write attributes to an L2 frame. */
//        // FIXME: Moar circular references, dun dun dun...
//        void write(L2Frame& target, size_t& writeIndex) const;
//
//    private:
//        unsigned mSAPI; // service access point indicator
//        unsigned mCR; // command/response flag
//        unsigned mLPD; // link protocol discriminator
//    };
}

#endif //OBTS_SMQUEUE_L2ADDRESS_H
