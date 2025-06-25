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
                const std::shared_ptr<feather_tk::Context>&,
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
                const std::shared_ptr<feather_tk::Context>&,
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
                const feather_tk::TickEvent&) override;
            void sizeHintEvent(const feather_tk::SizeHintEvent&) override;
            void clipEvent(const feather_tk::Box2I&, bool) override;
            void drawEvent(const feather_tk::Box2I&, const feather_tk::DrawEvent&) override;

        private:
            void _drawWaveforms(
                const feather_tk::Box2I&,
                const feather_tk::DrawEvent&);

            void _cancelRequests();

            FEATHER_TK_PRIVATE();
        };
    }
}
