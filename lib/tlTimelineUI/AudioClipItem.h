// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/IBasicItem.h>

#include <opentimelineio/clip.h>

namespace tl
{
    namespace timelineui
    {
        class ThumbnailGenerator;

        //! Audio clip item.
        class AudioClipItem : public IBasicItem
        {
        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ThumbnailGenerator>,
                const std::shared_ptr<IWidget>& parent);

            AudioClipItem();

        public:
            virtual ~AudioClipItem();

            //! Create a new item.
            static std::shared_ptr<AudioClipItem> create(
                const std::shared_ptr<dtk::Context>&,
                const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip>&,
                double scale,
                const ItemOptions&,
                const DisplayOptions&,
                const std::shared_ptr<ItemData>&,
                const std::shared_ptr<ThumbnailGenerator>,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setScale(double) override;
            void setDisplayOptions(const DisplayOptions&) override;

            void tickEvent(
                bool,
                bool,
                const dtk::TickEvent&) override;
            void sizeHintEvent(const dtk::SizeHintEvent&) override;
            void clipEvent(const dtk::Box2I&, bool) override;
            void drawEvent(const dtk::Box2I&, const dtk::DrawEvent&) override;

        private:
            void _drawWaveforms(
                const dtk::Box2I&,
                const dtk::DrawEvent&);

            void _cancelRequests();

            DTK_PRIVATE();
        };
    }
}
