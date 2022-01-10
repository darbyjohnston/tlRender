// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
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
                    _render->drawVideo(_videoData);
                    _render->end();

                    _framebufferObject->window()->resetOpenGLState();
                }
                    
                void synchronize(QQuickFramebufferObject*) override
                {
                    _videoData = _framebufferObject->video();
                }

            private:
                const GLFramebufferObject* _framebufferObject = nullptr;
                bool _init = false;
                timeline::VideoData _videoData;
                std::shared_ptr<gl::Render> _render;
            };
        }

        struct GLFramebufferObject::Private
        {
            timeline::VideoData videoData;
        };

        GLFramebufferObject::GLFramebufferObject(QQuickItem* parent) :
            QQuickFramebufferObject(parent),
            _p(new Private)
        {
            setMirrorVertically(true);
        }

        GLFramebufferObject::~GLFramebufferObject()
        {}
        
        const tlr::timeline::VideoData& GLFramebufferObject::video() const
        {
            return _p->videoData;
        }

        QQuickFramebufferObject::Renderer* GLFramebufferObject::createRenderer() const
        {
            return new quick::Renderer(this);
        }

        void GLFramebufferObject::setVideo(const timeline::VideoData& value)
        {
            _p->videoData = value;
            update();
        }
    }
}

