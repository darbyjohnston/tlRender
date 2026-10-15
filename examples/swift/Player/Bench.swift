// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

import AVFoundation
import Foundation
import QuartzCore
import VideoToolbox
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

    /// Seek to a random frame every so often, forever, for a profiler
    /// to see nothing but the seek path. Each seek's time to the frame
    /// is logged.
    func runSeekLoop() {
        Task { @MainActor in
            engine.open(url)
            guard let player = engine.player else { return }
            while tl_player_get_video_time(player) < 0 { await tick() }
            let rate = tl_player_get_rate(player)
            let duration = tl_player_get_duration(player)
            var count = 0
            var sum = 0.0
            while engine.bench != nil {
                let target = (Double.random(in: 0.05..<0.95) * duration * rate).rounded() / rate
                let s0 = now()
                engine.seek(target)
                while abs(tl_player_get_video_time(player) - target) > 0.5 / rate {
                    await tick()
                    if now() - s0 > 10 { break }
                }
                let latency = now() - s0
                count += 1
                sum += latency
                if count % 10 == 0 {
                    let line = String(format: "seekloop: %d seeks, last %.3f s, mean %.3f s\n", count, latency, sum / Double(count))
                    FileHandle.standardError.write(line.data(using: .utf8)!)
                }
                try? await Task.sleep(nanoseconds: 300_000_000)
            }
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

        // Play, and let the player's pre-roll run first: the time until
        // its clock starts is the prebuffer. Played from a cache of one
        // frame without it, the playhead outran the reader and nothing was
        // ever shown.
        engine.setPlaying(true)
        let b0 = now()
        await tick()
        while tl_player_get_stats(player).buffering != 0 {
            await tick()
            if now() - b0 > 10 { break }
        }
        r.prebuffer = now() - b0

        // Then count the frames that reach the screen.
        let seconds = min(playSeconds, max(duration - 0.5, 1))
        let cpu0 = cpuSeconds()
        let p0 = now()
        var last = -1.0
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

    // MARK: VideoToolbox probe

    /// Decode the file's first frames straight through VideoToolbox, one
    /// frame at a time and then with frames in flight, and report the
    /// time a frame costs each way: what the hardware can do against
    /// what the one-at-a-time use of it gets. Off the main thread, which
    /// the watchdog kills an application for blocking.
    func runVTProbe() {
        let url = self.url
        DispatchQueue.global(qos: .userInitiated).async {
            Bench.vtProbe(url: url)
            DispatchQueue.main.async { [weak self] in self?.engine.bench = nil }
        }
    }

    nonisolated private static func log(_ text: String) {
        FileHandle.standardError.write(String(format: "%.3f %@\n", CACurrentMediaTime(), text).data(using: .utf8)!)
    }

    nonisolated private static func vtProbe(url: URL) {
        log("vtprobe: start \(url.path)")
        let asset = AVURLAsset(url: url)
        guard let track = asset.tracks(withMediaType: .video).first,
              let format = track.formatDescriptions.first else {
            FileHandle.standardError.write("vtprobe: no video track\n".data(using: .utf8)!)
            return
        }
        let formatDescription = format as! CMFormatDescription
        log("vtprobe: track loaded")
        for async in [false, true] {
            var count = 0
            let countLock = NSLock()
            let attrs: [String: Any] = [
                kCVPixelBufferPixelFormatTypeKey as String: kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange,
                kCVPixelBufferIOSurfacePropertiesKey as String: [:]]
            var session: VTDecompressionSession?
            let handler: VTDecompressionOutputHandler = { status, _, image, _, _ in
                if status == noErr && image != nil {
                    countLock.lock(); count += 1; countLock.unlock()
                }
            }
            let status = VTDecompressionSessionCreate(
                allocator: nil, formatDescription: formatDescription, decoderSpecification: nil,
                imageBufferAttributes: attrs as CFDictionary, outputCallback: nil,
                decompressionSessionOut: &session)
            guard status == noErr, let session else {
                FileHandle.standardError.write("vtprobe: session failed \(status)\n".data(using: .utf8)!)
                return
            }
            guard let reader = try? AVAssetReader(asset: asset) else { return }
            let output = AVAssetReaderTrackOutput(track: track, outputSettings: nil)
            output.alwaysCopiesSampleData = false
            reader.add(output)
            reader.startReading()
            log("vtprobe: session + reader ready, async \(async)")
            // Four frames in flight at most when pipelined: unbounded, the
            // decoder allocated output for every frame of the file at once
            // and the application was killed for its memory.
            let frames = 240
            let depth = 4
            let t0 = CACurrentMediaTime()
            var sent = 0
            while sent < frames, let sample = output.copyNextSampleBuffer() {
                let flags: VTDecodeFrameFlags = async ? [._EnableAsynchronousDecompression] : []
                VTDecompressionSessionDecodeFrame(
                    session, sampleBuffer: sample, flags: flags,
                    infoFlagsOut: nil, outputHandler: handler)
                sent += 1
                if sent % 60 == 0 { log("vtprobe: sent \(sent)") }
                // The session's own wait for asynchronous frames never
                // returned on this device, so the frames in flight are
                // counted here instead.
                if async {
                    let deadline = CACurrentMediaTime() + 2
                    while CACurrentMediaTime() < deadline {
                        countLock.lock(); let done = count; countLock.unlock()
                        if sent - done < depth { break }
                        usleep(200)
                    }
                }
            }
            log("vtprobe: all sent, waiting")
            let deadline = CACurrentMediaTime() + 2
            while CACurrentMediaTime() < deadline {
                countLock.lock(); let done = count; countLock.unlock()
                if done >= sent { break }
                usleep(200)
            }
            let elapsed = CACurrentMediaTime() - t0
            log("vtprobe: waited, cancelling reader")
            reader.cancelReading()
            log("vtprobe: invalidating session")
            VTDecompressionSessionInvalidate(session)
            countLock.lock(); let n = count; countLock.unlock()
            let line = String(
                format: "vtprobe %@: %d frames in %.2f s = %.1f fps, %.1f ms/frame\n",
                async ? "async(4)" : "sync", n, elapsed, Double(n) / elapsed,
                1000 * elapsed / Double(max(n, 1)))
            FileHandle.standardError.write(line.data(using: .utf8)!)
        }
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
