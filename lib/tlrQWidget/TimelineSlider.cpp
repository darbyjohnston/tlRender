// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQWidget/TimelineSlider.h>

#include <tlrQt/TimelineThumbnailProvider.h>

#include <tlrCore/Math.h>
#include <tlrCore/StringFormat.h>

#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

namespace tlr
{
    namespace qwidget
    {
        struct TimelineSlider::Private
        {
            gl::ColorConfig colorConfig;
            qt::TimelinePlayer* timelinePlayer = nullptr;
            qt::TimelineThumbnailProvider* thumbnailProvider = nullptr;
            std::map<otime::RationalTime, QImage> thumbnails;
            qt::TimeUnits units = qt::TimeUnits::Timecode;
            qt::TimeObject* timeObject = nullptr;
        };

        TimelineSlider::TimelineSlider(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
            setMinimumHeight(50);
        }
        
        TimelineSlider::~TimelineSlider()
        {}

        void TimelineSlider::setTimeObject(qt::TimeObject* timeObject)
        {
            TLR_PRIVATE_P();
            if (timeObject == p.timeObject)
                return;
            if (p.timeObject)
            {
                disconnect(
                    p.timeObject,
                    SIGNAL(unitsChanged(tlr::qt::Time::Units)),
                    this,
                    SLOT(setUnits(tlr::qt::Time::Units)));
            }
            p.timeObject = timeObject;
            if (p.timeObject)
            {
                p.units = p.timeObject->units();
                connect(
                    p.timeObject,
                    SIGNAL(unitsChanged(tlr::qt::TimeUnits)),
                    SLOT(setUnits(tlr::qt::TimeUnits)));
            }
            update();
        }

        void TimelineSlider::setColorConfig(const gl::ColorConfig& colorConfig)
        {
            TLR_PRIVATE_P();
            p.colorConfig = colorConfig;
            if (p.thumbnailProvider)
            {
                p.thumbnailProvider->setColorConfig(p.colorConfig);
            }
        }

        void TimelineSlider::setTimelinePlayer(qt::TimelinePlayer* timelinePlayer)
        {
            TLR_PRIVATE_P();
            if (timelinePlayer == p.timelinePlayer)
                return;
            if (p.timelinePlayer)
            {
                p.thumbnailProvider->setParent(nullptr);
                delete p.thumbnailProvider;
                p.thumbnailProvider = nullptr;
                disconnect(
                    p.timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
            }
            p.timelinePlayer = timelinePlayer;
            if (p.timelinePlayer)
            {
                if (auto context = p.timelinePlayer->context().lock())
                {
                    timeline::Options options;
                    options.requestCount = 1;
                    options.requestTimeout = std::chrono::milliseconds(100);
                    options.avioOptions["SequenceIO/ThreadCount"] = string::Format("{0}").arg(1);
                    options.avioOptions["ffmpeg/ThreadCount"] = string::Format("{0}").arg(1);
                    auto timeline = timeline::Timeline::create(p.timelinePlayer->path(), context, options);
                    p.thumbnailProvider = new qt::TimelineThumbnailProvider(timeline, context, this);
                    p.thumbnailProvider->setColorConfig(p.colorConfig);
                    connect(
                        p.timelinePlayer,
                        SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                        SLOT(_currentTimeCallback(const otime::RationalTime&)));
                    connect(
                        p.timelinePlayer,
                        SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                        SLOT(_inOutRangeCallback(const otime::TimeRange&)));
                    connect(
                        p.timelinePlayer,
                        SIGNAL(cachedFramesChanged(const std::vector<otime::TimeRange>&)),
                        SLOT(_cachedFramesCallback(const std::vector<otime::TimeRange>&)));
                    connect(
                        p.thumbnailProvider,
                        SIGNAL(thumbails(const QList<QPair<otime::RationalTime, QImage> >&)),
                        SLOT(_thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >&)));
                }
            }
            _thumbnailsUpdate();
        }

        void TimelineSlider::setUnits(qt::TimeUnits units)
        {
            TLR_PRIVATE_P();
            if (p.units == units)
                return;
            p.units = units;
            update();
        }

        void TimelineSlider::resizeEvent(QResizeEvent* event)
        {
            if (event->oldSize() != size())
            {
                _thumbnailsUpdate();
            }
        }

        namespace
        {
            const int stripeSize = 5;
            const int handleSize = 5;
        }

        void TimelineSlider::paintEvent(QPaintEvent*)
        {
            TLR_PRIVATE_P();
            QPainter painter(this);
            auto rect = this->rect();
            auto rect2 = rect.adjusted(0, handleSize, 0, -handleSize);
            painter.fillRect(rect2, QColor(0, 0, 0));
            if (p.timelinePlayer)
            {
                int x0 = 0;
                int y0 = 0;
                int x1 = 0;
                int y1 = 0;
                int h = 0;

                // Draw the current time.
                x0 = _timeToPos(p.timelinePlayer->currentTime());
                y0 = 0;
                painter.fillRect(QRect(x0 - handleSize / 2, y0, handleSize, rect.height()), QColor(0, 0, 0));

                // Draw thumbnails.
                y0 = rect2.y();
                for (const auto& i : p.thumbnails)
                {
                    painter.drawImage(QPoint(_timeToPos(i.first), y0), i.second);
                }

                // Draw in/out points.
                const auto& inOutRange = p.timelinePlayer->inOutRange();
                x0 = _timeToPos(inOutRange.start_time());
                x1 = _timeToPos(inOutRange.end_time_inclusive());
                y1 = y0 + rect2.height();
                h = stripeSize;
                painter.fillRect(QRect(x0, y1 - h, x1 - x0, h), QColor(90, 90, 90));

                // Draw cached frames.
                auto color = QColor(40, 190, 40);
                const auto& cachedFrames = p.timelinePlayer->cachedFrames();
                for (const auto& i : cachedFrames)
                {
                    x0 = _timeToPos(i.start_time());
                    x1 = _timeToPos(i.end_time_inclusive());
                    painter.fillRect(QRect(x0, y1 - h, x1 - x0, h), color);
                }
            }
        }

        void TimelineSlider::mousePressEvent(QMouseEvent* event)
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                const auto& duration = p.timelinePlayer->duration();
                p.timelinePlayer->seek(_posToTime(event->x()));
            }
        }

        void TimelineSlider::mouseReleaseEvent(QMouseEvent*)
        {}

        void TimelineSlider::mouseMoveEvent(QMouseEvent* event)
        {
            TLR_PRIVATE_P();
            if (p.timelinePlayer)
            {
                const auto& duration = p.timelinePlayer->duration();
                p.timelinePlayer->seek(_posToTime(event->x()));
            }
        }

        void TimelineSlider::_currentTimeCallback(const otime::RationalTime&)
        {
            update();
        }

        void TimelineSlider::_inOutRangeCallback(const otime::TimeRange&)
        {
            update();
        }

        void TimelineSlider::_cachedFramesCallback(const std::vector<otime::TimeRange>&)
        {
            update();
        }

        void TimelineSlider::_thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >& thumbnails)
        {
            TLR_PRIVATE_P();
            for (const auto& i : thumbnails)
            {
                p.thumbnails[i.first] = i.second;
            }
            update();
        }

        otime::RationalTime TimelineSlider::_posToTime(int value) const
        {
            TLR_PRIVATE_P();
            otime::RationalTime out = time::invalidTime;
            if (p.timelinePlayer)
            {
                const auto& globalStartTime = p.timelinePlayer->globalStartTime();
                const auto& duration = p.timelinePlayer->duration();
                out = otime::RationalTime(
                    floor(math::clamp(value, 0, width()) / static_cast<double>(width()) * (duration.value() - 1) + globalStartTime.value()),
                    duration.rate());
            }
            return out;
        }

        int TimelineSlider::_timeToPos(const otime::RationalTime& value) const
        {
            TLR_PRIVATE_P();
            int out = 0;
            if (p.timelinePlayer)
            {
                const auto& globalStartTime = p.timelinePlayer->globalStartTime();
                const auto& duration = p.timelinePlayer->duration();
                out = (value.value() - globalStartTime.value()) / (duration.value() - 1) * width();
            }
            return out;
        }

        void TimelineSlider::_thumbnailsUpdate()
        {
            TLR_PRIVATE_P();
            p.thumbnails.clear();
            if (p.timelinePlayer && p.thumbnailProvider)
            {
                p.thumbnailProvider->cancelRequests();

                const auto& duration = p.timelinePlayer->duration();
                const auto& videoInfo = p.timelinePlayer->videoInfo();
                const auto rect = this->rect().adjusted(0, 0, 0, -(stripeSize + handleSize * 2));
                const int width = rect.width();
                const int height = rect.height();
                const int thumbnailWidth = !videoInfo.empty() ?
                    static_cast<int>(height * videoInfo[0].size.getAspect()) :
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
                    p.thumbnailProvider->request(requests, QSize(thumbnailWidth, thumbnailHeight));
                }
            }
            update();
        }
    }
}
