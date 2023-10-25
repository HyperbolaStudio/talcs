#ifndef AUDIOSOURCECLIPSERIES_H
#define AUDIOSOURCECLIPSERIES_H

#include <TalcsCore/PositionableAudioSource.h>
#include <TalcsCore/AudioClipBase.h>

namespace talcs {

    using AudioSourceClip = AudioClipBase<PositionableAudioSource>;

    class AudioSourceClipSeriesPrivate;

    template <class ClipClass, class SeriesClass>
    class AudioSourceClipSeriesImpl;

    class TALCSCORE_EXPORT AudioSourceClipSeries
        : public PositionableAudioSource,
          public AudioClipSeriesBase<PositionableAudioSource> {
        Q_DECLARE_PRIVATE(AudioSourceClipSeries)
        friend class AudioSourceClipSeriesImpl<AudioSourceClip, AudioSourceClipSeries>;

    public:
        AudioSourceClipSeries();
        ~AudioSourceClipSeries() override;
        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        qint64 nextReadPosition() const override;
        void setNextReadPosition(qint64 pos) override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        bool addClip(const AudioSourceClip &clip) override;
        bool removeClipAt(qint64 pos) override;
        void clearClips() override;

    protected:
        explicit AudioSourceClipSeries(AudioSourceClipSeriesPrivate &d);
    };

}

#endif // AUDIOSOURCECLIPSERIES_H
