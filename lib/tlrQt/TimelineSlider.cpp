// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineSlider.h>

#include <tlrCore/Math.h>

#include <QMouseEvent>
#include <QPainter>
#include <QStyle>

namespace tlr
{
    namespace qt
    {
        TimelineSlider::TimelineSlider(QWidget* parent) :
            QWidget(parent)
        {
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
            setMinimumHeight(50);
        }

        void TimelineSlider::setTimeObject(TimeObject* timeObject)
        {
            if (timeObject == _timeObject)
                return;
            if (_timeObject)
            {
                disconnect(
                    _timeObject,
                    SIGNAL(unitsChanged(qt::Time::Units)),
                    this,
                    SLOT(setUnits(qt::Time::Units)));
            }
            _timeObject = timeObject;
            if (_timeObject)
            {
                _units = _timeObject->units();
                connect(
                    _timeObject,
                    SIGNAL(unitsChanged(qt::TimeObject::Units)),
                    SLOT(setUnits(qt::TimeObject::Units)));
            }
            update();
        }

        void TimelineSlider::setColorConfig(const gl::ColorConfig& colorConfig)
        {
            _colorConfig = colorConfig;
            if (_thumbnailProvider)
            {
                _thumbnailProvider->setColorConfig(_colorConfig);
            }
        }

        void TimelineSlider::setTimelinePlayer(TimelinePlayer* timelinePlayer)
        {
            if (timelinePlayer == _timelinePlayer)
                return;
            if (_timelinePlayer)
            {
                _thumbnailProvider->setParent(nullptr);
                delete _thumbnailProvider;
                _thumbnailProvider = nullptr;
                disconnect(
                    _timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
            }
            _timelinePlayer = timelinePlayer;
            if (_timelinePlayer)
            {
                _thumbnailProvider = new TimelineThumbnailProvider(
                    timeline::Timeline::create(_timelinePlayer->fileName().toLatin1().data()),
                    this);
                _thumbnailProvider->setColorConfig(_colorConfig);
                connect(
                    _timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
                connect(
                    _timelinePlayer,
                    SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                    SLOT(_inOutRangeCallback(const otime::TimeRange&)));
                connect(
                    _timelinePlayer,
                    SIGNAL(cachedFramesChanged(const std::vector<otime::TimeRange>&)),
                    SLOT(_cachedFramesCallback(const std::vector<otime::TimeRange>&)));
                connect(
                    _thumbnailProvider,
                    SIGNAL(thumbails(const QList<QPair<otime::RationalTime, QImage> >&)),
                    SLOT(_thumbnailsCallback(const QList<QPair<otime::RationalTime, QImage> >&)));
            }
            _thumbnailsUpdate();
        }

        void TimelineSlider::setUnits(TimeObject::Units units)
        {
            if (_units == units)
                return;
            _units = units;
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
            QPainter painter(this);
            auto rect = this->rect();
            auto rect2 = rect.adjusted(0, handleSize, 0, -handleSize);
            painter.fillRect(rect2, QColor(0, 0, 0));
            if (_timelinePlayer)
            {
                int x0 = 0;
                int y0 = 0;
                int x1 = 0;
                int y1 = 0;
                int h = 0;

                // Draw the current time.
                x0 = _timeToPos(_timelinePlayer->currentTime());
                y0 = 0;
                painter.fillRect(QRect(x0 - handleSize / 2, y0, handleSize, rect.height()), QColor(0, 0, 0));

                // Draw thumbnails.
                y0 = rect2.y();
                for (const auto& i : _thumbnails)
                {
                    painter.drawImage(QPoint(_timeToPos(i.first), y0), i.second);
                }

                // Draw in/out points.
                const auto& inOutRange = _timelinePlayer->inOutRange();
                x0 = _timeToPos(inOutRange.start_time());
                x1 = _timeToPos(inOutRange.end_time_inclusive());
                y1 = y0 + rect2.height();
                h = stripeSize;
                painter.fillRect(QRect(x0, y1 - h, x1 - x0, h), QColor(90, 90, 90));

                // Draw cached frames.
                auto color = QColor(40, 190, 40);
                const auto& cachedFrames = _timelinePlayer->cachedFrames();
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
            if (_timelinePlayer)
            {
                const auto& duration = _timelinePlayer->duration();
                _timelinePlayer->seek(_posToTime(event->x()));
            }
        }

        void TimelineSlider::mouseReleaseEvent(QMouseEvent*)
        {}

        void TimelineSlider::mouseMoveEvent(QMouseEvent* event)
        {
            if (_timelinePlayer)
            {
                const auto& duration = _timelinePlayer->duration();
                _timelinePlayer->seek(_posToTime(event->x()));
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
            for (const auto& i : thumbnails)
            {
                _thumbnails[i.first] = i.second;
            }
            update();
        }

        otime::RationalTime TimelineSlider::_posToTime(int value) const
        {
            otime::RationalTime out = invalidTime;
            if (_timelinePlayer)
            {
                const auto& globalStartTime = _timelinePlayer->globalStartTime();
                const auto& duration = _timelinePlayer->duration();
                out = otime::RationalTime(
                    floor(math::clamp(value, 0, width()) / static_cast<double>(width()) * (duration.value() - 1) + globalStartTime.value()),
                    duration.rate());
            }
            return out;
        }

        int TimelineSlider::_timeToPos(const otime::RationalTime& value) const
        {
            int out = 0;
            if (_timelinePlayer)
            {
                const auto& globalStartTime = _timelinePlayer->globalStartTime();
                const auto& duration = _timelinePlayer->duration();
                out = (value.value() - globalStartTime.value()) / (duration.value() - 1) * width();
            }
            return out;
        }

        void TimelineSlider::_thumbnailsUpdate()
        {
            _thumbnails.clear();
            if (_timelinePlayer && _thumbnailProvider)
            {
                _thumbnailProvider->cancelRequests();

                const auto& duration = _timelinePlayer->duration();
                const auto& imageInfo = _timelinePlayer->imageInfo();
                const auto rect = this->rect().adjusted(0, 0, 0, -(stripeSize * 2 + handleSize * 2));
                const int width = rect.width();
                const int height = rect.height();
                const int thumbnailWidth = static_cast<int>(height * imageInfo.size.getAspect());
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
                    _thumbnailProvider->request(requests, QSize(thumbnailWidth, thumbnailHeight));
                }
            }
            update();
        }
    }
}
