// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/ui/IWidget.h>

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
                const std::shared_ptr<feather_tk::Context>&,
                const std::shared_ptr<feather_tk::Style>&,
                QWidget* parent = nullptr);

            virtual ~ContainerWidget();

            //! Get the widget.
            const std::shared_ptr<feather_tk::IWidget>& getWidget() const;

            //! Set the widget.
            void setWidget(const std::shared_ptr<feather_tk::IWidget>&);

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
            feather_tk::V2I _toUI(const feather_tk::V2I&) const;
            int _fromUI(int) const;
            feather_tk::V2I _fromUI(const feather_tk::V2I&) const;

        private:
            void _tickEvent();
            void _tickEvent(
                const std::shared_ptr<feather_tk::IWidget>&,
                bool visible,
                bool enabled,
                const feather_tk::TickEvent&);

            bool _hasSizeUpdate(const std::shared_ptr<feather_tk::IWidget>&) const;
            void _sizeHintEvent();
            void _sizeHintEvent(
                const std::shared_ptr<feather_tk::IWidget>&,
                const feather_tk::SizeHintEvent&);

            void _setGeometry();

            void _clipEvent();
            void _clipEvent(
                const std::shared_ptr<feather_tk::IWidget>&,
                const feather_tk::Box2I&,
                bool clipped);

            bool _hasDrawUpdate(const std::shared_ptr<feather_tk::IWidget>&) const;
            void _drawEvent(
                const std::shared_ptr<feather_tk::IWidget>&,
                const feather_tk::Box2I&,
                const feather_tk::DrawEvent&);

            void _inputUpdate();
            void _timerUpdate();
            void _styleUpdate();

            FEATHER_TK_PRIVATE();
        };
    }
}
