// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/IWidget.h>

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
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<dtk::Style>&,
                QWidget* parent = nullptr);

            virtual ~ContainerWidget();

            //! Get the widget.
            const std::shared_ptr<dtk::IWidget>& getWidget() const;

            //! Set the widget.
            void setWidget(const std::shared_ptr<dtk::IWidget>&);

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
            dtk::V2I _toUI(const dtk::V2I&) const;
            int _fromUI(int) const;
            dtk::V2I _fromUI(const dtk::V2I&) const;

        private:
            void _tickEvent();
            void _tickEvent(
                const std::shared_ptr<dtk::IWidget>&,
                bool visible,
                bool enabled,
                const dtk::TickEvent&);

            bool _hasSizeUpdate(const std::shared_ptr<dtk::IWidget>&) const;
            void _sizeHintEvent();
            void _sizeHintEvent(
                const std::shared_ptr<dtk::IWidget>&,
                const dtk::SizeHintEvent&);

            void _setGeometry();

            void _clipEvent();
            void _clipEvent(
                const std::shared_ptr<dtk::IWidget>&,
                const dtk::Box2I&,
                bool clipped);

            bool _hasDrawUpdate(const std::shared_ptr<dtk::IWidget>&) const;
            void _drawEvent(
                const std::shared_ptr<dtk::IWidget>&,
                const dtk::Box2I&,
                const dtk::DrawEvent&);

            void _inputUpdate();
            void _timerUpdate();
            void _styleUpdate();

            DTK_PRIVATE();
        };
    }
}
