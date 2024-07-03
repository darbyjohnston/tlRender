// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/ColorPickerTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>
#include <tlPlayQtApp/MainWindow.h>

#include <tlQtWidget/ColorSwatch.h>

#include <tlPlay/Viewport.h>

#include <tlCore/StringFormat.h>

#include <QAction>
#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>

namespace tl
{
    namespace play_qt
    {
        struct ColorPickerTool::Private
        {
            image::Color4f color;
            qtwidget::ColorSwatch* colorSwatch = nullptr;
            std::vector<QLabel*> labels;
            std::shared_ptr<observer::ValueObserver<image::Color4f> > observer;
        };

        ColorPickerTool::ColorPickerTool(MainWindow* mainWindow, App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.colorSwatch = new qtwidget::ColorSwatch;
            p.colorSwatch->setSwatchSize(40);

            p.labels.push_back(new QLabel);
            p.labels.push_back(new QLabel);
            p.labels.push_back(new QLabel);
            p.labels.push_back(new QLabel);

            auto widget = new QWidget;
            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.colorSwatch);
            auto formLayout = new QFormLayout;
            layout->addLayout(formLayout);
            formLayout->addRow("Red:", p.labels[0]);
            formLayout->addRow("Green:", p.labels[1]);
            formLayout->addRow("Blue:", p.labels[2]);
            formLayout->addRow("Alpha:", p.labels[3]);
            widget->setLayout(layout);
            addWidget(widget);

            addStretch();

            _widgetUpdate();

            p.observer = observer::ValueObserver<image::Color4f>::create(
                mainWindow->viewport()->observeColorPicker(),
                [this](const image::Color4f& value)
                {
                    _p->color = value;
                    _widgetUpdate();
                });
        }

        ColorPickerTool::~ColorPickerTool()
        {}

        void ColorPickerTool::_widgetUpdate()
        {
            TLRENDER_P();
            p.colorSwatch->setColor(p.color);
            p.labels[0]->setText(QString::fromUtf8(
                std::string(string::Format("{0}").arg(p.color.r)).c_str()));
            p.labels[1]->setText(QString::fromUtf8(
                std::string(string::Format("{0}").arg(p.color.g)).c_str()));
            p.labels[2]->setText(QString::fromUtf8(
                std::string(string::Format("{0}").arg(p.color.b)).c_str()));
            p.labels[3]->setText(QString::fromUtf8(
                std::string(string::Format("{0}").arg(p.color.a)).c_str()));
        }

        ColorPickerDockWidget::ColorPickerDockWidget(
            ColorPickerTool* colorPickerTool,
            QWidget* parent)
        {
            setObjectName("ColorPickerTool");
            setWindowTitle(tr("Color Picker"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Color Picker"));
            dockTitleBar->setIcon(QIcon(":/Icons/ColorPicker.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(colorPickerTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/ColorPicker.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F4));
            toggleViewAction()->setToolTip(tr("Show color picker"));
        }
    }
}
