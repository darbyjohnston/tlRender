// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "App.h"

#include "TestPatterns.h"

#include <tlTimelineGL/Render.h>

#include <tlIO/System.h>

#include <dtk/gl/GL.h>
#include <dtk/gl/OffscreenBuffer.h>
#include <dtk/gl/Util.h>
#include <dtk/gl/Window.h>
#include <dtk/core/Context.h>
#include <dtk/core/Format.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/track.h>
#include <opentimelineio/timeline.h>

namespace tl
{
    namespace examples
    {
        namespace test_patterns
        {
            void App::_init(
                const std::shared_ptr<dtk::Context>& context,
                std::vector<std::string>& argv)
            {
                IApp::_init(
                    context,
                    argv,
                    "test-patterns",
                    "Example test patterns application.");

                _window = dtk::gl::Window::create(
                    context,
                    "test-patterns",
                    dtk::Size2I(1, 1),
                    static_cast<int>(dtk::gl::WindowOptions::MakeCurrent));
            }

            App::App()
            {}

            App::~App()
            {}

            std::shared_ptr<App> App::create(
                const std::shared_ptr<dtk::Context>& context,
                std::vector<std::string>& argv)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(context, argv);
                return out;
            }

            void App::run()
            {
                for (const auto& size : {
                    dtk::Size2I(1920, 1080),
                    dtk::Size2I(3840, 2160),
                    dtk::Size2I(4096, 2160)
                    })
                {
                    OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline(new OTIO_NS::Timeline);
                    OTIO_NS::SerializableObject::Retainer<OTIO_NS::Track> otioTrack(new OTIO_NS::Track);
                    otioTimeline->tracks()->append_child(otioTrack);

                    for (const auto& name : {
                        CountTestPattern::getClassName(),
                        SwatchesTestPattern::getClassName(),
                        GridTestPattern::getClassName()
                        })
                    {
                        //const std::string output = dtk::Format("{0}_{1}_{2}_pattern.mp4").arg(name).arg(size.w).arg(size.h);
                        const std::string output = dtk::Format("{0}_{1}_{2}.0.dpx").arg(name).arg(size.w).arg(size.h);
                        std::cout << "Output: " << output << std::endl;
                        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Clip> otioClip(new OTIO_NS::Clip);
                        //OTIO_NS::SerializableObject::Retainer<OTIO_NS::MediaReference> mediaReference(
                        //    new OTIO_NS::ExternalReference(dtk::Format("{0}").arg(output)));
                        OTIO_NS::SerializableObject::Retainer<OTIO_NS::ImageSequenceReference> mediaReference(
                            new OTIO_NS::ImageSequenceReference(
                                "file://",
                                file::Path(output).getBaseName(),
                                file::Path(output).getExtension(),
                                0,
                                1,
                                24));
                        const OTIO_NS::TimeRange timeRange(
                            OTIO_NS::RationalTime(0.0, 24.0),
                            OTIO_NS::RationalTime(24 * 3, 24.0));
                        mediaReference->set_available_range(timeRange);
                        otioClip->set_media_reference(mediaReference);
                        otioTrack->append_child(otioClip);

                        // Create the I/O plugin.
                        auto writerPlugin = _context->getSystem<io::System>()->getPlugin(file::Path(output));
                        if (!writerPlugin)
                        {
                            throw std::runtime_error(dtk::Format("{0}: Cannot open").arg(output));
                        }
                        dtk::ImageInfo info;
                        info.size.w = size.w;
                        info.size.h = size.h;
                        info.type = dtk::ImageType::RGB_U10;
                        info = writerPlugin->getWriteInfo(info);
                        if (dtk::ImageType::None == info.type)
                        {
                            throw std::runtime_error(dtk::Format("{0}: Cannot open").arg(output));
                        }
                        io::Info ioInfo;
                        ioInfo.video.push_back(info);
                        ioInfo.videoTime = timeRange;
                        auto writer = writerPlugin->write(file::Path(output), ioInfo);
                        if (!writer)
                        {
                            throw std::runtime_error(dtk::Format("{0}: Cannot open").arg(output));
                        }

                        // Create the offscreen buffer.
                        dtk::gl::OffscreenBufferOptions offscreenBufferOptions;
                        offscreenBufferOptions.color = dtk::ImageType::RGBA_F32;
                        auto buffer = dtk::gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                        dtk::gl::OffscreenBufferBinding binding(buffer);
                        auto image = dtk::Image::create(info);

                        // Render the test pattern.
                        auto render = timeline_gl::Render::create(_context);
                        auto pattern = TestPatternFactory::create(_context, name, size);
                        for (double i = ioInfo.videoTime.start_time().value(); i < ioInfo.videoTime.duration().value(); i = i + 1.0)
                        {
                            const OTIO_NS::RationalTime time(i, 24.0);

                            render->begin(size);
                            pattern->render(render, time);
                            render->end();

                            // Write the image.
                            glPixelStorei(GL_PACK_ALIGNMENT, info.layout.alignment);
#if defined(dtk_API_GL_4_1)
                            glPixelStorei(GL_PACK_SWAP_BYTES, info.layout.endian != dtk::getEndian());
#endif // dtk_API_GL_4_1
                            const GLenum format = dtk::gl::getReadPixelsFormat(info.type);
                            const GLenum type = dtk::gl::getReadPixelsType(info.type);
                            if (GL_NONE == format || GL_NONE == type)
                            {
                                throw std::runtime_error(dtk::Format("{0}: Cannot open").arg(output));
                            }
                            glReadPixels(
                                0,
                                0,
                                info.size.w,
                                info.size.h,
                                format,
                                type,
                                image->getData());
                            writer->writeVideo(time, image);
                        }
                    }

                    otioTimeline->to_json_file(dtk::Format("{0}.otio").arg(size));
                }
            }
        }
    }
}
