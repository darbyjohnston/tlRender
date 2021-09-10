// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrQuick/GLFramebufferObject.h>

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
                    const GLFramebufferObject* framebufferObject) :
                    _framebufferObject(framebufferObject)
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
                        if (auto context = quick::context().lock())
                        {
                            _render = gl::Render::create(context);
                        }
                    }

                    QOpenGLFramebufferObject* fbo = framebufferObject();
                    _render->begin(imaging::Size(fbo->width(), fbo->height()));
                    _render->drawFrame(_frame);
                    _render->end();

                    _framebufferObject->window()->resetOpenGLState();
                }
                    
                void synchronize(QQuickFramebufferObject*) override
                {
                    _frame = _framebufferObject->frame();
                }

            private:
                const GLFramebufferObject* _framebufferObject = nullptr;
                bool _init = false;
                timeline::Frame _frame;
                std::shared_ptr<gl::Render> _render;
            };
        }

        struct GLFramebufferObject::Private
        {
            timeline::Frame frame;
        };

        GLFramebufferObject::GLFramebufferObject(QQuickItem* parent) :
            QQuickFramebufferObject(parent),
            _p(new Private)
        {
            setMirrorVertically(true);
        }

        GLFramebufferObject::~GLFramebufferObject()
        {}
        
        const tlr::timeline::Frame& GLFramebufferObject::frame() const
        {
            return _p->frame;
        }

        QQuickFramebufferObject::Renderer* GLFramebufferObject::createRenderer() const
        {
            return new quick::Renderer(this);
        }

        void GLFramebufferObject::setFrame(const timeline::Frame& frame)
        {
            _p->frame = frame;
            update();
        }
    }
}

