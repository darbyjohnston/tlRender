// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/FilesTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <QBoxLayout>
#include <QComboBox>
#include <QGridLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QToolButton>

namespace tl
{
    namespace play_qt
    {
        struct FileWidget::Private
        {
            QLabel* label = nullptr;
            QToolButton* aButton = nullptr;
            QToolButton* bButton = nullptr;
            QComboBox* layerComboBox = nullptr;
        };

        FileWidget::FileWidget(
            const std::shared_ptr<play::FilesModelItem>& item,
            QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            //setAutoFillBackground(true);
            //setBackgroundRole(QPalette::Button);
            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

            p.label = new QLabel;
            p.label->setText(QString::fromUtf8(item->path.get(-1, false).c_str()));
            p.label->setContentsMargins(5, 5, 5, 5);

            p.aButton = new QToolButton;
            p.aButton->setText("A");
            p.aButton->setCheckable(true);
            p.aButton->setAutoRaise(true);

            p.bButton = new QToolButton;
            p.bButton->setText("B");
            p.bButton->setCheckable(true);
            p.bButton->setAutoRaise(true);

            p.layerComboBox = new QComboBox;
            for (const auto& layer : item->videoLayers)
            {
                p.layerComboBox->addItem(QString::fromUtf8(layer.c_str()));
            }
            p.layerComboBox->setCurrentIndex(item->videoLayer);

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.label);
            layout->addStretch();
            layout->addWidget(p.aButton);
            layout->addWidget(p.bButton);
            layout->addWidget(p.layerComboBox);
            setLayout(layout);

            connect(
                p.aButton,
                &QToolButton::toggled,
                [this](bool value)
                {
                    Q_EMIT aChanged(value);
                });

            connect(
                p.bButton,
                &QToolButton::toggled,
                [this](bool value)
                {
                    Q_EMIT bChanged(value);
                });

            connect(
                p.layerComboBox,
                &QComboBox::currentIndexChanged,
                [this](int value)
                {
                    Q_EMIT layerChanged(value);
                });
        }

        FileWidget::~FileWidget()
        {}

        void FileWidget::setA(bool value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.aButton);
            p.aButton->setChecked(value);
        }

        void FileWidget::setB(bool value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.bButton);
            p.bButton->setChecked(value);
        }

        void FileWidget::setLayer(int value)
        {
            TLRENDER_P();
            QSignalBlocker signalBlocker(p.layerComboBox);
            p.layerComboBox->setCurrentIndex(value);
        }

        struct FilesTool::Private
        {
            App* app = nullptr;
            std::vector<std::shared_ptr<play::FilesModelItem> > items;

            //std::vector<FileWidget*> fileWidgets;
            std::vector<QLabel*> labels;
            std::vector<QToolButton*> aButtons;
            std::vector<QToolButton*> bButtons;
            std::vector<QComboBox*> layerComboBoxes;
            QGridLayout* itemsLayout = nullptr;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ValueObserver<std::shared_ptr<play::FilesModelItem> > > aObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > bObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareObserver;
        };

        FilesTool::FilesTool(
            const QMap<QString, QAction*>& actions,
            App* app,
            QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            auto widget = new QWidget;
            p.itemsLayout = new QGridLayout;
            p.itemsLayout->setColumnStretch(0, 1);
            p.itemsLayout->setContentsMargins(0, 0, 0, 0);
            p.itemsLayout->setSpacing(0);
            widget->setLayout(p.itemsLayout);
            addWidget(widget);
            addStretch();

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

            for (auto i : p.labels)
            {
                delete i;
            }
            p.labels.clear();
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

            p.items = items;
            for (size_t i = 0; i < p.items.size(); ++i)
            {
                auto item = p.items[i];

                auto label = new QLabel;
                std::string s = string::elide(item->path.get(-1, false));
                label->setText(QString::fromUtf8(s.c_str()));
                label->setToolTip(QString::fromUtf8(item->path.get().c_str()));
                label->setContentsMargins(5, 5, 5, 5);
                p.labels.push_back(label);

                auto aButton = new QToolButton;
                aButton->setText("A");
                aButton->setCheckable(true);
                aButton->setAutoRaise(true);
                aButton->setToolTip("Set the A file");
                p.aButtons.push_back(aButton);

                auto bButton = new QToolButton;
                bButton->setText("B");
                bButton->setCheckable(true);
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

                p.itemsLayout->addWidget(label, i, 0);
                p.itemsLayout->addWidget(aButton, i, 1);
                p.itemsLayout->addWidget(bButton, i, 2);
                p.itemsLayout->addWidget(layerComboBox, i, 3);

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
                    &QComboBox::currentIndexChanged,
                    [this, item](int value)
                    {
                        _p->app->filesModel()->setLayer(item, value);
                    });
            }

            /*for (auto i : p.fileWidgets)
            {
                delete i;
            }
            p.fileWidgets.clear();
            p.itemToWidget.clear();
            for (size_t i = 0; i < items.size(); ++i)
            {
                auto item = items[i];
                auto fileWidget = new FileWidget(item);
                p.fileWidgets.push_back(fileWidget);
                p.itemToWidget[item] = fileWidget;
                p.fileLayout->addWidget(fileWidget);
                connect(
                    fileWidget,
                    &FileWidget::aChanged,
                    [this, i](bool)
                    {
                        _p->app->filesModel()->setA(i);
                    });
                connect(
                    fileWidget,
                    &FileWidget::bChanged,
                    [this, i](bool value)
                    {
                        _p->app->filesModel()->setB(i, value);
                    });
                connect(
                    fileWidget,
                    &FileWidget::layerChanged,
                    [this, item](int value)
                    {
                        _p->app->filesModel()->setLayer(item, value);
                    });
            }*/
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
                p.bButtons[i]->setChecked(j != items.end());
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

        void FilesTool::_compareUpdate(const timeline::CompareOptions&)
        {}

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
