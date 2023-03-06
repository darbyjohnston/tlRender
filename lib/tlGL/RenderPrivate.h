// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlGL/Render.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>
#include <tlGL/Texture.h>

#include <tlCore/LRUCache.h>

#if defined(TLRENDER_OCIO)
#include <OpenColorIO/OpenColorIO.h>
#endif // TLRENDER_OCIO

#include <list>

#if defined(TLRENDER_OCIO)
namespace OCIO = OCIO_NAMESPACE;
#endif // TLRENDER_OCIO

namespace tl
{
    namespace gl
    {
        std::string vertexSource();
        std::string meshFragmentSource();
        std::string textFragmentSource();
        std::string textureFragmentSource();
        std::string imageFragmentSource();
        std::string displayFragmentSource(
            const std::string& colorConfigDef,
            const std::string& colorConfig,
            const std::string& lutDef,
            const std::string& lut,
            timeline::LUTOrder);
        std::string differenceFragmentSource();

        struct Pos2_F32
        {
            float vx;
            float vy;
        };

        struct Pos2_F32_UV_U16
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

            std::vector<std::shared_ptr<Texture> > get(
                const imaging::Info&,
                const timeline::ImageFilters&,
                size_t offset = 0);

            void add(
                const imaging::Info&,
                const timeline::ImageFilters&,
                const std::vector<std::shared_ptr<Texture> >&);

        private:
            void _cacheUpdate();

            size_t _size = 6;

            struct TextureData
            {
                imaging::Info info;
                timeline::ImageFilters imageFilters;
                std::vector<std::shared_ptr<Texture> > texture;
            };

            std::list<TextureData> _cache;
        };

#if defined(TLRENDER_OCIO)
        struct OCIOTexture
        {
            OCIOTexture(
                unsigned    id,
                std::string name,
                std::string sampler,
                unsigned    type);

            unsigned    id = -1;
            std::string name;
            std::string sampler;
            unsigned    type = -1;
        };

        struct OCIOColorConfigData
        {
            ~OCIOColorConfigData();

            OCIO::ConstConfigRcPtr config;
            OCIO::DisplayViewTransformRcPtr transform;
            OCIO::LegacyViewingPipelineRcPtr lvp;
            OCIO::ConstProcessorRcPtr processor;
            OCIO::ConstGPUProcessorRcPtr gpuProcessor;
            OCIO::GpuShaderDescRcPtr shaderDesc;
            std::vector<OCIOTexture> textures;
        };

        struct OCIOLUTData
        {
            ~OCIOLUTData();

            OCIO::ConstConfigRcPtr config;
            OCIO::FileTransformRcPtr transform;
            OCIO::ConstProcessorRcPtr processor;
            OCIO::ConstGPUProcessorRcPtr gpuProcessor;
            OCIO::GpuShaderDescRcPtr shaderDesc;
            std::vector<OCIOTexture> textures;
        };
#endif // TLRENDER_OCIO

        struct Render::Private
        {
            imaging::Size size;
            timeline::ColorConfigOptions colorConfigOptions;
            timeline::LUTOptions lutOptions;
            timeline::RenderOptions renderOptions;

#if defined(TLRENDER_OCIO)
            std::unique_ptr<OCIOColorConfigData> colorConfigData;
            std::unique_ptr<OCIOLUTData> lutData;
#endif // TLRENDER_OCIO

            math::BBox2i viewport;
            math::Matrix4x4f transform;
            bool clipRectEnabled = false;
            math::BBox2i clipRect;

            std::map<std::string, std::shared_ptr<Shader> > shaders;
            std::map<std::string, std::shared_ptr<OffscreenBuffer> > buffers;
            TextureCache textureCache;
            memory::LRUCache<imaging::GlyphInfo, std::shared_ptr<Texture> > glyphTextureCache;
            std::map<std::string, std::shared_ptr<gl::VBO> > vbos;
            std::map<std::string, std::shared_ptr<gl::VAO> > vaos;
        };
    }
}
