// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineSlider.h>

#include <tlQt/TimelineThumbnailProvider.h>

#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>

#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

namespace tl
{
    namespace qtwidget
    {
        namespace
        {
            const int stripeSize = 5;
            const int handleSize = 3;
        }

        struct TimelineSlider::Private
        {
            std::weak_ptr<system::Context> context;
            qt::TimelineThumbnailProvider* thumbnailProvider = nullptr;
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
            qt::TimelinePlayer* timelinePlayer = nullptr;
            qt::TimeUnits units = qt::TimeUnits::Timecode;
            qt::TimeObject* timeObject = nullptr;
            bool thumbnails = true;
            qint64 thumbnailRequestId = 0;
            std::map<otime::RationalTime, QImage> thumbnailImages;
            bool stopOnScrub = true;
        };

        TimelineSlider::TimelineSlider(
            qt::TimelineThumbnailProvider* thumbnailProvider,
            const std::shared_ptr<system::Context>& context,
            QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            p.thumbnailProvider = thumbnailProvider;
            if (p.thumbnailProvider)
            {
                connect(
                    p.thumbnailProvider,
                    SIGNAL(thumbails(qint64, const QList<QPair<otime::RationalTime, QImage> >&)),
                    SLOT(_thumbnailsCallback(qint64, const QList<QPair<otime::RationalTime, QImage> >&)));
            }

            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);

            _thumbnailsUpdate();
        }

        TimelineSlider::~TimelineSlider()
        {}

        void TimelineSlider::setTimeObject(qt::TimeObject* timeObject)
        {
            TLRENDER_P();
            if (timeObject == p.timeObject)
                return;
            if (p.timeObject)
            {
                disconnect(
                    p.timeObject,
                    SIGNAL(unitsChanged(tl::qt::Time::Units)),
                    this,
                    SLOT(setUnits(tl::qt::Time::Units)));
            }
            p.timeObject = timeObject;
            if (p.timeObject)
            {
                p.units = p.timeObject->units();
                connect(
                    p.timeObject,
                    SIGNAL(unitsChanged(tl::qt::TimeUnits)),
                    SLOT(setUnits(tl::qt::TimeUnits)));
            }
            update();
        }

        void TimelineSlider::setColorConfigOptions(const timeline::ColorConfigOptions& value)
        {
            TLRENDER_P();
            if (value == p.colorConfigOptions)
                return;
            p.colorConfigOptions = value;
            _thumbnailsUpdate();
        }

        void TimelineSlider::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            _thumbnailsUpdate();
        }

        void TimelineSlider::setTimelinePlayer(qt::TimelinePlayer* timelinePlayer)
        {
            TLRENDER_P();
            if (timelinePlayer == p.timelinePlayer)
                return;
            if (p.timelinePlayer)
            {
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(update()));
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                    this,
                    SLOT(update()));
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(cacheInfoChanged(const tl::timeline::PlayerCacheInfo&)),
                    this,
                    SLOT(update()));
            }
            p.timelinePlayer = timelinePlayer;
            if (p.timelinePlayer)
            {
                connect(
                    p.timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    SLOT(update()));
                connect(
                    p.timelinePlayer,
                    SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                    SLOT(update()));
                connect(
                    p.timelinePlayer,
                    SIGNAL(cacheInfoChanged(const tl::timeline::PlayerCacheInfo&)),
                    SLOT(update()));
            }
            _thumbnailsUpdate();
        }

        qt::TimeUnits TimelineSlider::units() const
        {
            return _p->units;
        }

        bool TimelineSlider::hasThumbnails() const
        {
            return _p->thumbnails;
        }

        bool TimelineSlider::hasStopOnScrub() const
        {
            return _p->stopOnScrub;
        }

        void TimelineSlider::setUnits(qt::TimeUnits value)
        {
            TLRENDER_P();
            if (value == p.units)
                return;
            p.units = value;
            update();
        }

        void TimelineSlider::setThumbnails(bool value)
        {
            TLRENDER_P();
            if (value == p.thumbnails)
                return;
            p.thumbnails = value;
            _thumbnailsUpdate();
            updateGeometry();
        }

        void TimelineSlider::setStopOnScrub(bool value)
        {
            TLRENDER_P();
            if (value == p.stopOnScrub)
                return;
            p.stopOnScrub = value;
        }

        void TimelineSlider::resizeEvent(QResizeEvent* event)
        {
            if (event->oldSize() != size())
            {
                _thumbnailsUpdate();
            }
        }

        void TimelineSlider::paintEvent(QPaintEvent*)
        {
            TLRENDER_P();
            QPainter painter(this);
            const QPalette& palette = this->palette();
            QRect rect = this->rect();
            painter.fillRect(rect, palette.color(QPalette::ColorRole::Base));
            if (p.timelinePlayer)
            {
                QRect rect2 = rect.adjusted(0, handleSize, 0, -handleSize);
                int x0 = 0;
                int y0 = 0;
                int x1 = 0;
                int y1 = 0;
                int h = 0;

                // Draw thumbnails.
                x0 = _timeToPos(p.timelinePlayer->currentTime());
                y0 = rect2.y();
                for (const auto& i : p.thumbnailImages)
                {
                    painter.drawImage(QPoint(_timeToPos(i.first), y0), i.second);
                }

                // Draw in/out points.
                const auto& inOutRange = p.timelinePlayer->inOutRange();
                x0 = _timeToPos(inOutRange.start_time());
                x1 = _timeToPos(inOutRange.end_time_inclusive());
                y1 = y0 + rect2.height();
                h = stripeSize * 2;
                painter.fillRect(
                    QRect(x0, y1 - h, x1 - x0, h),
                    palette.color(QPalette::ColorRole::Button));

                // Draw cached frames.
                const auto& cacheInfo = p.timelinePlayer->cacheInfo();
                auto color = QColor(40, 190, 40);
                h = stripeSize;
                for (const auto& i : cacheInfo.videoFrames)
                {
                    x0 = _timeToPos(i.start_time());
                    x1 = _timeToPos(i.end_time_inclusive());
                    painter.fillRect(QRect(x0, y1 - h * 2, x1 - x0, h), color);
                }
                color = QColor(190, 190, 40);
                for (const auto& i : cacheInfo.audioFrames)
                {
                    x0 = _timeToPos(i.start_time());
                    x1 = _timeToPos(i.end_time_inclusive());
                    painter.fillRect(QRect(x0, y1 - h, x1 - x0, h), color);
                }

                // Draw the current time.
                x0 = _timeToPos(p.timelinePlayer->currentTime());
                y0 = 0;
                painter.fillRect(
                    QRect(x0 - handleSize / 2, y0, handleSize, rect.height()),
                    palette.color(QPalette::ColorRole::Text));
            }
        }

        void TimelineSlider::mousePressEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            if (p.timelinePlayer)
            {
                if (p.stopOnScrub)
                {
                    p.timelinePlayer->setPlayback(timeline::Playback::Stop);
                }
                p.timelinePlayer->seek(_posToTime(event->x()));
            }
        }

        void TimelineSlider::mouseReleaseEvent(QMouseEvent*)
        {}

        void TimelineSlider::mouseMoveEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            if (p.timelinePlayer)
            {
                p.timelinePlayer->seek(_posToTime(event->x()));
            }
        }

        void TimelineSlider::wheelEvent(QWheelEvent* event)
        {
            TLRENDER_P();
            if (p.timelinePlayer)
            {
                const auto t = p.timelinePlayer->currentTime();
                const float delta = event->angleDelta().y() / 8.F / 15.F;
                p.timelinePlayer->seek(t + otime::RationalTime(delta, t.rate()));
            }
        }

        void TimelineSlider::_thumbnailsCallback(qint64 id, const QList<QPair<otime::RationalTime, QImage> >& thumbnails)
        {
            TLRENDER_P();
            if (p.thumbnails)
            {
                if (id == p.thumbnailRequestId)
                {
                    for (const auto& i : thumbnails)
                    {
                        p.thumbnailImages[i.first] = i.second;
                    }
                }
                update();
            }
        }

        otime::RationalTime TimelineSlider::_posToTime(int value) const
        {
            TLRENDER_P();
            otime::RationalTime out = time::invalidTime;
            if (p.timelinePlayer)
            {
                const auto& timeRange = p.timelinePlayer->timeRange();
                out = otime::RationalTime(
                    floor(math::clamp(value, 0, width()) / static_cast<double>(width()) * (timeRange.duration().value() - 1) + timeRange.start_time().value()),
                    timeRange.duration().rate());
            }
            return out;
        }

        int TimelineSlider::_timeToPos(const otime::RationalTime& value) const
        {
            TLRENDER_P();
            int out = 0;
            if (p.timelinePlayer)
            {
                const auto& timeRange = p.timelinePlayer->timeRange();
                out = (value.value() - timeRange.start_time().value()) /
                    (timeRange.duration().value() > 1 ? (timeRange.duration().value() - 1) : 1) *
                    width();
            }
            return out;
        }

        void TimelineSlider::_thumbnailsUpdate()
        {
            TLRENDER_P();
            QString fileName;
            if (p.timelinePlayer)
            {
                fileName = QString::fromUtf8(p.timelinePlayer->path().get().c_str());
            }
            if (p.thumbnailProvider)
            {
                p.thumbnailProvider->cancelRequests(p.thumbnailRequestId);
                p.thumbnailRequestId = 0;
            }
            p.thumbnailImages.clear();
            if (p.thumbnailProvider && p.timelinePlayer && p.thumbnails)
            {
                setMinimumHeight(50);

                const auto& info = p.timelinePlayer->ioInfo();
                const auto rect = this->rect().adjusted(0, 0, 0, -(stripeSize * 2 + handleSize * 2));
                const int width = rect.width();
                const int height = rect.height();
                const int thumbnailWidth = !info.video.empty() ?
                    static_cast<int>(height * info.video[0].size.getAspect()) :
                    0;
                const int thumbnailHeight = height;
                if (thumbnailWidth > 0)
                {
                    QList<otime::RationalTime> requests;
                    int x = rect.x();
                    while (x < width)
                    {
                        requests.push_back(_posToTime(x));
                        x += thumbnailWidth;
                    }
                    p.thumbnailRequestId = p.thumbnailProvider->request(
                        fileName,
                        requests,
                        QSize(thumbnailWidth, thumbnailHeight),
                        p.colorConfigOptions,
                        p.lutOptions);
                }
            }
            else
            {
                setMinimumHeight(stripeSize * 2 + handleSize * 2);
            }
            update();
        }
    }
}
