#ifndef OBTS_SMQUEUE_L2LENGTH_H
#define OBTS_SMQUEUE_L2LENGTH_H

#include <cstdint>

namespace kneedeepbts::gsm {
    /** Length Indicator Field Extension Bit (EL) - GSM 04.06 3.7.1 */
    enum class L2_EL {
        MORE_OCTETS = 0,
        FINAL_OCTET = 1
    };

    /** More Data Bit (M) - GSM 04.06 3.7.2 */
    enum class L2_M {
        FINAL_DATA = 0,
        MORE_DATA = 1
    };

    /** L2 frame length field, GSM 04.06 3.6, 3.7 */
    class L2Length {
    public:
        //void set_el(L2_EL el);
        void set_m(L2_M m);
        void set_l(std::uint8_t l);

        std::uint8_t get_octet();

    private:
        L2_EL m_el = L2_EL::FINAL_OCTET;
        L2_M m_m = L2_M::FINAL_DATA;
        std::uint8_t m_l = 0;
    };

//    class L2Length {
//    public:
//        L2Length(unsigned wL = 0, bool wM = 0) : mL(wL), mM(wM) {}
//
//        unsigned L() const {
//            return mL;
//        }
//
//        void L(unsigned wL) {
//            mL=wL;
//        }
//
//        unsigned M() const {
//            return mM;
//        }
//
//        void M(unsigned wM) {
//            mM=wM;
//        }
//
//        // FIXME: The circular references, duke, the circular references...
//        void write(L2Frame& target, size_t &writeIndex) const;
//
//    private:
//        unsigned mL; // payload length in the frame
//        unsigned mM; // more data flag ("1" indicates segmentation)
//    };
}

#endif //OBTS_SMQUEUE_L2LENGTH_H
