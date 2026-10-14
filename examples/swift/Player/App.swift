// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

import SwiftUI

@main
struct PlayerApp: App {
    @StateObject private var engine = Engine()

    var body: some Scene {
        WindowGroup {
            ContentView()
                .environmentObject(engine)
                .frame(minWidth: 640, minHeight: 400)
        }
    }
}
