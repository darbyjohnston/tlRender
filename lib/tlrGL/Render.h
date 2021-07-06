// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrGL/FontSystem.h>

#include <tlrCore/BBox.h>
#include <tlrCore/Cache.h>
#include <tlrCore/Color.h>
#include <tlrCore/Timeline.h>

#include <OpenColorIO/OpenColorIO.h>

#include <glad.h>

namespace tlr
{
    namespace imaging
    {
        class Image;
        class Size;
    }

    namespace OCIO = OCIO_NAMESPACE;

    namespace gl
    {
        class Shader;
        class Texture;

        //! OpenColorIO configuration.
        struct ColorConfig
        {
            std::string config;
            std::string input;
            std::string display;
            std::string view;

            bool operator == (const ColorConfig&) const;
            bool operator != (const ColorConfig&) const;
        };

        //! OpenGL renderer.
        class Render : public std::enable_shared_from_this<Render>
        {
            TLR_NON_COPYABLE(Render);

        protected:
            void _init();
            Render();

        public:
            ~Render();

            //! Create a new renderer.
            static std::shared_ptr<Render> create();

            //! Set the color configuration.
            void setColorConfig(const ColorConfig&);

            //! Start a render.
            void begin(const imaging::Size&, bool flipY = false);

            //! Finish a render.
            void end();

            //! Draw a rectangle.
            void drawRect(
                const math::BBox2f&,
                const imaging::Color4f&);

            //! Draw an image.
            void drawImage(
                const std::shared_ptr<imaging::Image>&,
                const math::BBox2f&,
                const imaging::Color4f& = imaging::Color4f(1.F, 1.F, 1.F));

            //! Draw a timeline frame.
            void drawFrame(const timeline::Frame&);

            //! Draw text.
            void drawText(
                const std::vector<std::shared_ptr<Glyph> >& glyphs,
                const math::Vector2f& position,
                const imaging::Color4f&);

        private:
            ColorConfig _colorConfig;
            OCIO::ConstConfigRcPtr _ocioConfig;
            OCIO::ConstProcessorRcPtr _ocioProcessor;
            OCIO::ConstGPUProcessorRcPtr _ocioGpuProcessor;
            OCIO::GpuShaderDescRcPtr _ocioShaderDesc;
            struct TextureId
            {
                TextureId(
                    unsigned    id,
                    std::string name,
                    std::string sampler,
                    unsigned    type);

                unsigned    id = -1;
                std::string name;
                std::string sampler;
                unsigned    type = -1;
            };
            std::vector<TextureId> _colorTextures;

            imaging::Size _size;

            std::shared_ptr<Shader> _shader;

            memory::Cache<GlyphInfo, std::shared_ptr<Texture> > _glyphTextureCache;
        };
    }
}