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

#ifndef AUDIOFORMATINPUTSOURCE_H
#define AUDIOFORMATINPUTSOURCE_H

#include <TalcsCore/PositionableAudioSource.h>
#include <TalcsCore/DuplicatableObject.h>
#include <TalcsFormat/TalcsFormatGlobal.h>

namespace talcs {

    class AudioFormatInputSourcePrivate;
    class AudioFormatIO;

    class TALCSFORMAT_EXPORT AudioFormatInputSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(AudioFormatInputSource);
    public:
        explicit AudioFormatInputSource(AudioFormatIO *audioFormatIo = nullptr, bool takeOwnership = false);
        ~AudioFormatInputSource() override;

        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        void setNextReadPosition(qint64 pos) override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void setAudioFormatIo(AudioFormatIO *audioFormatIo, bool takeOwnership = false);
        AudioFormatIO *audioFormatIo() const;

        void flush();

        void setStereoize(bool stereoize);
        bool stereoize() const;

        bool isDuplicatable() const override;
        DuplicatableObject *duplicate() const override;

    protected:
        explicit AudioFormatInputSource(AudioFormatInputSourcePrivate &d);
    };
    
}

#endif // AUDIOFORMATINPUTSOURCE_H
