#ifndef TALCS_AUDIOCLIPBASE_H
#define TALCS_AUDIOCLIPBASE_H

#include <set>

#include <QPair>
#include <QScopedPointer>

#include "interval-tree/interval_tree.hpp"

namespace talcs {

    using AudioClipBaseInterval_ = lib_interval_tree::interval<qint64>;

    template <class T>
    struct AudioClipBase : public AudioClipBaseInterval_ {
        AudioClipBase(qint64 position, T *content, qint64 startPos, qint64 length)
            : AudioClipBaseInterval_(position, position + length - 1), m_content(content), m_startPos(startPos) {
        }
        AudioClipBase(qint64 position, qint64 length) : AudioClipBase(position, nullptr, 0, length) {
        }
        AudioClipBase() : AudioClipBase(-1, 0) {
        }

        qint64 position() const {
            return low_;
        }

        T *content() const {
            return m_content;
        }

        qint64 contentStartPosition() const {
            return m_startPos;
        }

        qint64 endPosition() const {
            return high_ + 1;
        }

        qint64 length() const {
            return high_ - low_ + 1;
        }

    protected:
        T *m_content;
        qint64 m_startPos;
    };

    template <class T>
    class AudioClipSeriesBase {
        using AudioClipBaseIntervalTree_ = lib_interval_tree::interval_tree<AudioClipBase<T>>;

    public:
        virtual bool addClip(const AudioClipBase<T> &clip) {
            if (qAsConst(m_clips).overlap_find(clip) != m_clips.cend())
                return false;
            m_clips.insert(clip);
            m_endSet.insert(clip.endPosition());
            return true;
        }

        AudioClipBase<T> findClipAt(qint64 pos) const {
            auto it = findClipIt(pos);
            if (it == m_clips.end())
                return {};
            return *it;
        }

        QList<AudioClipBase<T>> clips() const {
            QList<AudioClipBase<T>> l;
            for (const auto &v : m_clips) {
                l.push_back(v);
            }
            return l;
        }

        virtual bool removeClipAt(qint64 pos) {
            auto it = findClipIt(pos);
            if (it == m_clips.end())
                return false;
            m_clips.erase(it);
            m_endSet.erase(it->interval().endPosition());
            return true;
        }

        virtual void clearClips() {
            m_clips.clear();
        }

        qint64 effectiveLength() const {
            if (m_endSet.empty())
                return 0;
            return *m_endSet.rbegin();
        }

        static QPair<qint64, AudioClipBase<T>> calculateClipReadData(const AudioClipBase<T> &clip,
                                                                     const AudioClipBase<T> &readDataInterval) {
            auto headCut = std::max(0ll, readDataInterval.position() - clip.position());
            auto tailCut = std::max(0ll, clip.endPosition() - readDataInterval.endPosition());
            auto readStart = std::max(0ll, clip.position() - readDataInterval.position());
            qint64 clipReadPosition = headCut + clip.contentStartPosition();
            AudioClipBase<T> clipReadInterval(readStart, clip.length() - headCut - tailCut);
            return {clipReadPosition, clipReadInterval};
        }

    protected:
        typename AudioClipBaseIntervalTree_::iterator findClipIt(qint64 pos) {
            return m_clips.overlap_find({pos, pos + 1});
        }
        typename AudioClipBaseIntervalTree_::const_iterator findClipIt(qint64 pos) const {
            return m_clips.overlap_find({pos, pos + 1});
        }
        AudioClipBaseIntervalTree_ m_clips;
        std::set<qint64> m_endSet;
    };
}

#endif // TALCS_AUDIOCLIPBASE_H
