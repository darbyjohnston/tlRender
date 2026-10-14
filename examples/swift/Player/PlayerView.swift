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
        engine.pictureView = view
        engine.relayout = { [weak self] in self?.sync() }
        engine.attach(ObjectIdentifier(self), view: view)
        sync()
    }

    private func detach() {
        engine?.detach(ObjectIdentifier(self))
        engine?.relayout = nil
        engine?.pictureView = nil
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

    /// The child window covers this view and the timeline view both,
    /// since the engine draws the timeline widget in the same buffer.
    private func childFrameInWindow() -> NSRect {
        var frame = convert(bounds, to: nil)
        if let engine, !engine.timelineFrameInWindow.isEmpty {
            frame = frame.union(engine.timelineFrameInWindow)
        }
        return frame
    }

    private func screenFrame() -> NSRect {
        guard let window else { return .zero }
        return window.convertToScreen(childFrameInWindow())
    }

    private func sync() {
        guard let child, let window, let engine else { return }
        let frame = screenFrame()
        if child.frame != frame {
            child.setFrame(frame, display: true)
        }
        // Where the picture goes in the child window, in its pixels,
        // from its top left.
        let union = childFrameInWindow()
        engine.childFrameInWindow = union
        let own = convert(bounds, to: nil)
        let scale = window.backingScaleFactor
        if let view {
            tl_view_set_frame(
                view,
                Int32(((own.minX - union.minX) * scale).rounded()),
                Int32(((union.maxY - own.maxY) * scale).rounded()),
                Int32((own.width * scale).rounded()),
                Int32((own.height * scale).rounded()))
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

/// Where the timeline widget goes. The engine draws it in the child
/// window the player view keeps over the picture and this view; the
/// frame and the mouse are sent in that window's pixels.
final class TimelineNSView: NSView {
    weak var engine: Engine?

    override func layout() {
        super.layout()
        sync()
    }

    override func viewDidMoveToWindow() {
        super.viewDidMoveToWindow()
        sync()
    }

    private func sync() {
        guard let engine, let window else { return }
        engine.timelineFrameInWindow = convert(bounds, to: nil)
        guard let view = engine.pictureView else { return }
        let own = convert(bounds, to: nil)
        let union = engine.childFrameInWindow
        let scale = window.backingScaleFactor
        tl_view_set_timeline_frame(
            view,
            Int32(((own.minX - union.minX) * scale).rounded()),
            Int32(((union.maxY - own.maxY) * scale).rounded()),
            Int32((own.width * scale).rounded()),
            Int32((own.height * scale).rounded()),
            Float(scale))
    }

    private func touch(_ event: NSEvent, _ phase: tl_touch_phase) {
        guard let engine, let view = engine.pictureView, let window else { return }
        let p = event.locationInWindow
        let union = engine.childFrameInWindow
        let scale = window.backingScaleFactor
        tl_view_touch(
            view, phase,
            Int32(((p.x - union.minX) * scale).rounded()),
            Int32(((union.maxY - p.y) * scale).rounded()))
    }

    override func mouseDown(with event: NSEvent) { touch(event, TL_TOUCH_BEGAN) }
    override func mouseDragged(with event: NSEvent) { touch(event, TL_TOUCH_MOVED) }
    override func mouseUp(with event: NSEvent) { touch(event, TL_TOUCH_ENDED) }
}

struct TimelineView: NSViewRepresentable {
    @EnvironmentObject var engine: Engine

    func makeNSView(context: Context) -> TimelineNSView {
        let view = TimelineNSView()
        view.engine = engine
        return view
    }

    func updateNSView(_ nsView: TimelineNSView, context: Context) {}
}

#else
import UIKit

/// Where the picture goes. SDL's window draws it underneath the hosted
/// SwiftUI content; this view is clear, and tells the engine's view
/// where it is laid out, in pixels of the window.
final class PlayerUIView: UIView {
    weak var engine: Engine?

    override init(frame: CGRect) {
        super.init(frame: frame)
        backgroundColor = .clear
        isOpaque = false
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) is not supported")
    }

    override func didMoveToWindow() {
        super.didMoveToWindow()
        sync()
    }

    override func layoutSubviews() {
        super.layoutSubviews()
        sync()
    }

    private func sync() {
        guard let view = engine?.pictureView, let window else { return }
        let frame = convert(bounds, to: window)
        let scale = window.screen.scale
        tl_view_set_frame(
            view,
            Int32((frame.origin.x * scale).rounded()),
            Int32((frame.origin.y * scale).rounded()),
            Int32((frame.size.width * scale).rounded()),
            Int32((frame.size.height * scale).rounded()))
    }
}

/// Where the timeline widget goes: the engine draws it there, in SDL's
/// window under this one, and this view sends it the touches, in the
/// window's pixels.
final class TimelineUIView: UIView {
    weak var engine: Engine?

    override init(frame: CGRect) {
        super.init(frame: frame)
        backgroundColor = .clear
        isOpaque = false
        isMultipleTouchEnabled = false
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) is not supported")
    }

    override func didMoveToWindow() {
        super.didMoveToWindow()
        sync()
    }

    override func layoutSubviews() {
        super.layoutSubviews()
        sync()
    }

    private func sync() {
        guard let view = engine?.pictureView, let window else { return }
        let frame = convert(bounds, to: window)
        let scale = window.screen.scale
        tl_view_set_timeline_frame(
            view,
            Int32((frame.origin.x * scale).rounded()),
            Int32((frame.origin.y * scale).rounded()),
            Int32((frame.size.width * scale).rounded()),
            Int32((frame.size.height * scale).rounded()),
            Float(scale))
    }

    private func touch(_ touches: Set<UITouch>, _ phase: tl_touch_phase) {
        guard let view = engine?.pictureView, let window,
              let t = touches.first else { return }
        let p = t.location(in: window)
        let scale = window.screen.scale
        tl_view_touch(
            view, phase,
            Int32((p.x * scale).rounded()),
            Int32((p.y * scale).rounded()))
    }

    override func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        touch(touches, TL_TOUCH_BEGAN)
    }

    override func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        touch(touches, TL_TOUCH_MOVED)
    }

    override func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        touch(touches, TL_TOUCH_ENDED)
    }

    override func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        touch(touches, TL_TOUCH_CANCELLED)
    }
}

struct TimelineView: UIViewRepresentable {
    @EnvironmentObject var engine: Engine

    func makeUIView(context: Context) -> TimelineUIView {
        let view = TimelineUIView()
        view.engine = engine
        return view
    }

    func updateUIView(_ uiView: TimelineUIView, context: Context) {}
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
