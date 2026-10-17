// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/UI/OffscreenRender.h>

#include <tlRender/GL/Render.h>
#if defined(TLRENDER_GPU)
#include <tlRender/GPU/Render.h>
#endif // TLRENDER_GPU

#include <ftk/GL/GL.h>
#include <ftk/GL/Init.h>
#include <ftk/GL/OffscreenBuffer.h>
#include <ftk/GL/Util.h>
#if defined(TLRENDER_GPU)
#include <ftk/GPU/OffscreenBuffer.h>
#include <ftk/GPU/Render.h>
#include <ftk/GPU/System.h>
#endif // TLRENDER_GPU

#include <ftk/Core/Context.h>
#include <ftk/Core/FontSystem.h>

namespace tl
{
    namespace ui
    {
        struct OffscreenRender::Private
        {
            ftk::Size2I size;
            std::shared_ptr<IRender> render;
            std::shared_ptr<ftk::gl::OffscreenBuffer> buffer;
            std::unique_ptr<ftk::gl::OffscreenBufferBinding> binding;
#if defined(TLRENDER_GPU)
            std::shared_ptr<ftk::gpu::OffscreenBuffer> gpuBuffer;
#endif // TLRENDER_GPU
        };

        namespace
        {
            bool gpuEnabled()
            {
#if defined(TLRENDER_GPU)
                return ftk::gpu::isEnabled();
#else // TLRENDER_GPU
                return false;
#endif // TLRENDER_GPU
            }
        }

        void OffscreenRender::_init(
            const std::shared_ptr<ftk::Context>& context,
            const ftk::Size2I& size,
            ftk::ImageType colorBuffer)
        {
            FTK_P();
            p.size = size;
#if defined(TLRENDER_GPU)
            if (gpuEnabled())
            {
                auto gpuSystem = context->getSystem<ftk::gpu::System>();
                p.render = gpu::Render::create(
                    gpuSystem,
                    context->getLogSystem(),
                    context->getSystem<ftk::FontSystem>());
                ftk::gpu::BufferType type = ftk::gpu::BufferType::RGBA_F32;
                switch (colorBuffer)
                {
                case ftk::ImageType::RGBA_U8: type = ftk::gpu::BufferType::RGBA_U8; break;
                case ftk::ImageType::RGBA_F16: type = ftk::gpu::BufferType::RGBA_F16; break;
                default: break;
                }
                p.gpuBuffer = ftk::gpu::OffscreenBuffer::create(gpuSystem, size, type);
                return;
            }
#endif // TLRENDER_GPU
            p.render = gl::Render::create(
                context->getLogSystem(),
                context->getSystem<ftk::FontSystem>());
            // The wipe comparison masks with the stencil buffer, so the
            // buffer has one; paired with depth, as the viewport does, for
            // the combined format rather than a stencil-only attachment.
            ftk::gl::OffscreenBufferOptions options;
            if (!ftk::gl::isGLES())
            {
                options.depth = ftk::gl::OffscreenDepth::_24;
            }
            options.stencil = ftk::gl::OffscreenStencil::_8;
            p.buffer = ftk::gl::OffscreenBuffer::create(
                size,
                ftk::gl::getRenderableType(ftk::gl::getTextureType(colorBuffer)),
                options);
        }

        OffscreenRender::OffscreenRender() :
            _p(new Private)
        {}

        OffscreenRender::~OffscreenRender()
        {}

        std::shared_ptr<OffscreenRender> OffscreenRender::create(
            const std::shared_ptr<ftk::Context>& context,
            const ftk::Size2I& size,
            ftk::ImageType colorBuffer)
        {
            auto out = std::shared_ptr<OffscreenRender>(new OffscreenRender);
            out->_init(context, size, colorBuffer);
            return out;
        }

        bool OffscreenRender::canRead(ftk::ImageType type)
        {
#if defined(TLRENDER_GPU)
            if (gpuEnabled())
            {
                return ftk::gpu::OffscreenBuffer::canRead(type);
            }
#endif // TLRENDER_GPU
            return ftk::gl::OffscreenBuffer::canRead(type);
        }

        const ftk::Size2I& OffscreenRender::getSize() const
        {
            return _p->size;
        }

        const std::shared_ptr<IRender>& OffscreenRender::getRender() const
        {
            return _p->render;
        }

        void OffscreenRender::begin()
        {
            FTK_P();
#if defined(TLRENDER_GPU)
            if (p.gpuBuffer)
            {
                ftk::gpu::getRender(p.render)->setTarget(p.gpuBuffer);
            }
            else
#endif // TLRENDER_GPU
            {
                p.binding.reset(new ftk::gl::OffscreenBufferBinding(p.buffer));
            }
            p.render->begin(p.size);
        }

        void OffscreenRender::end()
        {
            _p->render->end();
        }

        std::shared_ptr<ftk::Image> OffscreenRender::read(const ftk::ImageInfo& info)
        {
            FTK_P();
            std::shared_ptr<ftk::Image> out;
#if defined(TLRENDER_GPU)
            if (p.gpuBuffer)
            {
                out = p.gpuBuffer->read(info);
            }
            else
#endif // TLRENDER_GPU
            {
                out = p.buffer->read(info);
                // What was bound before goes back.
                p.binding.reset();
            }
            return out;
        }
    }
}
