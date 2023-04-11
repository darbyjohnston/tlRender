// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/ITimelineItem.h>

#include <opentimelineio/clip.h>

namespace tl
{
    namespace ui
    {
        //! Timeline audio clip item.
        class TimelineAudioClipItem : public ITimelineItem
        {
        protected:
            void _init(
                const otio::Clip*,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            TimelineAudioClipItem();

        public:
            ~TimelineAudioClipItem() override;

            static std::shared_ptr<TimelineAudioClipItem> create(
                const otio::Clip*,
                const TimelineItemData&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setOptions(const TimelineItemOptions&) override;
            void setViewport(const math::BBox2i&) override;

            void tickEvent(const ui::TickEvent&) override;
            void sizeEvent(const ui::SizeEvent&) override;
            void drawEvent(const ui::DrawEvent&) override;

        private:
            void _textUpdate();

            void _drawInfo(const ui::DrawEvent&);
            void _drawWaveforms(const ui::DrawEvent&);

            TLRENDER_PRIVATE();
        };
    }
}
