// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

namespace tl
{
    namespace timeline
    {
        //! Software renderer.
        class SoftwareRender : public IRender
        {
            TLRENDER_NON_COPYABLE(SoftwareRender);

        protected:
            void _init(const std::shared_ptr<core::Context>&);
            SoftwareRender();

        public:
            ~SoftwareRender() override;

            //! Create a new renderer.
            static std::shared_ptr<SoftwareRender> create(const std::shared_ptr<core::Context>&);

            //! Get the frame buffer.
            const std::shared_ptr<core::imaging::Image> getFrameBuffer() const;

            //! Copy the frame buffer.
            std::shared_ptr<core::imaging::Image> copyFrameBuffer(core::imaging::PixelType) const;

            void setTextureCacheSize(size_t) override;
            void setColorConfig(const core::imaging::ColorConfig&) override;
            void begin(const core::imaging::Size&) override;
            void end() override;
            void drawRect(
                const core::math::BBox2i&,
                const core::imaging::Color4f&) override;
            void drawImage(
                const std::shared_ptr<core::imaging::Image>&,
                const core::math::BBox2i&,
                const core::imaging::Color4f & = core::imaging::Color4f(1.F, 1.F, 1.F),
                const ImageOptions & = ImageOptions()) override;
            void drawVideo(
                const std::vector<timeline::VideoData>&,
                const std::vector<ImageOptions> & = {},
                const CompareOptions & = CompareOptions()) override;
            void drawText(
                const std::vector<std::shared_ptr<core::imaging::Glyph> >& glyphs,
                const core::math::Vector2i& position,
                const core::imaging::Color4f&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
