// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlRender/C/Engine.h>

#include <tlRender/GPU/Render.h>
#include <tlRender/Timeline/CompareOptions.h>
#include <tlRender/Timeline/Init.h>
#include <tlRender/Timeline/Player.h>
#include <tlRender/Timeline/Timeline.h>
#include <tlRender/UI/Init.h>
#include <tlRender/UI/TimelineWidget.h>

#include <ftk/UI/Event.h>
#include <ftk/UI/IWindow.h>
#include <ftk/UI/IconSystem.h>
#include <ftk/UI/Style.h>

#include <ftk/GPU/OffscreenBuffer.h>
#include <ftk/GPU/Present.h>
#include <ftk/GPU/Render.h>
#include <ftk/GPU/System.h>
#include <ftk/Core/Context.h>
#include <ftk/Core/FontSystem.h>
#include <ftk/Core/Format.h>
#include <ftk/Core/LogSystem.h>
#include <ftk/Core/Matrix.h>
#include <ftk/Core/OS.h>
#include <ftk/Core/ObservableList.h>

#include <SDL3/SDL.h>
// For SDL_SetMainReady() only: SDL did not start this program, and the
// main it would otherwise insist on is the application's own.
#define SDL_MAIN_HANDLED
#include <SDL3/SDL_main.h>

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif // __APPLE__

#include <algorithm>
#include <cstdio>
#include <string>

struct tl_engine
{
    std::shared_ptr<ftk::Context> context;
    std::shared_ptr<ftk::gpu::System> gpu;
    std::string error;
    std::shared_ptr<ftk::ListObserver<ftk::LogItem> > logObserver;
};

struct tl_player
{
    tl_engine* engine = nullptr;
    std::shared_ptr<tl::Timeline> timeline;
    std::shared_ptr<tl::Player> player;
};

namespace
{
    //! A feather-tk window with no window of its own: the widgets in it
    //! are drawn into the view's buffer after the picture, and get their
    //! events from the view. The window is the whole buffer, with no
    //! background, and the timeline widget is placed in it where the
    //! view says.
    class WidgetHost : public ftk::IWindow
    {
    protected:
        void _init(const std::shared_ptr<ftk::Context>& context)
        {
            IWindow::_init(context, nullptr, "tl_view");
            setBackgroundRole(ftk::ColorRole::None);
            _style = ftk::Style::create(context);
            _fontSystem = context->getSystem<ftk::FontSystem>();
            _iconSystem = context->getSystem<ftk::IconSystem>();
            _timeline = tl::ui::TimelineWidget::create(context, shared_from_this());
            _timeline->setFrameView(true);
            _timeline->setVisible(false);
        }

    public:
        static std::shared_ptr<WidgetHost> create(const std::shared_ptr<ftk::Context>& context)
        {
            auto out = std::shared_ptr<WidgetHost>(new WidgetHost);
            out->_init(context);
            return out;
        }

        uint32_t getID() const override { return 0; }
        int getScreen() const override { return 0; }

        void setPlayer(const std::shared_ptr<tl::Player>& player)
        {
            _timeline->setPlayer(player);
        }

        //! The buffer the window is drawn into, and where in it the
        //! timeline widget goes.
        void setFrames(const ftk::Size2I& bufferSize, const ftk::Box2I& timelineFrame, float displayScale)
        {
            if (bufferSize != getBufferSize())
            {
                _setSize(bufferSize, bufferSize);
            }
            setDisplayScale(displayScale);
            if (timelineFrame != _timelineFrame)
            {
                _timelineFrame = timelineFrame;
                _timeline->setVisible(_timelineFrame.isValid());
                setSizeUpdate();
            }
        }

        void setGeometry(const ftk::Box2I& value) override
        {
            IWindow::setGeometry(value);
            _timeline->setGeometry(_timelineFrame);
        }

        void tick()
        {
            ftk::TickEvent event;
            _tickRecursive(shared_from_this(), true, true, event);
        }

        //! Lay the widgets out and draw them; the render is between its
        //! begin and end, with the buffer's transform.
        void draw(const std::shared_ptr<ftk::IRender>& render)
        {
            IWindow::_update(_fontSystem, _iconSystem, _style);
            const ftk::Box2I drawRect(ftk::V2I(), getBufferSize());
            render->setClipRectEnabled(false);
            render->setClipRect(drawRect);
            ftk::DrawEvent event(_fontSystem, _iconSystem, getDisplayScale(), _style, render);
            _drawEventRecursive(shared_from_this(), drawRect, event);
            render->setClipRectEnabled(false);
        }

    private:
        void _tickRecursive(
            const std::shared_ptr<ftk::IWidget>& widget,
            bool visible,
            bool enabled,
            const ftk::TickEvent& event)
        {
            const bool parentsVisible = visible && widget->isVisible(false);
            const bool parentsEnabled = enabled && widget->isEnabled(false);
            for (const auto& child : widget->getChildren())
            {
                _tickRecursive(child, parentsVisible, parentsEnabled, event);
            }
            widget->tickEvent(visible, enabled, event);
        }

        std::shared_ptr<ftk::Style> _style;
        std::shared_ptr<ftk::FontSystem> _fontSystem;
        std::shared_ptr<ftk::IconSystem> _iconSystem;
        std::shared_ptr<tl::ui::TimelineWidget> _timeline;
        ftk::Box2I _timelineFrame;
    };
}

struct tl_view
{
    tl_engine* engine = nullptr;
    SDL_Window* window = nullptr;
    bool claimed = false;
    ftk::gpu::Composition composition = ftk::gpu::Composition::SDR;
    std::shared_ptr<ftk::gpu::Present> present;
    std::shared_ptr<tl::gpu::Render> render;
    std::shared_ptr<ftk::gpu::OffscreenBuffer> buffer;
    tl_player* player = nullptr;
    //! Where the picture goes, in pixels; empty for the whole view.
    ftk::Box2I frame;
    //! The widgets drawn over it, and where the timeline widget goes.
    std::shared_ptr<WidgetHost> widgets;
    ftk::Box2I timelineFrame;
    float displayScale = 1.F;
};

namespace
{
    void setError(tl_engine* engine, const std::string& what)
    {
        engine->error = what;
        if (engine->context)
        {
            engine->context->log("tl_engine", what, ftk::LogType::Error);
        }
    }
}

extern "C"
{

tl_engine* tl_engine_create(void)
{
    auto engine = new tl_engine;
    try
    {
        // SDL did not start this program, which on iOS it checks for:
        // said before anything initializes SDL, which tl::init does for
        // audio.
        SDL_SetMainReady();
        engine->context = ftk::Context::create();
        // The widgets as well as the engine: the timeline widget draws
        // in the view.
        tl::ui::init(engine->context);
        // The log goes to stderr, where Xcode and the simulator show it:
        // a failure below says why there, not just that it failed.
        engine->logObserver = ftk::ListObserver<ftk::LogItem>::create(
            engine->context->getSystem<ftk::LogSystem>()->observeLogItems(),
            [](const std::vector<ftk::LogItem>& items)
            {
                for (const auto& item : items)
                {
                    std::fprintf(stderr, "%s\n", ftk::getLabel(item).c_str());
                }
            });
        // The GPU renderer is the only one here: there is no OpenGL on
        // iOS, and a Swift view is a Metal layer.
        if (!ftk::gpu::start(engine->context))
        {
            throw std::runtime_error("The GPU renderer cannot start");
        }
        engine->gpu = engine->context->getSystem<ftk::gpu::System>();
    }
    catch (const std::exception& e)
    {
        setError(engine, e.what());
        // Kept, so the caller can read the error; nothing else works.
        engine->gpu.reset();
    }
    return engine;
}

void tl_engine_destroy(tl_engine* engine)
{
    delete engine;
}

void tl_engine_tick(tl_engine* engine)
{
    if (engine && engine->context)
    {
        engine->context->tick();
    }
}

const char* tl_engine_get_error(tl_engine* engine)
{
    return engine ? engine->error.c_str() : "";
}

tl_player* tl_player_create(tl_engine* engine, const char* path)
{
    if (!engine || !engine->gpu || !path)
        return nullptr;
    auto out = new tl_player;
    out->engine = engine;
    try
    {
        // The GPU decodes what it can, which on a phone or a tablet is
        // the difference between playing and not; the reader falls back
        // to software for the rest.
        tl::Options timelineOptions;
        timelineOptions.ioOptions["FFmpeg/HWAccel"] = "1";
        out->timeline = tl::Timeline::create(engine->context, ftk::Path(path), timelineOptions);
        tl::PlayerOptions options;
#if defined(__APPLE__) && TARGET_OS_IOS
        // The desktop cache default is as big as an iPad's whole memory,
        // and iOS kills an application that takes much of it: a quarter
        // of the memory for video, and audio in proportion.
        const size_t ram = ftk::getSysInfo().ram;
        if (ram > 0)
        {
            const float ramGB = ram / float(1024 * 1024 * 1024);
            options.cache.videoGB = ramGB / 4.F;
            options.cache.audioGB = ramGB / 32.F;
        }
#endif // TARGET_OS_IOS
        out->player = tl::Player::create(engine->context, out->timeline, options);
    }
    catch (const std::exception& e)
    {
        setError(engine, e.what());
        delete out;
        out = nullptr;
    }
    return out;
}

void tl_player_destroy(tl_player* player)
{
    delete player;
}

double tl_player_get_duration(tl_player* player)
{
    if (!player)
        return 0.0;
    const auto& range = player->player->getTimeRange();
    return range.duration().rescaled_to(1.0).value();
}

double tl_player_get_rate(tl_player* player)
{
    return player ? player->player->getTimeRange().duration().rate() : 0.0;
}

double tl_player_get_time(tl_player* player)
{
    if (!player)
        return 0.0;
    const auto& range = player->player->getTimeRange();
    const auto time = player->player->observeCurrentTime()->get();
    return (time - range.start_time()).rescaled_to(1.0).value();
}

void tl_player_seek(tl_player* player, double seconds)
{
    if (!player)
        return;
    const auto& range = player->player->getTimeRange();
    const double rate = range.duration().rate();
    player->player->seek(
        range.start_time() +
        OTIO_NS::RationalTime(seconds * rate, rate).round());
}

void tl_player_set_playback(tl_player* player, tl_playback value)
{
    if (!player)
        return;
    tl::Playback playback = tl::Playback::Stop;
    switch (value)
    {
    case TL_PLAYBACK_FORWARD: playback = tl::Playback::Forward; break;
    case TL_PLAYBACK_REVERSE: playback = tl::Playback::Reverse; break;
    default: break;
    }
    player->player->setPlayback(playback);
}

tl_player_stats tl_player_get_stats(tl_player* player)
{
    tl_player_stats out = {};
    if (!player)
        return out;
    const auto& p = player->player;
    out.speed = p->getSpeed();
    out.actualSpeed = p->getActualSpeed();
    out.droppedFrames = p->getDroppedFrames();
    const auto time = p->observeCurrentTime()->get();
    const auto info = p->observeCacheInfo()->get();
    for (const auto& range : info.video)
    {
        if (range.contains(time))
        {
            out.cacheAhead = (range.end_time_exclusive() - time).rescaled_to(1.0).value();
            break;
        }
    }
    const auto& options = p->getCacheOptions();
    out.videoCachePercent = info.videoPercentage;
    out.videoCacheGB = options.videoGB;
    out.audioCachePercent = info.audioPercentage;
    out.audioCacheGB = options.audioGB;
    return out;
}

tl_playback tl_player_get_playback(tl_player* player)
{
    if (!player)
        return TL_PLAYBACK_STOP;
    switch (player->player->observePlayback()->get())
    {
    case tl::Playback::Forward: return TL_PLAYBACK_FORWARD;
    case tl::Playback::Reverse: return TL_PLAYBACK_REVERSE;
    default: return TL_PLAYBACK_STOP;
    }
}

int tl_player_get_video_width(tl_player* player)
{
    if (!player)
        return 0;
    const auto& info = player->player->getIOInfo();
    return !info.video.empty() ? info.video.front().size.w : 0;
}

int tl_player_get_video_height(tl_player* player)
{
    if (!player)
        return 0;
    const auto& info = player->player->getIOInfo();
    return !info.video.empty() ? info.video.front().size.h : 0;
}

namespace
{
    //! Finish a view once SDL has a window for it: claim the window for
    //! the device and make the renderer.
    tl_view* viewInit(tl_engine* engine, SDL_PropertiesID props)
    {
        auto out = new tl_view;
        out->engine = engine;
        try
        {
            SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_METAL_BOOLEAN, true);
            SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIGH_PIXEL_DENSITY_BOOLEAN, true);
            out->window = SDL_CreateWindowWithProperties(props);
            SDL_DestroyProperties(props);
            if (!out->window)
            {
                throw std::runtime_error(std::string("Cannot make the view: ") + SDL_GetError());
            }
            SDL_GPUDevice* device = engine->gpu->getDevice();
            out->claimed = SDL_ClaimWindowForGPUDevice(device, out->window);
            if (!out->claimed)
            {
                throw std::runtime_error(std::string("Cannot claim the view: ") + SDL_GetError());
            }
            out->present = ftk::gpu::Present::create(engine->gpu);
            out->composition = ftk::gpu::setComposition(
                engine->gpu,
                out->window,
                ftk::gpu::getComposition(engine->gpu, out->window));
            out->render = tl::gpu::Render::create(
                engine->gpu,
                engine->context->getLogSystem(),
                engine->context->getSystem<ftk::FontSystem>());
            out->widgets = WidgetHost::create(engine->context);
            engine->context->log(
                "tl_view",
                ftk::Format("Swapchain: {0}").arg(ftk::gpu::getLabel(out->composition)));
        }
        catch (const std::exception& e)
        {
            setError(engine, e.what());
            tl_view_destroy(out);
            out = nullptr;
        }
        return out;
    }
}

tl_view* tl_view_create(tl_engine* engine, void* nsWindow, void* nsView)
{
    if (!engine || !engine->gpu)
        return nullptr;
    // SDL wraps the native view rather than making a window of its own:
    // the swapchain is its, the layout and the events stay the
    // application's.
    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_WINDOW_POINTER, nsWindow);
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_COCOA_VIEW_POINTER, nsView);
    return viewInit(engine, props);
}

tl_view* tl_view_create_ios(tl_engine* engine, void* uiWindowScene)
{
    if (!engine || !engine->gpu)
        return nullptr;
    // SDL makes a window of its own in the scene; the application
    // layers it under its own and tells the view where the picture
    // goes.
    const SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WINDOWSCENE_POINTER, uiWindowScene);
    return viewInit(engine, props);
}

void* tl_view_get_native_window(tl_view* view)
{
    if (!view || !view->window)
        return nullptr;
    const SDL_PropertiesID props = SDL_GetWindowProperties(view->window);
#if TARGET_OS_IOS
    return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_UIKIT_WINDOW_POINTER, nullptr);
#else
    return SDL_GetPointerProperty(props, SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#endif
}

void tl_view_set_frame(tl_view* view, int x, int y, int w, int h)
{
    if (view)
    {
        view->frame = ftk::Box2I(x, y, w, h);
    }
}


void tl_view_set_timeline_frame(tl_view* view, int x, int y, int w, int h, float displayScale)
{
    if (view)
    {
        view->timelineFrame = ftk::Box2I(x, y, w, h);
        view->displayScale = displayScale;
    }
}

void tl_view_touch(tl_view* view, tl_touch_phase phase, int x, int y)
{
    if (!view || !view->widgets)
        return;
    const ftk::V2I pos(x, y);
    switch (phase)
    {
    case TL_TOUCH_BEGAN: view->widgets->press(pos); break;
    case TL_TOUCH_MOVED: view->widgets->hover(pos); break;
    case TL_TOUCH_ENDED:
    case TL_TOUCH_CANCELLED: view->widgets->release(pos); break;
    }
}

void tl_view_destroy(tl_view* view)
{
    if (!view)
        return;
    view->widgets.reset();
    view->buffer.reset();
    view->render.reset();
    view->present.reset();
    if (view->window)
    {
        if (view->claimed)
        {
            SDL_ReleaseWindowFromGPUDevice(view->engine->gpu->getDevice(), view->window);
        }
        SDL_DestroyWindow(view->window);
    }
    delete view;
}

void tl_view_set_player(tl_view* view, tl_player* player)
{
    if (view)
    {
        view->player = player;
        if (view->widgets)
        {
            view->widgets->setPlayer(player ? player->player : nullptr);
        }
    }
}

void tl_view_draw(tl_view* view)
{
    if (!view || !view->render)
        return;
    int w = 0;
    int h = 0;
    SDL_GetWindowSizeInPixels(view->window, &w, &h);
    if (w <= 0 || h <= 0)
        return;
    const ftk::Size2I size(w, h);
    try
    {
        auto& gpu = view->engine->gpu;
        if (!view->buffer || view->buffer->getSize() != size)
        {
            view->buffer = ftk::gpu::OffscreenBuffer::create(
                gpu, size, ftk::gpu::BufferType::RGBA_F16);
        }

        std::vector<tl::VideoFrame> frames;
        if (view->player)
        {
            frames = view->player->player->observeCurrentVideo()->get();
        }

        // The picture fitted to the frame, or the whole view, as the
        // viewport frames it.
        const ftk::Box2I frame = view->frame.isValid() ?
            view->frame :
            ftk::Box2I(0, 0, w, h);
        const tl::CompareOptions compare;
        const tl::AspectRatioOptions aspect;
        const auto boxes = tl::getBoxes(compare, aspect, frames);
        const ftk::Size2I renderSize = tl::getRenderSize(compare, aspect, frames);
        float zoom = 1.F;
        ftk::V2I pos(frame.x(), frame.y());
        if (renderSize.isValid())
        {
            zoom = std::min(
                static_cast<float>(frame.w()) / renderSize.w,
                static_cast<float>(frame.h()) / renderSize.h);
            pos.x += (frame.w() - renderSize.w * zoom) / 2.F;
            pos.y += (frame.h() - renderSize.h * zoom) / 2.F;
        }
        const ftk::M44F vm =
            ftk::translate(ftk::V3F(pos.x, pos.y, 0.F)) *
            ftk::scale(ftk::V3F(zoom, zoom, 1.F));
        const ftk::M44F pm = ftk::ortho(
            0.F, static_cast<float>(w), static_cast<float>(h), 0.F, -1.F, 1.F);

        auto base = view->render->getBaseRender();
        base->setTarget(view->buffer);
        view->render->begin(size);
        view->render->setTransform(pm * vm);
        view->render->drawVideo(
            frames,
            boxes,
            {},
            {},
            compare,
            ftk::ImageType::RGBA_F16);
        // The widgets over the picture, in the buffer's own coordinates.
        if (view->widgets)
        {
            view->render->setTransform(pm);
            view->widgets->setFrames(size, view->timelineFrame, view->displayScale);
            view->widgets->tick();
            view->widgets->draw(view->render);
        }
        view->render->end();

        // To the view's swapchain: the texture is the view's for this
        // frame only, and submitting the command buffer presents it.
        SDL_GPUDevice* device = gpu->getDevice();
        SDL_GPUCommandBuffer* cmd = SDL_AcquireGPUCommandBuffer(device);
        SDL_GPUTexture* swapchain = nullptr;
        Uint32 sw = 0;
        Uint32 sh = 0;
        if (cmd &&
            SDL_WaitAndAcquireGPUSwapchainTexture(cmd, view->window, &swapchain, &sw, &sh) &&
            swapchain)
        {
            view->present->draw(
                cmd,
                view->buffer->getTexture(),
                swapchain,
                static_cast<int>(SDL_GetGPUSwapchainTextureFormat(device, view->window)),
                view->composition,
                ftk::gpu::getSDRWhiteLevel(view->window, view->composition));
        }
        if (cmd)
        {
            SDL_SubmitGPUCommandBuffer(cmd);
        }
    }
    catch (const std::exception& e)
    {
        setError(view->engine, e.what());
    }
}

}
