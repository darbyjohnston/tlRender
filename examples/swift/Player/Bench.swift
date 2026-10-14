// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

import AVFoundation
import Foundation
import QuartzCore
import tlRenderC

/// The engine against AVPlayer on one file, the same script for both:
/// the time to the first frame, a play of some seconds (frames shown,
/// frames dropped, CPU), then seeks to the same ten times (the time
/// until the frame is shown). The numbers go to standard error.
///
/// Both are polled every millisecond for the frame on show: the engine
/// through the time of its current video frame, which its own tick
/// updates (ticking it from here as well starved the reader), AVPlayer
/// through a video output, which hands out each new frame once.
@MainActor
final class Bench {
    private let engine: Engine
    private let url: URL
    private let playSeconds = 20.0
    private let seekFractions = [0.5, 0.1, 0.9, 0.3, 0.7, 0.2, 0.8, 0.4, 0.6, 0.05]

    struct Result {
        var name = ""
        var firstFrame = 0.0
        var prebuffer = 0.0
        var seconds = 0.0
        var shown = 0
        var dropped = 0
        var cpu = 0.0
        var seeks: [Double] = []
        var note = ""
    }

    init(engine: Engine, url: URL) {
        self.engine = engine
        self.url = url
    }

    func run() {
        Task { @MainActor in
            let a = await runEngine()
            let b = await runAVPlayer()
            report([a, b])
            engine.bench = nil
        }
    }

    // MARK: The engine

    private func runEngine() async -> Result {
        var r = Result(name: "tlRender")
        let t0 = now()
        engine.open(url)
        guard let player = engine.player else {
            r.note = engine.error
            return r
        }
        while tl_player_get_video_time(player) < 0 {
            await tick()
            if now() - t0 > 30 { r.note = "no first frame"; return r }
        }
        r.firstFrame = now() - t0
        let rate = tl_player_get_rate(player)
        let duration = tl_player_get_duration(player)

        // A second of cache before playing, as AVPlayer prerolls: played
        // from a cache of one frame, the playhead outruns the reader and
        // nothing is ever shown (what the stats line reports as a cache
        // zero seconds ahead).
        let b0 = now()
        while tl_player_get_stats(player).cacheAhead < 1.0 {
            await tick()
            if now() - b0 > 10 { break }
        }
        r.prebuffer = now() - b0

        // Play, counting the frames that reach the screen.
        let seconds = min(playSeconds, max(duration - 0.5, 1))
        let cpu0 = cpuSeconds()
        let p0 = now()
        var last = -1.0
        engine.setPlaying(true)
        while now() - p0 < seconds {
            await tick()
            let t = tl_player_get_video_time(player)
            if t != last {
                last = t
                r.shown += 1
            }
        }
        engine.setPlaying(false)
        r.seconds = now() - p0
        r.cpu = (cpuSeconds() - cpu0) / r.seconds
        r.dropped = Int(tl_player_get_stats(player).droppedFrames)

        // Seeks: until the frame at the target is the one shown.
        for f in seekFractions {
            let target = (f * duration * rate).rounded() / rate
            let s0 = now()
            engine.seek(target)
            while abs(tl_player_get_video_time(player) - target) > 0.5 / rate {
                await tick()
                if now() - s0 > 10 { break }
            }
            r.seeks.append(now() - s0)
        }
        engine.close()
        return r
    }

    private func tick() async {
        try? await Task.sleep(nanoseconds: 1_000_000)
    }

    // MARK: AVPlayer

    private func runAVPlayer() async -> Result {
        var r = Result(name: "AVPlayer")
        let item = AVPlayerItem(url: url)
        let output = AVPlayerItemVideoOutput(pixelBufferAttributes: nil)
        item.add(output)
        let player = AVPlayer(playerItem: item)
        player.automaticallyWaitsToMinimizeStalling = false
        player.actionAtItemEnd = .pause
        let layer = AVPlayerLayer(player: player)
        layer.videoGravity = .resizeAspect
        if let host = engine.pictureLayer {
            layer.frame = host.bounds
            host.addSublayer(layer)
        } else {
            r.note = "no layer to show in"
        }

        let t0 = now()
        while !layer.isReadyForDisplay {
            try? await Task.sleep(nanoseconds: 1_000_000)
            if now() - t0 > 30 {
                r.note = "not ready for display: \(item.error?.localizedDescription ?? "")"
                layer.removeFromSuperlayer()
                return r
            }
        }
        r.firstFrame = now() - t0
        let duration = item.duration.seconds
        var rate = 24.0
        if let track = try? await item.asset.loadTracks(withMediaType: .video).first,
           let nominal = try? await track.load(.nominalFrameRate), nominal > 0 {
            rate = Double(nominal)
        }

        let seconds = min(playSeconds, max(duration - 0.5, 1))
        let cpu0 = cpuSeconds()
        let p0 = now()
        player.play()
        while now() - p0 < seconds {
            try? await Task.sleep(nanoseconds: 1_000_000)
            let t = output.itemTime(forHostTime: CACurrentMediaTime())
            if output.hasNewPixelBuffer(forItemTime: t) {
                _ = output.copyPixelBuffer(forItemTime: t, itemTimeForDisplay: nil)
                r.shown += 1
            }
        }
        player.pause()
        r.seconds = now() - p0
        r.cpu = (cpuSeconds() - cpu0) / r.seconds
        r.dropped = item.accessLog()?.events.reduce(0) {
            $0 + Int(max($1.numberOfDroppedVideoFrames, 0))
        } ?? 0

        for f in seekFractions {
            let target = (f * duration * rate).rounded() / rate
            let time = CMTime(seconds: target, preferredTimescale: 600)
            let s0 = now()
            await withCheckedContinuation { c in
                player.seek(to: time, toleranceBefore: .zero, toleranceAfter: .zero) { _ in
                    c.resume()
                }
            }
            while !output.hasNewPixelBuffer(forItemTime: time) {
                try? await Task.sleep(nanoseconds: 1_000_000)
                if now() - s0 > 10 { break }
            }
            _ = output.copyPixelBuffer(forItemTime: time, itemTimeForDisplay: nil)
            r.seeks.append(now() - s0)
        }
        layer.removeFromSuperlayer()
        return r
    }

    // MARK: Report

    private func report(_ results: [Result]) {
        var lines = ["bench: \(url.lastPathComponent)"]
        func pad(_ s: String, _ n: Int, left: Bool = false) -> String {
            let fill = String(repeating: " ", count: max(n - s.count, 0))
            return left ? s + fill : fill + s
        }
        lines.append(pad("", 14, left: true) + pad(results[0].name, 12) + pad(results[1].name, 12))
        func row(_ label: String, _ f: (Result) -> String) {
            lines.append(pad(label, 14, left: true) + pad(f(results[0]), 12) + pad(f(results[1]), 12))
        }
        row("first frame") { String(format: "%.3f s", $0.firstFrame) }
        row("prebuffer") { String(format: "%.3f s", $0.prebuffer) }
        row("shown") { String(format: "%.1f fps", $0.seconds > 0 ? Double($0.shown) / $0.seconds : 0) }
        row("dropped") { "\($0.dropped)" }
        row("cpu") { String(format: "%.2f cores", $0.cpu) }
        row("seek min") { String(format: "%.3f s", $0.seeks.min() ?? 0) }
        row("seek median") { String(format: "%.3f s", median($0.seeks)) }
        row("seek max") { String(format: "%.3f s", $0.seeks.max() ?? 0) }
        for r in results where !r.note.isEmpty {
            lines.append("\(r.name): \(r.note)")
        }
        let text = lines.joined(separator: "\n") + "\n"
        FileHandle.standardError.write(text.data(using: .utf8)!)
    }

    private func median(_ values: [Double]) -> Double {
        let sorted = values.sorted()
        guard !sorted.isEmpty else { return 0 }
        return sorted[sorted.count / 2]
    }

    private func now() -> Double {
        CACurrentMediaTime()
    }

    private func cpuSeconds() -> Double {
        Double(clock()) / Double(CLOCKS_PER_SEC)
    }
}
