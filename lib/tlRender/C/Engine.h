// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

//! \file Engine.h
//! A C interface to the engine: the player and a view that draws it,
//! for Swift and any other language that speaks C. Everything here is
//! called on the main thread.

#include <stddef.h>
#include <tlRender/Timeline/Export.h>

#ifdef __cplusplus
extern "C" {
#endif

//! The engine: a context, the systems, and the GPU renderer's device.
typedef struct tl_engine tl_engine;

//! A timeline player.
typedef struct tl_player tl_player;

//! A view that draws a player into a native view.
typedef struct tl_view tl_view;

typedef enum
{
    TL_PLAYBACK_STOP,
    TL_PLAYBACK_FORWARD,
    TL_PLAYBACK_REVERSE
} tl_playback;

//! Create the engine. NULL when the GPU renderer cannot start; the
//! reason is in tl_engine_get_error() of a NULL-free call, or the log.
TL_TIMELINE_API tl_engine* tl_engine_create(void);
TL_TIMELINE_API void tl_engine_destroy(tl_engine*);

//! Tick the engine: once per display frame, before the views draw.
TL_TIMELINE_API void tl_engine_tick(tl_engine*);

//! Set an I/O option for the files opened from now on (see IO.h): for
//! instance "Texture" "0" to copy decoded frames rather than draw them
//! where the decoder put them.
TL_TIMELINE_API void tl_engine_set_io_option(tl_engine*, const char* name, const char* value);

//! Override the video cache size and the number of readers kept open
//! for the files opened from now on; zero leaves the platform's choice.
TL_TIMELINE_API void tl_engine_set_cache(tl_engine*, float videoGB, int readCacheMax);

//! The last error, or an empty string.
TL_TIMELINE_API const char* tl_engine_get_error(tl_engine*);

//! Open a file or timeline. NULL on failure, with the reason in the
//! engine's error.
TL_TIMELINE_API tl_player* tl_player_create(tl_engine*, const char* path);
TL_TIMELINE_API void tl_player_destroy(tl_player*);

//! Times are seconds from the start of the timeline.
TL_TIMELINE_API double tl_player_get_duration(tl_player*);
TL_TIMELINE_API double tl_player_get_rate(tl_player*);
TL_TIMELINE_API double tl_player_get_time(tl_player*);
//! The time of the video frame being shown, which trails the current
//! time while the reader catches up; negative when there is none yet.
TL_TIMELINE_API double tl_player_get_video_time(tl_player*);
TL_TIMELINE_API void tl_player_seek(tl_player*, double seconds);
TL_TIMELINE_API void tl_player_set_playback(tl_player*, tl_playback);
TL_TIMELINE_API tl_playback tl_player_get_playback(tl_player*);
TL_TIMELINE_API int tl_player_get_video_width(tl_player*);
TL_TIMELINE_API int tl_player_get_video_height(tl_player*);

//! How playback is going, for a readout: whether frames are being
//! dropped, and whether the cache is keeping ahead of the playhead.
typedef struct tl_player_stats
{
    //! The speed asked for, and the speed measured, in frames a second.
    double speed;
    double actualSpeed;

    //! Frames dropped since playback started or the last seek.
    size_t droppedFrames;

    //! Whether the clock is held while the cache fills (the pre-roll).
    int buffering;

    //! Seconds of video cached ahead of the current time: zero when
    //! playback is waiting on the reader.
    double cacheAhead;

    //! The caches: how full, and how big, in gigabytes.
    float videoCachePercent;
    float videoCacheGB;
    float audioCachePercent;
    float audioCacheGB;
} tl_player_stats;
TL_TIMELINE_API tl_player_stats tl_player_get_stats(tl_player*);

//! Create a view in a native view: on macOS an NSWindow and the NSView
//! in it, both unretained. NULL on failure, with the reason in the
//! engine's error.
TL_TIMELINE_API tl_view* tl_view_create(tl_engine*, void* nsWindow, void* nsView);

//! Create a view on iOS: a window of its own in the UIWindowScene,
//! which the application keeps under its own window and draws its
//! controls over; tl_view_set_frame() says where the picture goes.
//! The window is tl_view_get_native_window().
TL_TIMELINE_API tl_view* tl_view_create_ios(tl_engine*, void* uiWindowScene);
TL_TIMELINE_API void tl_view_destroy(tl_view*);

//! The native window the view draws into: the UIWindow on iOS, the
//! NSWindow on macOS.
TL_TIMELINE_API void* tl_view_get_native_window(tl_view*);

//! Set the player the view draws; NULL clears it.
TL_TIMELINE_API void tl_view_set_player(tl_view*, tl_player*);

//! Set where in the view the picture is drawn, in pixels from the top
//! left; a zero size means the whole view, which is the default.
TL_TIMELINE_API void tl_view_set_frame(tl_view*, int x, int y, int w, int h);

//! Where the timeline widget goes in the view, in pixels: an empty frame
//! hides it. The display scale is the screen's, which sizes its text
//! and lines.
TL_TIMELINE_API void tl_view_set_timeline_frame(
    tl_view*, int x, int y, int w, int h, float displayScale);

//! A touch, or a mouse button, on the view: where it is in pixels, and
//! what it is doing. The widgets take it as the mouse.
typedef enum tl_touch_phase
{
    TL_TOUCH_BEGAN,
    TL_TOUCH_MOVED,
    TL_TOUCH_ENDED,
    TL_TOUCH_CANCELLED
} tl_touch_phase;
TL_TIMELINE_API void tl_view_touch(tl_view*, tl_touch_phase, int x, int y);

//! Draw the player's current frame, fitted to the view, and present it.
//! Whether the view has something new to draw: a video frame that has
//! not been drawn, a widget that asked for a redraw, a change of size or
//! frame. Ticks the widgets, so it is asked every tick, and the draw is
//! made only when it says so: drawing at the display's rate when the
//! picture changes at the movie's wasted most of the draws.
TL_TIMELINE_API int tl_view_needs_draw(tl_view*);
//! How many times the view has been drawn, for a readout of draws a second.
TL_TIMELINE_API size_t tl_view_get_draw_count(tl_view*);
TL_TIMELINE_API void tl_view_draw(tl_view*);

#ifdef __cplusplus
}
#endif
