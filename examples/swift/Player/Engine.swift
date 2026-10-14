// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

import Foundation
import tlRenderC
#if os(macOS)
import AppKit
#endif

/// The engine and the open player, ticked once a display frame; the
/// views draw in the same tick, after the engine.
final class Engine: ObservableObject {
    let handle: OpaquePointer?
    private(set) var player: OpaquePointer?
    private var timer: Timer?
    private var views: [ObjectIdentifier: () -> Void] = [:]

    /// The engine's view: the player view tells it where the picture
    /// goes, the timeline view where the timeline widget goes.
    var pictureView: OpaquePointer?
#if os(macOS)
    /// On macOS the engine draws in a child window that the player view
    /// keeps over the picture and the timeline together: the timeline
    /// view says where it is, in window coordinates, and the player view
    /// says where the child window ended up.
    var timelineFrameInWindow = NSRect.zero {
        didSet { if timelineFrameInWindow != oldValue { relayout?() } }
    }
    var childFrameInWindow = NSRect.zero
    var relayout: (() -> Void)?
#endif

    @Published var fileName = ""
    @Published var error = ""
    @Published var time = 0.0
    @Published var duration = 0.0
    @Published var playing = false
    /// A line about how playback is going, a few times a second.
    @Published var stats = ""
    private var statsTime = Date.distantPast

    init() {
        handle = tl_engine_create()
        if let handle, let e = tl_engine_get_error(handle), e.pointee != 0 {
            error = String(cString: e)
        }
        // The display's rate is enough for a player; the swapchain
        // paces the draws to the display's vertical sync.
        timer = Timer.scheduledTimer(withTimeInterval: 1.0 / 60.0, repeats: true) { [weak self] _ in
            self?.tick()
        }
        RunLoop.main.add(timer!, forMode: .common)
    }

    deinit {
        timer?.invalidate()
        close()
        if let handle { tl_engine_destroy(handle) }
    }

    /// The player reads the file for as long as it lives, so the access
    /// a picked file grants is kept until the player goes.
    private var accessed: URL?

    func open(_ url: URL) {
        close()
        guard let handle else { return }
        if url.startAccessingSecurityScopedResource() {
            accessed = url
        }
        player = url.withUnsafeFileSystemRepresentation { tl_player_create(handle, $0) }
        if let player {
            fileName = url.lastPathComponent
            duration = tl_player_get_duration(player)
            error = ""
        } else {
            fileName = ""
            duration = 0
            stats = ""
            error = String(cString: tl_engine_get_error(handle))
        }
        for attach in views.values { attach() }
    }

    func close() {
        if let player {
            tl_player_destroy(player)
            self.player = nil
        }
        if let accessed {
            accessed.stopAccessingSecurityScopedResource()
            self.accessed = nil
        }
    }

    func setPlaying(_ value: Bool) {
        guard let player else { return }
        tl_player_set_playback(player, value ? TL_PLAYBACK_FORWARD : TL_PLAYBACK_STOP)
    }

    func seek(_ seconds: Double) {
        guard let player else { return }
        tl_player_seek(player, seconds)
    }

    /// A native view attaches the engine's view it made: it is given the
    /// player, now and whenever the player changes, and drawn each tick.
    func attach(_ id: ObjectIdentifier, view: OpaquePointer?) {
        guard let view else { return }
        views[id] = { [weak self] in
            tl_view_set_player(view, self?.player)
        }
        views[id]?()
        drawers[id] = { tl_view_draw(view) }
    }

    func detach(_ id: ObjectIdentifier) {
        views[id] = nil
        drawers[id] = nil
    }

    private var drawers: [ObjectIdentifier: () -> Void] = [:]

    private func tick() {
        guard let handle else { return }
        tl_engine_tick(handle)
        if let player {
            let t = tl_player_get_time(player)
            if t != time { time = t }
            let p = tl_player_get_playback(player) != TL_PLAYBACK_STOP
            if p != playing { playing = p }
            let now = Date()
            if now.timeIntervalSince(statsTime) > 0.25 {
                statsTime = now
                let s = tl_player_get_stats(player)
                stats = String(
                    format: "%.1f fps (%.1f)  dropped %d  cache %.1f s ahead, video %.0f%% of %.2f GB, audio %.0f%% of %.2f GB",
                    s.speed, s.actualSpeed, s.droppedFrames, s.cacheAhead,
                    s.videoCachePercent, s.videoCacheGB,
                    s.audioCachePercent, s.audioCacheGB)
            }
        }
        for draw in drawers.values { draw() }
        if let e = tl_engine_get_error(handle), e.pointee != 0 {
            let s = String(cString: e)
            if s != error { error = s }
        }
    }
}
