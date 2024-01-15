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

#include "FutureAudioSourceClipSeries.h"
#include "FutureAudioSourceClipSeries_p.h"

#include "FutureAudioSource.h"
#include <TalcsCore/TransportAudioSource.h>

namespace talcs {

    FutureAudioSourceClipSeriesPrivate::FutureAudioSourceClipSeriesPrivate()
        : AudioSourceClipSeriesBase(this) {
    }
    void FutureAudioSourceClipSeriesPrivate::postAddClip(const ClipInterval &clip) {
        Q_Q(FutureAudioSourceClipSeries);
        cachedClipsLength += clip.length();
        emit q->progressChanged(cachedLengthAvailable, cachedLengthLoaded, cachedClipsLength, q->effectiveLength());
        QObject::connect(reinterpret_cast<FutureAudioSource *>(clip.content()), &FutureAudioSource::progressChanged, q, [=](int value) {
            cachedLengthLoaded += (value - clipLengthLoadedDict[clip.position()]);
            clipLengthLoadedDict[clip.position()] = value;
            emit q->progressChanged(cachedLengthAvailable, cachedLengthLoaded, cachedClipsLength, q->effectiveLength());
        });
        QObject::connect(reinterpret_cast<FutureAudioSource *>(clip.content()), &FutureAudioSource::statusChanged, q, [=](FutureAudioSource::Status status) {
            if (status == FutureAudioSource::Ready) {
                cachedLengthAvailable += clip.length();
                clipLengthCachedDict[clip.position()] = true;
                emit q->progressChanged(cachedLengthAvailable, cachedLengthLoaded, cachedClipsLength, q->effectiveLength());
                checkAndNotify();
            }
        });
    }
    void FutureAudioSourceClipSeriesPrivate::postRemoveClip(const ClipInterval &clip) {
        Q_Q(FutureAudioSourceClipSeries);
        QObject::disconnect(reinterpret_cast<FutureAudioSource *>(clip.content()), nullptr, q, nullptr);
        cachedClipsLength -= clip.length();
        cachedLengthLoaded -= clipLengthLoadedDict[clip.position()];
        if (clipLengthCachedDict[clip.position()]) {
            cachedLengthAvailable -= clip.length();
        }
        clipLengthLoadedDict.remove(clip.position());
        clipLengthCachedDict.remove(clip.position());
        emit q->progressChanged(cachedLengthAvailable, cachedLengthLoaded, cachedClipsLength, q->effectiveLength());
    }
    void FutureAudioSourceClipSeriesPrivate::postRemoveAllClips() {
        Q_Q(FutureAudioSourceClipSeries);
        for (auto p = clips.cbegin(); p != clips.cend(); p++) {
            postRemoveClip(p->interval());
        }
    }
    void FutureAudioSourceClipSeriesPrivate::notifyPause() {
        if (bufferingTarget && !isPauseRequiredEmitted) {
            bufferingTarget->acquireBuffering();
            isPauseRequiredEmitted = true;
        }
    }
    void FutureAudioSourceClipSeriesPrivate::notifyResume() {
        if (bufferingTarget && isPauseRequiredEmitted) {
            bufferingTarget->releaseBuffering();
            isPauseRequiredEmitted = false;
        }
    }
    void FutureAudioSourceClipSeriesPrivate::checkAndNotify(qint64 position, qint64 length) {
        Q_Q(FutureAudioSourceClipSeries);
        if (readMode == FutureAudioSourceClipSeries::Notify) {
            if (!q->canRead(position + length, length)) {
                notifyPause();
            } else {
                notifyResume();
            }
        }
    }
    void FutureAudioSourceClipSeriesPrivate::checkAndNotify() {
        Q_Q(FutureAudioSourceClipSeries);
        checkAndNotify(position, q->bufferSize());
    }

    bool FutureAudioSourceClipSeriesPrivate::resetClipRange(qintptr content, qint64 newPosition, qint64 newLength) {
        auto oldInterval = intervalLookup(clipPositionDict.value(content));
        if (IClipSeriesPrivate::resetClipRange(content, newPosition, newLength)) {
            postRemoveClip(oldInterval);
            postAddClip({content, newPosition, newLength});
            return true;
        }
        return false;
    }

    /**
     * @class FutureAudioSourceClipSeries
     * @brief An AudioClipsSeriesBase object that uses FutureAudioSource
     */

    /**
     * Default constructor.
     */
    FutureAudioSourceClipSeries::FutureAudioSourceClipSeries(QObject *parent)
        : QObject(parent), PositionableAudioSource(*new FutureAudioSourceClipSeriesPrivate) {
    }

    /**
     * Destructor.
     */
    FutureAudioSourceClipSeries::~FutureAudioSourceClipSeries() {
        FutureAudioSourceClipSeries::close();
    }

    qint64 FutureAudioSourceClipSeries::read(const AudioSourceReadData &readData) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        FutureAudioSourceClipSeriesPrivate::ClipInterval readDataInterval(0, d->position, readData.length);
        d->checkAndNotify(d->position + readData.length, readData.length);
        for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
            readData.buffer->clear(ch, readData.startPos, readData.length);
        }
        qAsConst(d->clips).overlap_find_all(
            readDataInterval, [=, &readDataInterval](const decltype(d->clips)::const_iterator &it) {
                auto clip = it->interval();
                auto [clipReadPosition, clipReadData] = d->calculateClipReadData(clip, d->position, readData);
                auto clipSrc = reinterpret_cast<FutureAudioSource *>(clip.content());
                clipSrc->setNextReadPosition(clipReadPosition);

                if (d->readMode == Block)
                    clipSrc->wait();
                clipSrc->read(clipReadData);
                return true;
            });
        d->position += readData.length;
        return readData.length;
    }

    qint64 FutureAudioSourceClipSeries::length() const {
        return std::numeric_limits<qint64>::max();
    }

    qint64 FutureAudioSourceClipSeries::nextReadPosition() const {
        return PositionableAudioSource::nextReadPosition();
    }

    void FutureAudioSourceClipSeries::setNextReadPosition(qint64 pos) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        if (d->position != pos) {
            d->position = pos;
            d->checkAndNotify();
        }
    }

    bool FutureAudioSourceClipSeries::open(qint64 bufferSize, double sampleRate) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        if (d->openAllClips(bufferSize, sampleRate))
            return AudioStreamBase::open(bufferSize, sampleRate);
        return false;
    }

    void FutureAudioSourceClipSeries::close() {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->closeAllClips();
        AudioStreamBase::close();
    }

    FutureAudioSourceClipSeries::ClipView
    FutureAudioSourceClipSeries::insertClip(FutureAudioSource *content, qint64 position, qint64 startPos,
                                            qint64 length) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        if (!d->preInsertClip(content))
            return ClipView(d->nullClipViewImpl());
        auto ret = ClipView(d->insertClip(reinterpret_cast<qintptr>(content), position, startPos, length));
        d->postAddClip(d->intervalLookup(ret.position()));
        d->checkAndNotify();
        return ret;
    }

    FutureAudioSourceClipSeries::ClipView FutureAudioSourceClipSeries::findClip(FutureAudioSource *content) const {
        Q_D(const FutureAudioSourceClipSeries);
        return ClipView(d->findClipByContent(reinterpret_cast<qintptr>(content)));
    }

    FutureAudioSourceClipSeries::ClipView FutureAudioSourceClipSeries::findClip(qint64 position) const {\
        Q_D(const FutureAudioSourceClipSeries);
        return ClipView(d->findClipByPosition(position));
    }

    void FutureAudioSourceClipSeries::removeClip(const FutureAudioSourceClipSeries::ClipView &clip) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        auto clipInterval = d->intervalLookup(clip.position());
        d->removeClip(clip);
        d->postRemoveClip(clipInterval);
        d->checkAndNotify();
    }

    void FutureAudioSourceClipSeries::removeAllClips() {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->removeAllClips();
        d->postRemoveAllClips();
        d->checkAndNotify();
    }

    QList<FutureAudioSourceClipSeries::ClipView> FutureAudioSourceClipSeries::clips() const {
        Q_D(const FutureAudioSourceClipSeries);
        QList<ClipView> list;
        for (const auto &impl : d->clipViewImplList())
            list.append(ClipView(impl));
        return list;
    }

    qint64 FutureAudioSourceClipSeries::effectiveLength() const {
        Q_D(const FutureAudioSourceClipSeries);
        return d->effectiveLength();
    }

    /**
     * Gets the length of audio that is able to be played within the series (including blank intervals).
     */
    qint64 FutureAudioSourceClipSeries::lengthAvailable() const {
        Q_D(const FutureAudioSourceClipSeries);
        return d->cachedLengthAvailable;
    }

    /**
     * Gets the total length of the audio that is already loaded in each clips.
     */
    qint64 FutureAudioSourceClipSeries::lengthLoaded() const {
        Q_D(const FutureAudioSourceClipSeries);
        return d->cachedLengthLoaded;
    }

    /**
     * Gets the total length of all clips.
     */
    qint64 FutureAudioSourceClipSeries::lengthOfAllClips() const {
        Q_D(const FutureAudioSourceClipSeries);
        return d->cachedClipsLength;
    }

    /**
     * Gets whether a block of audio of a specified length can be read from the series.
     * @param from the start position of reading
     * @param length the length of audio measured in samples
     */
    bool FutureAudioSourceClipSeries::canRead(qint64 from, qint64 length) const {
        Q_D(const FutureAudioSourceClipSeries);
        FutureAudioSourceClipSeriesPrivate::ClipInterval queryInterval(0, from, length);
        bool flag = true;
        qAsConst(d->clips).overlap_find_all(queryInterval, [=, &flag](const decltype(d->clips)::const_iterator &it) {
            auto clip = it->interval();
            if (reinterpret_cast<FutureAudioSource *>(clip.content())->status() != FutureAudioSource::Ready) {
                flag = false;
                return false;
            }
            return true;
        });
        return flag;
    }

    /**
     * @enum FutureAudioSourceClipSeries::ReadMode
     * The read modes.
     *
     * @var FutureAudioSourceClipSeries::Notify
     * Default mode. Corresponding signals are emitted when the clip to read is preparing.
     * @see pauseRequired(), resumeRequired()
     *
     * @var FutureAudioSourceClipSeries::Skip
     * The unready clip is skipped.
     *
     * @var FutureAudioSourceClipSeries::Block
     * Waits for the unready clip to finish.
     */

    /**
     * Sets the read mode.
     */
    void FutureAudioSourceClipSeries::setReadMode(FutureAudioSourceClipSeries::ReadMode readMode) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->readMode = readMode;
    }

    /**
     * Gets the read mode.
     */
    FutureAudioSourceClipSeries::ReadMode FutureAudioSourceClipSeries::readMode() const {
        Q_D(const FutureAudioSourceClipSeries);
        return d->readMode;
    }

    void FutureAudioSourceClipSeries::setBufferingTarget(TransportAudioSource *target) {
        Q_D(FutureAudioSourceClipSeries);
        QMutexLocker locker(&d->mutex);
        d->bufferingTarget = target;
    }

    TransportAudioSource *FutureAudioSourceClipSeries::bufferingTarget() const {
        Q_D(const FutureAudioSourceClipSeries);
        return d->bufferingTarget;
    }

    /**
     * @fn void FutureAudioSourceClipSeries::progressChanged(qint64 lengthAvailable, qint64 lengthLoaded, qint64 lengthOfAllClips, qint64 effectiveLength)
     * Emitted when one of these parameters are changed.
     */

}