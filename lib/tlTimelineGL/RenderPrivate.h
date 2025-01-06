// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineGL/Render.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>
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
    namespace timeline_gl
    {
        std::string vertexSource();
        std::string meshFragmentSource();
        std::string colorMeshVertexSource();
        std::string colorMeshFragmentSource();
        std::string textFragmentSource();
        std::string textureFragmentSource();
        std::string imageFragmentSource();
        std::string displayFragmentSource(
            const std::string& ocioDef,
            const std::string& ocio,
            const std::string& lutDef,
            const std::string& lut,
            timeline::LUTOrder);
        std::string differenceFragmentSource();

        std::vector<std::shared_ptr<gl::Texture> > getTextures(
            const image::Info&,
            const timeline::ImageFilters&,
            size_t offset = 0);

        void copyTextures(
            const std::shared_ptr<image::Image>&,
            const std::vector<std::shared_ptr<gl::Texture> >&,
            size_t offset = 0);

        void setActiveTextures(
            const image::Info& info,
            const std::vector<std::shared_ptr<gl::Texture> >&,
            size_t offset = 0);

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

        struct OCIOData
        {
            ~OCIOData();

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
            math::Size2i renderSize;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            timeline::RenderOptions renderOptions;

#if defined(TLRENDER_OCIO)
            //! \todo Add a cache for OpenColorIO data.
            std::unique_ptr<OCIOData> ocioData;
            std::unique_ptr<OCIOLUTData> lutData;
#endif // TLRENDER_OCIO

            math::Box2i viewport;
            math::Matrix4x4f transform;
            bool clipRectEnabled = false;
            math::Box2i clipRect;

            std::map<std::string, std::shared_ptr<gl::Shader> > shaders;
            std::map<std::string, std::shared_ptr<gl::OffscreenBuffer> > buffers;
            std::shared_ptr<TextureCache> textureCache;
            std::shared_ptr<gl::TextureAtlas> glyphTextureAtlas;
            std::map<image::GlyphInfo, gl::TextureAtlasID> glyphIDs;
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
