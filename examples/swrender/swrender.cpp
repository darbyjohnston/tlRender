// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include "swrender.h"

#include <tlrGL/Render.h>

#include <tlrCore/Mesh.h>
#include <tlrCore/StringFormat.h>

#include <glad/gl.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace tlr
{
    namespace
    {
        void glfwErrorCallback(int, const char* description)
        {
            std::cerr << "GLFW ERROR: " << description << std::endl;
        }
    }

    void App::_init(int argc, char* argv[])
    {
        IApp::_init(
            argc,
            argv,
            "swrender",
            "Experimental software rendering.",
            {
                app::CmdLineValueArg<std::string>::create(
                    _input,
                    "input",
                    "The input timeline.")
            });
    }

    App::App()
    {}

    App::~App()
    {
        _glRender.reset();
        if (_glfwWindow)
        {
            glfwDestroyWindow(_glfwWindow);
        }
        glfwTerminate();
    }

    std::shared_ptr<App> App::create(int argc, char* argv[])
    {
        auto out = std::shared_ptr<App>(new App);
        out->_init(argc, argv);
        return out;
    }

    void App::run()
    {
        if (_exit != 0)
        {
            return;
        }

        // Read the timeline.
        auto timeline = timeline::Timeline::create(_input, _context);
        _timelinePlayer = timeline::TimelinePlayer::create(timeline, _context);

        // Initialize GLFW.
        glfwSetErrorCallback(glfwErrorCallback);
        int glfwMajor = 0;
        int glfwMinor = 0;
        int glfwRevision = 0;
        glfwGetVersion(&glfwMajor, &glfwMinor, &glfwRevision);
        _log(string::Format("GLFW version: {0}.{1}.{2}").arg(glfwMajor).arg(glfwMinor).arg(glfwRevision));
        if (!glfwInit())
        {
            throw std::runtime_error("Cannot initialize GLFW");
        }

        // Create the window.
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);
        //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
        _glfwWindow = glfwCreateWindow(
            1280,
            720,
            "swrender",
            NULL,
            NULL);
        if (!_glfwWindow)
        {
            throw std::runtime_error("Cannot create window");
        }
        glfwSetWindowUserPointer(_glfwWindow, this);
        int width = 0;
        int height = 0;
        glfwGetFramebufferSize(_glfwWindow, &width, &height);
        _frameBuffer = imaging::Image::create(imaging::Info(width, height, imaging::PixelType::RGB_F32));
        glfwGetWindowContentScale(_glfwWindow, &_contentScale.x, &_contentScale.y);
        glfwMakeContextCurrent(_glfwWindow);
        if (!gladLoaderLoadGL())
        {
            throw std::runtime_error("Cannot initialize GLAD");
        }
        const int glMajor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MAJOR);
        const int glMinor = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_VERSION_MINOR);
        const int glRevision = glfwGetWindowAttrib(_glfwWindow, GLFW_CONTEXT_REVISION);
        _log(string::Format("OpenGL version: {0}.{1}.{2}").arg(glMajor).arg(glMinor).arg(glRevision));
        glfwSetFramebufferSizeCallback(_glfwWindow, _frameBufferSizeCallback);
        glfwSetWindowContentScaleCallback(_glfwWindow, _windowContentScaleCallback);
        glfwShowWindow(_glfwWindow);
        glfwShowWindow(_glfwWindow);

        // Create the renderer.
        _glRender = gl::Render::create(_context);

        // Start the main loop.
        _timelinePlayer->setPlayback(timeline::Playback::Forward);
        while (_running && !glfwWindowShouldClose(_glfwWindow))
        {
            glfwPollEvents();
            _tick();
        }
    }

    void App::_frameBufferSizeCallback(GLFWwindow* glfwWindow, int width, int height)
    {
        App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
        app->_frameBuffer = imaging::Image::create(imaging::Info(width, height, imaging::PixelType::RGB_F32));
        app->_renderDirty = true;
    }

    void App::_windowContentScaleCallback(GLFWwindow* glfwWindow, float x, float y)
    {
        App* app = reinterpret_cast<App*>(glfwGetWindowUserPointer(glfwWindow));
        app->_contentScale.x = x;
        app->_contentScale.y = y;
        app->_renderDirty = true;
    }

    namespace
    {
        struct Pixel
        {
            float r;
            float g;
            float b;
            float a;
        };

        Pixel sample_RGBA_U8(const imaging::U8_T* p, size_t w, size_t h, float x, float y)
        {
            const float max = imaging::U8Range.getMax();

            const size_t x0 = floor(x * (w - 1));
            const size_t x1 = ceil(x * (w - 1));
            const size_t y0 = floor(y * (h - 1));
            const size_t y1 = ceil(y * (h - 1));
            const imaging::U8_T* p0 = p + y0 * w * 4 + x0 * 4;
            const imaging::U8_T* p1 = p + y0 * w * 4 + x1 * 4;
            const imaging::U8_T* p2 = p + y1 * w * 4 + x0 * 4;
            const imaging::U8_T* p3 = p + y1 * w * 4 + x1 * 4;
            const float a = x * (w - 1) - x0;
            const float b = y * (h - 1) - y0;

            return {
                (p0[0] / max * (1.F - a) + p1[0] / max * a)* (1.F - b) +
                (p2[0] / max * (1.F - a) + p3[0] / max * a) * b,
                (p0[1] / max * (1.F - a) + p1[1] / max * a)* (1.F - b) +
                (p2[1] / max * (1.F - a) + p3[1] / max * a) * b,
                (p0[2] / max * (1.F - a) + p1[2] / max * a)* (1.F - b) +
                (p2[2] / max * (1.F - a) + p3[2] / max * a) * b,
                (p0[3] / max * (1.F - a) + p1[3] / max * a)* (1.F - b) +
                (p2[3] / max * (1.F - a) + p3[3] / max * a) * b
            };
        }

        Pixel sample_YUV_420P(const imaging::U8_T* p, size_t w, size_t h, float x, float y)
        {
            const float max = imaging::U8Range.getMax();

            size_t x0 = floor(x * (w - 1));
            size_t x1 = ceil(x * (w - 1));
            size_t y0 = floor(y * (h - 1));
            size_t y1 = ceil(y * (h - 1));
            const imaging::U8_T* p0 = p + y0 * w + x0;
            const imaging::U8_T* p1 = p + y0 * w + x1;
            const imaging::U8_T* p2 = p + y1 * w + x0;
            const imaging::U8_T* p3 = p + y1 * w + x1;
            float a = x * (w - 1) - x0;
            float b = y * (h - 1) - y0;
            const float _y =
                (*p0 / max * (1.F - a) + *p1 / max * a) * (1.F - b) +
                (*p2 / max * (1.F - a) + *p3 / max * a) * b;

            const size_t w2 = w / 2;
            const size_t h2 = h / 2;
            x0 = floor(x * (w2 - 1));
            x1 = ceil(x * (w2 - 1));
            y0 = floor(y * (h2 - 1));
            y1 = ceil(y * (h2 - 1));
            p0 = p + h * w + y0 * w2 + x0;
            p1 = p + h * w + y0 * w2 + x1;
            p2 = p + h * w + y1 * w2 + x0;
            p3 = p + h * w + y1 * w2 + x1;
            a = x * (w2 - 1) - x0;
            b = y * (h2 - 1) - y0;
            const float cb =
                (*p0 / max * (1.F - a) + *p1 / max * a) * (1.F - b) +
                (*p2 / max * (1.F - a) + *p3 / max * a) * b -
                .5F;

            p0 = p + h * w + h2 * w2 + y0 * w2 + x0;
            p1 = p + h * w + h2 * w2 + y0 * w2 + x1;
            p2 = p + h * w + h2 * w2 + y1 * w2 + x0;
            p3 = p + h * w + h2 * w2 + y1 * w2 + x1;
            const float cr =
                (*p0 / max * (1.F - a) + *p1 / max * a) * (1.F - b) +
                (*p2 / max * (1.F - a) + *p3 / max * a) * b -
                .5F;

            return {
                _y + (0.F    * cb) + (1.4F   * cr),
                _y + (-.343F * cb) + (-.711F * cr),
                _y + (1.765F * cb) + (0.F    * cr),
                1.F
            };
        }

        Pixel sample(const imaging::U8_T* p, size_t w, size_t h, imaging::PixelType t, float x, float y)
        {
            switch (t)
            {
            case imaging::PixelType::RGBA_U8: return sample_RGBA_U8(p, w, h, x, y);
            case imaging::PixelType::YUV_420P: return sample_YUV_420P(p, w, h, x, y);
            default: break;
            }
            return { 0.F, 0.F, 0.F, 0.F };
        }

        void renderImage(
            const std::shared_ptr<imaging::Image>& fb,
            const math::BBox2i& bbox,
            const std::shared_ptr<imaging::Image>& image)
        {
            const imaging::Size& fbSize = fb->getSize();
            const imaging::Size& videoSize = image->getSize();
            const imaging::PixelType videoType = image->getPixelType();
            const uint8_t* videoP = image->getData();

            geom::TriangleMesh2 mesh;
            mesh.v.push_back(glm::vec2(0.F, 0.F));
            mesh.v.push_back(glm::vec2(fbSize.w, 0.F));
            mesh.v.push_back(glm::vec2(fbSize.w, fbSize.h));
            mesh.v.push_back(glm::vec2(0.F, fbSize.h));
            mesh.t.push_back(glm::vec2(0.F, 0.F));
            mesh.t.push_back(glm::vec2(1.F, 0.F));
            mesh.t.push_back(glm::vec2(1.F, 1.F));
            mesh.t.push_back(glm::vec2(0.F, 1.F));
            geom::Triangle2 triangle;
            triangle.v[0].v = 0;
            triangle.v[1].v = 1;
            triangle.v[2].v = 2;
            triangle.v[0].t = 0;
            triangle.v[1].t = 1;
            triangle.v[2].t = 2;
            mesh.triangles.push_back(triangle);
            triangle.v[0].v = 2;
            triangle.v[1].v = 3;
            triangle.v[2].v = 0;
            triangle.v[0].t = 2;
            triangle.v[1].t = 3;
            triangle.v[2].t = 0;
            mesh.triangles.push_back(triangle);

            for (size_t y = bbox.min.y; y <= bbox.max.y; ++y)
            {
                float* fbP = reinterpret_cast<float*>(fb->getData()) + (fbSize.h - 1 - y) * fbSize.w * 3 + bbox.min.x * 3;
                for (size_t x = bbox.min.x; x <= bbox.max.x; ++x)
                {
                    for (const auto& t : mesh.triangles)
                    {
                        float w0 = geom::edge(glm::vec2(x, y), mesh.v[t.v[2].v], mesh.v[t.v[1].v]);
                        float w1 = geom::edge(glm::vec2(x, y), mesh.v[t.v[0].v], mesh.v[t.v[2].v]);
                        float w2 = geom::edge(glm::vec2(x, y), mesh.v[t.v[1].v], mesh.v[t.v[0].v]);
                        if (w0 >= 0 && w1 >= 0 && w2 >= 0)
                        {
                            const float area = geom::edge(
                                mesh.v[t.v[2].v],
                                mesh.v[t.v[1].v],
                                mesh.v[t.v[0].v]);
                            w0 /= area;
                            w1 /= area;
                            w2 /= area;
                            const float u = w0 * mesh.t[t.v[0].t].x + w1 * mesh.t[t.v[1].t].x + w2 * mesh.t[t.v[2].t].x;
                            const float v = w0 * mesh.t[t.v[0].t].y + w1 * mesh.t[t.v[1].t].y + w2 * mesh.t[t.v[2].t].y;
                            const Pixel pixel = sample(videoP, videoSize.w, videoSize.h, videoType, u, v);
                            fbP[0] = pixel.r + fbP[0] * (1.F - pixel.a);
                            fbP[1] = pixel.g + fbP[1] * (1.F - pixel.a);
                            fbP[2] = pixel.b + fbP[2] * (1.F - pixel.a);
                        }
                    }
                    fbP += 3;
                }
            }
        }
    }

    void App::_tick()
    {
        // Update.
        _timelinePlayer->tick();
        const auto& videoData = _timelinePlayer->observeVideo()->get();
        if (!timeline::isTimeEqual(videoData, _videoData))
        {
            _videoData = videoData;
            _renderDirty = true;
        }

        // Render the video.
        if (_renderDirty)
        {
            _frameBuffer->zero();

            const size_t stripCount = 24;
            std::vector<math::BBox2i> strips;
            const imaging::Size& fbSize = _frameBuffer->getSize();
            const size_t stripHeight = fbSize.h / stripCount;
            size_t y = 0;
            for (size_t i = 0; i < stripCount - 1; ++i, y += stripHeight)
            {
                strips.push_back(math::BBox2i(0, y, fbSize.w, stripHeight));
            }
            strips.push_back(math::BBox2i(0, y, fbSize.w, fbSize.h - y));

            std::vector<std::future<void> > stripFutures;
            for (const auto& strip : strips)
            {
                stripFutures.push_back(std::async(
                    std::launch::async,
                    [this, strip]
                    {
                        for (const auto& layer : _videoData.layers)
                        {
                            renderImage(_frameBuffer, strip, layer.image);
                        }
                    }));
            }
            for (auto& future : stripFutures)
            {
                future.get();
            }

            _glRender->begin(fbSize);
            _glRender->drawImage(_frameBuffer, math::BBox2f(0, 0, fbSize.w, fbSize.h));
            _glRender->end();
            glfwSwapBuffers(_glfwWindow);
            _renderDirty = false;

            _renderDirty = true;
        }
        else
        {
            time::sleep(std::chrono::microseconds(1000));
        }
    }
}

int main(int argc, char* argv[])
{
    int r = 0;
    try
    {
        auto app = tlr::App::create(argc, argv);
        app->run();
        r = app->getExit();
    }
    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
    }
    return r;
}
