// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

import AppKit
import SwiftUI
import tlRenderC

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
        let id = ObjectIdentifier(self)
        engine.register(id) { [weak self] in
            guard let self, let view = self.view else { return }
            tl_view_set_player(view, engine.player)
        }
        engine.drawers[id] = { [weak self] in
            guard let self, let view = self.view else { return }
            tl_view_draw(view)
        }
    }

    private func detach() {
        if let engine {
            let id = ObjectIdentifier(self)
            engine.unregister(id)
            engine.drawers[id] = nil
        }
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
