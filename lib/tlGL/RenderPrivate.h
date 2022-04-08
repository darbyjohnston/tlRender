// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlGL/Render.h>

#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>
#include <tlGL/Texture.h>

#include <tlCore/FontSystem.h>
#include <tlCore/LRUCache.h>
#include <tlCore/OCIO.h>

#include <OpenColorIO/OpenColorIO.h>

namespace OCIO = OCIO_NAMESPACE;

namespace tl
{
    namespace gl
    {
        std::string colorFunctionName();
        std::string colorFunctionNoOp();
        std::string vertexSource();
        std::string rectFragmentSource();
        std::string textFragmentSource();
        std::string textureFragmentSource();
        std::string imageFragmentSource();
        std::string displayFragmentSource();
        std::string wipeFragmentSource();
        std::string differenceFragmentSource();

        struct VBOVertex
        {
            float    vx;
            float    vy;
            uint16_t tx;
            uint16_t ty;
        };
        
        void copyTextures(
            const std::shared_ptr<imaging::Image>&,
            const std::vector<std::shared_ptr<Texture> >&,
            size_t offset = 0);

        class TextureCache
        {
        public:
            void setSize(size_t);

            std::vector<std::shared_ptr<Texture> > get(const imaging::Info&, size_t offset = 0);

            void add(const imaging::Info&, const std::vector<std::shared_ptr<Texture> >&);

        private:
            void _cacheUpdate();

            size_t _size = 6;
            std::list<std::pair<imaging::Info, std::vector<std::shared_ptr<Texture> > > > _cache;
        };

        struct Render::Private
        {
            std::weak_ptr<system::Context> context;

            imaging::ColorConfig colorConfig;
            OCIO::ConstConfigRcPtr ocioConfig;
            OCIO::DisplayViewTransformRcPtr ocioTransform;
            OCIO::LegacyViewingPipelineRcPtr ocioVP;
            OCIO::ConstProcessorRcPtr ocioProcessor;
            OCIO::ConstGPUProcessorRcPtr ocioGpuProcessor;
            OCIO::GpuShaderDescRcPtr ocioShaderDesc;
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
            std::vector<TextureId> colorTextures;

            imaging::Size size;

            std::shared_ptr<Shader> rectShader;
            std::shared_ptr<Shader> textShader;
            std::shared_ptr<Shader> textureShader;
            std::shared_ptr<Shader> imageShader;
            std::shared_ptr<Shader> displayShader;
            std::shared_ptr<Shader> wipeShader;
            std::shared_ptr<Shader> differenceShader;

            std::shared_ptr<OffscreenBuffer> buffer;
            std::shared_ptr<OffscreenBuffer> transitionBuffer;
            std::shared_ptr<OffscreenBuffer> overlayBuffer;
            std::array<std::shared_ptr<OffscreenBuffer>, 2> differenceBuffers;

            TextureCache textureCache;

            memory::LRUCache<imaging::GlyphInfo, std::shared_ptr<Texture> > glyphTextureCache;
        };
    }
}
