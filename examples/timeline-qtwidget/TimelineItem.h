// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "IItem.h"

#include <tlTimeline/TimelinePlayer.h>

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
                    const std::shared_ptr<timeline::TimelinePlayer>&,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

            public:
                static std::shared_ptr<TimelineItem> create(
                    const std::shared_ptr<timeline::TimelinePlayer>&,
                    const ItemData&,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                ~TimelineItem() override;

                void setGeometry(const math::BBox2i&) override;
                void sizeEvent(const ui::SizeEvent&) override;
                void drawEvent(const ui::DrawEvent&) override;
                void enterEvent() override;
                void leaveEvent() override;
                void mouseMoveEvent(ui::MouseMoveEvent&) override;
                void mousePressEvent(ui::MouseClickEvent&) override;
                void mouseReleaseEvent(ui::MouseClickEvent&) override;

            private:
                void _drawTimeTicks(const ui::DrawEvent&);
                void _drawCurrentTime(const ui::DrawEvent&);

                math::BBox2i _getCurrentTimeBBox() const;

                otime::RationalTime _posToTime(float) const;
                float _timeToPos(const otime::RationalTime&) const;

                std::shared_ptr<timeline::TimelinePlayer> _timelinePlayer;
                otime::TimeRange _timeRange = time::invalidTimeRange;
                otime::RationalTime _currentTime = time::invalidTime;
                otime::TimeRange _inOutRange = time::invalidTimeRange;
                timeline::PlayerCacheInfo _cacheInfo;
                ui::FontRole _fontRole = ui::FontRole::Label;
                int _margin = 0;
                int _spacing = 0;
                imaging::FontMetrics _fontMetrics;
                bool _mousePress = false;
                math::Vector2i _mousePos;
                math::Vector2i _mousePressPos;
                bool _currentTimeDrag = false;
                std::shared_ptr<observer::ValueObserver<otime::RationalTime> > _currentTimeObserver;
                std::shared_ptr<observer::ValueObserver<otime::TimeRange> > _inOutRangeObserver;
                std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> > _cacheInfoObserver;
            };
        }
    }
}
