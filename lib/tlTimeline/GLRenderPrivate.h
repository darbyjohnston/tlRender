// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/GLRender.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>
#include <tlGL/Texture.h>
#include <tlGL/TextureAtlas.h>

#if defined(TLRENDER_OCIO)
#include <OpenColorIO/OpenColorIO.h>
#endif // TLRENDER_OCIO

#include <list>

#if defined(TLRENDER_OCIO)
namespace OCIO = OCIO_NAMESPACE;
#endif // TLRENDER_OCIO

namespace tl
{
    namespace timeline
    {
        std::string vertexSource();
        std::string meshFragmentSource();
        std::string colorMeshVertexSource();
        std::string colorMeshFragmentSource();
        std::string textFragmentSource();
        std::string textureFragmentSource();
        std::string imageFragmentSource();
        std::string displayFragmentSource(
            const std::string& colorConfigDef,
            const std::string& colorConfig,
            const std::string& lutDef,
            const std::string& lut,
            LUTOrder);
        std::string differenceFragmentSource();

        void copyTextures(
            const std::shared_ptr<imaging::Image>&,
            const std::vector<std::shared_ptr<gl::Texture> >&,
            size_t offset = 0);

        class TextureCache
        {
        public:
            void setSize(size_t);

            std::vector<std::shared_ptr<gl::Texture> > get(
                const imaging::Info&,
                const ImageFilters&,
                size_t offset = 0);

            void add(
                const imaging::Info&,
                const ImageFilters&,
                const std::vector<std::shared_ptr<gl::Texture> >&);

        private:
            void _cacheUpdate();

            size_t _size = 6;

            struct TextureData
            {
                imaging::Info info;
                ImageFilters imageFilters;
                std::vector<std::shared_ptr<gl::Texture> > texture;
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

        struct GLRender::Private
        {
            imaging::Size renderSize;
            ColorConfigOptions colorConfigOptions;
            LUTOptions lutOptions;
            RenderOptions renderOptions;

#if defined(TLRENDER_OCIO)
            std::unique_ptr<OCIOColorConfigData> colorConfigData;
            std::unique_ptr<OCIOLUTData> lutData;
#endif // TLRENDER_OCIO

            math::BBox2i viewport;
            math::Matrix4x4f transform;
            bool clipRectEnabled = false;
            math::BBox2i clipRect;

            std::map<std::string, std::shared_ptr<gl::Shader> > shaders;
            std::map<std::string, std::shared_ptr<gl::OffscreenBuffer> > buffers;
            TextureCache textureCache;
            std::shared_ptr<gl::TextureAtlas> glyphTextureAtlas;
            std::map<imaging::GlyphInfo, gl::TextureAtlasID> glyphIDs;
            std::map<std::string, std::shared_ptr<gl::VBO> > vbos;
            std::map<std::string, std::shared_ptr<gl::VAO> > vaos;

            std::chrono::steady_clock::time_point timer;
            struct Stats
            {
                int time = 0;
                size_t rects = 0;
                size_t meshes = 0;
                size_t meshTriangles = 0;
                size_t text = 0;
                size_t textTriangles = 0;
                size_t textures = 0;
                size_t images = 0;
            };
            Stats currentStats;
            std::list<Stats> stats;
            std::chrono::steady_clock::time_point logTimer;

            void drawTextMesh(const geom::TriangleMesh2&);
        };
    }
}
