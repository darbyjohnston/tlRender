// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlIO/USDPrivate.h>

#include <tlCore/File.h>
#include <tlCore/LRUCache.h>
#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

#include <pxr/pxr.h>
#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdUtils/pipeline.h>
#include <pxr/usdImaging/usdAppUtils/api.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>
#include <pxr/usdImaging/usdAppUtils/frameRecorder.h>
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
        struct Render::Private
        {
            std::weak_ptr<log::System> logSystem;
            GLFWwindow* glfwWindow = nullptr;
            
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
                RenderOptions renderOptions;
                std::list<std::shared_ptr<InfoRequest> > infoRequests;
                std::list<std::shared_ptr<Request> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            Mutex mutex;
            
            struct StageCacheItem
            {
                UsdStageRefPtr stage;
                std::shared_ptr<UsdImagingGLEngine> engine;
            };
            
            struct DiskCacheKey
            {
                std::string fileName;
                otime::RationalTime time = time::invalidTime;
                
                bool operator < (const DiskCacheKey& other) const
                {
                    return std::tie(fileName, time) < std::tie(other.fileName, other.time);
                }
            };
            
            struct DiskCacheItem
            {
                ~DiskCacheItem()
                {
                    file::rm(fileName);
                }
                
                std::string fileName;
            };
            
            struct Thread
            {
                memory::LRUCache<std::string, StageCacheItem> stageCache;
                memory::LRUCache<DiskCacheKey, std::shared_ptr<DiskCacheItem> > diskCache;
                std::string tempDir;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };
        
        void Render::_init(const std::weak_ptr<log::System>& logSystem)
        {
            TLRENDER_P();
            p.logSystem = logSystem;

#if defined(__APPLE__)
            const int glVersionMinor = 1;
            const int glProfile = GLFW_OPENGL_CORE_PROFILE;
#else //__APPLE__
            const int glVersionMinor = 5;
            const int glProfile = GLFW_OPENGL_COMPAT_PROFILE;
#endif //__APPLE__
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glVersionMinor);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, glProfile);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
            p.glfwWindow = glfwCreateWindow(1, 1, "tl::usd::Render", NULL, NULL);
            if (!p.glfwWindow)
            {
                throw std::runtime_error("Cannot create window");
            }

            p.thread.logTimer = std::chrono::steady_clock::now();
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    try
                    {
                        glfwMakeContextCurrent(p.glfwWindow);
                        _run();
                        p.thread.stageCache.clear();
                        p.thread.diskCache.clear();
                    }
                    catch (const std::exception& e)
                    {
                        //std::cout << e.what() << std::endl;
                        if (auto logSystem = p.logSystem.lock())
                        {
                            const std::string id = string::Format("tl::usd::Render ({0}: {1})").
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
            
            if (auto logSystem = p.logSystem.lock())
            {
                std::vector<std::string> renderers;
                for (const auto& id : UsdImagingGLEngine::GetRendererPlugins())
                {
                    renderers.push_back(UsdImagingGLEngine::GetRendererDisplayName(id));
                }
                logSystem->print(
                    "tl::usd::Render",
                    string::Format(
                        "\n"
                        "    Renderers: {0}").
                    arg(string::join(renderers, ", ")));
            }
        }

        Render::Render() :
            _p(new Private)
        {}

        Render::~Render()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
            if (p.glfwWindow)
            {
                glfwDestroyWindow(p.glfwWindow);
            }
        }

        std::shared_ptr<Render> Render::create(const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Render>(new Render);
            out->_init(logSystem);
            return out;
        }
        
        void Render::setRenderOptions(const RenderOptions& value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            p.mutex.renderOptions = value;
        }
        
        std::future<io::Info> Render::getInfo(int64_t id, const file::Path& path)
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

        std::future<io::VideoData> Render::render(
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
        
        void Render::cancelRequests(int64_t id)
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
        
        void Render::cancelRequests()
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
                    planeCorner = GfVec2d(dim[0], dim[1]) / 2;
                }
                else
                {
                    planeCorner = GfVec2d(dim[0], dim[2]) / 2;
                }
                const float planeRadius = sqrt(GfDot(planeCorner, planeCorner));

                const float halfFov = gfCamera.GetFieldOfView(GfCamera::FOVHorizontal) / 2.0;
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
                    xf.SetRotate(GfRotation(GfVec3d(1, 0, 0), 90));
                    xf.SetTranslateOnly(center + GfVec3d(0, -distance, 0));
                }
                gfCamera.SetTransform(xf);
                return gfCamera;
            }
            
            UsdImagingGLDrawMode toUSD(DrawMode value)
            {
                const std::vector<UsdImagingGLDrawMode> data =
                {
                    UsdImagingGLDrawMode::DRAW_POINTS,
                    UsdImagingGLDrawMode::DRAW_WIREFRAME,
                    UsdImagingGLDrawMode::DRAW_WIREFRAME_ON_SURFACE,
                    UsdImagingGLDrawMode::DRAW_SHADED_FLAT,
                    UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH,
                    UsdImagingGLDrawMode::DRAW_GEOM_ONLY,
                    UsdImagingGLDrawMode::DRAW_GEOM_FLAT,
                    UsdImagingGLDrawMode::DRAW_GEOM_SMOOTH
                };
                return data[static_cast<size_t>(value)];
            };
        }

        void Render::_open(
            const std::string& fileName,
            UsdStageRefPtr& stage,
            std::shared_ptr<UsdImagingGLEngine>& engine)
        {
            TLRENDER_P();
            stage = UsdStage::Open(fileName);
            const bool gpuEnabled = true;
            engine = std::make_shared<UsdImagingGLEngine>(HdDriver(), TfToken(), gpuEnabled);
            if (stage && engine)
            {
                if (auto logSystem = p.logSystem.lock())
                {
                    const std::string renderer =
                        UsdImagingGLEngine::GetRendererDisplayName(
                            engine->GetCurrentRendererId());
                    std::vector<std::string> aovs;
                    for (const auto& i : engine->GetRendererAovs())
                    {
                        aovs.push_back(i.GetText());
                    }
                    logSystem->print(
                        "tl::usd::Render",
                        string::Format(
                            "\n"
                            "    File name: {0}\n"
                            "    Time code: {1}-{2}:{3}\n"
                            "    GPU enabled: {4}\n"
                            "    Renderer ID: {5}\n"
                            "    Renderer AOVs available: {6}").
                        arg(fileName).
                        arg(stage->GetStartTimeCode()).
                        arg(stage->GetEndTimeCode()).
                        arg(stage->GetTimeCodesPerSecond()).
                        arg(engine->GetGPUEnabled()).
                        arg(renderer).
                        arg(string::join(aovs, ", ")));
                }
            }
        }
        
        void Render::_run()
        {
            TLRENDER_P();
                        
            TfDiagnosticMgr::GetInstance().SetQuiet(true);

            const TfTokenVector purposes({ UsdGeomTokens->default_, UsdGeomTokens->proxy });

            RenderOptions renderOptions;
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
                        [this, &renderOptions]
                        {
                            return
                                _p->mutex.renderOptions != renderOptions ||
                                !_p->mutex.infoRequests.empty() ||
                                !_p->mutex.requests.empty();
                        }))
                    {
                        renderOptions = p.mutex.renderOptions;
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

                // Set options.
                p.thread.stageCache.setMax(renderOptions.stageCacheSize);
                p.thread.diskCache.setMax(renderOptions.diskCacheSize);
                if (renderOptions.diskCacheSize > 0 && p.thread.tempDir.empty())
                {
                    p.thread.tempDir = file::createTempDir();
                    if (auto logSystem = p.logSystem.lock())
                    {
                        logSystem->print(
                            "tl::usd::Render",
                            string::Format(
                                "\n"
                                "    Temp directory: {0}\n"
                                "    Disk cache size: {1}").
                            arg(p.thread.tempDir).
                            arg(renderOptions.diskCacheSize));
                    }
                }
                else if (0 == renderOptions.diskCacheSize && !p.thread.tempDir.empty())
                {
                    p.thread.tempDir = std::string();
                }

                // Handle information requests.
                if (infoRequest)
                {
                    const std::string fileName = infoRequest->path.get();
                    Private::StageCacheItem stageCacheItem;
                    if (!p.thread.stageCache.get(fileName, stageCacheItem))
                    {
                        _open(fileName, stageCacheItem.stage, stageCacheItem.engine);
                        p.thread.stageCache.add(fileName, stageCacheItem);
                    }
                    io::Info info;
                    if (stageCacheItem.stage)
                    {
                        const double startTimeCode = stageCacheItem.stage->GetStartTimeCode();
                        const double endTimeCode = stageCacheItem.stage->GetEndTimeCode();
                        const double timeCodesPerSecond = stageCacheItem.stage->GetTimeCodesPerSecond();
                        GfCamera gfCamera;
                        auto camera = getCamera(stageCacheItem.stage);
                        if (camera)
                        {
                            //std::cout << fileName << " camera: " <<
                            //    camera.GetPath().GetAsToken().GetText() << std::endl;
                            gfCamera = camera.GetCamera(startTimeCode);
                        }
                        else
                        {
                            gfCamera = getCameraToFrameStage(stageCacheItem.stage, startTimeCode, purposes);
                        }
                        float aspectRatio = gfCamera.GetAspectRatio();
                        if (GfIsClose(aspectRatio, 0.F, 1e-4))
                        {
                            aspectRatio = 1.F;
                        }
                        info.video.push_back(imaging::Info(
                            renderOptions.renderWidth,
                            renderOptions.renderWidth / aspectRatio,
                            imaging::PixelType::RGBA_F16));
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
                    // Check the disk cache for a previously rendered frame.
                    const std::string fileName = request->path.get();
                    std::shared_ptr<imaging::Image> image;
                    std::shared_ptr<Private::DiskCacheItem> diskCacheItem;
                    if (renderOptions.diskCacheSize > 0 &&
                        p.thread.diskCache.get(Private::DiskCacheKey({ fileName, request->time }), diskCacheItem))
                    {
                        //std::cout << "read temp file: " << diskCacheItem->fileName << std::endl;
                        auto fileIO = file::FileIO::create(diskCacheItem->fileName, file::Mode::Read);
                        uint16_t w = 0;
                        uint16_t h = 0;
                        fileIO->readU16(&w);
                        fileIO->readU16(&h);
                        uint32_t pixelType = 0;
                        fileIO->readU32(&pixelType);
                        image = imaging::Image::create(w, h, static_cast<imaging::PixelType>(pixelType));
                        fileIO->read(image->getData(), image->getDataByteCount());
                    }
                    else
                    {
                        // Check the stage cache for a previously opened stage.
                        Private::StageCacheItem stageCacheItem;
                        if (!p.thread.stageCache.get(fileName, stageCacheItem))
                        {
                            _open(fileName, stageCacheItem.stage, stageCacheItem.engine);
                            p.thread.stageCache.add(fileName, stageCacheItem);
                        }
                        if (stageCacheItem.stage && stageCacheItem.engine)
                        {
                            const double timeCode = request->time.rescaled_to(
                                stageCacheItem.stage->GetTimeCodesPerSecond()).value();
                            //std::cout << fileName << " timeCode: " << timeCode << std::endl;
                            
                            // Setup the camera.
                            GfCamera gfCamera;
                            auto camera = getCamera(stageCacheItem.stage);
                            if (camera)
                            {
                                gfCamera = camera.GetCamera(timeCode);
                            }
                            else
                            {
                                gfCamera = getCameraToFrameStage(stageCacheItem.stage, timeCode, purposes);
                            }
                            const GfFrustum frustum = gfCamera.GetFrustum();
                            const GfVec3d cameraPos = frustum.GetPosition();
                            stageCacheItem.engine->SetCameraState(
                                frustum.ComputeViewMatrix(),
                                frustum.ComputeProjectionMatrix());
                            float aspectRatio = gfCamera.GetAspectRatio();
                            if (GfIsClose(aspectRatio, 0.F, 1e-4))
                            {
                                aspectRatio = 1.F;
                            }
                            const size_t renderHeight = renderOptions.renderWidth / aspectRatio;
                            stageCacheItem.engine->SetRenderViewport(GfVec4d(
                                0.0,
                                0.0,
                                static_cast<double>(renderOptions.renderWidth),
                                static_cast<double>(renderHeight)));

                            //for (const auto& token : stageCacheItem.engine->GetRendererAovs())
                            //{
                            //    std::cout << token.GetText() << std::endl;
                            //}
                            stageCacheItem.engine->SetRendererAov(HdAovTokens->color);

                            // Setup a light.
                            GlfSimpleLight cameraLight(
                                GfVec4f(cameraPos[0], cameraPos[1], cameraPos[2], 1.F));
                            cameraLight.SetAmbient(GfVec4f(.01F, .01F, .01F, 01.F));
                            const GlfSimpleLightVector lights({ cameraLight });

                            // Setup a material.
                            GlfSimpleMaterial material;
                            material.SetAmbient(GfVec4f(0.2f, 0.2f, 0.2f, 1.0));
                            material.SetSpecular(GfVec4f(0.1f, 0.1f, 0.1f, 1.0f));
                            material.SetShininess(32.F);
                            const GfVec4f ambient(0.01f, 0.01f, 0.01f, 1.0f);
                            stageCacheItem.engine->SetLightingState(lights, material, ambient);

                            // Render the frame.
                            UsdImagingGLRenderParams renderParams;
                            renderParams.frame = timeCode;
                            renderParams.complexity = renderOptions.complexity;
                            renderParams.drawMode = toUSD(renderOptions.drawMode);
                            renderParams.enableLighting = renderOptions.enableLighting;
                            renderParams.colorCorrectionMode = HdxColorCorrectionTokens->sRGB;
                            const UsdPrim& pseudoRoot = stageCacheItem.stage->GetPseudoRoot();
                            unsigned int sleepTime = 10;
                            while (true)
                            {
                                stageCacheItem.engine->Render(pseudoRoot, renderParams);
                                if (stageCacheItem.engine->IsConverged())
                                {
                                    break;
                                }
                                else
                                {
                                    std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
                                    sleepTime = std::min(100u, sleepTime + 5);
                                }
                            }

                            // Copy the rendered frame.
                            if (stageCacheItem.engine->GetGPUEnabled())
                            {
                                const auto colorTextureHandle = stageCacheItem.engine->GetAovTexture(HdAovTokens->color);
                                if (colorTextureHandle)
                                {
                                    size_t size = 0;
                                    const auto mappedColorTextureBuffer = HdStTextureUtils::HgiTextureReadback(
                                        stageCacheItem.engine->GetHgi(),
                                        colorTextureHandle,
                                        &size);
                                    //std::cout << colorTextureHandle->GetDescriptor().format << std::endl;
                                    switch (HdxGetHioFormat(colorTextureHandle->GetDescriptor().format))
                                    {
                                    case HioFormat::HioFormatFloat16Vec4:
                                        image = imaging::Image::create(
                                            renderOptions.renderWidth,
                                            renderHeight,
                                            imaging::PixelType::RGBA_F16);
                                        memcpy(image->getData(), mappedColorTextureBuffer.get(), image->getDataByteCount());
                                        break;
                                    default: break;
                                    }
                                }
                            }
                            else
                            {
                                const auto colorRenderBuffer = stageCacheItem.engine->GetAovRenderBuffer(HdAovTokens->color);
                                if (colorRenderBuffer)
                                {
                                    colorRenderBuffer->Resolve();
                                    colorRenderBuffer->Map();
                                    switch (HdStHioConversions::GetHioFormat(colorRenderBuffer->GetFormat()))
                                    {
                                    case HioFormat::HioFormatFloat16Vec4:
                                        image = imaging::Image::create(
                                            renderOptions.renderWidth,
                                            renderHeight,
                                            imaging::PixelType::RGBA_F16);
                                        memcpy(image->getData(), colorRenderBuffer->Map(), image->getDataByteCount());
                                        break;
                                    default: break;
                                    }
                                }
                            }

                            // Add the rendered frame to the disk cache.
                            if (renderOptions.diskCacheSize > 0 && image)
                            {
                                auto diskCacheItem = std::make_shared<Private::DiskCacheItem>();
                                diskCacheItem->fileName = string::Format("{0}/{1}.img").
                                    arg(p.thread.tempDir).
                                    arg(diskCacheItem);
                                //std::cout << "write temp file: " << diskCacheItem->fileName << std::endl;
                                auto tempFile = file::FileIO::create(diskCacheItem->fileName, file::Mode::Write);
                                tempFile->writeU16(image->getWidth());
                                tempFile->writeU16(image->getHeight());
                                tempFile->writeU32(static_cast<uint32_t>(image->getPixelType()));
                                tempFile->write(image->getData(), image->getDataByteCount());
                                p.thread.diskCache.add({ fileName, request->time }, diskCacheItem);
                            }
                        }
                    }
                    
                    io::VideoData data;
                    data.time = request->time;
                    data.image = image;
                    request->promise.set_value(data);
                }

                // Logging.
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff = now - p.thread.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.thread.logTimer = now;
                        if (auto logSystem = p.logSystem.lock())
                        {
                            size_t requestsSize = 0;
                            {
                                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                                requestsSize = p.mutex.requests.size();
                            }
                            logSystem->print(
                                "tl::usd::Render",
                                string::Format(
                                    "\n"
                                    "    Requests: {0}\n"
                                    "    Stage cache size: {1}/{2}\n"
                                    "    Disk cache size: {3}/{4}").
                                arg(requestsSize).
                                arg(p.thread.stageCache.getSize()).
                                arg(p.thread.stageCache.getMax()).
                                arg(p.thread.diskCache.getSize()).
                                arg(p.thread.diskCache.getMax()));
                        }
                    }
            }
            }
        }
    }
}

