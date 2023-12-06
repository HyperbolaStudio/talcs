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

#include <cmath>

#include <QCoreApplication>
#include <QFile>
#include <QDebug>
#include <QTime>
#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/R8BrainMultichannelResampler.h>
#include <TalcsCore/InterleavedAudioDataWrapper.h>
#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/AudioSource.h>
#include <TalcsCore/InterleavedAudioDataWrapper.h>

using namespace talcs;

class MyResampler : public R8BrainMultichannelResampler {
public:
    MyResampler(double ratio, qint64 bufferSize, int channelCount, AudioFormatIO *io) :
        R8BrainMultichannelResampler(ratio, bufferSize, channelCount),
        io(io) {}
private:
    AudioFormatIO *io;
    QVector<float> data;
    void read(const AudioSourceReadData &readData) override {
        data.resize(readData.length * io->channelCount());
        io->read(data.data(), readData.length);
        InterleavedAudioDataWrapper wrapper(data.data(), io->channelCount(), readData.length);
        readData.buffer->setSampleRange(0, readData.startPos, readData.length, wrapper, 0, 0);
        readData.buffer->setSampleRange(1, readData.startPos, readData.length, wrapper, 1, 0);
    }
};

void testFunc() {
    {
        QFile f("D:\\CloudMusic\\07.恋染色.flac");
        AudioFormatIO io(&f);
        io.open(QFile::ReadOnly);
        MyResampler resampler(96000.0 / 44100.0, 1024, 2, &io);
        QVector<float> data(1024 * 2);
        InterleavedAudioDataWrapper wrapper(data.data(), 2, 1024);
        QFile fOut("D:\\test.pcm");
        fOut.open(QFile::WriteOnly);
        QTime t0 = QTime::currentTime();
        for (int i = 0; i < std::ceil(io.length() * 96000.0 / 44100.0 / 1024.0); i++) {
            resampler.process(&wrapper);
            fOut.write((char*)(data.data()), 2048 * sizeof(float));
        }
        qDebug() << t0.msecsTo(QTime::currentTime());
        fOut.flush();
    }

    qDebug() << "Finished";

}


int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);
    testFunc();
    return a.exec();
}
