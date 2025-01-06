// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

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
                const std::shared_ptr<ui::Style>&,
                const std::shared_ptr<system::Context>&,
                QWidget* parent = nullptr);

            virtual ~ContainerWidget();

            //! Get the widget.
            const std::shared_ptr<ui::IWidget>& getWidget() const;

            //! Set the widget.
            void setWidget(const std::shared_ptr<ui::IWidget>&);

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

        private:
            void _tickEvent();
            void _tickEvent(
                const std::shared_ptr<ui::IWidget>&,
                bool visible,
                bool enabled,
                const ui::TickEvent&);

            bool _hasSizeUpdate(const std::shared_ptr<ui::IWidget>&) const;
            void _sizeHintEvent();
            void _sizeHintEvent(
                const std::shared_ptr<ui::IWidget>&,
                const ui::SizeHintEvent&);

            void _setGeometry();

            void _clipEvent();
            void _clipEvent(
                const std::shared_ptr<ui::IWidget>&,
                const math::Box2i&,
                bool clipped);

            bool _hasDrawUpdate(const std::shared_ptr<ui::IWidget>&) const;
            void _drawEvent(
                const std::shared_ptr<ui::IWidget>&,
                const math::Box2i&,
                const ui::DrawEvent&);

            int _toUI(int) const;
            math::Vector2i _toUI(const math::Vector2i&) const;
            int _fromUI(int) const;
            math::Vector2i _fromUI(const math::Vector2i&) const;

            void _inputUpdate();
            void _timerUpdate();
            void _styleUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
