// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/IRender.h>

namespace tlr
{
    namespace gl
    {
        //! OpenGL renderer.
        class Render : public render::IRender
        {
            TLR_NON_COPYABLE(Render);

        protected:
            void _init(const std::shared_ptr<core::Context>&);
            Render();

        public:
            ~Render() override;

            //! Create a new renderer.
            static std::shared_ptr<Render> create(const std::shared_ptr<core::Context>&);

            void setTextureCacheSize(size_t) override;
            void setColorConfig(const imaging::ColorConfig&) override;
            void begin(const imaging::Size&) override;
            void end() override;
            void drawRect(
                const math::BBox2f&,
                const imaging::Color4f&) override;
            void drawImage(
                const std::shared_ptr<imaging::Image>&,
                const math::BBox2f&,
                const imaging::Color4f& = imaging::Color4f(1.F, 1.F, 1.F),
                const render::ImageOptions& = render::ImageOptions()) override;
            void drawVideo(
                const timeline::VideoData&,
                const render::ImageOptions& = render::ImageOptions()) override;
            void drawText(
                const std::vector<std::shared_ptr<imaging::Glyph> >& glyphs,
                const glm::vec2& position,
                const imaging::Color4f&) override;

        private:
            void _setColorConfig(const imaging::ColorConfig&);
            void _delColorConfig();

            TLR_PRIVATE();
        };
    }
}
