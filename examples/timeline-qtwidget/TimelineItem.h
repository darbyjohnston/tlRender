// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "IItem.h"

#include <tlCore/ValueObserver.h>

#include <opentimelineio/timeline.h>

namespace tl
{
    namespace examples
    {
        namespace timeline_qtwidget
        {
            //! Timeline item.
            class TimelineItem : public IItem
            {
            protected:
                void _init(
                    const otio::SerializableObject::Retainer<otio::Timeline>&,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

            public:
                static std::shared_ptr<TimelineItem> create(
                    const otio::SerializableObject::Retainer<otio::Timeline>&,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ~TimelineItem() override;

                void setCurrentTime(const otime::RationalTime&);

                std::shared_ptr<observer::IValue<otime::RationalTime> > observeCurrentTime() const;

                void setGeometry(const math::BBox2i&) override;
                void sizeEvent(const ui::SizeEvent&) override;
                void drawEvent(const ui::DrawEvent&) override;
                void enterEvent() override;
                void leaveEvent() override;
                void mouseMoveEvent(const ui::MouseMoveEvent&) override;
                void mousePressEvent(const ui::MouseClickEvent&) override;
                void mouseReleaseEvent(const ui::MouseClickEvent&) override;

            private:
                void _drawTimeTicks(const ui::DrawEvent&);
                void _drawCurrentTime(const ui::DrawEvent&);

                math::BBox2i _getCurrentTimeBBox() const;

                otime::RationalTime _posToTime(float) const;
                float _timeToPos(const otime::RationalTime&) const;

                otio::SerializableObject::Retainer<otio::Timeline> _timeline;
                otime::TimeRange _timeRange = time::invalidTimeRange;
                std::shared_ptr<observer::Value<otime::RationalTime> > _currentTime;
                imaging::FontInfo _fontInfo;
                int _margin = 0;
                int _spacing = 0;
                imaging::FontMetrics _fontMetrics;
                bool _mousePress = false;
                math::Vector2i _mousePos;
                math::Vector2i _mousePressPos;
                bool _currentTimeDrag = false;
            };
        }
    }
}
