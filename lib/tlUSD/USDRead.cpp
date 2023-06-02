// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUSD/USD.h>

#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

#include <pxr/pxr.h>
#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usdImaging/usdAppUtils/api.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hdSt/hioConversions.h>
#include <pxr/imaging/hdSt/textureUtils.h>
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/imaging/hdx/types.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <thread>
#include <atomic>

using namespace PXR_NS;

namespace tl
{
    namespace usd
    {
        namespace
        {
            void glfwErrorCallback(int, const char* description)
            {
                std::cerr << "GLFW ERROR: " << description << std::endl;
            }

            /*void APIENTRY glDebugOutput(
                GLenum         source,
                GLenum         type,
                GLuint         id,
                GLenum         severity,
                GLsizei        length,
                const GLchar * message,
                const void *   userParam)
            {
                switch (severity)
                {
                case GL_DEBUG_SEVERITY_HIGH:
                case GL_DEBUG_SEVERITY_MEDIUM:
                case GL_DEBUG_SEVERITY_LOW:
                case GL_DEBUG_SEVERITY_NOTIFICATION:
                default:
                    std::cerr << "DEBUG: " << message << std::endl;
                    break;
                }
            }*/
            
            GfCamera ComputeCameraToFrameStage(
                const UsdStagePtr& stage,
                UsdTimeCode timeCode,
                const TfTokenVector& includedPurposes)
            {
                GfCamera gfCamera;
                UsdGeomBBoxCache bboxCache(timeCode, includedPurposes, true);
                const GfBBox3d bbox = bboxCache.ComputeWorldBound(stage->GetPseudoRoot());
                const GfVec3d center = bbox.ComputeCentroid();
                const GfRange3d range = bbox.ComputeAlignedRange();
                const GfVec3d dim = range.GetSize();
                const TfToken upAxis = UsdGeomGetStageUpAxis(stage);

                GfVec2d planeCorner;
                if (upAxis == UsdGeomTokens->y)
                {
                    planeCorner = GfVec2d(dim[0], dim[1])/2;
                }
                else
                {
                    planeCorner = GfVec2d(dim[0], dim[2])/2;
                }
                const float planeRadius = sqrt(GfDot(planeCorner, planeCorner));

                const float halfFov = gfCamera.GetFieldOfView(GfCamera::FOVHorizontal)/2.0;
                float distance = planeRadius / tan(GfDegreesToRadians(halfFov));

                if (upAxis == UsdGeomTokens->y)
                {
                    distance += dim[2] / 2;
                }
                else
                {
                    distance += dim[1] / 2;
                }

                GfMatrix4d xf;
                if (upAxis == UsdGeomTokens->y)
                {
                    xf.SetTranslate(center + GfVec3d(0, 0, distance));
                } else
                {
                    xf.SetRotate(GfRotation(GfVec3d(1,0,0), 90));
                    xf.SetTranslateOnly(center + GfVec3d(0, -distance, 0));
                }
                gfCamera.SetTransform(xf);
                return gfCamera;
            }
        }
        
        struct Read::Private
        {
            UsdStageRefPtr stage;
            UsdGeomCamera camera;
            
            io::Info info;
            struct InfoRequest
            {
                std::promise<io::Info> promise;
            };

            struct Request
            {
                otime::RationalTime time = time::invalidTime;
                std::promise<io::VideoData> promise;
            };
            
            struct Mutex
            {
                std::list<std::shared_ptr<InfoRequest> > infoRequests;
                std::list<std::shared_ptr<Request> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            Mutex mutex;
            
            struct Thread
            {
                GLFWwindow* glfwWindow = nullptr;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };
                
        void Read::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IRead::_init(path, memory, options, logSystem);
            TLRENDER_P();
            p.thread.logTimer = std::chrono::steady_clock::now();
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this, path]
                {
                    TLRENDER_P();
                    try
                    {
                        _open(path);
                        _createWindow();
                        _run();
                        if (p.thread.glfwWindow)
                        {
                            glfwDestroyWindow(p.thread.glfwWindow);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        std::cout << e.what() << std::endl;
                        if (auto logSystem = _logSystem.lock())
                        {
                            const std::string id = string::Format("tl::usd::USDRead ({0}: {1})").
                                arg(__FILE__).
                                arg(__LINE__);
                            logSystem->print(id, string::Format("{0}: {1}").
                                arg(_path.get()).
                                arg(e.what()),
                                log::Type::Error);
                        }
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.stopped = true;
                    }
                    cancelRequests();
                });
        }

        Read::Read() :
            _p(new Private)
        {}

        Read::~Read()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<Read> Read::create(
            const file::Path& path,
            const io::Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(path, {}, options, logSystem);
            return out;
        }

        std::future<io::Info> Read::getInfo()
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::InfoRequest>();
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.infoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::Info());
            }
            return future;
        }
        
        std::future<io::VideoData> Read::readVideo(const otime::RationalTime& time, uint16_t layer)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::Request>();
            request->time = time;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::VideoData());
            }
            return future;
        }
        
        void Read::cancelRequests()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::Request> > requests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                infoRequests = std::move(p.mutex.infoRequests);
                requests = std::move(p.mutex.requests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(io::Info());
            }
            for (auto& request : requests)
            {
                request->promise.set_value(io::VideoData());
            }
        }

        void Read::_open(const file::Path& path)
        {
            TLRENDER_P();
            p.stage = UsdStage::Open(path.get());
            p.camera = UsdAppUtilsGetCameraAtPath(p.stage, SdfPath::EmptyPath());
            p.info.video.push_back(
                imaging::Info(1920, 1080, imaging::PixelType::RGBA_F32));
            p.info.videoTime = otime::TimeRange::range_from_start_end_time_inclusive(
                otime::RationalTime(p.stage->GetStartTimeCode(), p.stage->GetTimeCodesPerSecond()),
                otime::RationalTime(p.stage->GetEndTimeCode(), p.stage->GetTimeCodesPerSecond()));                        
            //std::cout << "range: " << p.info.videoTime << std::endl;
        }
        
        void Read::_createWindow()
        {
            TLRENDER_P();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
            //glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);
            p.thread.glfwWindow = glfwCreateWindow(1, 1, "tlUSD", NULL, NULL);
            if (!p.thread.glfwWindow)
            {
                throw std::runtime_error("Cannot create window");
            }
            glfwMakeContextCurrent(p.thread.glfwWindow);
            /*GLint flags = 0;
            glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            if (flags & static_cast<GLint>(GL_CONTEXT_FLAG_DEBUG_BIT))
            {
                glEnable(GL_DEBUG_OUTPUT);
                glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
                glDebugMessageCallback(glDebugOutput, _context.get());
                glDebugMessageControl(
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE),
                    static_cast<GLenum>(GL_DONT_CARE),
                    0,
                    nullptr,
                    GL_TRUE);
            }*/
        }
        
        void Read::_run()
        {
            TLRENDER_P();
            const TfTokenVector purposes({ UsdGeomTokens->default_, UsdGeomTokens->proxy });
            const bool gpuEnabled = true;
            UsdImagingGLEngine imagingEngine(HdDriver(), TfToken(), gpuEnabled);
                                                                    
            while (p.thread.running)
            {
                // Check requests.
                std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
                std::shared_ptr<Private::Request> request;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    if (p.thread.cv.wait_for(
                        lock,
                        std::chrono::milliseconds(5),
                        [this]
                        {
                            return
                                !_p->mutex.infoRequests.empty() ||
                                !_p->mutex.requests.empty();
                        }))
                    {
                        infoRequests = std::move(p.mutex.infoRequests);
                        if (!p.mutex.requests.empty())
                        {
                            request = p.mutex.requests.front();
                            p.mutex.requests.pop_front();
                        }
                    }
                }

                // Information requests.
                for (auto& request : infoRequests)
                {
                    request->promise.set_value(p.info);
                }
                
                // Requests.
                if (request)
                {
                    const double timeCode = request->time.rescaled_to(
                        p.stage->GetTimeCodesPerSecond()).value();
                    GfCamera gfCamera;
                    if (p.camera)
                    {
                        gfCamera = p.camera.GetCamera(timeCode);
                    }
                    else
                    {
                        gfCamera = ComputeCameraToFrameStage(p.stage, timeCode, purposes);
                    }
                    float aspectRatio = gfCamera.GetAspectRatio();
                    if (GfIsClose(aspectRatio, 0.F, 1e-4))
                    {
                        aspectRatio = 1.F;
                    }
                    const size_t imageHeight = std::max<size_t>(
                        static_cast<size_t>(static_cast<float>(p.info.video[0].size.w) / aspectRatio),
                        1u);
                    const GfVec2i renderResolution(p.info.video[0].size.w, imageHeight);
                    const GfFrustum frustum = gfCamera.GetFrustum();
                    const GfVec3d cameraPos = frustum.GetPosition();
                    
                    imagingEngine.SetRendererAov(HdAovTokens->color);
                    imagingEngine.SetCameraState(
                        frustum.ComputeViewMatrix(),
                        frustum.ComputeProjectionMatrix());
                    imagingEngine.SetRenderViewport(GfVec4d(
                        0.0,
                        0.0,
                        static_cast<double>(p.info.video[0].size.w),
                        static_cast<double>(imageHeight)));


                    const GfVec4f ambient(.1F, .1F, .1F, 1.F);
                    GlfSimpleLight cameraLight(
                        GfVec4f(cameraPos[0], cameraPos[1], cameraPos[2], 1.F));
                        cameraLight.SetAmbient(ambient);
                    const GlfSimpleLightVector lights({cameraLight});

                    GlfSimpleMaterial material;
                    material.SetAmbient(GfVec4f(.2F, .2F, .2F, 1.F));
                    material.SetSpecular(GfVec4f(.1F, .1F, .1F, 1.F));
                    material.SetShininess(32.F);
                    imagingEngine.SetLightingState(lights, material, ambient);

                    UsdImagingGLRenderParams renderParams;
                    renderParams.frame = timeCode;
                    renderParams.complexity = 1.F;
                    renderParams.colorCorrectionMode = HdxColorCorrectionTokens->disabled;
                    renderParams.clearColor = GfVec4f(0.F);
                    renderParams.showProxy = true;
                    renderParams.showRender = true;
                    renderParams.showGuides = false;
                    const UsdPrim& pseudoRoot = p.stage->GetPseudoRoot();
                    unsigned int sleepTime = 10;
                    while (true)
                    {
                        imagingEngine.Render(pseudoRoot, renderParams);
                        if (imagingEngine.IsConverged())
                        {
                            break;
                        }
                        else
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
                            sleepTime = std::min(100u, sleepTime + 5);
                        }
                    }
                    
                    io::VideoData data;
                    data.time = request->time;

                    if (imagingEngine.GetGPUEnabled())
                    {
                        const auto colorTextureHandle = imagingEngine.GetAovTexture(HdAovTokens->color);
                        if (colorTextureHandle)
                        {
                            size_t size = 0;
                            const auto mappedColorTextureBuffer = HdStTextureUtils::HgiTextureReadback(
                                imagingEngine.GetHgi(),
                                colorTextureHandle,
                                &size);
                            switch (HdxGetHioFormat(colorTextureHandle->GetDescriptor().format))
                            {
                            case HioFormat::HioFormatFloat16Vec4:
                                data.image = imaging::Image::create(
                                    imaging::Info(p.info.video[0].size.w, imageHeight, imaging::PixelType::RGBA_F16));
                                memcpy(data.image->getData(), mappedColorTextureBuffer.get(), data.image->getDataByteCount());
                                break;
                            }
                        }
                    }
                    else
                    {
                        const auto colorRenderBuffer = imagingEngine.GetAovRenderBuffer(HdAovTokens->color);
                        if (colorRenderBuffer)
                        {
                            colorRenderBuffer->Resolve();
                            colorRenderBuffer->Map();
                            switch (HdStHioConversions::GetHioFormat(colorRenderBuffer->GetFormat()))
                            {
                            case HioFormat::HioFormatFloat16Vec4:
                                data.image = imaging::Image::create(
                                    imaging::Info(p.info.video[0].size.w, imageHeight, imaging::PixelType::RGBA_F16));
                                memcpy(data.image->getData(), colorRenderBuffer->Map(), data.image->getDataByteCount());
                                break;
                            }
                        }
                    }
                    
                    request->promise.set_value(data);
                }
            }
        }
    }
}

