// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

//! \file Engine.h
//! A C interface to the engine: the player and a view that draws it,
//! for Swift and any other language that speaks C. Everything here is
//! called on the main thread.

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
TL_TIMELINE_API void tl_player_seek(tl_player*, double seconds);
TL_TIMELINE_API void tl_player_set_playback(tl_player*, tl_playback);
TL_TIMELINE_API tl_playback tl_player_get_playback(tl_player*);
TL_TIMELINE_API int tl_player_get_video_width(tl_player*);
TL_TIMELINE_API int tl_player_get_video_height(tl_player*);

//! Create a view in a native view: on macOS an NSWindow and the NSView
//! in it, both unretained. NULL on failure, with the reason in the
//! engine's error.
TL_TIMELINE_API tl_view* tl_view_create(tl_engine*, void* nsWindow, void* nsView);
TL_TIMELINE_API void tl_view_destroy(tl_view*);

//! Set the player the view draws; NULL clears it.
TL_TIMELINE_API void tl_view_set_player(tl_view*, tl_player*);

//! Draw the player's current frame, fitted to the view, and present it.
TL_TIMELINE_API void tl_view_draw(tl_view*);

#ifdef __cplusplus
}
#endif
