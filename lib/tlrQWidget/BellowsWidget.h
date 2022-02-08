// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Util.h>

#include <QWidget>

#include <memory>

namespace tlr
{
    namespace qwidget
    {
        //! Bellows button.
        class BellowsButton : public QWidget
        {
            Q_OBJECT

        public:
            BellowsButton(QWidget* parent = nullptr);

            ~BellowsButton() override;

            //! Get the text.
            QString text() const;

            //! Is the bellows open?
            bool isOpen() const;

        public Q_SLOTS:
            //! Set the text.
            void setText(const QString&);

            //! Set whether the bellows is open.
            void setOpen(bool);

        Q_SIGNALS:
            //! This signal is emitted when the bellows is opened or closed.
            void openChanged(bool);

        protected:
            void mousePressEvent(QMouseEvent*) override;
            void mouseReleaseEvent(QMouseEvent*) override;
            void mouseMoveEvent(QMouseEvent*) override;

        private:
            void _widgetUpdate();

            TLR_PRIVATE();
        };

        //! Bellows widget.
        class BellowsWidget : public QWidget
        {
            Q_OBJECT
            Q_PROPERTY(
                QString title
                READ title
                WRITE setTitle)
            Q_PROPERTY(
                bool open
                READ isOpen
                WRITE setOpen
                NOTIFY openChanged)

        public:
            BellowsWidget(QWidget* parent = nullptr);

            ~BellowsWidget() override;

            //! Set the widget.
            void setWidget(QWidget*);

            //! Get the title.
            QString title() const;

            //! Is the bellows open?
            bool isOpen() const;

        public Q_SLOTS:
            //! Set the title text.
            void setTitle(const QString&);

            //! Set whether the bellows is open.
            void setOpen(bool);

        Q_SIGNALS:
            //! This signal is emitted when the bellows is opened or closed.
            void openChanged(bool);

        private Q_SLOTS:
            void _openCallback();

        private:
            void _widgetUpdate();

            TLR_PRIVATE();
        };
    }
}
