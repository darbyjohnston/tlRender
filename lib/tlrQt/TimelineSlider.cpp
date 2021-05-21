// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQt/TimelineSlider.h>

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
            setBackgroundRole(QPalette::ColorRole::Base);
            setAutoFillBackground(true);
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
            setMinimumHeight(20);
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

        void TimelineSlider::setTimelinePlayer(TimelinePlayer* timelinePlayer)
        {
            if (timelinePlayer == _timelinePlayer)
                return;
            if (_timelinePlayer)
            {
                _clipRanges.clear();
                disconnect(
                    _timelinePlayer,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
            }
            _timelinePlayer = timelinePlayer;
            if (_timelinePlayer)
            {
                _clipRanges = _timelinePlayer->clipRanges();
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
            }
            update();
        }

        void TimelineSlider::setUnits(TimeObject::Units units)
        {
            if (_units == units)
                return;
            _units = units;
            update();
        }

        void TimelineSlider::resizeEvent(QResizeEvent*)
        {}

        namespace
        {
            const int border = 1;
        }

        void TimelineSlider::paintEvent(QPaintEvent*)
        {
            QPainter painter(this);
            const auto& palette = this->palette();
            painter.setPen(palette.color(QPalette::ColorRole::Mid));
            painter.setBrush(QBrush());
            auto rect = this->rect().adjusted(0, 0, -1, -1);
            painter.drawRect(rect);
            rect = rect.adjusted(border, border, -border, -border);
            if (_timelinePlayer)
            {
                int x0 = 0;
                int x1 = 0;
                int y0 = 0;
                int y1 = 0;
                int h = 0;

                // Draw clips.
                y0 = rect.y();
                h = rect.height();
                for (size_t i = 0; i < _clipRanges.size(); ++i)
                {
                    if (0 == i % 2)
                    {
                        auto color = palette.color(QPalette::ColorRole::AlternateBase);
                        painter.setPen(color);
                        painter.setBrush(color);
                        x0 = _timeToPos(_clipRanges[i].start_time().value());
                        x1 = _timeToPos(_clipRanges[i].end_time_inclusive().value());
                        painter.drawRect(QRect(x0, y0, x1 - x0, h));
                    }
                }

                // Draw in/out points.
                auto color = palette.color(QPalette::ColorRole::WindowText);
                painter.setPen(color);
                painter.setBrush(color);
                const auto& inOutRange = _timelinePlayer->inOutRange();
                x0 = _timeToPos(inOutRange.start_time().value());
                x1 = _timeToPos(inOutRange.end_time_inclusive().value());
                y1 = y0 + rect.height();
                h = 1;
                painter.drawRect(QRect(x0, y1 - h, x1 - x0, h));

                // Draw cached frames.
                color = QColor(40, 190, 40);
                painter.setPen(color);
                painter.setBrush(color);
                const auto& cachedFrames = _timelinePlayer->cachedFrames();
                for (const auto& i : cachedFrames)
                {
                    x0 = _timeToPos(i.start_time().value());
                    x1 = _timeToPos(i.end_time_inclusive().value());
                    painter.drawRect(QRect(x0, y1 - h, x1 - x0, h));
                }

                // Draw the current time.
                color = palette.color(QPalette::ColorRole::WindowText);
                painter.setPen(QPen(color, 1));
                painter.setBrush(color);
                x0 = _timeToPos(_timelinePlayer->currentTime().value());
                painter.drawLine(QLine(QPoint(x0, y0), QPoint(x0, y1)));
            }
        }

        void TimelineSlider::mousePressEvent(QMouseEvent* event)
        {
            if (_timelinePlayer)
            {
                const auto& duration = _timelinePlayer->duration();
                _timelinePlayer->seek(otime::RationalTime(_posToTime(event->x()), duration.rate()));
            }
        }

        void TimelineSlider::mouseReleaseEvent(QMouseEvent*)
        {}

        void TimelineSlider::mouseMoveEvent(QMouseEvent* event)
        {
            if (_timelinePlayer)
            {
                const auto& duration = _timelinePlayer->duration();
                _timelinePlayer->seek(otime::RationalTime(_posToTime(event->x()), duration.rate()));
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

        int64_t TimelineSlider::_posToTime(int value) const
        {
            int64_t out = 0;
            if (_timelinePlayer)
            {
                const auto& globalStartTime = _timelinePlayer->globalStartTime();
                const auto& duration = _timelinePlayer->duration();
                out = (value - border) / static_cast<double>(width() - border * 2 - 1) * (duration.value() - 1) + globalStartTime.value();
            }
            return out;
        }

        int TimelineSlider::_timeToPos(int64_t value) const
        {
            int out = 0;
            if (_timelinePlayer)
            {
                const auto& globalStartTime = _timelinePlayer->globalStartTime();
                const auto& duration = _timelinePlayer->duration();
                out = border + (value - globalStartTime.value()) / (duration.value() - 1) * (width() - border * 2 - 1);
            }
            return out;
        }
    }
}
