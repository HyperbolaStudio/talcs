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

#ifndef TALCS_SDLAUDIODEVICE_P_H
#define TALCS_SDLAUDIODEVICE_P_H

#include <SDL2/SDL.h>

#include <TalcsDevice/private/AudioDevice_p.h>

namespace talcs {

    class SDLAudioDevicePrivate;
    class SDLAudioDriver;

    class SDLAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(SDLAudioDevice)
    public:
        ~SDLAudioDevice() override;
        bool start(AudioDeviceCallback *audioDeviceCallback) override;
        void stop() override;
        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        void lock() override;
        void unlock() override;
        bool openControlPanel() override;

    protected:
        friend class SDLAudioDriver;
        SDLAudioDevice(const QString &name, SDLAudioDriver *driver);
    };

    class SDLAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(SDLAudioDevice)
    public:
        quint32 devId = 0;
        AudioDeviceCallback *audioDeviceCallback = nullptr;
        SDL_AudioSpec spec = {};

        void sdlCallback(quint8 *rawBuf, int length);
    };
}

#endif // TALCS_SDLAUDIODEVICE_P_H
