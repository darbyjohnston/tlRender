// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/ColorTool.h>

#include <tlPlay/ColorModel.h>

#include <tlCore/Path.h>

#include <QBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
#include <QToolButton>

namespace tl
{
    namespace play
    {
        struct ColorTool::Private
        {
            std::shared_ptr<ColorModel> colorModel;
            ColorModelData data;
            QLineEdit* fileNameLineEdit = nullptr;
            QListView* inputListView = nullptr;
            QListView* displayListView = nullptr;
            QListView* viewListView = nullptr;
            std::shared_ptr<observer::ValueObserver<ColorModelData> > dataObserver;
        };

        ColorTool::ColorTool(
            const std::shared_ptr<ColorModel>& colorModel,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.colorModel = colorModel;

            p.fileNameLineEdit = new QLineEdit;
            auto fileNameButton = new QToolButton;
            fileNameButton->setIcon(QIcon(":/Icons/FileBrowser.svg"));
            fileNameButton->setAutoRaise(true);

            p.inputListView = new QListView;
            p.inputListView->setAlternatingRowColors(true);
            p.inputListView->setSelectionMode(QAbstractItemView::NoSelection);
            p.inputListView->setModel(new ColorInputListModel(colorModel, this));

            p.displayListView = new QListView;
            p.displayListView->setAlternatingRowColors(true);
            p.displayListView->setSelectionMode(QAbstractItemView::NoSelection);
            p.displayListView->setModel(new ColorDisplayListModel(colorModel, this));

            p.viewListView = new QListView;
            p.viewListView->setAlternatingRowColors(true);
            p.viewListView->setSelectionMode(QAbstractItemView::NoSelection);
            p.viewListView->setModel(new ColorViewListModel(colorModel, this));

            auto hLayout = new QHBoxLayout;
            hLayout->addWidget(p.fileNameLineEdit);
            hLayout->addWidget(fileNameButton);
            auto widget = new QWidget;
            widget->setLayout(hLayout);
            addBellows(tr("Configuration"), widget);
            addBellows(tr("Input"), p.inputListView);
            addBellows(tr("Display"), p.displayListView);
            addBellows(tr("View"), p.viewListView);
            addStretch();

            connect(
                fileNameButton,
                &QToolButton::clicked,
                [this]
                {
                    QString dir;
                    if (!_p->data.fileName.empty())
                    {
                        dir = QString::fromUtf8(file::Path(_p->data.fileName).get().c_str());
                    }

                    const auto fileName = QFileDialog::getOpenFileName(
                        window(),
                        tr("Open"),
                        dir,
                        tr("Files") + " (*.ocio)");
                    if (!fileName.isEmpty())
                    {
                        _p->colorModel->setConfig(fileName.toUtf8().data());
                    }
                });

            connect(
                p.fileNameLineEdit,
                &QLineEdit::editingFinished,
                [this]
                {
                    _p->colorModel->setConfig(_p->fileNameLineEdit->text().toUtf8().data());
                });

            connect(
                p.inputListView,
                &QAbstractItemView::activated,
                [this](const QModelIndex& index)
                {
                    _p->colorModel->setInputIndex(index.row());
                });

            connect(
                p.displayListView,
                &QAbstractItemView::activated,
                [this](const QModelIndex& index)
                {
                    _p->colorModel->setDisplayIndex(index.row());
                });

            connect(
                p.viewListView,
                &QAbstractItemView::activated,
                [this](const QModelIndex& index)
                {
                    _p->colorModel->setViewIndex(index.row());
                });

            p.dataObserver = observer::ValueObserver<ColorModelData>::create(
                colorModel->observeData(),
                [this](const ColorModelData& value)
                {
                    _p->data = value;
                    _widgetUpdate();
                });
        }

        ColorTool::~ColorTool()
        {}

        void ColorTool::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.fileNameLineEdit);
                p.fileNameLineEdit->setText(QString::fromUtf8(p.data.fileName.c_str()));
            }
        }
    }
}
