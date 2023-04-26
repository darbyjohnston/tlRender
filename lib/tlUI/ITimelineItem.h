// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/TimelineIOManager.h>

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Timeline time units.
        enum class TimelineTimeUnits
        {
            Seconds,
            Frames,
            Timecode,

            Count,
            First = Seconds
        };
        TLRENDER_ENUM(TimelineTimeUnits);
        TLRENDER_ENUM_SERIALIZE(TimelineTimeUnits);

        //! Timeline item data.
        struct TimelineItemData
        {
            std::string directory;
            file::PathOptions pathOptions;
            std::shared_ptr<TimelineIOManager> ioManager;
        };

        //! Timeline item options.
        struct TimelineItemOptions
        {
            TimelineTimeUnits timeUnits = TimelineTimeUnits::Seconds;
            float scale = 500.F;
            bool thumbnails = true;
            int thumbnailHeight = 100;
            int waveformHeight = 50;

            bool operator == (const TimelineItemOptions&) const;
            bool operator != (const TimelineItemOptions&) const;
        };

        //! Base class for timeline items.
        class ITimelineItem : public ui::IWidget
        {
        protected:
            void _init(
                const std::string& name,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            ITimelineItem();

        public:
            ~ITimelineItem() override;

            virtual void setOptions(const TimelineItemOptions&);

            virtual void setViewport(const math::BBox2i&);

        protected:
            math::BBox2i _getTransformedViewport() const;
            bool _isInsideViewport() const;

            static std::string _durationLabel(
                const otime::RationalTime&,
                TimelineTimeUnits);
            static std::string _timeLabel(
                const otime::RationalTime&,
                TimelineTimeUnits);

            TimelineItemData _data;
            TimelineItemOptions _options;
            math::BBox2i _viewport;
        };
    }
}
