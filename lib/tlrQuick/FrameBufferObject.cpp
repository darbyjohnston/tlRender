// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQuick/FrameBufferObject.h>

#include <tlrQt/TimelinePlayer.h>

#include <tlrGL/Render.h>

#include <QOpenGLFramebufferObject>

#include <QQuickWindow>

namespace tlr
{
    namespace quick
    {
        namespace
        {
            class Renderer : public QQuickFramebufferObject::Renderer
            {
            public:
                Renderer(
                    const file::Path& path,
                    const std::shared_ptr<core::Context>& context,
                    const FrameBufferObject* frameBufferObject) :
                    _frameBufferObject(frameBufferObject)
                {}

                ~Renderer() override
                {}

                QOpenGLFramebufferObject* createFramebufferObject(const QSize& size) override
                {
                    return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
                }
                
                void render() override
                {
                    if (!_init)
                    {
                        _init = true;
                        gladLoaderLoadGL();
                        _render = gl::Render::create();
                    }

                    QOpenGLFramebufferObject* fbo = framebufferObject();

                    _render->begin(imaging::Size(fbo->width(), fbo->height()));
                    _render->drawFrame(_frame);
                    _render->end();

                    _frameBufferObject->window()->resetOpenGLState();
                }
                    
                void synchronize(QQuickFramebufferObject*) override
                {
                    _frame = _frameBufferObject->getFrame();
                }

            private:
                const FrameBufferObject* _frameBufferObject = nullptr;
                bool _init = false;
                timeline::Frame _frame;
                std::shared_ptr<gl::Render> _render;
            };
        }

        struct FrameBufferObject::Private
        {
            std::shared_ptr<core::Context> context;
            file::Path path;
            qt::TimelinePlayer* timelinePlayer = nullptr;
            timeline::Frame frame;
        };

        FrameBufferObject::FrameBufferObject(QQuickItem* parent) :
            QQuickFramebufferObject(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();

            p.context = core::Context::create();
            p.timelinePlayer = new qt::TimelinePlayer(file::Path("/dev/otio/tlRender/etc/SampleData/transition.otio"), p.context, this);

            connect(
                p.timelinePlayer,
                SIGNAL(frameChanged(const tlr::timeline::Frame&)),
                SLOT(_frameCallback(const tlr::timeline::Frame&)));

            p.timelinePlayer->setPlayback(timeline::Playback::Forward);
        }

        FrameBufferObject::~FrameBufferObject()
        {}

        const tlr::timeline::Frame& FrameBufferObject::getFrame() const
        {
            return _p->frame;
        }

        QQuickFramebufferObject::Renderer* FrameBufferObject::createRenderer() const
        {
            TLR_PRIVATE_P();
            return new quick::Renderer(p.path, p.context, this);
        }

        void FrameBufferObject::_frameCallback(const timeline::Frame& frame)
        {
            _p->frame = frame;
            update();
        }
    }
}

