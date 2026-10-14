// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

import SwiftUI
import tlRenderC

#if os(macOS)
import AppKit

/// The native view the engine draws into. SDL takes over the content
/// view of any window it is given, so it is not given this view's
/// window: it gets a borderless child window kept over this view's
/// frame, which follows the window as it moves and this view as it is
/// laid out. The engine's view is made once there is a window to
/// attach the child to.
final class PlayerNSView: NSView {
    weak var engine: Engine?
    private var view: OpaquePointer?
    private var child: NSWindow?

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        if let window {
            attach(to: window)
        } else {
            detach()
        }
    }

    override func layout() {
        super.layout()
        sync()
    }

    override func setFrameSize(_ newSize: NSSize) {
        super.setFrameSize(newSize)
        sync()
    }

    private func attach(to window: NSWindow) {
        guard child == nil, let engine, let handle = engine.handle else { return }
        let child = NSWindow(
            contentRect: screenFrame(),
            styleMask: .borderless,
            backing: .buffered,
            defer: false)
        child.isOpaque = true
        child.backgroundColor = .black
        child.hasShadow = false
        child.isReleasedWhenClosed = false
        // The mouse goes through to this view, and the keyboard stays
        // with the main window.
        child.ignoresMouseEvents = true
        let content = NSView(frame: NSRect(origin: .zero, size: child.frame.size))
        content.autoresizingMask = [.width, .height]
        child.contentView = content
        window.addChildWindow(child, ordered: .above)
        self.child = child

        let nsWindow = Unmanaged.passUnretained(child).toOpaque()
        let nsView = Unmanaged.passUnretained(content).toOpaque()
        view = tl_view_create(handle, nsWindow, nsView)
        engine.attach(ObjectIdentifier(self), view: view)
    }

    private func detach() {
        engine?.detach(ObjectIdentifier(self))
        if let view {
            tl_view_destroy(view)
            self.view = nil
        }
        if let child {
            child.parent?.removeChildWindow(child)
            child.orderOut(nil)
            self.child = nil
        }
    }

    private func screenFrame() -> NSRect {
        guard let window else { return .zero }
        return window.convertToScreen(convert(bounds, to: nil))
    }

    private func sync() {
        guard let child, window != nil else { return }
        let frame = screenFrame()
        if child.frame != frame {
            child.setFrame(frame, display: true)
        }
    }

    deinit {
        detach()
    }
}

struct PlayerView: NSViewRepresentable {
    @EnvironmentObject var engine: Engine

    func makeNSView(context: Context) -> PlayerNSView {
        let view = PlayerNSView()
        view.engine = engine
        return view
    }

    func updateNSView(_ nsView: PlayerNSView, context: Context) {}
}

#else
import UIKit

/// The view the picture shows through. SDL makes a window of its own
/// in the scene, which is kept under the application's window; this
/// view is clear, as is the window behind it, and tells the engine's
/// view where its frame is in the window, in pixels, as it is laid
/// out.
final class PlayerUIView: UIView {
    weak var engine: Engine?
    private var view: OpaquePointer?

    override func didMoveToWindow() {
        super.didMoveToWindow()
        if let window {
            attach(to: window)
        } else {
            detach()
        }
    }

    override func layoutSubviews() {
        super.layoutSubviews()
        sync()
    }

    private func attach(to window: UIWindow) {
        guard view == nil, let engine, let handle = engine.handle,
              let scene = window.windowScene else { return }
        backgroundColor = .clear
        isOpaque = false
        // What is drawn over the picture: the application's window and
        // its root, cleared so the picture shows through.
        window.backgroundColor = .clear
        window.rootViewController?.view.backgroundColor = .clear
        let scenePointer = Unmanaged.passUnretained(scene).toOpaque()
        view = tl_view_create_ios(handle, scenePointer)
        if let view, let native = tl_view_get_native_window(view) {
            let sdlWindow = Unmanaged<UIWindow>.fromOpaque(native).takeUnretainedValue()
            sdlWindow.windowLevel = UIWindow.Level(rawValue: UIWindow.Level.normal.rawValue - 1)
            sdlWindow.isUserInteractionEnabled = false
            window.makeKeyAndVisible()
        }
        engine.attach(ObjectIdentifier(self), view: view)
        sync()
    }

    private func detach() {
        engine?.detach(ObjectIdentifier(self))
        if let view {
            tl_view_destroy(view)
            self.view = nil
        }
    }

    private func sync() {
        guard let view, let window else { return }
        let frame = convert(bounds, to: window)
        let scale = window.screen.scale
        tl_view_set_frame(
            view,
            Int32((frame.origin.x * scale).rounded()),
            Int32((frame.origin.y * scale).rounded()),
            Int32((frame.size.width * scale).rounded()),
            Int32((frame.size.height * scale).rounded()))
    }

    deinit {
        detach()
    }
}

struct PlayerView: UIViewRepresentable {
    @EnvironmentObject var engine: Engine

    func makeUIView(context: Context) -> PlayerUIView {
        let view = PlayerUIView()
        view.engine = engine
        return view
    }

    func updateUIView(_ uiView: PlayerUIView, context: Context) {}
}
#endif
