// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlQWidget/RadioButtonGroup.h>

#include <QBoxLayout>
#include <QButtonGroup>
#include <QMap>
#include <QRadioButton>

namespace tl
{
    namespace qwidget
    {
        struct RadioButtonGroup::Private
        {
            Qt::Orientation orientation = Qt::Orientation::Horizontal;
            QMap<QVariant, QString> toText;
            QMap<QVariant, QAbstractButton*> toButton;
            QMap<QAbstractButton*, QVariant> fromButton;
            QButtonGroup* group = nullptr;
            QBoxLayout* layout = nullptr;
            QBoxLayout* buttonLayout = nullptr;
        };

        RadioButtonGroup::RadioButtonGroup(Qt::Orientation orientation, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.group = new QButtonGroup(this);
            p.group->setExclusive(true);
            
            _widgetUpdate();

            connect(
                p.group,
                SIGNAL(buttonToggled(QAbstractButton*, bool)),
                SLOT(_groupCallback(QAbstractButton*, bool)));
        }
        
        RadioButtonGroup::~RadioButtonGroup()
        {}

        void RadioButtonGroup::addButton(const QString& text, const QVariant& value)
        {
            TLRENDER_P();

            auto button = new QRadioButton;
            button->setText(text);
            
            p.toText[value] = text;
            p.toButton[value] = button;
            p.fromButton[button] = value;
            
            p.group->addButton(button);
            p.buttonLayout->addWidget(button);
        }

        void RadioButtonGroup::clear()
        {
            TLRENDER_P();

            for (auto i : p.toButton)
            {
                p.buttonLayout->removeWidget(i);
                p.group->removeButton(i);
                delete i;
            }

            p.toText.clear();
            p.toButton.clear();
            p.fromButton.clear();
        }

        void RadioButtonGroup::setChecked(const QVariant& value)
        {
            TLRENDER_P();
            QSignalBlocker blocker(p.group);
            const auto i = p.toButton.find(value);
            if (i != p.toButton.end())
            {
                QSignalBlocker blocker(i.value());
                i.value()->setChecked(true);
            }
        }

        void RadioButtonGroup::setOrientation(Qt::Orientation value)
        {
            TLRENDER_P();
            if (value == p.orientation)
                return;
            p.orientation = value;
            _widgetUpdate();
        }

        void RadioButtonGroup::_groupCallback(QAbstractButton* button, bool value)
        {
            TLRENDER_P();
            if (value)
            {
                const auto i = p.fromButton.find(button);
                if (i != p.fromButton.end())
                {
                    Q_EMIT checked(i.value());
                }
            }
        }

        void RadioButtonGroup::_widgetUpdate()
        {
            TLRENDER_P();
            for (auto i : p.toButton)
            {
                p.buttonLayout->removeWidget(i);
            }
            delete p.buttonLayout;
            delete p.layout;
            switch (p.orientation)
            {
            case Qt::Horizontal:
                p.buttonLayout = new QHBoxLayout;
                p.layout = new QHBoxLayout;
                break;
            case Qt::Vertical:
                p.buttonLayout = new QVBoxLayout;
                p.layout = new QVBoxLayout;
                break;
            default: break;
            }
            for (auto i : p.toButton)
            {
                p.buttonLayout->addWidget(i);
            }
            p.layout->addLayout(p.buttonLayout);
            p.layout->addStretch();
            setLayout(p.layout);
        }
    }
}
