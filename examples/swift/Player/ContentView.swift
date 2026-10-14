// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

import SwiftUI
import UniformTypeIdentifiers

struct ContentView: View {
    @EnvironmentObject var engine: Engine
    @State private var scrubbing = false
    @State private var scrubTime = 0.0

    var body: some View {
        VStack(spacing: 0) {
            PlayerView()
                .background(Color.black)
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
            if !engine.error.isEmpty {
                Text(engine.error)
                    .foregroundColor(.red)
                    .padding(.horizontal, 10)
                    .padding(.bottom, 6)
            }
        }
        .navigationTitle(engine.fileName.isEmpty ? "Player" : engine.fileName)
    }

    private func open() {
        let panel = NSOpenPanel()
        panel.allowsMultipleSelection = false
        panel.canChooseDirectories = false
        if panel.runModal() == .OK, let url = panel.url {
            engine.open(url)
        }
    }

    private func timecode(_ seconds: Double) -> String {
        let total = Int(seconds.rounded(.down))
        return String(format: "%02d:%02d:%02d", total / 3600, (total / 60) % 60, total % 60)
    }
}
