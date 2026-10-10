// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/IO/WebCodecs.h>

#include <tlRender/IO/RequestQueuePrivate.h>

#include <ftk/Core/Format.h>
#include <ftk/Core/LogSystem.h>

#include <emscripten.h>
#include <emscripten/proxying.h>
#include <emscripten/threading.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cerrno>
#include <cstddef>
#include <cstring>
#include <map>
#include <mutex>
#include <thread>

namespace tl
{
    namespace webcodecs
    {
        namespace
        {
            //! The block a reader shares with the decode worker; see the
            //! layout comment in WebCodecsWorker.js, which this must
            //! match. The wasm heap is shared memory, so the worker reads
            //! and writes it directly, and the sequence numbers carry the
            //! handshake through Atomics.
            struct Control
            {
                int32_t requestSeq = 0;
                int32_t command = 0;
                double targetUs = 0.0;
                uint32_t ptr = 0;
                uint32_t cap = 0;
                int32_t responseSeq = 0;
                // 0 is RGBA_U8; 1 is planar 4:2:0, written Y then U
                // then V.
                int32_t deliveredFormat = 0;
                double deliveredTs = -1.0;
                int32_t width = 0;
                int32_t height = 0;
                int32_t frameCount = 0;
                int32_t pad2 = 0;
                double duration = 0.0;
                double frameDur = 0.0;
                // 1 asks for frames as textures: the worker hands the
                // decoded frame to the main thread, which imports it
                // into a WebGL texture and answers with its name in
                // texture, as format 2. Format -1 is an import that
                // failed, which the reader answers by asking for pixels
                // from then on.
                int32_t mode = 0;
                int32_t texture = 0;
            };
            static_assert(offsetof(Control, targetUs) == 8, "Control layout");
            static_assert(offsetof(Control, responseSeq) == 24, "Control layout");
            static_assert(offsetof(Control, deliveredFormat) == 28, "Control layout");
            static_assert(offsetof(Control, deliveredTs) == 32, "Control layout");
            static_assert(offsetof(Control, width) == 40, "Control layout");
            static_assert(offsetof(Control, duration) == 56, "Control layout");
            static_assert(offsetof(Control, frameDur) == 64, "Control layout");
            static_assert(offsetof(Control, mode) == 72, "Control layout");
            static_assert(offsetof(Control, texture) == 76, "Control layout");
            static_assert(sizeof(Control) == 80, "Control layout");

            //! What the main thread needs for the one dispatch it still
            //! performs: creating the worker and introducing a reader to
            //! it. Everything after that runs without the main thread.
            struct Registration
            {
                std::string url;
                Control* control = nullptr;
                bool audio = false;
            };
            std::mutex registryMutex;
            std::map<int, Registration> registry;
            int registryHandle = 0;
            //! A scratch buffer cannot be freed while the worker may
            //! still write into it; one that is retired parks here for
            //! good, like the control blocks.
            std::vector<std::shared_ptr<void> > parkedForever;

            EM_JS(void, wcRegister, (int handle, const char* url, void* ctrl, int audio), {
                if (!globalThis.__tlWCWorker)
                {
                    // The shell can name the worker with its build id
                    // for cache busting.
                    var worker = new Worker(
                        globalThis.__tlWCWorkerUrl || 'WebCodecsWorker.js');
                    worker.postMessage({ memory: wasmMemory });
                    // A frame wanted as a texture comes here, where the
                    // WebGL context is: the worker transfers the
                    // VideoFrame, the import is one copy on the GPU, and
                    // the frame is closed at once, which hands its
                    // surface back to the decoder. The answer the worker
                    // writes on the pixel path is written here instead.
                    // The texture joins Emscripten's table so the
                    // renderer binds it by name like its own.
                    worker.onmessage = function(event)
                    {
                        var d = event.data;
                        var i32 = new Int32Array(wasmMemory.buffer, d.ctrl, 20);
                        var f64 = new Float64Array(wasmMemory.buffer, d.ctrl, 10);
                        var id = 0;
                        if (d.frame)
                        {
                            var gl = typeof GLctx !== 'undefined' ? GLctx : null;
                            var tex = null;
                            try
                            {
                                tex = gl.createTexture();
                                var prev = gl.getParameter(gl.TEXTURE_BINDING_2D);
                                gl.bindTexture(gl.TEXTURE_2D, tex);
                                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
                                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
                                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
                                gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
                                gl.texImage2D(
                                    gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA,
                                    gl.UNSIGNED_BYTE, d.frame);
                                gl.bindTexture(gl.TEXTURE_2D, prev);
                                id = GL.getNewId(GL.textures);
                                tex.name = id;
                                GL.textures[id] = tex;
                            }
                            catch (e)
                            {
                                console.error('tl::webcodecs: import:', String(e));
                                if (gl && tex)
                                {
                                    gl.deleteTexture(tex);
                                }
                                id = 0;
                            }
                            d.frame.close();
                        }
                        i32[7] = id ? 2 : -1;
                        f64[4] = id ? d.ts : -1;
                        i32[19] = id;
                        Atomics.store(i32, 6, d.seq);
                        Atomics.notify(i32, 6);
                    };
                    globalThis.__tlWCWorker = worker;
                }
                globalThis.__tlWCWorker.postMessage({
                    handle: handle,
                    url: UTF8ToString(url),
                    ctrl: ctrl,
                    audio: 0 !== audio });
            });

            EM_JS(void, wcReleaseTexture, (int id), {
                var tex = GL.textures[id];
                if (tex)
                {
                    GLctx.deleteTexture(tex);
                    GL.textures[id] = null;
                }
            });

            void releaseTextureMain(void* arg)
            {
                wcReleaseTexture(static_cast<int>(
                    reinterpret_cast<intptr_t>(arg)));
            }

            //! Release an imported texture: on the main thread, which owns
            //! the context, from wherever the image is dropped -- the
            //! player's cache runs on its own thread.
            void releaseTexture(uintptr_t id)
            {
                if (emscripten_is_main_runtime_thread())
                {
                    releaseTextureMain(reinterpret_cast<void*>(id));
                }
                else
                {
                    emscripten_proxy_async(
                        emscripten_proxy_get_system_queue(),
                        emscripten_main_runtime_thread_id(),
                        releaseTextureMain,
                        reinterpret_cast<void*>(id));
                }
            }

            void registerMain(void* arg)
            {
                const int handle = static_cast<int>(
                    reinterpret_cast<intptr_t>(arg));
                std::string url;
                Control* control = nullptr;
                bool audio = false;
                {
                    std::unique_lock<std::mutex> lock(registryMutex);
                    const auto i = registry.find(handle);
                    if (i != registry.end())
                    {
                        url = i->second.url;
                        control = i->second.control;
                        audio = i->second.audio;
                    }
                }
                if (control)
                {
                    wcRegister(handle, url.c_str(), control, audio ? 1 : 0);
                }
            }

            //! How many times in a row a texture import may fail before
            //! the reader stops asking: the spells between tries double
            //! from one second, so this is about half a minute of trying.
            const int textureFailureMax = 6;

            //! Wait for the response sequence number to reach a value,
            //! or the timeout. Returns whether it arrived.
            bool waitResponse(Control* control, int32_t value, double timeoutMs)
            {
                for (;;)
                {
                    const int32_t current = __atomic_load_n(
                        &control->responseSeq, __ATOMIC_SEQ_CST);
                    if (current >= value)
                    {
                        return true;
                    }
                    if (emscripten_futex_wait(
                        &control->responseSeq,
                        current,
                        timeoutMs) == -ETIMEDOUT)
                    {
                        return __atomic_load_n(
                            &control->responseSeq, __ATOMIC_SEQ_CST) >= value;
                    }
                }
            }
        }

        struct VideoRead::Private
        {
            int handle = 0;
            Control* control = nullptr;

            IOInfo info;
            //! Set once the open has filled the information, which
            //! never changes afterwards, so getInfo() can answer
            //! immediately instead of queueing behind the request in
            //! flight -- the caller can be the main thread, where a
            //! blocked future busy-spins the page.
            std::atomic<bool> infoValid{ false };
            struct InfoRequest
            {
                std::promise<IOInfo> promise;
            };
            struct VideoRequest
            {
                OTIO_NS::RationalTime time;
                std::promise<VideoData> promise;
            };
            RequestCondition condition;
            RequestQueue<InfoRequest, IOInfo> infoRequests{ condition };
            RequestQueue<VideoRequest, VideoData> videoRequests{ condition };

            //! The one buffer the worker ever writes into. A late
            //! write after a timeout can land arbitrarily long after
            //! the request -- a stalled fetch completes when it
            //! completes -- so the answer is copied out on success
            //! rather than decoded in place, and this buffer never
            //! moves while the reader lives.
            std::shared_ptr<ftk::Image> scratch;

            //! Bumped by cancelRequests(): an in-flight wait whose
            //! generation is stale serves a request nobody wants
            //! anymore, and yields to the fresh one.
            std::atomic<int> cancelGen{ 0 };

            //! Whether frames are asked for as textures (the "Texture"
            //! option). An import can fail for a while -- the page was
            //! hidden, the GPU was short of memory -- so a failure means
            //! pixels for a spell that doubles each time, up to a limit,
            //! and then textures are tried again; a browser that never
            //! manages it is given up on after the last spell, since
            //! each try restarts the decode.
            bool texture = false;
            int textureFailures = 0;
            std::chrono::steady_clock::time_point textureRetryTime;

            //! What a frame costs end to end, logged now and then: the
            //! two paths are compared by this.
            size_t statsFrames = 0;
            double statsMs = 0.0;
            std::chrono::steady_clock::time_point statsTime;

            std::thread thread;

            struct ErrorMutex
            {
                std::string error;
                size_t count = 0;
                std::mutex mutex;
            };
            ErrorMutex errorMutex;
        };

        void VideoRead::_init(
            const ftk::Path& path,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IRead::_init(path, {}, options, logSystem);
            FTK_P();

            if (auto i = options.find("Texture"); i != options.end())
            {
                p.texture = "1" == i->second;
            }
            p.statsTime = std::chrono::steady_clock::now();

            p.control = new Control;
            {
                std::unique_lock<std::mutex> lock(registryMutex);
                p.handle = ++registryHandle;
                // The page fetches the path as a URL.
                registry[p.handle] = { path.get(), p.control, false };
            }

            p.thread = std::thread(
                [this]
                {
                    FTK_P();
                    try
                    {
                        emscripten_proxy_async(
                            emscripten_proxy_get_system_queue(),
                            emscripten_main_runtime_thread_id(),
                            registerMain,
                            reinterpret_cast<void*>(
                                static_cast<intptr_t>(p.handle)));
                        // The open covers the fetch and the demux, so it
                        // gets a generous timeout.
                        if (!waitResponse(p.control, 1, 30000.0) ||
                            p.control->width <= 0 ||
                            p.control->frameCount <= 0)
                        {
                            throw std::runtime_error(
                                "Cannot open: " + _path.get());
                        }
                        const double rate =
                            p.control->duration > 0.0 ?
                            p.control->frameCount /
                                (p.control->duration / 1000000.0) :
                            24.0;
                        p.info.video.push_back(ftk::ImageInfo(
                            p.control->width,
                            p.control->height,
                            ftk::ImageType::RGBA_U8));
                        p.info.videoTime = OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(0.0, rate),
                            OTIO_NS::RationalTime(
                                p.control->frameCount, rate));
                        p.infoValid = true;

                        _run();
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::webcodecs::VideoRead",
                                e.what(),
                                ftk::LogType::Error);
                        }
                        std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
                        ++p.errorMutex.count;
                        if (p.errorMutex.error.empty())
                        {
                            p.errorMutex.error = e.what();
                        }
                    }

                    p.condition.stopQueues();
                });
        }

        VideoRead::VideoRead() :
            _p(new Private)
        {}

        VideoRead::~VideoRead()
        {
            FTK_P();
            p.condition.stop();
            if (p.thread.joinable())
            {
                p.thread.join();
            }
            p.control->command = 2;
            __atomic_add_fetch(&p.control->requestSeq, 1, __ATOMIC_SEQ_CST);
            emscripten_futex_wake(&p.control->requestSeq, 1);
            {
                std::unique_lock<std::mutex> lock(registryMutex);
                if (p.scratch)
                {
                    parkedForever.push_back(p.scratch);
                }
                registry.erase(p.handle);
            }
            // The block leaks by design: the worker may still read it
            // after the close lands, and it is 80 bytes per file opened.
        }

        std::shared_ptr<VideoRead> VideoRead::create(
            const ftk::Path& path,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<VideoRead>(new VideoRead);
            out->_init(path, options, logSystem);
            return out;
        }

        std::future<IOInfo> VideoRead::getInfo()
        {
            FTK_P();
            if (p.infoValid)
            {
                std::promise<IOInfo> promise;
                promise.set_value(p.info);
                return promise.get_future();
            }
            return p.infoRequests.push(
                std::make_shared<Private::InfoRequest>());
        }

        std::future<VideoData> VideoRead::readVideo(
            const OTIO_NS::RationalTime& time,
            const IOOptions&)
        {
            FTK_P();
            auto request = std::make_shared<Private::VideoRequest>();
            request->time = time;
            return p.videoRequests.push(request);
        }

        void VideoRead::cancelRequests()
        {
            FTK_P();
            ++p.cancelGen;
            p.infoRequests.cancel();
            p.videoRequests.cancel();
        }

        std::string VideoRead::getError() const
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
            return p.errorMutex.error;
        }

        size_t VideoRead::getErrorCount() const
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
            return p.errorMutex.count;
        }

        void VideoRead::_run()
        {
            FTK_P();
            int32_t seq = 1;
            while (p.condition.wait())
            {
                for (const auto& request : p.infoRequests.popAll())
                {
                    request->promise.set_value(p.info);
                }

                if (auto videoRequest = p.videoRequests.pop())
                {
                    PromiseGuard<VideoData> guard(videoRequest->promise);

                    if (!p.scratch)
                    {
                        p.scratch = ftk::Image::create(p.info.video[0]);
                    }
                    VideoData data;
                    data.time = videoRequest->time;
                    // Once more as pixels when the texture path fails.
                    bool again = true;
                    while (again)
                    {
                        again = false;
                        // Half a frame of margin so that a frame's own
                        // exact time cannot round to just before it.
                        const double tUs =
                            videoRequest->time.value() /
                                videoRequest->time.rate() * 1000000.0 +
                            p.control->frameDur / 2.0;
                        p.control->targetUs = tUs;
                        p.control->ptr = reinterpret_cast<uintptr_t>(
                            p.scratch->getData());
                        p.control->cap = p.scratch->getByteCount();
                        const auto now = std::chrono::steady_clock::now();
                        const bool asTexture = p.texture &&
                            (0 == p.textureFailures ||
                                (p.textureFailures < textureFailureMax &&
                                    now >= p.textureRetryTime));
                        p.control->mode = asTexture ? 1 : 0;
                        p.control->command = 1;
                        ++seq;
                        __atomic_store_n(
                            &p.control->requestSeq, seq, __ATOMIC_SEQ_CST);
                        emscripten_futex_wake(&p.control->requestSeq, 1);

                        // The wait is sliced so a cancellation -- a
                        // scrub has moved on -- abandons this request
                        // within a slice. The total stays short: in
                        // Firefox a pending wait stalls the page
                        // (unexplained), so every wait is a bound on
                        // that stall; empty answers are retried by the
                        // callers.
                        const auto t0 = std::chrono::steady_clock::now();
                        const int gen = p.cancelGen;
                        bool delivered = false;
                        for (int i = 0; i < 10 && !delivered; ++i)
                        {
                            delivered = waitResponse(p.control, seq, 100.0);
                            if (!delivered && gen != p.cancelGen)
                            {
                                break;
                            }
                        }
                        if (delivered && p.control->deliveredTs >= 0.0)
                        {
                            switch (p.control->deliveredFormat)
                            {
                            case 2:
                            {
                                if (p.textureFailures > 0)
                                {
                                    p.textureFailures = 0;
                                    if (auto logSystem = _logSystem.lock())
                                    {
                                        logSystem->print(
                                            "tl::webcodecs::VideoRead",
                                            "Frames are textures again");
                                    }
                                }
                                // The frame is a texture on the main
                                // thread, top down as the browser keeps
                                // it; the image is that texture, and
                                // releases it there.
                                ftk::ImageInfo info = p.info.video[0];
                                info.layout.mirror.y = true;
                                data.image = ftk::Image::create(
                                    info,
                                    p.control->texture,
                                    releaseTexture);
                                break;
                            }
                            case 1:
                            {
                                // The worker delivered the decoder's
                                // native planes; the renderer draws
                                // planar YUV itself.
                                auto yuv = ftk::Image::create(ftk::ImageInfo(
                                    p.info.video[0].size,
                                    ftk::ImageType::YUV_420P_U8));
                                memcpy(
                                    yuv->getData(),
                                    p.scratch->getData(),
                                    yuv->getByteCount());
                                data.image = yuv;
                                break;
                            }
                            default:
                            {
                                auto image = ftk::Image::create(
                                    p.info.video[0]);
                                memcpy(
                                    image->getData(),
                                    p.scratch->getData(),
                                    image->getByteCount());
                                data.image = image;
                                break;
                            }
                            }
                            const auto t1 = std::chrono::steady_clock::now();
                            ++p.statsFrames;
                            p.statsMs += std::chrono::duration<double, std::milli>(
                                t1 - t0).count();
                            if (t1 - p.statsTime > std::chrono::seconds(5))
                            {
                                if (auto logSystem = _logSystem.lock())
                                {
                                    logSystem->print(
                                        "tl::webcodecs::VideoRead",
                                        ftk::Format("{0} frames, {1} ms each, as {2}").
                                            arg(p.statsFrames).
                                            arg(p.statsMs / p.statsFrames, 1).
                                            arg(asTexture ? "textures" : "pixels"));
                                }
                                p.statsFrames = 0;
                                p.statsMs = 0.0;
                                p.statsTime = t1;
                            }
                        }
                        else if (delivered && asTexture &&
                            -1 == p.control->deliveredFormat)
                        {
                            // The browser did not import the frame into a
                            // texture: pixels for a spell, this frame
                            // included.
                            ++p.textureFailures;
                            const int seconds = std::min(
                                1 << std::min(p.textureFailures - 1, 5), 30);
                            p.textureRetryTime = std::chrono::steady_clock::now() +
                                std::chrono::seconds(seconds);
                            if (auto logSystem = _logSystem.lock())
                            {
                                logSystem->print(
                                    "tl::webcodecs::VideoRead",
                                    p.textureFailures < textureFailureMax ?
                                        ftk::Format(
                                            "A frame could not be imported as a "
                                            "texture; frames are copied for {0} "
                                            "seconds").arg(seconds).str() :
                                        std::string(
                                            "Frames cannot be imported as "
                                            "textures; they are copied from "
                                            "here on"),
                                    ftk::LogType::Warning);
                            }
                            again = true;
                        }
                    }
                    guard.setValue(std::move(data));
                }
            }
        }


        struct AudioRead::Private
        {
            int handle = 0;
            Control* control = nullptr;

            IOInfo info;
            //! See VideoRead::Private::infoValid; media without an
            //! audio track sets it too, since empty is the answer.
            std::atomic<bool> infoValid{ false };
            struct InfoRequest
            {
                std::promise<IOInfo> promise;
            };
            struct AudioRequest
            {
                OTIO_NS::TimeRange timeRange;
                std::promise<AudioData> promise;
            };
            RequestCondition condition;
            RequestQueue<InfoRequest, IOInfo> infoRequests{ condition };
            RequestQueue<AudioRequest, AudioData> audioRequests{ condition };

            //! See VideoRead::Private::scratch; grown when a request
            //! wants more than it holds.
            std::shared_ptr<Audio> scratch;

            //! See VideoRead::Private::cancelGen.
            std::atomic<int> cancelGen{ 0 };

            std::thread thread;

            struct ErrorMutex
            {
                std::string error;
                size_t count = 0;
                std::mutex mutex;
            };
            ErrorMutex errorMutex;
        };

        void AudioRead::_init(
            const ftk::Path& path,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IRead::_init(path, {}, options, logSystem);
            FTK_P();

            p.control = new Control;
            {
                std::unique_lock<std::mutex> lock(registryMutex);
                p.handle = ++registryHandle;
                registry[p.handle] = { path.get(), p.control, true };
            }

            p.thread = std::thread(
                [this]
                {
                    FTK_P();
                    try
                    {
                        emscripten_proxy_async(
                            emscripten_proxy_get_system_queue(),
                            emscripten_main_runtime_thread_id(),
                            registerMain,
                            reinterpret_cast<void*>(
                                static_cast<intptr_t>(p.handle)));
                        // The open covers the fetch, the demux, and the
                        // whole track's decode.
                        if (!waitResponse(p.control, 1, 60000.0))
                        {
                            throw std::runtime_error(
                                "Cannot open: " + _path.get());
                        }
                        // No audio track is an answer rather than an
                        // error; the information stays empty.
                        if (p.control->width > 0 &&
                            p.control->height > 0 &&
                            p.control->frameCount > 0)
                        {
                            p.info.audio = AudioInfo(
                                p.control->height,
                                AudioType::F32,
                                p.control->width);
                            p.info.audioTime = OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(
                                    0.0, p.control->width),
                                OTIO_NS::RationalTime(
                                    p.control->frameCount,
                                    p.control->width));
                        }
                        p.infoValid = true;

                        _run();
                    }
                    catch (const std::exception& e)
                    {
                        if (auto logSystem = _logSystem.lock())
                        {
                            logSystem->print(
                                "tl::webcodecs::AudioRead",
                                e.what(),
                                ftk::LogType::Error);
                        }
                        std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
                        ++p.errorMutex.count;
                        if (p.errorMutex.error.empty())
                        {
                            p.errorMutex.error = e.what();
                        }
                    }

                    p.condition.stopQueues();
                });
        }

        AudioRead::AudioRead() :
            _p(new Private)
        {}

        AudioRead::~AudioRead()
        {
            FTK_P();
            p.condition.stop();
            if (p.thread.joinable())
            {
                p.thread.join();
            }
            p.control->command = 2;
            __atomic_add_fetch(&p.control->requestSeq, 1, __ATOMIC_SEQ_CST);
            emscripten_futex_wake(&p.control->requestSeq, 1);
            {
                std::unique_lock<std::mutex> lock(registryMutex);
                if (p.scratch)
                {
                    parkedForever.push_back(p.scratch);
                }
                registry.erase(p.handle);
            }
            // The block leaks by design, like the video reader's.
        }

        std::shared_ptr<AudioRead> AudioRead::create(
            const ftk::Path& path,
            const IOOptions& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<AudioRead>(new AudioRead);
            out->_init(path, options, logSystem);
            return out;
        }

        std::future<IOInfo> AudioRead::getInfo()
        {
            FTK_P();
            if (p.infoValid)
            {
                std::promise<IOInfo> promise;
                promise.set_value(p.info);
                return promise.get_future();
            }
            return p.infoRequests.push(
                std::make_shared<Private::InfoRequest>());
        }

        std::future<AudioData> AudioRead::readAudio(
            const OTIO_NS::TimeRange& timeRange,
            const IOOptions&)
        {
            FTK_P();
            auto request = std::make_shared<Private::AudioRequest>();
            request->timeRange = timeRange;
            return p.audioRequests.push(request);
        }

        void AudioRead::cancelRequests()
        {
            FTK_P();
            ++p.cancelGen;
            p.infoRequests.cancel();
            p.audioRequests.cancel();
        }

        std::string AudioRead::getError() const
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
            return p.errorMutex.error;
        }

        size_t AudioRead::getErrorCount() const
        {
            FTK_P();
            std::unique_lock<std::mutex> lock(p.errorMutex.mutex);
            return p.errorMutex.count;
        }

        void AudioRead::_run()
        {
            FTK_P();
            int32_t seq = 1;
            while (p.condition.wait())
            {
                for (const auto& request : p.infoRequests.popAll())
                {
                    request->promise.set_value(p.info);
                }

                if (auto audioRequest = p.audioRequests.pop())
                {
                    PromiseGuard<AudioData> guard(audioRequest->promise);

                    AudioData data;
                    data.time = audioRequest->timeRange.start_time();
                    if (p.info.audio.isValid())
                    {
                        const double rate = p.info.audio.sampleRate;
                        const size_t count = std::max(
                            0.0,
                            audioRequest->timeRange.duration().
                                rescaled_to(rate).value());
                        if (!p.scratch || p.scratch->getSampleCount() < count)
                        {
                            if (p.scratch)
                            {
                                // The worker may still write the old one.
                                std::unique_lock<std::mutex> lock(
                                    registryMutex);
                                parkedForever.push_back(p.scratch);
                            }
                            p.scratch = Audio::create(p.info.audio, count);
                        }
                        p.control->targetUs =
                            audioRequest->timeRange.start_time().
                                rescaled_to(rate).value();
                        p.control->ptr = reinterpret_cast<uintptr_t>(
                            p.scratch->getData());
                        p.control->cap =
                            count * p.info.audio.getByteCount();
                        p.control->command = 1;
                        ++seq;
                        __atomic_store_n(
                            &p.control->requestSeq, seq, __ATOMIC_SEQ_CST);
                        emscripten_futex_wake(&p.control->requestSeq, 1);

                        // Sliced like the video wait.
                        const int gen = p.cancelGen;
                        bool delivered = false;
                        for (int i = 0; i < 10 && !delivered; ++i)
                        {
                            delivered = waitResponse(p.control, seq, 100.0);
                            if (!delivered && gen != p.cancelGen)
                            {
                                break;
                            }
                        }
                        if (delivered && p.control->deliveredTs >= 0.0)
                        {
                            auto audio = Audio::create(p.info.audio, count);
                            memcpy(
                                audio->getData(),
                                p.scratch->getData(),
                                audio->getByteCount());
                            data.audio = audio;
                        }
                    }
                    guard.setValue(std::move(data));
                }
            }
        }

        void ReadPlugin::_init(const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IReadPlugin::_init(
                "WebCodecs",
                {
                    { ".mp4", FileType::Media },
                    { ".m4v", FileType::Media },
                    { ".mov", FileType::Media }
                },
                logSystem);
        }

        ReadPlugin::ReadPlugin()
        {}

        ReadPlugin::~ReadPlugin()
        {}

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<IVideoRead> ReadPlugin::videoRead(
            const ftk::Path& path,
            const std::vector<ftk::MemFile>&,
            const IOOptions& options)
        {
            return VideoRead::create(path, options, _logSystem.lock());
        }

        std::shared_ptr<IAudioRead> ReadPlugin::audioRead(
            const ftk::Path& path,
            const std::vector<ftk::MemFile>&,
            const IOOptions& options)
        {
            return AudioRead::create(path, options, _logSystem.lock());
        }
    }
}
