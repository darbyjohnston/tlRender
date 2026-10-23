// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/GL/OffscreenRender.h>

#include <tlRender/GL/Render.h>

#include <ftk/GL/OffscreenBuffer.h>
#include <ftk/GL/Window.h>

#include <ftk/Core/Context.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/Matrix.h>
#include <ftk/Core/Memory.h>

#include <algorithm>
#include <list>
#include <stdexcept>

namespace tl
{
    namespace gl
    {
        struct OffscreenRender::Private
        {
            // First, so that it goes last: the renderer and the buffer are
            // the context's, and are let go of while it is current.
            std::shared_ptr<ftk::gl::Window> window;
            std::shared_ptr<Render> render;
            // The buffers of the last two renders, most recent first. A
            // render that alternates between two output sizes, as a compare
            // of mismatched sources does, would otherwise make a buffer each
            // frame -- and a 4K one is a sixth of a gigabyte to allocate
            // and page in.
            std::list<std::shared_ptr<ftk::gl::OffscreenBuffer> > buffers;
            OCIOOptions ocioOptions;
            LUTOptions lutOptions;
            // Enough for a pair of 4K frames in floating point.
            size_t cacheByteCount = ftk::megabyte * 512;
            // The images of the last render. Frames are kept as textures
            // only once one comes round again: a sequence of new frames is
            // faster copied into the same few textures than given a new
            // texture each.
            std::vector<std::weak_ptr<ftk::Image> > images;
        };

        namespace
        {
            std::vector<std::shared_ptr<ftk::Image> > getImages(
                const std::vector<VideoFrame>& videoFrames)
            {
                std::vector<std::shared_ptr<ftk::Image> > out;
                for (const auto& videoFrame : videoFrames)
                {
                    for (const auto& layer : videoFrame.layers)
                    {
                        if (layer.image)
                        {
                            out.push_back(layer.image);
                        }
                        if (layer.imageB)
                        {
                            out.push_back(layer.imageB);
                        }
                    }
                }
                return out;
            }
        }

        void OffscreenRender::_init(const std::shared_ptr<ftk::Context>& context)
        {
            FTK_P();
            // Hidden, and only for its context: everything is drawn into the
            // offscreen buffer.
            p.window = ftk::gl::Window::create(
                context,
                "tl::gl::OffscreenRender",
                ftk::Size2I(1, 1),
                static_cast<int>(ftk::gl::WindowOptions::MakeCurrent));
            p.render = Render::create(
                context->getLogSystem(),
                context->getSystem<ftk::FontSystem>());
            p.window->clearCurrent();
        }

        OffscreenRender::OffscreenRender() :
            _p(new Private)
        {}

        OffscreenRender::~OffscreenRender()
        {
            FTK_P();
            if (p.window)
            {
                p.window->makeCurrent();
                p.buffers.clear();
                p.render.reset();
                p.window->clearCurrent();
            }
        }

        std::shared_ptr<OffscreenRender> OffscreenRender::create(
            const std::shared_ptr<ftk::Context>& context)
        {
            auto out = std::shared_ptr<OffscreenRender>(new OffscreenRender);
            out->_init(context);
            return out;
        }

        void OffscreenRender::setOCIOOptions(const OCIOOptions& value)
        {
            _p->ocioOptions = value;
        }

        void OffscreenRender::setLUTOptions(const LUTOptions& value)
        {
            _p->lutOptions = value;
        }

        size_t OffscreenRender::getCacheByteCount() const
        {
            return _p->cacheByteCount;
        }

        void OffscreenRender::setCacheByteCount(size_t value)
        {
            FTK_P();
            if (value == p.cacheByteCount)
                return;
            p.cacheByteCount = value;
            if (0 == value)
            {
                clearCache();
            }
        }

        void OffscreenRender::clearCache()
        {
            FTK_P();
            p.window->makeCurrent();
            p.render->clearTextureCache();
            p.window->clearCurrent();
        }

        std::shared_ptr<ftk::Image> OffscreenRender::render(
            const std::vector<VideoFrame>& videoFrames,
            const CompareOptions& compareOptions,
            const ftk::Size2I& size,
            const std::vector<ftk::ImageOptions>& imageOptions,
            const std::vector<DisplayOptions>& displayOptions,
            ftk::ImageType type)
        {
            FTK_P();
            if (!ftk::gl::OffscreenBuffer::canRead(type))
            {
                throw std::runtime_error(ftk::Format(
                    "A render cannot be read back as {0}.").arg(ftk::to_string(type)));
            }

            // Laid out as a viewport lays them out, at the frames' own size.
            const AspectRatioOptions aspectRatio = !displayOptions.empty() ?
                displayOptions.front().aspectRatio :
                AspectRatioOptions();
            const ftk::Size2I renderSize = getRenderSize(
                compareOptions,
                aspectRatio,
                videoFrames);
            if (!renderSize.isValid())
            {
                throw std::runtime_error("There are no video frames to render.");
            }
            const auto boxes = getBoxes(compareOptions, aspectRatio, videoFrames);

            // Fitted into the size asked for, keeping its shape and centered,
            // as a viewport fits it to its window.
            const ftk::Size2I outputSize = size.isValid() ? size : renderSize;
            const float zoom = std::min(
                outputSize.w / static_cast<float>(renderSize.w),
                outputSize.h / static_cast<float>(renderSize.h));
            const ftk::V2F viewPos(
                (outputSize.w - renderSize.w * zoom) / 2.F,
                (outputSize.h - renderSize.h * zoom) / 2.F);
            const ftk::M44F vm =
                ftk::translate(ftk::V3F(viewPos.x, viewPos.y, 0.F)) *
                ftk::scale(ftk::V3F(zoom, zoom, 1.F));
            const ftk::M44F pm = ftk::ortho(
                0.F,
                static_cast<float>(outputSize.w),
                static_cast<float>(outputSize.h),
                0.F,
                -1.F,
                1.F);

            // Kept as textures when an image was in the last render too.
            const auto images = getImages(videoFrames);
            bool videoCache = false;
            if (p.cacheByteCount > 0)
            {
                for (const auto& weak : p.images)
                {
                    const auto image = weak.lock();
                    if (image &&
                        std::find(images.begin(), images.end(), image) != images.end())
                    {
                        videoCache = true;
                        break;
                    }
                }
            }
            p.images.assign(images.begin(), images.end());

            p.window->makeCurrent();
            std::shared_ptr<ftk::Image> out;
            try
            {
                // Rendered in floating point, as a viewport is, whatever
                // the image read back from it is.
                const ftk::ImageType colorBuffer = ftk::ImageType::RGBA_F32;
                const ftk::gl::TextureType bufferType = ftk::gl::getRenderableType(
                    ftk::gl::getTextureType(colorBuffer));
                // With a stencil, as a viewport's buffer has: a wipe draws
                // each side through a stencil mask, and without one the mask
                // passes everything and the last side drawn covers the lot.
                ftk::gl::OffscreenBufferOptions bufferOptions;
                bufferOptions.depth = ftk::gl::OffscreenDepth::_24;
                bufferOptions.stencil = ftk::gl::OffscreenStencil::_8;
                std::shared_ptr<ftk::gl::OffscreenBuffer> buffer;
                for (auto i = p.buffers.begin(); i != p.buffers.end(); ++i)
                {
                    if (!ftk::gl::doCreate(*i, outputSize, bufferType, bufferOptions))
                    {
                        buffer = *i;
                        p.buffers.erase(i);
                        break;
                    }
                }
                if (!buffer)
                {
                    buffer = ftk::gl::OffscreenBuffer::create(
                        outputSize,
                        bufferType,
                        bufferOptions);
                }
                p.buffers.push_front(buffer);
                while (p.buffers.size() > 2)
                {
                    p.buffers.pop_back();
                }
                {
                    ftk::gl::OffscreenBufferBinding binding(buffer);
                    ftk::RenderOptions renderOptions;
                    renderOptions.textureCacheByteCount = p.cacheByteCount;
                    p.render->begin(outputSize, renderOptions);
                    p.render->setViewport(ftk::Box2I(0, 0, outputSize.w, outputSize.h));
                    p.render->setClipRectEnabled(false);
                    p.render->clearViewport(ftk::Color4F(0.F, 0.F, 0.F, 0.F));
                    p.render->setVideoCache(videoCache);
                    p.render->setOCIOOptions(p.ocioOptions);
                    p.render->setLUTOptions(p.lutOptions);
                    p.render->setTransform(pm * vm);
                    p.render->drawVideo(
                        videoFrames,
                        boxes,
                        imageOptions,
                        displayOptions,
                        compareOptions,
                        colorBuffer);
                    p.render->end();
                    out = buffer->read(ftk::ImageInfo(outputSize, type));
                }
            }
            catch (...)
            {
                p.window->clearCurrent();
                throw;
            }
            p.window->clearCurrent();
            if (!out)
            {
                throw std::runtime_error("The render could not be read back.");
            }
            return out;
        }
    }
}
