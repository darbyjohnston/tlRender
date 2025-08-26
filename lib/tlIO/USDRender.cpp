// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/USDPrivate.h>

#include <feather-tk/core/File.h>
#include <feather-tk/core/FileIO.h>
#include <feather-tk/core/Format.h>
#include <feather-tk/core/LRUCache.h>
#include <feather-tk/core/LogSystem.h>
#include <feather-tk/core/Memory.h>

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

#include <SDL2/SDL.h>

#include <filesystem>

using namespace PXR_NS;

namespace tl
{
    namespace usd
    {
        namespace
        {
            std::string getCacheKey(
                const file::Path& path,
                const OTIO_NS::RationalTime& time,
                const io::Options& options)
            {
                std::stringstream ss;
                ss << path.get() << ";" << path.getNumber() << ";" << time << ";";
                for (const auto& i : options)
                {
                    ss << i.first << ":" << i.second << ";";
                }
                return ss.str();
            }
        }

        struct Render::Private
        {
            std::weak_ptr<feather_tk::LogSystem> logSystem;

            SDL_Window* sdlWindow = nullptr;
            SDL_GLContext sdlGLContext = nullptr;
            
            struct InfoRequest
            {
                int64_t id = -1;
                file::Path path;
                io::Options options;
                std::promise<io::Info> promise;
            };

            struct Request
            {
                int64_t id = -1;
                file::Path path;
                OTIO_NS::RationalTime time = time::invalidTime;
                io::Options options;
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
            
            struct StageCacheItem
            {
                UsdStageRefPtr stage;
                std::shared_ptr<UsdImagingGLEngine> engine;
            };
            
            struct DiskCacheItem
            {
                ~DiskCacheItem()
                {
                    std::filesystem::remove(std::filesystem::u8path(fileName));
                }
                
                std::string fileName;
            };
            
            struct Thread
            {
                feather_tk::LRUCache<std::string, StageCacheItem> stageCache;
                feather_tk::LRUCache<std::string, std::shared_ptr<DiskCacheItem> > diskCache;
                std::string tempDir;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };
        
        void Render::_init(const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            FEATHER_TK_P();

            p.logSystem = logSystem;

            try
            {
#if defined(__APPLE__)
                const int glVersionMinor = 1;
#else //__APPLE__
                const int glVersionMinor = 5;
#endif //__APPLE__
#if defined(FEATHER_TK_API_GL_4_1)
                SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, glVersionMinor);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#elif defined(FEATHER_TK_API_GLES_2)
                SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#endif // FEATHER_TK_API_GL_4_1
                p.sdlWindow = SDL_CreateWindow(
                    "USD",
                    SDL_WINDOWPOS_UNDEFINED,
                    SDL_WINDOWPOS_UNDEFINED,
                    100,
                    100,
                    SDL_WINDOW_OPENGL |
                    SDL_WINDOW_RESIZABLE |
                    SDL_WINDOW_HIDDEN);
                if (!p.sdlWindow)
                {
                    throw std::runtime_error(feather_tk::Format("Cannot create window: {0}").
                        arg(SDL_GetError()));
                }

                p.sdlGLContext = SDL_GL_CreateContext(p.sdlWindow);
                if (!p.sdlGLContext)
                {
                    throw std::runtime_error(feather_tk::Format("Cannot create OpenGL context: {0}").
                        arg(SDL_GetError()));
                }
            }
            catch (const std::exception&e)
            {
                logSystem->print(
                    "tl::usd::Render",
                    e.what(),
                    feather_tk::LogType::Error);
            }

            p.thread.logTimer = std::chrono::steady_clock::now();
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    FEATHER_TK_P();
                    if (p.sdlWindow && p.sdlGLContext)
                    {
                        SDL_GL_MakeCurrent(p.sdlWindow, p.sdlGLContext);
                    }
                    _run();
                    p.thread.stageCache.clear();
                    p.thread.diskCache.clear();
                    _finish();
                    if (p.sdlWindow && p.sdlGLContext)
                    {
                        SDL_GL_MakeCurrent(p.sdlWindow, nullptr);
                    }
                });
            
            {
                std::vector<std::string> renderers;
                for (const auto& id : UsdImagingGLEngine::GetRendererPlugins())
                {
                    renderers.push_back(UsdImagingGLEngine::GetRendererDisplayName(id));
                }
                logSystem->print(
                    "tl::usd::Render",
                    feather_tk::Format(
                        "\n"
                        "    Renderers: {0}").
                    arg(feather_tk::join(renderers, ", ")));
            }
        }

        Render::Render() :
            _p(new Private)
        {}

        Render::~Render()
        {
            FEATHER_TK_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
            if (p.sdlGLContext)
            {
                SDL_GL_DeleteContext(p.sdlGLContext);
            }
            if (p.sdlWindow)
            {
                SDL_DestroyWindow(p.sdlWindow);
            }
        }

        std::shared_ptr<Render> Render::create(
            const std::shared_ptr<feather_tk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Render>(new Render);
            out->_init(logSystem);
            return out;
        }
        
        std::future<io::Info> Render::getInfo(
            int64_t id,
            const file::Path& path,
            const io::Options& options)
        {
            FEATHER_TK_P();
            auto request = std::make_shared<Private::InfoRequest>();
            request->id = id;
            request->path = path;
            request->options = options;
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
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            FEATHER_TK_P();
            auto request = std::make_shared<Private::Request>();
            request->id = id;
            request->path = path;
            request->time = time;
            request->options = options;
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
            FEATHER_TK_P();
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
                        
        namespace
        {
            UsdGeomCamera getCamera(
                const UsdStageRefPtr& stage,
                const std::string& name = std::string())
            {
                UsdGeomCamera out;
                if (!name.empty())
                {
                    out = UsdAppUtilsGetCameraAtPath(stage, SdfPath(name));
                }
                if (!out)
                {
                    const TfToken primaryCameraName = UsdUtilsGetPrimaryCameraName();
                    out = UsdAppUtilsGetCameraAtPath(stage, SdfPath(primaryCameraName));
                }
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
            FEATHER_TK_P();
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
                        feather_tk::Format(
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
                        arg(feather_tk::join(aovs, ", ")));
                }
            }
        }
        
        void Render::_run()
        {
            FEATHER_TK_P();
                        
            TfDiagnosticMgr::GetInstance().SetQuiet(true);

            const TfTokenVector purposes({ UsdGeomTokens->default_, UsdGeomTokens->proxy });

            size_t stageCacheCount = 10;
            size_t diskCacheByteCount = 0;
            int renderWidth = 1920;
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
                        else if (!p.mutex.requests.empty())
                        {
                            request = p.mutex.requests.front();
                            p.mutex.requests.pop_front();
                        }
                    }
                }

                // Set options.
                io::Options ioOptions;
                if (infoRequest)
                {
                    ioOptions = infoRequest->options;
                }
                else if (request)
                {
                    ioOptions = request->options;
                }
                auto i = ioOptions.find("USD/StageCache");
                if (i != ioOptions.end())
                {
                    stageCacheCount = std::atoll(i->second.c_str());
                }
                i = ioOptions.find("USD/DiskCache");
                if (i != ioOptions.end())
                {
                    diskCacheByteCount = std::atoll(i->second.c_str());
                }
                p.thread.stageCache.setMax(stageCacheCount);
                p.thread.diskCache.setMax(diskCacheByteCount);
                if (diskCacheByteCount > 0 && p.thread.tempDir.empty())
                {
                    p.thread.tempDir = std::tmpnam(nullptr);
                    std::filesystem::create_directory(std::filesystem::u8path(p.thread.tempDir));
                    if (auto logSystem = p.logSystem.lock())
                    {
                        logSystem->print(
                            "tl::usd::Render",
                            feather_tk::Format(
                                "\n"
                                "    Temp directory: {0}\n"
                                "    Disk cache: {1}GB").
                            arg(p.thread.tempDir).
                            arg(diskCacheByteCount / feather_tk::gigabyte));
                    }
                }
                else if (0 == diskCacheByteCount &&
                    !p.thread.tempDir.empty())
                {
                    p.thread.tempDir = std::string();
                }

                // Handle information requests.
                i = ioOptions.find("USD/RenderWidth");
                if (i != ioOptions.end())
                {
                    renderWidth = std::atoi(i->second.c_str());
                }
                std::string cameraName;
                i = ioOptions.find("USD/CameraName");
                if (i != ioOptions.end())
                {
                    cameraName = i->second;
                }
                if (infoRequest)
                {
                    const std::string fileName = infoRequest->path.get(-1, file::PathType::Path);
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
                        auto camera = getCamera(stageCacheItem.stage, cameraName);
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
                        info.video.push_back(feather_tk::ImageInfo(
                            renderWidth,
                            renderWidth / aspectRatio,
                            feather_tk::ImageType::RGBA_F16));
                        info.videoTime = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                            OTIO_NS::RationalTime(startTimeCode, timeCodesPerSecond),
                            OTIO_NS::RationalTime(endTimeCode, timeCodesPerSecond));
                        //std::cout << fileName << " range: " << info.videoTime << std::endl;
                    }
                    infoRequest->promise.set_value(info);
                }

                // Check the disk cache.
                if (request)
                {
                    std::shared_ptr<Private::DiskCacheItem> diskCacheItem;
                    const std::string cacheKey = getCacheKey(
                        request->path,
                        request->time,
                        ioOptions);
                    if (diskCacheByteCount > 0 &&
                        p.thread.diskCache.get(cacheKey, diskCacheItem))
                    {
                        std::shared_ptr<feather_tk::Image> image;
                        try
                        {
                            //std::cout << "read temp file: " << diskCacheItem->fileName << std::endl;
                            auto fileIO = feather_tk::FileIO::create(diskCacheItem->fileName, feather_tk::FileMode::Read);
                            uint16_t w = 0;
                            uint16_t h = 0;
                            fileIO->readU16(&w);
                            fileIO->readU16(&h);
                            uint32_t pixelType = 0;
                            fileIO->readU32(&pixelType);
                            image = feather_tk::Image::create(w, h, static_cast<feather_tk::ImageType>(pixelType));
                            fileIO->read(image->getData(), image->getInfo().getByteCount());
                        }
                        catch (const std::exception& e)
                        {
                            //std::cout << e.what() << std::endl;
                            if (auto logSystem = p.logSystem.lock())
                            {
                                const std::string id = feather_tk::Format("tl::usd::Render ({0}: {1})").
                                    arg(__FILE__).
                                    arg(__LINE__);
                                logSystem->print(id, e.what(), feather_tk::LogType::Error);
                            }
                        }

                        io::VideoData videoData;
                        videoData.time = request->time;
                        videoData.image = image;
                        request->promise.set_value(videoData);
                        request.reset();
                    }
                }

                // Handle requests.
                if (request)
                {
                    std::shared_ptr<feather_tk::Image> image;
                    const std::string cacheKey = getCacheKey(
                        request->path,
                        request->time,
                        ioOptions);
                    try
                    {
                        // Check the stage cache for a previously opened stage.
                        const std::string fileName = request->path.get(-1, file::PathType::Path);
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

                            // Get options.
                            i = ioOptions.find("USD/RenderWidth");
                            if (i != ioOptions.end())
                            {
                                renderWidth = std::atoi(i->second.c_str());
                            }
                            float complexity = 1.F;
                            i = ioOptions.find("USD/Complexity");
                            if (i != ioOptions.end())
                            {
                                complexity = std::atof(i->second.c_str());
                            }
                            DrawMode drawMode = DrawMode::ShadedSmooth;
                            i = ioOptions.find("USD/DrawMode");
                            if (i != ioOptions.end())
                            {
                                std::stringstream ss(i->second);
                                ss >> drawMode;
                            }
                            bool enableLighting = true;
                            i = ioOptions.find("USD/EnableLighting");
                            if (i != ioOptions.end())
                            {
                                enableLighting = std::atoi(i->second.c_str());
                            }
                            bool sRGB = true;
                            i = ioOptions.find("USD/sRGB");
                            if (i != ioOptions.end())
                            {
                                sRGB = std::atoi(i->second.c_str());
                            }

                            // Setup the camera.
                            std::string cameraName;
                            i = ioOptions.find("USD/CameraName");
                            if (i != ioOptions.end())
                            {
                                cameraName = i->second;
                            }
                            GfCamera gfCamera;
                            auto camera = getCamera(stageCacheItem.stage, cameraName);
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
                            const size_t renderHeight = renderWidth / aspectRatio;
                            stageCacheItem.engine->SetRenderViewport(GfVec4d(
                                0.0,
                                0.0,
                                static_cast<double>(renderWidth),
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
                            renderParams.complexity = complexity;
                            renderParams.drawMode = toUSD(drawMode);
                            renderParams.enableLighting = enableLighting;
                            renderParams.clearColor = GfVec4f(0.F, 0.F, 0.F, 0.F);
                            renderParams.colorCorrectionMode = sRGB ?
                                HdxColorCorrectionTokens->sRGB :
                                HdxColorCorrectionTokens->disabled;
                            const UsdPrim& pseudoRoot = stageCacheItem.stage->GetPseudoRoot();
                            unsigned int sleepTime = 10;
                            while (p.thread.running)
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
                                        image = feather_tk::Image::create(
                                            renderWidth,
                                            renderHeight,
                                            feather_tk::ImageType::RGBA_F16);
                                        memcpy(image->getData(), mappedColorTextureBuffer.get(), image->getInfo().getByteCount());
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
                                        image = feather_tk::Image::create(
                                            renderWidth,
                                            renderHeight,
                                            feather_tk::ImageType::RGBA_F16);
                                        memcpy(image->getData(), colorRenderBuffer->Map(), image->getInfo().getByteCount());
                                        break;
                                    default: break;
                                    }
                                }
                            }

                            // Add the rendered frame to the disk cache.
                            if (diskCacheByteCount > 0 && image)
                            {
                                auto diskCacheItem = std::make_shared<Private::DiskCacheItem>();
                                diskCacheItem->fileName = feather_tk::Format("{0}/{1}.img").
                                    arg(p.thread.tempDir).
                                    arg(diskCacheItem);
                                //std::cout << "write temp file: " << diskCacheItem->fileName << std::endl;
                                auto tempFile = feather_tk::FileIO::create(diskCacheItem->fileName, feather_tk::FileMode::Write);
                                tempFile->writeU16(image->getWidth());
                                tempFile->writeU16(image->getHeight());
                                tempFile->writeU32(static_cast<uint32_t>(image->getType()));
                                const size_t byteCount = image->getInfo().getByteCount();
                                tempFile->write(image->getData(), byteCount);
                                p.thread.diskCache.add(cacheKey, diskCacheItem, byteCount);
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        //std::cout << e.what() << std::endl;
                        if (auto logSystem = p.logSystem.lock())
                        {
                            const std::string id = feather_tk::Format("tl::usd::Render ({0}: {1})").
                                arg(__FILE__).
                                arg(__LINE__);
                            logSystem->print(id, e.what(), feather_tk::LogType::Error);
                        }
                    }

                    io::VideoData videoData;
                    videoData.time = request->time;
                    videoData.image = image;
                    request->promise.set_value(videoData);
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
                                feather_tk::Format(
                                    "\n"
                                    "    Requests: {0}\n"
                                    "    Stage cache: {1}/{2}\n"
                                    "    Disk cache: {3}/{4}GB").
                                arg(requestsSize).
                                arg(p.thread.stageCache.getSize()).
                                arg(p.thread.stageCache.getMax()).
                                arg(p.thread.diskCache.getSize() / feather_tk::gigabyte).
                                arg(p.thread.diskCache.getMax() / feather_tk::gigabyte));
                        }
                    }
                }
            }
        }

        void Render::_finish()
        {
            FEATHER_TK_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::Request> > requests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.stopped = true;
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
    }
}

