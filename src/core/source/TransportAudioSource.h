/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
 *                                                                            *
 * This file is part of TALCS.                                                *
 *                                                                            *
 * TALCS is free software: you can redistribute it and/or modify it under the *
 * terms of the GNU Lesser General Public License as published by the Free    *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY   *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  *
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for    *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with TALCS. If not, see <https://www.gnu.org/licenses/>.             *
 ******************************************************************************/

#ifndef TALCS_TRANSPORTAUDIOSOURCE_H
#define TALCS_TRANSPORTAUDIOSOURCE_H

#include <QObject>

#include <TalcsCore/AudioSource.h>
#include <TalcsCore/PositionableAudioSource.h>

namespace talcs {

    class TransportAudioSourcePrivate;
    class TransportAudioSourceWriter;

    class TALCSCORE_EXPORT TransportAudioSource : public QObject, public AudioSource {
        Q_OBJECT
        Q_DECLARE_PRIVATE_D(AudioSource::d_ptr, TransportAudioSource)
        friend class TransportAudioSourceWriter;

    public:
        explicit TransportAudioSource(QObject *parent = nullptr);
        explicit TransportAudioSource(PositionableAudioSource *src, bool takeOwnership = false, QObject *parent = nullptr);
        ~TransportAudioSource() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void setSource(PositionableAudioSource *src, bool takeOwnership = false);
        PositionableAudioSource *source() const;

        enum PlaybackStatus {
            Paused,
            AboutToPlay,
            Playing,
            AboutToPause,
        };
        Q_ENUM(PlaybackStatus)
        PlaybackStatus playbackStatus() const;

        [[deprecated("use playbackStatus() instead.")]]
        bool isPlaying() const;

        qint64 position() const;
        void setPosition(qint64 position);
        qint64 length() const;

        QPair<qint64, qint64> loopingRange() const;
        void setLoopingRange(qint64 l, qint64 r);

        int bufferingCounter() const;

    public slots:
        void play();
        void pause();
        void acquireBuffering();
        void releaseBuffering();

    signals:
        void positionAboutToChange(qint64 nextPosition);
        void playbackStatusChanged(PlaybackStatus playbackStatus);
        void bufferingCounterChanged(int counter);

    protected:
        TransportAudioSource(TransportAudioSourcePrivate &d, QObject *parent);
        qint64 processReading(const AudioSourceReadData &readData) override;
    };

    class TransportAudioSourceStateSaverPrivate;

    class TALCSCORE_EXPORT TransportAudioSourceStateSaver {
    public:
        explicit TransportAudioSourceStateSaver(TransportAudioSource *src);
        ~TransportAudioSourceStateSaver();

    private:
        QScopedPointer<TransportAudioSourceStateSaverPrivate> d;
    };

}

#endif // TALCS_TRANSPORTAUDIOSOURCE_H
