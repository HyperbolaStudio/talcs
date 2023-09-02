#ifndef TALCS_TRANSPORTAUDIOSOURCE_P_H
#define TALCS_TRANSPORTAUDIOSOURCE_P_H

#include "TransportAudioSource.h"
#include "AudioSource_p.h"

#include <QMutex>

namespace talcs {
    class TransportAudioSourcePrivate : public AudioSourcePrivate {
        Q_DECLARE_PUBLIC(TransportAudioSource)
    public:
        PositionableAudioSource *src = nullptr;
        bool takeOwnership = false;

        qint64 position = 0;
        bool isPlaying = false;
        qint64 loopingStart = -1;
        qint64 loopingEnd = -1;
        QMutex mutex;

        void _q_positionAboutToChange(qint64 pos);
    };
}

#endif // TALCS_TRANSPORTAUDIOSOURCE_P_H
