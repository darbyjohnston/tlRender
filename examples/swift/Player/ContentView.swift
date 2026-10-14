// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

import SwiftUI
import tlRenderC
import UniformTypeIdentifiers

struct ContentView: View {
    @EnvironmentObject var engine: Engine
    @State private var scrubbing = false
    @State private var scrubTime = 0.0

    var body: some View {
        VStack(spacing: 0) {
            PlayerView()
                .background(pictureBackground)
            TimelineView()
                .frame(height: 140)
                .background(pictureBackground)
            Divider()
            HStack(spacing: 12) {
                Button(action: open) {
                    Image(systemName: "folder")
                }
                .help("Open a file")
                Button(action: { engine.setPlaying(!engine.playing) }) {
                    Image(systemName: engine.playing ? "stop.fill" : "play.fill")
                }
                .help(engine.playing ? "Stop" : "Play")
                .disabled(engine.player == nil)
                .keyboardShortcut(.space, modifiers: [])
                Slider(
                    value: Binding(
                        get: { scrubbing ? scrubTime : engine.time },
                        set: { scrubTime = $0; engine.seek($0) }),
                    in: 0...max(engine.duration, 0.001),
                    onEditingChanged: { editing in
                        scrubbing = editing
                        if editing { scrubTime = engine.time }
                    })
                    .disabled(engine.player == nil)
                Text(timecode(engine.time) + " / " + timecode(engine.duration))
                    .font(.system(.body, design: .monospaced))
                    .frame(minWidth: 150, alignment: .trailing)
            }
            .padding(10)
            .background(controlsBackground)
            if !engine.stats.isEmpty {
                Text(engine.stats)
                    .font(.system(.caption, design: .monospaced))
                    .foregroundColor(.secondary)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .padding(.horizontal, 10)
                    .padding(.bottom, 6)
                    .background(controlsBackground)
            }
            if !engine.error.isEmpty {
                Text(engine.error)
                    .foregroundColor(.red)
                    .padding(.horizontal, 10)
                    .padding(.bottom, 6)
            }
        }
        .navigationTitle(engine.fileName.isEmpty ? "Player" : engine.fileName)
        .onAppear {
            // "-open <path>" on the command line opens a file at launch,
            // which is how the simulator is driven without a picker.
            let args = CommandLine.arguments
            // "-copy" copies decoded frames rather than drawing them where
            // the decoder put them, to compare the two.
            if args.contains("-copy"), let handle = engine.handle {
                tl_engine_set_io_option(handle, "Texture", "0")
            }
            // "-cache <GB>" and "-readers <N>" override the platform's
            // cache size and readers kept open.
            if let handle = engine.handle {
                var gb: Float = 0
                var readers: Int32 = 0
                if let i = args.firstIndex(of: "-cache"), i + 1 < args.count {
                    gb = Float(args[i + 1]) ?? 0
                }
                if let i = args.firstIndex(of: "-readers"), i + 1 < args.count {
                    readers = Int32(args[i + 1]) ?? 0
                }
                if gb > 0 || readers > 0 {
                    tl_engine_set_cache(handle, gb, readers)
                }
            }
            if let i = args.firstIndex(of: "-open"), i + 1 < args.count {
                // A relative path is in the application's documents,
                // where a file copied to a device lands.
                let documents = FileManager.default.urls(
                    for: .documentDirectory, in: .userDomainMask).first
                engine.open(URL(fileURLWithPath: args[i + 1], relativeTo: documents))
                if args.contains("-play") {
                    engine.setPlaying(true)
                }
            }
            // "-bench <path>" runs the engine against AVPlayer on the
            // file and prints the numbers.
            if let i = args.firstIndex(of: "-bench"), i + 1 < args.count {
                let documents = FileManager.default.urls(
                    for: .documentDirectory, in: .userDomainMask).first
                let bench = Bench(
                    engine: engine,
                    url: URL(fileURLWithPath: args[i + 1], relativeTo: documents))
                engine.bench = bench
                bench.run()
            }
        }
#if os(iOS)
        // The system's document picker; the file is read while the
        // access it grants lasts, which the player's open does.
        .fileImporter(isPresented: $importing, allowedContentTypes: [.movie, .item]) { result in
            if case .success(let url) = result {
                engine.open(url)
            }
        }
#endif
    }

#if os(macOS)
    // The picture is drawn in a child window over this view: black
    // behind it until the first frame.
    private var pictureBackground: Color { .black }
    private var controlsBackground: Color { .clear }

    private func open() {
        let panel = NSOpenPanel()
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        if panel.runModal() == .OK, let url = panel.url {
            engine.open(url)
        }
    }
#else
    // The picture is drawn by SDL's window under the hosted content, so
    // everything in front of it is clear except the controls.
    private var pictureBackground: Color { .clear }
    private var controlsBackground: Color { Color(uiColor: .systemBackground) }

    @State private var importing = false

    private func open() {
        importing = true
    }
#endif

    private func timecode(_ seconds: Double) -> String {
        let total = Int(seconds.rounded(.down))
        return String(format: "%02d:%02d:%02d", total / 3600, (total / 60) % 60, total % 60)
    }
}
