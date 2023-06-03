// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUSD/USDPrivate.h>

#include <tlCore/LRUCache.h>
#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

#include <pxr/pxr.h>
#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdUtils/pipeline.h>
#include <pxr/usdImaging/usdAppUtils/api.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>
#include <pxr/usdImaging/usdAppUtils/frameRecorder.h>
#include <pxr/usdImaging/usdImagingGL/engine.h>
#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hdSt/hioConversions.h>
#include <pxr/imaging/hdSt/textureUtils.h>
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/imaging/hdx/types.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using namespace PXR_NS;

namespace tl
{
    namespace usd
    {
        struct Renderer::Private
        {
            std::weak_ptr<log::System> logSystem;
            
            io::Info info;
            struct InfoRequest
            {
                int64_t id = -1;
                file::Path path;
                std::promise<io::Info> promise;
            };

            struct Request
            {
                int64_t id = -1;
                file::Path path;
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
            
            struct CacheItem
            {
                UsdStageRefPtr stage;
                std::shared_ptr<UsdImagingGLEngine> engine;
            };
            
            struct Thread
            {
                memory::LRUCache<std::string, CacheItem> cache;
                GLFWwindow* glfwWindow = nullptr;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };
        
        void Renderer::_init(const std::weak_ptr<log::System>& logSystem)
        {
            TLRENDER_P();
            p.thread.cache.setMax(10);
            p.logSystem = logSystem;
            p.thread.logTimer = std::chrono::steady_clock::now();
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    try
                    {
                        _createWindow();
                        _run();
                        p.thread.cache.clear();
                        if (p.thread.glfwWindow)
                        {
                            glfwDestroyWindow(p.thread.glfwWindow);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        //std::cout << e.what() << std::endl;
                        if (auto logSystem = p.logSystem.lock())
                        {
                            const std::string id = string::Format("tl::usd::Renderer ({0}: {1})").
                                arg(__FILE__).
                                arg(__LINE__);
                            logSystem->print(id, e.what(), log::Type::Error);
                        }
                    }
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        p.mutex.stopped = true;
                    }
                    cancelRequests();
                });
        }

        Renderer::Renderer() :
            _p(new Private)
        {}

        Renderer::~Renderer()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<Renderer> Renderer::create(const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Renderer>(new Renderer);
            out->_init(logSystem);
            return out;
        }
        
        std::future<io::Info> Renderer::getInfo(int64_t id, const file::Path& path)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::InfoRequest>();
            request->id = id;
            request->path = path;
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

        std::future<io::VideoData> Renderer::render(
            int64_t id,
            const file::Path& path,
            const otime::RationalTime& time,
            uint16_t layer)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::Request>();
            request->id = id;
            request->path = path;
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
        
        void Renderer::cancelRequests(int64_t id)
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::Request> > requests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                auto i = p.mutex.infoRequests.begin();
                while (i != p.mutex.infoRequests.end())
                {
                    if (id == (*i)->id)
                    {
                        infoRequests.push_back(*i);
                        i = p.mutex.infoRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
                auto j = p.mutex.requests.begin();
                while (j != p.mutex.requests.end())
                {
                    if (id == (*j)->id)
                    {
                        requests.push_back(*j);
                        j = p.mutex.requests.erase(j);
                    }
                    else
                    {
                        ++j;
                    }
                }
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
        
        void Renderer::cancelRequests()
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
        
        void Renderer::_createWindow()
        {
            TLRENDER_P();
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
            p.thread.glfwWindow = glfwCreateWindow(1, 1, "tl::usd::Renderer", NULL, NULL);
            if (!p.thread.glfwWindow)
            {
                throw std::runtime_error("Cannot create window");
            }
            glfwMakeContextCurrent(p.thread.glfwWindow);
        }
        
        namespace
        {
            UsdGeomCamera getCamera(
                const UsdStageRefPtr& stage,
                const std::string& name = std::string())
            {
                const TfToken primaryCameraName = UsdUtilsGetPrimaryCameraName();
                UsdGeomCamera out = UsdAppUtilsGetCameraAtPath(
                    stage,
                    SdfPath(!name.empty() ? name : primaryCameraName));
                if (!out)
                {
                    for (const auto& prim : stage->Traverse())
                    {
                        if (prim.IsA<UsdGeomCamera>())
                        {
                            out = UsdGeomCamera(prim);
                            break;
                        }
                    }
                }
                return out;
            }

            GfCamera getCameraToFrameStage(
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
        
        void Renderer::_run()
        {
            TLRENDER_P();
                        
            TfDiagnosticMgr::GetInstance().SetQuiet(true);
            
            while (p.thread.running)
            {
                // Check requests.
                std::shared_ptr<Private::InfoRequest> infoRequest;
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
                        if (!p.mutex.infoRequests.empty())
                        {
                            infoRequest = p.mutex.infoRequests.front();
                            p.mutex.infoRequests.pop_front();
                        }
                        if (!p.mutex.requests.empty())
                        {
                            request = p.mutex.requests.front();
                            p.mutex.requests.pop_front();
                        }
                    }
                }

                // Handle information requests.
                if (infoRequest)
                {
                    const std::string fileName = infoRequest->path.get();
                    Private::CacheItem item;
                    if (!p.thread.cache.get(fileName, item))
                    {
                        item.stage = UsdStage::Open(fileName);
                        const bool gpuEnabled = true;
                        item.engine = std::make_shared<UsdImagingGLEngine>(HdDriver(), TfToken(), gpuEnabled);
                        p.thread.cache.add(fileName, item);
                    }
                    io::Info info;
                    if (item.stage)
                    {
                        info.video.push_back(
                            imaging::Info(1920, 1080, imaging::PixelType::RGBA_F16));
                        const double startTimeCode = item.stage->GetStartTimeCode();
                        const double endTimeCode = item.stage->GetEndTimeCode();
                        const double timeCodesPerSecond = item.stage->GetTimeCodesPerSecond();
                        info.videoTime = otime::TimeRange::range_from_start_end_time_inclusive(
                            otime::RationalTime(startTimeCode, timeCodesPerSecond),
                            otime::RationalTime(endTimeCode, timeCodesPerSecond));
                        //std::cout << fileName << " range: " << info.videoTime << std::endl;
                    }
                    infoRequest->promise.set_value(info);
                }
                
                // Handle requests.
                if (request)
                {
                    const std::string fileName = request->path.get();
                    Private::CacheItem item;
                    if (!p.thread.cache.get(fileName, item))
                    {
                        item.stage = UsdStage::Open(fileName);
                        const bool gpuEnabled = true;
                        item.engine = std::make_shared<UsdImagingGLEngine>(HdDriver(), TfToken(), gpuEnabled);
                        p.thread.cache.add(fileName, item);
                    }
                    std::shared_ptr<imaging::Image> image;
                    if (item.stage && item.engine)
                    {
                        const TfTokenVector purposes({ UsdGeomTokens->default_, UsdGeomTokens->proxy });
                        const double timeCode = request->time.rescaled_to(
                            item.stage->GetTimeCodesPerSecond()).value();
                        //std::cout << fileName << " timeCode: " << timeCode << std::endl;
                        
                        auto camera = getCamera(item.stage);
                        GfCamera gfCamera;
                        if (camera)
                        {
                            //std::cout << fileName << " camera: " << camera.GetPath().GetAsToken().GetText() << std::endl;
                            gfCamera = camera.GetCamera(timeCode);
                        }
                        else
                        {
                            gfCamera = getCameraToFrameStage(item.stage, timeCode, purposes);
                        }

                        float aspectRatio = gfCamera.GetAspectRatio();
                        if (GfIsClose(aspectRatio, 0.F, 1e-4))
                        {
                            aspectRatio = 1.F;
                        }
                        const size_t imageWidth = 1920;
                        const size_t imageHeight = std::max<size_t>(
                            static_cast<size_t>(static_cast<float>(imageWidth) / aspectRatio),
                            1u);
                        const GfVec2i renderResolution(imageWidth, imageHeight);
                        const GfFrustum frustum = gfCamera.GetFrustum();
                        const GfVec3d cameraPos = frustum.GetPosition();
                        
                        item.engine->SetRendererAov(HdAovTokens->color);
                        item.engine->SetCameraState(
                            frustum.ComputeViewMatrix(),
                            frustum.ComputeProjectionMatrix());
                        item.engine->SetRenderViewport(GfVec4d(
                            0.0,
                            0.0,
                            static_cast<double>(imageWidth),
                            static_cast<double>(imageHeight)));

                        GlfSimpleLight cameraLight(
                            GfVec4f(cameraPos[0], cameraPos[1], cameraPos[2], 1.F));
                        cameraLight.SetAmbient(GfVec4f(.01F, .01F, .01F, 01.F));
                        const GlfSimpleLightVector lights({ cameraLight });

                        GlfSimpleMaterial material;
                        material.SetAmbient(GfVec4f(0.2f, 0.2f, 0.2f, 1.0));
                        material.SetSpecular(GfVec4f(0.1f, 0.1f, 0.1f, 1.0f));
                        material.SetShininess(32.F);
                        const GfVec4f ambient(0.01f, 0.01f, 0.01f, 1.0f);
                        item.engine->SetLightingState(lights, material, ambient);

                        UsdImagingGLRenderParams renderParams;
                        renderParams.frame = timeCode;
                        //renderParams.complexity = 1.F;
                        renderParams.colorCorrectionMode = HdxColorCorrectionTokens->sRGB;
                        //renderParams.clearColor = GfVec4f(0.F);
                        //renderParams.showProxy = true;
                        //renderParams.showRender = true;
                        //renderParams.showGuides = false;
                        const UsdPrim& pseudoRoot = item.stage->GetPseudoRoot();
                        unsigned int sleepTime = 10;
                        while (true)
                        {
                            item.engine->Render(pseudoRoot, renderParams);
                            if (item.engine->IsConverged())
                            {
                                break;
                            }
                            else
                            {
                                std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
                                sleepTime = std::min(100u, sleepTime + 5);
                            }
                        }

                        if (item.engine->GetGPUEnabled())
                        {
                            const auto colorTextureHandle = item.engine->GetAovTexture(HdAovTokens->color);
                            if (colorTextureHandle)
                            {
                                size_t size = 0;
                                const auto mappedColorTextureBuffer = HdStTextureUtils::HgiTextureReadback(
                                    item.engine->GetHgi(),
                                    colorTextureHandle,
                                    &size);
                                switch (HdxGetHioFormat(colorTextureHandle->GetDescriptor().format))
                                {
                                case HioFormat::HioFormatFloat16Vec4:
                                    image = imaging::Image::create(
                                        imaging::Info(imageWidth, imageHeight, imaging::PixelType::RGBA_F16));
                                    memcpy(image->getData(), mappedColorTextureBuffer.get(), image->getDataByteCount());
                                    break;
                                }
                            }
                        }
                        else
                        {
                            const auto colorRenderBuffer = item.engine->GetAovRenderBuffer(HdAovTokens->color);
                            if (colorRenderBuffer)
                            {
                                colorRenderBuffer->Resolve();
                                colorRenderBuffer->Map();
                                switch (HdStHioConversions::GetHioFormat(colorRenderBuffer->GetFormat()))
                                {
                                case HioFormat::HioFormatFloat16Vec4:
                                    image = imaging::Image::create(
                                        imaging::Info(imageWidth, imageHeight, imaging::PixelType::RGBA_F16));
                                    memcpy(image->getData(), colorRenderBuffer->Map(), image->getDataByteCount());
                                    break;
                                }
                            }
                        }
                    }
                    io::VideoData data;
                    data.time = request->time;
                    data.image = image;
                    request->promise.set_value(data);
                }
            }
        }
    }
}

