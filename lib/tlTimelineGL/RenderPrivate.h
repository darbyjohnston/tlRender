// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineGL/Render.h>

#include <feather-tk/gl/Mesh.h>
#include <feather-tk/gl/OffscreenBuffer.h>
#include <feather-tk/gl/Render.h>
#include <feather-tk/gl/Shader.h>
#include <feather-tk/gl/TextureAtlas.h>

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
        std::string textureFragmentSource();
        std::string displayFragmentSource(
            const std::string& ocioDef,
            const std::string& ocio,
            const std::string& lutDef,
            const std::string& lut,
            timeline::LUTOrder);
        std::string dissolveFragmentSource();
        std::string differenceFragmentSource();

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
            std::shared_ptr<ftk::gl::Render> baseRender;

            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;

#if defined(TLRENDER_OCIO)
            //! \todo Add a cache for OpenColorIO data.
            std::unique_ptr<OCIOData> ocioData;
            std::unique_ptr<OCIOLUTData> lutData;
#endif // TLRENDER_OCIO

            std::map<std::string, std::shared_ptr<ftk::gl::Shader> > shaders;
            std::map<std::string, std::shared_ptr<ftk::gl::OffscreenBuffer> > buffers;
            std::map<std::string, std::shared_ptr<ftk::gl::VBO> > vbos;
            std::map<std::string, std::shared_ptr<ftk::gl::VAO> > vaos;
        };
    }
}
