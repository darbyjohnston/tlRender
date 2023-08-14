// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "App.h"

#include "TestPatterns.h"

#include <tlTimeline/GLRender.h>

#include <tlGL/GL.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Util.h>

#include <tlIO/IOSystem.h>

#include <tlCore/StringFormat.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

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
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>& context)
            {
                IApp::_init(
                    argc,
                    argv,
                    context,
                    "test-patterns",
                    "Example test patterns application.");

                // Create the window.
                glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
                glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
                glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
                glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
                glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
                glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
                _glfwWindow = glfwCreateWindow(
                    100,
                    100,
                    "tlbake",
                    NULL,
                    NULL);
                if (!_glfwWindow)
                {
                    throw std::runtime_error("Cannot create window");
                }
                glfwMakeContextCurrent(_glfwWindow);
                if (!gladLoaderLoadGL())
                {
                    throw std::runtime_error("Cannot initialize GLAD");
                }
                const int glMajor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
                const int glMinor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
                const int glRevision = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_REVISION);
                _log(string::Format("OpenGL version: {0}.{1}.{2}").arg(glMajor).arg(glMinor).arg(glRevision));
            }

            App::App()
            {}

            App::~App()
            {
                if (_glfwWindow)
                {
                    glfwDestroyWindow(_glfwWindow);
                }
            }

            std::shared_ptr<App> App::create(
                int argc,
                char* argv[],
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<App>(new App);
                out->_init(argc, argv, context);
                return out;
            }

            void App::run()
            {
                if (_exit != 0)
                {
                    return;
                }

                for (const auto& size : {
                    math::Size2i(1920, 1080),
                    math::Size2i(3840, 2160),
                    math::Size2i(4096, 2160)
                    })
                {
                    otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(new otio::Timeline);
                    otio::SerializableObject::Retainer<otio::Track> otioTrack(new otio::Track);
                    otioTimeline->tracks()->append_child(otioTrack);

                    for (const auto& name : {
                        CountTestPattern::getClassName(),
                        SwatchesTestPattern::getClassName(),
                        GridTestPattern::getClassName()
                        })
                    {
                        //const std::string output = string::Format("{0}_{1}_pattern.mp4").arg(name).arg(size);
                        const std::string output = string::Format("{0}_{1}.0.dpx").arg(name).arg(size);
                        std::cout << "Output: " << output << std::endl;
                        otio::SerializableObject::Retainer<otio::Clip> otioClip(new otio::Clip);
                        //otio::SerializableObject::Retainer<otio::MediaReference> mediaReference(
                        //    new otio::ExternalReference(string::Format("{0}").arg(output)));
                        otio::SerializableObject::Retainer<otio::ImageSequenceReference> mediaReference(
                            new otio::ImageSequenceReference(
                                "file://",
                                file::Path(output).getBaseName(),
                                file::Path(output).getExtension(),
                                0,
                                1,
                                24));
                        const otime::TimeRange timeRange(
                            otime::RationalTime(0.0, 24.0),
                            otime::RationalTime(24 * 3, 24.0));
                        mediaReference->set_available_range(timeRange);
                        otioClip->set_media_reference(mediaReference);
                        otioTrack->append_child(otioClip);

                        // Create the I/O plugin.
                        auto writerPlugin = _context->getSystem<io::System>()->getPlugin(file::Path(output));
                        if (!writerPlugin)
                        {
                            throw std::runtime_error(string::Format("{0}: Cannot open").arg(output));
                        }
                        image::Info info;
                        info.size.w = size.w;
                        info.size.h = size.h;
                        info.pixelType = image::PixelType::RGB_U10;
                        info = writerPlugin->getWriteInfo(info);
                        if (image::PixelType::None == info.pixelType)
                        {
                            throw std::runtime_error(string::Format("{0}: Cannot open").arg(output));
                        }
                        io::Info ioInfo;
                        ioInfo.video.push_back(info);
                        ioInfo.videoTime = timeRange;
                        auto writer = writerPlugin->write(file::Path(output), ioInfo);
                        if (!writer)
                        {
                            throw std::runtime_error(string::Format("{0}: Cannot open").arg(output));
                        }

                        // Create the offscreen buffer.
                        gl::OffscreenBufferOptions offscreenBufferOptions;
                        offscreenBufferOptions.colorType = image::PixelType::RGBA_F32;
                        auto buffer = gl::OffscreenBuffer::create(size, offscreenBufferOptions);
                        gl::OffscreenBufferBinding binding(buffer);
                        auto image = image::Image::create(info);

                        // Render the test pattern.
                        auto render = timeline::GLRender::create(_context);
                        auto pattern = TestPatternFactory::create(name, size, _context);
                        for (double i = ioInfo.videoTime.start_time().value(); i < ioInfo.videoTime.duration().value(); i = i + 1.0)
                        {
                            const otime::RationalTime time(i, 24.0);

                            render->begin(size);
                            pattern->render(render, time);
                            render->end();

                            // Write the image.
                            glPixelStorei(GL_PACK_ALIGNMENT, info.layout.alignment);
                            glPixelStorei(GL_PACK_SWAP_BYTES, info.layout.endian != memory::getEndian());
                            const GLenum format = gl::getReadPixelsFormat(info.pixelType);
                            const GLenum type = gl::getReadPixelsType(info.pixelType);
                            if (GL_NONE == format || GL_NONE == type)
                            {
                                throw std::runtime_error(string::Format("{0}: Cannot open").arg(output));
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

                    otioTimeline->to_json_file(string::Format("{0}.otio").arg(size));
                }
            }
        }
    }
}
