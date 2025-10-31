// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/UI/IWidget.h>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

namespace tl
{
    namespace qtwidget
    {
        //! Container widget.
        class ContainerWidget :
            public QOpenGLWidget,
            protected QOpenGLFunctions_4_1_Core
        {
            Q_OBJECT

        public:
            ContainerWidget(
                const std::shared_ptr<ftk::Context>&,
                const std::shared_ptr<ftk::Style>&,
                QWidget* parent = nullptr);

            virtual ~ContainerWidget();

            //! Get the widget.
            const std::shared_ptr<ftk::IWidget>& getWidget() const;

            //! Set the widget.
            void setWidget(const std::shared_ptr<ftk::IWidget>&);

            //! Get whether input is enabled.
            bool isInputEnabled() const;

            //! Set whether input is enabled.
            void setInputEnabled(bool);

            QSize minimumSizeHint() const override;
            QSize sizeHint() const override;

        protected:
            void initializeGL() override;
            void resizeGL(int w, int h) override;
            void paintGL() override;

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            void enterEvent(QEvent*) override;
#else
            void enterEvent(QEnterEvent*) override;
#endif // QT_VERSION
            void leaveEvent(QEvent*) override;
            void mousePressEvent(QMouseEvent*) override;
            void mouseReleaseEvent(QMouseEvent*) override;
            void mouseMoveEvent(QMouseEvent*) override;
            void wheelEvent(QWheelEvent*) override;
            void keyPressEvent(QKeyEvent*) override;
            void keyReleaseEvent(QKeyEvent*) override;

            bool event(QEvent*) override;

            int _toUI(int) const;
            ftk::V2I _toUI(const ftk::V2I&) const;
            int _fromUI(int) const;
            ftk::V2I _fromUI(const ftk::V2I&) const;

        private:
            void _tickEvent();
            void _tickEvent(
                const std::shared_ptr<ftk::IWidget>&,
                bool visible,
                bool enabled,
                const ftk::TickEvent&);

            bool _hasSizeUpdate(const std::shared_ptr<ftk::IWidget>&) const;
            void _sizeHintEvent();
            void _sizeHintEvent(
                const std::shared_ptr<ftk::IWidget>&,
                const ftk::SizeHintEvent&);

            void _setGeometry();

            void _clipEvent();
            void _clipEvent(
                const std::shared_ptr<ftk::IWidget>&,
                const ftk::Box2I&,
                bool clipped);

            bool _hasDrawUpdate(const std::shared_ptr<ftk::IWidget>&) const;
            void _drawEvent(
                const std::shared_ptr<ftk::IWidget>&,
                const ftk::Box2I&,
                const ftk::DrawEvent&);

            void _inputUpdate();
            void _timerUpdate();
            void _styleUpdate();

            FTK_PRIVATE();
        };
    }
}
