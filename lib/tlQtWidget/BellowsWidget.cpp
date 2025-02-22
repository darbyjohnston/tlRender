// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/BellowsPrivate.h>

#include <tlQtWidget/Divider.h>

#include <QVBoxLayout>

namespace tl
{
    namespace qtwidget
    {
        struct BellowsWidget::Private
        {
            BellowsButton* button = nullptr;
            Divider* divider = nullptr;
            QWidget* widget = nullptr;
            QVBoxLayout* layout = nullptr;
        };

        BellowsWidget::BellowsWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            DTK_P();

            p.button = new BellowsButton;

            p.divider = new Divider(Qt::Horizontal);

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 1);
            layout->setSpacing(0);
            layout->addWidget(p.button);
            layout->addWidget(p.divider);
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
            DTK_P();
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

        QString BellowsWidget::title() const
        {
            return _p->button->text();
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
            DTK_P();
            if (p.widget)
            {
                p.widget->setVisible(p.button->isOpen());
            }
        }
    }
}
