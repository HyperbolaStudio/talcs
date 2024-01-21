/******************************************************************************
 * Copyright (c) 2024 CrSjimo                                                 *
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

#ifndef TALCS_ICLIPSERIES_H
#define TALCS_ICLIPSERIES_H

#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    class IClipSeriesPrivate;

    class TALCSCORE_EXPORT ClipViewImpl {
    public:
        ClipViewImpl();

        bool isValid() const;

        qintptr content() const;

        qint64 startPos() const;
        void setStartPos(qint64 startPos);

        qint64 position() const;
        qint64 length() const;
        bool resetRange(qint64 position, qint64 length);

        template<class ClipView>
        ClipViewImpl(const ClipView &clipView) : ClipViewImpl(clipView.m_impl) {
        }

    private:
        friend class IClipSeriesPrivate;
        ClipViewImpl(IClipSeriesPrivate *d, qintptr content);
        IClipSeriesPrivate *d;
        qintptr m_content;
    };

    template <class T>
    class IClipSeries {
    public:
        class ClipView {
        public:
            ClipView() = default;

            explicit ClipView(const ClipViewImpl &impl) : m_impl(impl) {
            }

            bool isValid() {
                return m_impl.isValid();
            }

            T *content() {
                return reinterpret_cast<T *>(m_impl.content());
            }

            qint64 startPos() const {
                return m_impl.startPos();
            }

            void setStartPos(qint64 startPos) {
                m_impl.setStartPos(startPos);
            }

            qint64 position() const {
                return m_impl.position();
            }

            qint64 length() const {
                return m_impl.length();
            }

            bool resetRange(qint64 position, qint64 length) {
                return m_impl.resetRange(position, length);
            }
        private:
            friend class ClipViewImpl;
            ClipViewImpl m_impl;
        };

        virtual ClipView insertClip(T *content, qint64 position, qint64 startPos, qint64 length) = 0;

        virtual ClipView findClip(T *content) const = 0;
        virtual ClipView findClip(qint64 position) const = 0;

        virtual void removeClip(const ClipView &clip) = 0;
        virtual void removeAllClips() = 0;

        virtual QList<ClipView> clips() const = 0;

        virtual qint64 effectiveLength() const = 0;

    };

} // talcs

#endif //TALCS_ICLIPSERIES_H