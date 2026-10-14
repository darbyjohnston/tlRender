// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

import Foundation
import tlRenderC

/// The engine and the open player, ticked once a display frame; the
/// views draw in the same tick, after the engine.
final class Engine: ObservableObject {
    let handle: OpaquePointer?
    private(set) var player: OpaquePointer?
    private var timer: Timer?
    private var views: [ObjectIdentifier: () -> Void] = [:]

    @Published var fileName = ""
    @Published var error = ""
    @Published var time = 0.0
    @Published var duration = 0.0
    @Published var playing = false

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
        if let player { tl_player_destroy(player) }
        if let handle { tl_engine_destroy(handle) }
    }

    func open(_ url: URL) {
        if let player {
            tl_player_destroy(player)
            self.player = nil
        }
        guard let handle else { return }
        player = url.withUnsafeFileSystemRepresentation { tl_player_create(handle, $0) }
        if let player {
            fileName = url.lastPathComponent
            duration = tl_player_get_duration(player)
            error = ""
        } else {
            fileName = ""
            duration = 0
            error = String(cString: tl_engine_get_error(handle))
        }
        for attach in views.values { attach() }
    }

    func setPlaying(_ value: Bool) {
        guard let player else { return }
        tl_player_set_playback(player, value ? TL_PLAYBACK_FORWARD : TL_PLAYBACK_STOP)
    }

    func seek(_ seconds: Double) {
        guard let player else { return }
        tl_player_seek(player, seconds)
    }

    /// A view registers what to do when the player changes, and what to
    /// draw each tick.
    func register(_ id: ObjectIdentifier, attach: @escaping () -> Void) {
        views[id] = attach
        attach()
    }

    func unregister(_ id: ObjectIdentifier) {
        views[id] = nil
    }

    var drawers: [ObjectIdentifier: () -> Void] = [:]

    private func tick() {
        guard let handle else { return }
        tl_engine_tick(handle)
        if let player {
            let t = tl_player_get_time(player)
            if t != time { time = t }
            let p = tl_player_get_playback(player) != TL_PLAYBACK_STOP
            if p != playing { playing = p }
        }
        for draw in drawers.values { draw() }
        if let e = tl_engine_get_error(handle), e.pointee != 0 {
            let s = String(cString: e)
            if s != error { error = s }
        }
    }
}
