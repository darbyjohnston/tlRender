// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlay/App.h>
#include <tlPlay/CompareTool.h>
#include <tlPlay/FilesModel.h>
#include <tlPlay/FilesView.h>

#include <tlQWidget/RadioButtonGroup.h>

#include <tlQt/Util.h>

#include <QBoxLayout>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QHeaderView>
#include <QLabel>
#include <QSignalBlocker>
#include <QSlider>
#include <QSettings>
#include <QTreeView>

namespace tl
{
    namespace play
    {
        namespace
        {
            const size_t sliderSteps = 1000;
        }

        struct CompareTool::Private
        {
            std::shared_ptr<FilesModel> filesModel;
            FilesBModel* filesBModel = nullptr;
            render::CompareOptions compareOptions;
            QTreeView* treeView = nullptr;
            qwidget::RadioButtonGroup* modeButtonGroup = nullptr;
            QDoubleSpinBox* wipeXSpinBox = nullptr;
            QSlider* wipeXSlider = nullptr;
            QDoubleSpinBox* wipeYSpinBox = nullptr;
            QSlider* wipeYSlider = nullptr;
            QDoubleSpinBox* wipeRotationSpinBox = nullptr;
            QSlider* wipeRotationSlider = nullptr;
        };

        CompareTool::CompareTool(
            const std::shared_ptr<FilesModel>& filesModel,
            const std::shared_ptr<core::Context>& context,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.filesModel = filesModel;

            p.filesBModel = new FilesBModel(filesModel, context, this);

            p.treeView = new QTreeView;
            p.treeView->setAllColumnsShowFocus(true);
            p.treeView->setAlternatingRowColors(true);
            p.treeView->setSelectionMode(QAbstractItemView::NoSelection);
            p.treeView->setItemDelegateForColumn(1, new FilesLayersItemDelegate);
            p.treeView->setEditTriggers(QAbstractItemView::CurrentChanged);
            p.treeView->setIndentation(0);
            //! \bug Setting the model causes this output to be printed on exit:
            //! QBasicTimer::start: QBasicTimer can only be used with threads started with QThread
            p.treeView->setModel(p.filesBModel);

            p.modeButtonGroup = new qwidget::RadioButtonGroup;
            for (const auto i : render::getCompareModeEnums())
            {
                p.modeButtonGroup->addButton(
                    QString::fromUtf8(render::getLabel(i).c_str()),
                    QVariant::fromValue<render::CompareMode>(i));
            }

            p.wipeXSpinBox = new QDoubleSpinBox;
            p.wipeXSpinBox->setRange(0.0, 1.0);
            p.wipeXSpinBox->setSingleStep(0.1);
            p.wipeXSlider = new QSlider(Qt::Orientation::Horizontal);
            p.wipeXSlider->setRange(0, sliderSteps);

            p.wipeYSpinBox = new QDoubleSpinBox;
            p.wipeYSpinBox->setRange(0.0, 1.0);
            p.wipeYSpinBox->setSingleStep(0.1);
            p.wipeYSlider = new QSlider(Qt::Orientation::Horizontal);
            p.wipeYSlider->setRange(0, sliderSteps);

            p.wipeRotationSpinBox = new QDoubleSpinBox;
            p.wipeRotationSpinBox->setRange(0.0, 360.0);
            p.wipeRotationSpinBox->setSingleStep(10.0);
            p.wipeRotationSlider = new QSlider(Qt::Orientation::Horizontal);
            p.wipeRotationSlider->setRange(0, sliderSteps);

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.treeView);
            auto vLayout = new QVBoxLayout;
            vLayout->setContentsMargins(10, 10, 10, 10);
            vLayout->setSpacing(10);
            vLayout->addWidget(p.modeButtonGroup);
            vLayout->addWidget(new QLabel(tr("Wipe")));
            auto formLayout = new QFormLayout;
            auto hLayout = new QHBoxLayout;
            hLayout->addWidget(p.wipeXSpinBox);
            hLayout->addWidget(p.wipeXSlider);
            formLayout->addRow(tr("X:"), hLayout);
            hLayout = new QHBoxLayout;
            hLayout->addWidget(p.wipeYSpinBox);
            hLayout->addWidget(p.wipeYSlider);
            formLayout->addRow(tr("Y:"), hLayout);
            hLayout = new QHBoxLayout;
            hLayout->addWidget(p.wipeRotationSpinBox);
            hLayout->addWidget(p.wipeRotationSlider);
            formLayout->addRow(tr("Rotation:"), hLayout);
            vLayout->addLayout(formLayout);
            layout->addLayout(vLayout);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget, 1);

            _widgetUpdate();

            QSettings settings;
            auto ba = settings.value(qt::versionedSettingsKey("CompareTool/Header")).toByteArray();
            if (!ba.isEmpty())
            {
                p.treeView->header()->restoreState(ba);
            }

            connect(
                p.treeView,
                SIGNAL(activated(const QModelIndex&)),
                SLOT(_activatedCallback(const QModelIndex&)));

            connect(
                p.modeButtonGroup,
                SIGNAL(checked(const QVariant&)),
                SLOT(_modeCallback(const QVariant&)));

            connect(
                p.wipeXSpinBox,
                SIGNAL(valueChanged(double)),
                SLOT(_wipeXSpinBoxCallback(double)));
            connect(
                p.wipeXSlider,
                SIGNAL(valueChanged(int)),
                SLOT(_wipeXSliderCallback(int)));

            connect(
                p.wipeYSpinBox,
                SIGNAL(valueChanged(double)),
                SLOT(_wipeYSpinBoxCallback(double)));
            connect(
                p.wipeYSlider,
                SIGNAL(valueChanged(int)),
                SLOT(_wipeYSliderCallback(int)));

            connect(
                p.wipeRotationSpinBox,
                SIGNAL(valueChanged(double)),
                SLOT(_wipeRotationSpinBoxCallback(double)));
            connect(
                p.wipeRotationSlider,
                SIGNAL(valueChanged(int)),
                SLOT(_wipeRotationSliderCallback(int)));
        }

        CompareTool::~CompareTool()
        {
            TLRENDER_P();
            QSettings settings;
            settings.setValue(qt::versionedSettingsKey("CompareTool/Header"), p.treeView->header()->saveState());
        }

        void CompareTool::setCompareOptions(const render::CompareOptions& value)
        {
            TLRENDER_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            _widgetUpdate();
        }

        void CompareTool::_activatedCallback(const QModelIndex& index)
        {
            TLRENDER_P();
            p.filesModel->toggleB(index.row());
        }

        void CompareTool::_modeCallback(const QVariant& value)
        {
            TLRENDER_P();
            p.compareOptions.mode = value.value<render::CompareMode>();
            _widgetUpdate();
            Q_EMIT compareOptionsChanged(p.compareOptions);
        }

        void CompareTool::_wipeXSpinBoxCallback(double value)
        {
            TLRENDER_P();
            p.compareOptions.wipeCenter.x = value;
            _widgetUpdate();
            Q_EMIT compareOptionsChanged(p.compareOptions);
        }

        void CompareTool::_wipeXSliderCallback(int value)
        {
            TLRENDER_P();
            p.compareOptions.wipeCenter.x = value / static_cast<float>(sliderSteps);
            _widgetUpdate();
            Q_EMIT compareOptionsChanged(p.compareOptions);
        }

        void CompareTool::_wipeYSliderCallback(int value)
        {
            TLRENDER_P();
            p.compareOptions.wipeCenter.y = value / static_cast<float>(sliderSteps);
            _widgetUpdate();
            Q_EMIT compareOptionsChanged(p.compareOptions);
        }

        void CompareTool::_wipeYSpinBoxCallback(double value)
        {
            TLRENDER_P();
            p.compareOptions.wipeCenter.y = value;
            _widgetUpdate();
            Q_EMIT compareOptionsChanged(p.compareOptions);
        }

        void CompareTool::_wipeRotationSpinBoxCallback(double value)
        {
            TLRENDER_P();
            p.compareOptions.wipeRotation = value;
            _widgetUpdate();
            Q_EMIT compareOptionsChanged(p.compareOptions);
        }

        void CompareTool::_wipeRotationSliderCallback(int value)
        {
            TLRENDER_P();
            p.compareOptions.wipeRotation = value / static_cast<float>(sliderSteps) * 360.F;
            _widgetUpdate();
            Q_EMIT compareOptionsChanged(p.compareOptions);
        }

        void CompareTool::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.modeButtonGroup);
                p.modeButtonGroup->setChecked(QVariant::fromValue<render::CompareMode>(p.compareOptions.mode));
            }
            {
                QSignalBlocker signalBlocker(p.wipeXSpinBox);
                p.wipeXSpinBox->setValue(p.compareOptions.wipeCenter.x);
            }
            {
                QSignalBlocker signalBlocker(p.wipeXSlider);
                p.wipeXSlider->setValue(p.compareOptions.wipeCenter.x * sliderSteps);
            }
            {
                QSignalBlocker signalBlocker(p.wipeYSpinBox);
                p.wipeYSpinBox->setValue(p.compareOptions.wipeCenter.y);
            }
            {
                QSignalBlocker signalBlocker(p.wipeYSlider);
                p.wipeYSlider->setValue(p.compareOptions.wipeCenter.y * sliderSteps);
            }
            {
                QSignalBlocker signalBlocker(p.wipeRotationSpinBox);
                p.wipeRotationSpinBox->setValue(p.compareOptions.wipeRotation);
            }
            {
                QSignalBlocker signalBlocker(p.wipeYSlider);
                p.wipeRotationSlider->setValue(p.compareOptions.wipeRotation / 360.F * sliderSteps);
            }
        }
    }
}
