// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlrQWidget/BellowsWidget.h>

#include <QLabel>
#include <QVBoxLayout>

namespace tlr
{
    namespace qwidget
    {
        struct BellowsButton::Private
        {
            QLabel* iconLabel = nullptr;
            QLabel* textLabel = nullptr;
            bool open = false;
        };

        BellowsButton::BellowsButton(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();

            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            setBackgroundRole(QPalette::Button);
            setAutoFillBackground(true);
            setMouseTracking(true);

            p.iconLabel = new QLabel;
            p.textLabel = new QLabel;

            auto layout = new QHBoxLayout;
            layout->setMargin(0);
            layout->addWidget(p.iconLabel);
            layout->addWidget(p.textLabel, 1);
            setLayout(layout);

            _widgetUpdate();
        }

        BellowsButton::~BellowsButton()
        {}

        bool BellowsButton::isOpen() const
        {
            return _p->open;
        }

        void BellowsButton::setText(const QString& value)
        {
            _p->textLabel->setText(value);
        }

        void BellowsButton::setOpen(bool value)
        {
            TLR_PRIVATE_P();
            if (value == p.open)
                return;
            p.open = value;
            _widgetUpdate();
            Q_EMIT openChanged(p.open);
        }

        void BellowsButton::mousePressEvent(QMouseEvent*)
        {
            setOpen(!_p->open);
        }

        void BellowsButton::mouseReleaseEvent(QMouseEvent*)
        {}

        void BellowsButton::mouseMoveEvent(QMouseEvent*)
        {}

        void BellowsButton::_widgetUpdate()
        {
            TLR_PRIVATE_P();
            p.iconLabel->setPixmap(
                p.open ?
                QPixmap(":/Icons/BellowsOpen.svg") :
                QPixmap(":/Icons/BellowsClosed.svg"));
        }

        struct BellowsWidget::Private
        {
            BellowsButton* button = nullptr;
            QWidget* widget = nullptr;
            QVBoxLayout* layout = nullptr;
        };

        BellowsWidget::BellowsWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLR_PRIVATE_P();

            p.button = new BellowsButton;

            auto layout = new QVBoxLayout;
            layout->setMargin(0);
            layout->setSpacing(0);
            layout->addWidget(p.button);
            p.layout = new QVBoxLayout;
            layout->addLayout(p.layout);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.button,
                SIGNAL(openChanged(bool)),
                SLOT(_openCallback()));
        }

        BellowsWidget::~BellowsWidget()
        {}

        void BellowsWidget::setWidget(QWidget* widget)
        {
            TLR_PRIVATE_P();
            if (p.widget)
            {
                delete p.widget;
            }
            p.widget = widget;
            if (p.widget)
            {
                p.layout->addWidget(p.widget);
            }
            _widgetUpdate();
        }

        bool BellowsWidget::isOpen() const
        {
            return _p->button->isOpen();
        }

        void BellowsWidget::setTitle(const QString& value)
        {
            _p->button->setText(value);
        }

        void BellowsWidget::setOpen(bool value)
        {
            _p->button->setOpen(value);
        }

        void BellowsWidget::_openCallback()
        {
            _widgetUpdate();
        }

        void BellowsWidget::_widgetUpdate()
        {
            TLR_PRIVATE_P();
            if (p.widget)
            {
                p.widget->setVisible(p.button->isOpen());
            }
        }
    }
}
