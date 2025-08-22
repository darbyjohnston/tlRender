// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtQuick/GLFramebufferObject.h>

#include <tlQtQuick/Init.h>

#include <tlTimelineGL/Render.h>

#include <feather-tk/gl/Init.h>
#include <feather-tk/core/Context.h>

#include <QOpenGLFramebufferObject>

#include <QQuickWindow>

namespace tl
{
    namespace qtquick
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

                virtual ~Renderer()
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
                        feather_tk::gl::initGLAD();
                        _render = timeline_gl::Render::create(qtquick::getContext()->getLogSystem());
                    }

                    QOpenGLFramebufferObject* fbo = framebufferObject();
                    const feather_tk::Size2I size(fbo->width(), fbo->height());
                    _render->begin(size);
                    if (!_videoData.empty())
                    {
                        _render->drawVideo(
                            { _videoData.front() },
                            { feather_tk::Box2I(0, 0, size.w, size.h) });
                    }
                    _render->end();

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                    _framebufferObject->window()->resetOpenGLState();
#endif // QT_VERSION
                }

                void synchronize(QQuickFramebufferObject*) override
                {
                    _videoData = _framebufferObject->video();
                }

            private:
                const GLFramebufferObject* _framebufferObject = nullptr;
                bool _init = false;
                std::vector<timeline::VideoData> _videoData;
                std::shared_ptr<timeline::IRender> _render;
            };
        }

        struct GLFramebufferObject::Private
        {
            std::vector<timeline::VideoData> videoData;
        };

        GLFramebufferObject::GLFramebufferObject(QQuickItem* parent) :
            QQuickFramebufferObject(parent),
            _p(new Private)
        {
            setMirrorVertically(true);
        }

        GLFramebufferObject::~GLFramebufferObject()
        {}

        const std::vector<timeline::VideoData>& GLFramebufferObject::video() const
        {
            return _p->videoData;
        }

        QQuickFramebufferObject::Renderer* GLFramebufferObject::createRenderer() const
        {
            return new qtquick::Renderer(this);
        }

        void GLFramebufferObject::setVideo(const std::vector<timeline::VideoData>& value)
        {
            _p->videoData = value;
            update();
        }
    }
}
