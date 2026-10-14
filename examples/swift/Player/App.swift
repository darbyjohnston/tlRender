// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

import SwiftUI

#if os(macOS)

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

#else

import UIKit
import tlRenderC

/// On iOS SDL makes the application's one window, as it does for any
/// SDL program, and the SwiftUI content is hosted over it as a child
/// of SDL's view controller: clear where the picture is, opaque where
/// the controls are. UIKit's own lifecycle is used for it, since the
/// window is made as the scene connects rather than by a WindowGroup.
@main
final class AppDelegate: UIResponder, UIApplicationDelegate {
    func application(
        _ application: UIApplication,
        configurationForConnecting session: UISceneSession,
        options: UIScene.ConnectionOptions) -> UISceneConfiguration
    {
        let config = UISceneConfiguration(name: nil, sessionRole: session.role)
        config.delegateClass = SceneDelegate.self
        return config
    }
}

final class SceneDelegate: UIResponder, UIWindowSceneDelegate {
    private let engine = Engine()
    private var view: OpaquePointer?
    var window: UIWindow?

    func scene(
        _ scene: UIScene,
        willConnectTo session: UISceneSession,
        options: UIScene.ConnectionOptions)
    {
        guard let windowScene = scene as? UIWindowScene,
              let handle = engine.handle else { return }
        // SDL's window in this scene; the engine draws into it.
        let scenePointer = Unmanaged.passUnretained(windowScene).toOpaque()
        view = tl_view_create_ios(handle, scenePointer)
        guard let view, let native = tl_view_get_native_window(view) else { return }
        let sdlWindow = Unmanaged<UIWindow>.fromOpaque(native).takeUnretainedValue()
        window = sdlWindow
        engine.pictureView = view
        engine.attach(ObjectIdentifier(self), view: view)
        // The SwiftUI content over it.
        guard let root = sdlWindow.rootViewController else { return }
        let host = UIHostingController(rootView: ContentView().environmentObject(engine))
        host.view.backgroundColor = .clear
        host.view.frame = root.view.bounds
        host.view.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        root.addChild(host)
        root.view.addSubview(host.view)
        host.didMove(toParent: root)
    }

    func sceneDidDisconnect(_ scene: UIScene) {
        engine.detach(ObjectIdentifier(self))
        engine.pictureView = nil
        engine.close()
        if let view {
            tl_view_destroy(view)
            self.view = nil
        }
    }
}

#endif
