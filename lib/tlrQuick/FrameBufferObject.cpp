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
                Renderer(const FrameBufferObject* frameBufferObject) :
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
                    _frame = _frameBufferObject->frame();
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
            timeline::Frame frame;
        };

        FrameBufferObject::FrameBufferObject(QQuickItem* parent) :
            QQuickFramebufferObject(parent),
            _p(new Private)
        {
            setMirrorVertically(true);
        }

        FrameBufferObject::~FrameBufferObject()
        {}

        const tlr::timeline::Frame& FrameBufferObject::frame() const
        {
            return _p->frame;
        }

        QQuickFramebufferObject::Renderer* FrameBufferObject::createRenderer() const
        {
            TLR_PRIVATE_P();
            return new quick::Renderer(this);
        }

        void FrameBufferObject::setFrame(const timeline::Frame& frame)
        {
            _p->frame = frame;
            update();
        }
    }
}

