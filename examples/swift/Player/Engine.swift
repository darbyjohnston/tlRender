// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

import Foundation
import QuartzCore
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
    /// The layer over the picture, for the benchmark to put AVPlayer's
    /// picture in.
    weak var pictureLayer: CALayer?
    /// The benchmark, while one runs.
    var bench: AnyObject?
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
            // The views let go of it first: they draw it every tick.
            self.player = nil
            for attach in views.values { attach() }
            tl_player_destroy(player)
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

    /// The process's memory footprint, as the system counts it against
    /// the application, in megabytes.
    static func footprintMB() -> Double {
        var info = task_vm_info_data_t()
        var count = mach_msg_type_number_t(MemoryLayout<task_vm_info_data_t>.size / MemoryLayout<natural_t>.size)
        let result = withUnsafeMutablePointer(to: &info) {
            $0.withMemoryRebound(to: integer_t.self, capacity: Int(count)) {
                task_info(mach_task_self_, task_flavor_t(TASK_VM_INFO), $0, &count)
            }
        }
        return result == KERN_SUCCESS ? Double(info.phys_footprint) / 1_048_576 : 0
    }

    /// The threads using the CPU most, by name, as a share of one core:
    /// a first profile without a profiler.
    static func threadCPU(top: Int = 8) -> String {
        var threads: thread_act_array_t?
        var count = mach_msg_type_number_t(0)
        guard task_threads(mach_task_self_, &threads, &count) == KERN_SUCCESS, let threads else { return "" }
        var rows: [(String, Double)] = []
        for i in 0..<Int(count) {
            var info = thread_extended_info_data_t()
            var infoCount = mach_msg_type_number_t(MemoryLayout<thread_extended_info_data_t>.size / MemoryLayout<natural_t>.size)
            let result = withUnsafeMutablePointer(to: &info) {
                $0.withMemoryRebound(to: integer_t.self, capacity: Int(infoCount)) {
                    thread_info(threads[i], thread_flavor_t(THREAD_EXTENDED_INFO), $0, &infoCount)
                }
            }
            if result == KERN_SUCCESS {
                let name = withUnsafePointer(to: &info.pth_name) {
                    $0.withMemoryRebound(to: CChar.self, capacity: 64) { String(cString: $0) }
                }
                let usage = Double(info.pth_cpu_usage) / Double(TH_USAGE_SCALE)
                rows.append((name.isEmpty ? "thread \(i)" : name, usage))
            }
            mach_port_deallocate(mach_task_self_, threads[i])
        }
        vm_deallocate(mach_task_self_, vm_address_t(bitPattern: threads), vm_size_t(Int(count) * MemoryLayout<thread_t>.size))
        rows.sort { $0.1 > $1.1 }
        let total = rows.reduce(0) { $0 + $1.1 }
        let shown = rows.prefix(top).map { String(format: "%@ %.2f", $0.0, $0.1) }
        return String(format: "cpu %.2f cores: ", total) + shown.joined(separator: ", ")
    }

    private var footprintTime = Date.distantPast

    private func tick() {
        guard let handle else { return }
        tl_engine_tick(handle)
        // The footprint every five seconds, beside the engine's own log.
        let now = Date()
        if now.timeIntervalSince(footprintTime) > 5 {
            footprintTime = now
            let line = String(format: "footprint %.0f MB\n", Engine.footprintMB()) + Engine.threadCPU() + "\n"
            FileHandle.standardError.write(line.data(using: .utf8)!)
        }
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
                    format: "%.1f fps (%.1f)  dropped %d%@  cache %.1f s ahead, video %.0f%% of %.2f GB, audio %.0f%% of %.2f GB",
                    s.speed, s.actualSpeed, s.droppedFrames, s.buffering != 0 ? "  buffering" : "", s.cacheAhead,
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
