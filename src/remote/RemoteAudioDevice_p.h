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

#ifndef TALCS_REMOTEAUDIODEVICE_P_H
#define TALCS_REMOTEAUDIODEVICE_P_H

#include <QMutex>
#include <QSharedMemory>

#include <TalcsCore/AudioDataWrapper.h>
#include <TalcsDevice/private/AudioDevice_p.h>
#include <TalcsRemote/RemoteAudioDevice.h>

namespace boost::interprocess {
    class named_condition;
    class named_mutex;
}

namespace talcs {
    class RemoteAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(RemoteAudioDevice);
        QRecursiveMutex mutex;

        RemoteSocket *socket;

        bool remoteIsOpened = false;

        QScopedPointer<boost::interprocess::named_condition> prepareBufferCondition;
        QScopedPointer<boost::interprocess::named_mutex> prepareBufferMutex;
        QSharedMemory sharedMemory;
        enum BufferPrepareStatus {
            NotPrepared,
            Prepared,
            GoingToClose,
        };
        char *bufferPrepareStatus;
        QVector<float *> sharedAudioData;
        RemoteAudioDevice::ProcessInfo *processInfo = nullptr;
        QScopedPointer<AudioDataWrapper> buffer;

        QScopedPointer<QThread> prepareBufferProducerThread;

        AudioDeviceCallback *audioDeviceCallback = nullptr;

        QList<RemoteAudioDevice::ProcessInfoCallback *> processInfoCallbackList;

        void remoteOpenRequired(qint64 bufferSize, double sampleRate, const QString &ipcKey, int maxChannelCount);
        void remoteCloseRequired();
        void remotePrepareBufferProducer();
        void remotePrepareBuffer();
    };
}



#endif // TALCS_REMOTEAUDIODEVICE_P_H
