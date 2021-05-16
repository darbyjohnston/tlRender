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

        void TimelineSlider::setTimeline(TimelineObject* timeline)
        {
            if (timeline == _timeline)
                return;
            if (_timeline)
            {
                disconnect(
                    _timeline,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    this,
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
            }
            _timeline = timeline;
            if (_timeline)
            {
                connect(
                    _timeline,
                    SIGNAL(currentTimeChanged(const otime::RationalTime&)),
                    SLOT(_currentTimeCallback(const otime::RationalTime&)));
                connect(
                    _timeline,
                    SIGNAL(inOutRangeChanged(const otime::TimeRange&)),
                    SLOT(_inOutRangeCallback(const otime::TimeRange&)));
                connect(
                    _timeline,
                    SIGNAL(cachedFramesChanged(const std::vector<otime::TimeRange>&)),
                    SLOT(_cachedFramesCallback(const std::vector<otime::TimeRange>&)));
            }
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

        void TimelineSlider::paintEvent(QPaintEvent*)
        {
            QPainter painter(this);
            const auto& palette = this->palette();
            painter.setPen(palette.color(QPalette::ColorRole::Mid));
            painter.setBrush(QBrush());
            const auto& rect = this->rect().adjusted(0, 0, -1, -1);
            painter.drawRect(rect);
            if (_timeline)
            {
                // Draw in/out points.
                auto color = palette.color(QPalette::ColorRole::WindowText);
                painter.setPen(color);
                painter.setBrush(color);
                const auto& inOutRange = _timeline->inOutRange();
                int x0 = _timeToPos(inOutRange.start_time().value());
                int x1 = _timeToPos(inOutRange.end_time_inclusive().value());
                int y0 = rect.y();
                int y1 = y0 + rect.height() - 1;
                int h = 1;
                painter.drawRect(QRect(x0, y1 - h, x1 - x0, h));

                // Draw cached frames.
                color = QColor(40, 190, 40);
                painter.setPen(color);
                painter.setBrush(color);
                const auto& cachedFrames = _timeline->cachedFrames();
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
                x0 = _timeToPos(_timeline->currentTime().value());
                painter.drawLine(QLine(QPoint(x0, y0), QPoint(x0, y1)));
            }
        }

        void TimelineSlider::mousePressEvent(QMouseEvent* event)
        {
            if (_timeline)
            {
                const auto& duration = _timeline->duration();
                _timeline->seek(otime::RationalTime(_posToTime(event->x()), duration.rate()));
            }
        }

        void TimelineSlider::mouseReleaseEvent(QMouseEvent*)
        {}

        void TimelineSlider::mouseMoveEvent(QMouseEvent* event)
        {
            if (_timeline)
            {
                const auto& duration = _timeline->duration();
                _timeline->seek(otime::RationalTime(_posToTime(event->x()), duration.rate()));
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
            if (_timeline)
            {
                const auto& globalStartTime = _timeline->globalStartTime();
                const auto& duration = _timeline->duration();
                out = value / static_cast<double>(width() - 1) * (duration.value() - 1) + globalStartTime.value();
            }
            return out;
        }

        int TimelineSlider::_timeToPos(int64_t value) const
        {
            int out = 0;
            if (_timeline)
            {
                const auto& globalStartTime = _timeline->globalStartTime();
                const auto& duration = _timeline->duration();
                out = (value - globalStartTime.value()) / (duration.value() - 1) * (width() - 1);
            }
            return out;
        }
    }
}
