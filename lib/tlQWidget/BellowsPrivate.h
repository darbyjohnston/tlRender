// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QWidget>

#include <memory>

namespace tl
{
    namespace qwidget
    {
        class BellowsButton : public QWidget
        {
            Q_OBJECT

        public:
            BellowsButton(QWidget* parent = nullptr);

            ~BellowsButton() override;

            QString text() const;

            bool isOpen() const;

        public Q_SLOTS:
            void setText(const QString&);

            void setOpen(bool);

        Q_SIGNALS:
            void openChanged(bool);

        protected:
            void mousePressEvent(QMouseEvent*) override;
            void mouseReleaseEvent(QMouseEvent*) override;
            void mouseMoveEvent(QMouseEvent*) override;

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
