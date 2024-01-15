// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/FilesTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlQtWidget/FloatEditSlider.h>

#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QToolButton>

namespace tl
{
    namespace play_qt
    {
        struct FilesTool::Private
        {
            App* app = nullptr;
            std::vector<std::shared_ptr<play::FilesModelItem> > items;

            std::vector<QCheckBox*> aButtons;
            std::vector<QToolButton*> bButtons;
            std::vector<QComboBox*> layerComboBoxes;
            QGridLayout* itemsLayout = nullptr;
            QLabel* noFilesOpenLabel = nullptr;
            qtwidget::FloatEditSlider* wipeXSlider = nullptr;
            qtwidget::FloatEditSlider* wipeYSlider = nullptr;
            qtwidget::FloatEditSlider* wipeRotationSlider = nullptr;
            qtwidget::FloatEditSlider* overlaySlider = nullptr;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ValueObserver<std::shared_ptr<play::FilesModelItem> > > aObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > bObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareObserver;
        };

        FilesTool::FilesTool(
            App* app,
            QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.wipeXSlider = new qtwidget::FloatEditSlider;

            p.wipeYSlider = new qtwidget::FloatEditSlider;

            p.wipeRotationSlider = new qtwidget::FloatEditSlider;
            p.wipeRotationSlider->setRange(math::FloatRange(0.F, 360.F));

            p.overlaySlider = new qtwidget::FloatEditSlider;

            auto widget = new QWidget;
            p.itemsLayout = new QGridLayout;
            p.itemsLayout->setColumnStretch(0, 1);
            p.itemsLayout->setSpacing(0);
            widget->setLayout(p.itemsLayout);
            addWidget(widget);

            auto formLayout = new QFormLayout;
            formLayout->addRow(tr("X:"), p.wipeXSlider);
            formLayout->addRow(tr("Y:"), p.wipeYSlider);
            formLayout->addRow(tr("Rotation:"), p.wipeRotationSlider);
            widget = new QWidget;
            widget->setLayout(formLayout);
            addBellows(tr("Wipe"), widget);

            auto layout = new QVBoxLayout;
            layout->addWidget(p.overlaySlider);
            widget = new QWidget;
            widget->setLayout(layout);
            addBellows(tr("Overlay"), widget);

            addStretch();

            connect(
                p.wipeXSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [this, app](double value)
                {
                    auto options = app->filesModel()->getCompareOptions();
                    options.wipeCenter.x = value;
                    app->filesModel()->setCompareOptions(options);
                });

            connect(
                p.wipeYSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [this, app](double value)
                {
                    auto options = app->filesModel()->getCompareOptions();
                    options.wipeCenter.y = value;
                    app->filesModel()->setCompareOptions(options);
                });

            connect(
                p.wipeRotationSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [this, app](double value)
                {
                    auto options = app->filesModel()->getCompareOptions();
                    options.wipeRotation = value;
                    app->filesModel()->setCompareOptions(options);
                });

            connect(
                p.overlaySlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [this, app](double value)
                {
                    auto options = app->filesModel()->getCompareOptions();
                    options.overlay = value;
                    app->filesModel()->setCompareOptions(options);
                });

            p.filesObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->filesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _filesUpdate(value);
                });

            p.aObserver = observer::ValueObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->filesModel()->observeA(),
                [this](const std::shared_ptr<play::FilesModelItem>& value)
                {
                    _aUpdate(value);
                });

            p.bObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->filesModel()->observeB(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _bUpdate(value);
                });

            p.layersObserver = observer::ListObserver<int>::create(
                app->filesModel()->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    _layersUpdate(value);
                });

            p.compareObserver = observer::ValueObserver<timeline::CompareOptions>::create(
                app->filesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _compareUpdate(value);
                });
        }

        void FilesTool::_filesUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();

            for (auto i : p.aButtons)
            {
                delete i;
            }
            p.aButtons.clear();
            for (auto i : p.bButtons)
            {
                delete i;
            }
            p.bButtons.clear();
            for (auto i : p.layerComboBoxes)
            {
                delete i;
            }
            p.layerComboBoxes.clear();
            delete p.noFilesOpenLabel;
            p.noFilesOpenLabel = nullptr;

            p.items = items;

            const auto& a = p.app->filesModel()->getA();
            const auto& b = p.app->filesModel()->getB();
            for (size_t i = 0; i < p.items.size(); ++i)
            {
                auto item = p.items[i];

                auto aButton = new QCheckBox;
                std::string s = string::elide(item->path.get(-1, file::PathType::FileName));
                aButton->setText(QString::fromUtf8(s.c_str()));
                aButton->setCheckable(true);
                aButton->setChecked(item == a);
                aButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                aButton->setToolTip(QString::fromUtf8(item->path.get().c_str()));
                p.aButtons.push_back(aButton);

                auto bButton = new QToolButton;
                bButton->setText("B");
                bButton->setCheckable(true);
                const auto j = std::find(b.begin(), b.end(), item);
                bButton->setChecked(j != b.end());
                bButton->setAutoRaise(true);
                bButton->setToolTip("Set the B file(s)");
                p.bButtons.push_back(bButton);

                auto layerComboBox = new QComboBox;
                for (const auto& layer : item->videoLayers)
                {
                    layerComboBox->addItem(QString::fromUtf8(layer.c_str()));
                }
                layerComboBox->setCurrentIndex(item->videoLayer);
                layerComboBox->setToolTip("Set the current layer");
                p.layerComboBoxes.push_back(layerComboBox);

                p.itemsLayout->addWidget(aButton, i, 0);
                p.itemsLayout->addWidget(bButton, i, 1);
                p.itemsLayout->addWidget(layerComboBox, i, 2);

                connect(
                    aButton,
                    &QToolButton::toggled,
                    [this, i](bool value)
                    {
                        _p->app->filesModel()->setA(i);
                    });

                connect(
                    bButton,
                    &QToolButton::toggled,
                    [this, i](bool value)
                    {
                        _p->app->filesModel()->setB(i, value);
                    });

                connect(
                    layerComboBox,
                    QOverload<int>::of(&QComboBox::currentIndexChanged),
                    [this, item](int value)
                    {
                        _p->app->filesModel()->setLayer(item, value);
                    });
            }
            if (p.items.empty())
            {
                p.noFilesOpenLabel = new QLabel("No files open");
                p.itemsLayout->addWidget(p.noFilesOpenLabel, 0, 0);
            }
        }

        void FilesTool::_aUpdate(const std::shared_ptr<play::FilesModelItem>& item)
        {
            TLRENDER_P();
            for (size_t i = 0; i < p.items.size() && i < p.aButtons.size(); ++i)
            {
                QSignalBlocker signalBlocker(p.aButtons[i]);
                p.aButtons[i]->setChecked(item == p.items[i]);
            }
        }

        void FilesTool::_bUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >& items)
        {
            TLRENDER_P();
            for (size_t i = 0; i < p.items.size() && i < p.bButtons.size(); ++i)
            {
                QSignalBlocker signalBlocker(p.bButtons[i]);
                const auto j = std::find(items.begin(), items.end(), p.items[i]);
                const bool checked = j != items.end();
                p.bButtons[i]->setChecked(checked);
            }
        }

        void FilesTool::_layersUpdate(const std::vector<int>& values)
        {
            TLRENDER_P();
            for (size_t i = 0; i < p.items.size() && i < values.size(); ++i)
            {
                QSignalBlocker signalBlocker(p.layerComboBoxes[i]);
                p.layerComboBoxes[i]->setCurrentIndex(values[i]);
            }
        }

        void FilesTool::_compareUpdate(const timeline::CompareOptions& options)
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.wipeXSlider);
                p.wipeXSlider->setValue(options.wipeCenter.x);
            }
            {
                QSignalBlocker signalBlocker(p.wipeYSlider);
                p.wipeYSlider->setValue(options.wipeCenter.y);
            }
            {
                QSignalBlocker signalBlocker(p.wipeYSlider);
                p.wipeRotationSlider->setValue(options.wipeRotation);
            }
            {
                QSignalBlocker signalBlocker(p.overlaySlider);
                p.overlaySlider->setValue(options.overlay);
            }
        }

        FilesTool::~FilesTool()
        {}

        FilesDockWidget::FilesDockWidget(
            FilesTool* filesTool,
            QWidget* parent)
        {
            setObjectName("FilesTool");
            setWindowTitle(tr("Files"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Files"));
            dockTitleBar->setIcon(QIcon(":/Icons/Files.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(filesTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Files.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F1));
            toggleViewAction()->setToolTip(tr("Show files"));
        }
    }
}
