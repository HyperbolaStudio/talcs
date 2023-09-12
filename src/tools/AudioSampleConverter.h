#ifndef TALCS_AUDIOSAMPLECONVERTER_H
#define TALCS_AUDIOSAMPLECONVERTER_H

#include <QtGlobal>

#include "global/TalcsGlobal.h"

namespace talcs {
    class TALCS_EXPORT AudioSampleConverter {
    public:
        static void convertToInt16(void *dest, const float *src, qint64 length, bool isLittleEndian);
        static void convertToInt24(void *dest, const float *src, qint64 length, bool isLittleEndian);
        static void convertToInt32(void *dest, const float *src, qint64 length, bool isLittleEndian);
        static void convertToFloat32(void *dest, const float *src, qint64 length, bool isLittleEndian);
        static void convertToFloat64(void *dest, const float *src, qint64 length, bool isLittleEndian);
    };
}



#endif // TALCS_AUDIOSAMPLECONVERTER_H
