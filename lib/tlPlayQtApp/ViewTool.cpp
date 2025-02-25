// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/ViewTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlQtWidget/ColorSwatch.h>
#include <tlQtWidget/ColorWidget.h>
#include <tlQtWidget/IntEditSlider.h>

#include <QAction>
#include <QBoxLayout>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>

namespace tl
{
    namespace play_qt
    {
        struct BackgroundWidget::Private
        {
            QComboBox* typeComboBox = nullptr;
            qtwidget::ColorSwatch* solidSwatch = nullptr;
            std::pair<qtwidget::ColorSwatch*, qtwidget::ColorSwatch*> checkersSwatch = { nullptr, nullptr };
            qtwidget::IntEditSlider* checkersSizeSlider = nullptr;
            std::pair<qtwidget::ColorSwatch*, qtwidget::ColorSwatch*> gradientSwatch = { nullptr, nullptr };
            QFormLayout* layout = nullptr;

            std::shared_ptr<dtk::ValueObserver<timeline::BackgroundOptions> > optionsObservers;
        };

        BackgroundWidget::BackgroundWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            DTK_P();

            p.typeComboBox = new QComboBox;
            for (const auto& i : timeline::getBackgroundLabels())
            {
                p.typeComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.solidSwatch = new qtwidget::ColorSwatch;
            p.solidSwatch->setEditable(true);

            p.checkersSwatch.first = new qtwidget::ColorSwatch;
            p.checkersSwatch.first->setEditable(true);
            p.checkersSwatch.second = new qtwidget::ColorSwatch;
            p.checkersSwatch.second->setEditable(true);

            p.checkersSizeSlider = new qtwidget::IntEditSlider;
            p.checkersSizeSlider->setRange(dtk::RangeI(10, 100));

            p.gradientSwatch.first = new qtwidget::ColorSwatch;
            p.gradientSwatch.first->setEditable(true);
            p.gradientSwatch.second = new qtwidget::ColorSwatch;
            p.gradientSwatch.second->setEditable(true);

            p.layout = new QFormLayout;
            p.layout->addRow(tr("Type:"), p.typeComboBox);
            p.layout->addRow(tr("Color:"), p.solidSwatch);
            p.layout->addRow(tr("Color 1:"), p.checkersSwatch.first);
            p.layout->addRow(tr("Color 2:"), p.checkersSwatch.second);
            p.layout->addRow(tr("Size:"), p.checkersSizeSlider);
            p.layout->addRow(tr("Color 1:"), p.gradientSwatch.first);
            p.layout->addRow(tr("Color 2:"), p.gradientSwatch.second);
            setLayout(p.layout);

            connect(
                p.typeComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                [app](int value)
                {
                    auto options = app->viewportModel()->getBackgroundOptions();
                    options.type = static_cast<timeline::Background>(value);
                    app->viewportModel()->setBackgroundOptions(options);
                });

            connect(
                p.solidSwatch,
                &qtwidget::ColorSwatch::colorChanged,
                [app](const dtk::Color4F& value)
                {
                    auto options = app->viewportModel()->getBackgroundOptions();
                    options.solidColor = value;
                    app->viewportModel()->setBackgroundOptions(options);
                });

            connect(
                p.checkersSwatch.first,
                &qtwidget::ColorSwatch::colorChanged,
                [app](const dtk::Color4F& value)
                {
                    auto options = app->viewportModel()->getBackgroundOptions();
                    options.checkersColor.first = value;
                    app->viewportModel()->setBackgroundOptions(options);
                });

            connect(
                p.checkersSwatch.second,
                &qtwidget::ColorSwatch::colorChanged,
                [app](const dtk::Color4F& value)
                {
                    auto options = app->viewportModel()->getBackgroundOptions();
                    options.checkersColor.second = value;
                    app->viewportModel()->setBackgroundOptions(options);
                });

            connect(
                p.checkersSizeSlider,
                &qtwidget::IntEditSlider::valueChanged,
                [app](int value)
                {
                    auto options = app->viewportModel()->getBackgroundOptions();
                    options.checkersSize.w = value;
                    options.checkersSize.h = value;
                    app->viewportModel()->setBackgroundOptions(options);
                });

            connect(
                p.gradientSwatch.first,
                &qtwidget::ColorSwatch::colorChanged,
                [app](const dtk::Color4F& value)
                {
                    auto options = app->viewportModel()->getBackgroundOptions();
                    options.gradientColor.first = value;
                    app->viewportModel()->setBackgroundOptions(options);
                });

            connect(
                p.gradientSwatch.second,
                &qtwidget::ColorSwatch::colorChanged,
                [app](const dtk::Color4F& value)
                {
                    auto options = app->viewportModel()->getBackgroundOptions();
                    options.gradientColor.second = value;
                    app->viewportModel()->setBackgroundOptions(options);
                });

            p.optionsObservers = dtk::ValueObserver<timeline::BackgroundOptions>::create(
                app->viewportModel()->observeBackgroundOptions(),
                [this](const timeline::BackgroundOptions& value)
                {
                    _optionsUpdate(value);
                });
        }

        BackgroundWidget::~BackgroundWidget()
        {}

        void BackgroundWidget::_optionsUpdate(const timeline::BackgroundOptions & value)
        {
            DTK_P();
            {
                QSignalBlocker signalBlocker(p.typeComboBox);
                p.typeComboBox->setCurrentIndex(static_cast<int>(value.type));
            }
            p.solidSwatch->setColor(value.solidColor);
            p.checkersSwatch.first->setColor(value.checkersColor.first);
            p.checkersSwatch.second->setColor(value.checkersColor.second);
            {
                QSignalBlocker signalBlocker(p.checkersSizeSlider);
                p.checkersSizeSlider->setValue(value.checkersSize.w);
            }
            p.gradientSwatch.first->setColor(value.gradientColor.first);
            p.gradientSwatch.second->setColor(value.gradientColor.second);

#if defined(TLRENDER_QT6)
            p.layout->setRowVisible(p.solidSwatch, value.type == timeline::Background::Solid);
            p.layout->setRowVisible(p.checkersSwatch.first, value.type == timeline::Background::Checkers);
            p.layout->setRowVisible(p.checkersSwatch.second, value.type == timeline::Background::Checkers);
            p.layout->setRowVisible(p.checkersSizeSlider, value.type == timeline::Background::Checkers);
            p.layout->setRowVisible(p.gradientSwatch.first, value.type == timeline::Background::Gradient);
            p.layout->setRowVisible(p.gradientSwatch.second, value.type == timeline::Background::Gradient);
#elif defined(TLRENDER_QT5)
            //! \todo Qt5 form layout.
#endif // TLRENDER_QT6
        }

        struct ViewTool::Private
        {
            BackgroundWidget* backgroundWidget = nullptr;
        };

        ViewTool::ViewTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            DTK_P();

            p.backgroundWidget = new BackgroundWidget(app);

            addBellows(tr("Background"), p.backgroundWidget);
            addStretch();
        }

        ViewTool::~ViewTool()
        {}

        ViewDockWidget::ViewDockWidget(
            ViewTool* viewTool,
            QWidget* parent)
        {
            setObjectName("ViewTool");
            setWindowTitle(tr("View"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("View"));
            dockTitleBar->setIcon(QIcon(":/Icons/View.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(viewTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/View.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F2));
            toggleViewAction()->setToolTip(tr("Show view controls"));
        }
    }
}
