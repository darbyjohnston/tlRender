// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ColorTool.h>

#include <tlPlayApp/ColorModel.h>
#include <tlPlayApp/DockTitleBar.h>

#include <tlQtWidget/FileWidget.h>
#include <tlQtWidget/FloatSlider.h>
#include <tlQtWidget/SearchWidget.h>

#include <tlCore/Path.h>

#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QListView>
#include <QSortFilterProxyModel>
#include <QTabWidget>
#include <QToolButton>

namespace tl
{
    namespace play
    {
        struct ConfigWidget::Private
        {
            std::shared_ptr<ColorModel> colorModel;
            ColorModelData data;
            qtwidget::FileWidget* fileWidget = nullptr;
            std::shared_ptr<observer::ValueObserver<ColorModelData> > dataObserver;
        };

        ConfigWidget::ConfigWidget(
            const std::shared_ptr<ColorModel>& colorModel,
            QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.colorModel = colorModel;

            auto inputListModel = new ColorInputListModel(colorModel, this);
            auto inputListProxyModel = new QSortFilterProxyModel(this);
            inputListProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            inputListProxyModel->setFilterKeyColumn(-1);
            inputListProxyModel->setSourceModel(inputListModel);

            auto displayListModel = new ColorDisplayListModel(colorModel, this);
            auto displayListProxyModel = new QSortFilterProxyModel(this);
            displayListProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            displayListProxyModel->setFilterKeyColumn(-1);
            displayListProxyModel->setSourceModel(displayListModel);

            auto viewListModel = new ColorViewListModel(colorModel, this);
            auto viewListProxyModel = new QSortFilterProxyModel(this);
            viewListProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            viewListProxyModel->setFilterKeyColumn(-1);
            viewListProxyModel->setSourceModel(viewListModel);

            p.fileWidget = new qtwidget::FileWidget({ ".ocio" });

            auto inputListView = new QListView;
            inputListView->setAlternatingRowColors(true);
            inputListView->setSelectionMode(QAbstractItemView::NoSelection);
            inputListView->setModel(inputListProxyModel);

            auto inputSearchWidget = new qtwidget::SearchWidget;
            inputSearchWidget->setContentsMargins(2, 2, 2, 2);

            auto displayListView = new QListView;
            displayListView->setAlternatingRowColors(true);
            displayListView->setSelectionMode(QAbstractItemView::NoSelection);
            displayListView->setModel(displayListProxyModel);

            auto displaySearchWidget = new qtwidget::SearchWidget;
            displaySearchWidget->setContentsMargins(2, 2, 2, 2);

            auto viewListView = new QListView;
            viewListView->setAlternatingRowColors(true);
            viewListView->setSelectionMode(QAbstractItemView::NoSelection);
            viewListView->setModel(viewListProxyModel);

            auto viewSearchWidget = new qtwidget::SearchWidget;
            viewSearchWidget->setContentsMargins(2, 2, 2, 2);

            auto formLayout = new QFormLayout;
            formLayout->addRow(tr("File name:"), p.fileWidget);

            auto tabWidget = new QTabWidget;
            auto widget = new QWidget;
            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(inputListView);
            layout->addWidget(inputSearchWidget);
            widget->setLayout(layout);
            tabWidget->addTab(widget, tr("Input"));
            widget = new QWidget;
            layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(displayListView);
            layout->addWidget(displaySearchWidget);
            widget->setLayout(layout);
            tabWidget->addTab(widget, tr("Display"));
            widget = new QWidget;
            layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(viewListView);
            layout->addWidget(viewSearchWidget);
            widget->setLayout(layout);
            tabWidget->addTab(widget, tr("View"));

            layout = new QVBoxLayout;
            layout->addLayout(formLayout);
            layout->addWidget(tabWidget);
            setLayout(layout);

            connect(
                p.fileWidget,
                &qtwidget::FileWidget::fileChanged,
                [this](const QString& value)
                {
                    _p->colorModel->setConfig(value.toUtf8().data());
                });

            connect(
                inputListView,
                &QAbstractItemView::activated,
                [this, inputListProxyModel](const QModelIndex& index)
                {
                    auto sourceIndex = inputListProxyModel->mapToSource(index);
                    _p->colorModel->setInputIndex(sourceIndex.row());
                });

            connect(
                inputSearchWidget,
                SIGNAL(searchChanged(const QString&)),
                inputListProxyModel,
                SLOT(setFilterFixedString(const QString&)));

            connect(
                displayListView,
                &QAbstractItemView::activated,
                [this, displayListProxyModel](const QModelIndex& index)
                {
                    auto sourceIndex = displayListProxyModel->mapToSource(index);
                    _p->colorModel->setDisplayIndex(sourceIndex.row());
                });

            connect(
                displaySearchWidget,
                SIGNAL(searchChanged(const QString&)),
                displayListProxyModel,
                SLOT(setFilterFixedString(const QString&)));

            connect(
                viewListView,
                &QAbstractItemView::activated,
                [this, viewListProxyModel](const QModelIndex& index)
                {
                    auto sourceIndex = viewListProxyModel->mapToSource(index);
                    _p->colorModel->setViewIndex(sourceIndex.row());
                });

            connect(
                viewSearchWidget,
                SIGNAL(searchChanged(const QString&)),
                viewListProxyModel,
                SLOT(setFilterFixedString(const QString&)));

            p.dataObserver = observer::ValueObserver<ColorModelData>::create(
                colorModel->observeData(),
                [this](const ColorModelData& value)
                {
                    _p->data = value;
                    _widgetUpdate();
                });
        }

        ConfigWidget::~ConfigWidget()
        {}

        void ConfigWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.fileWidget);
                p.fileWidget->setFile(QString::fromUtf8(p.data.fileName.c_str()));
            }
        }

        struct LUTWidget::Private
        {
            timeline::LUTOptions lutOptions;

            qtwidget::FileWidget* fileWidget = nullptr;
            QComboBox* orderComboBox = nullptr;
        };

        LUTWidget::LUTWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            QStringList extensions;
            for (const auto& i : timeline::getLUTFormatExtensions())
            {
                extensions.push_back(QString::fromUtf8(i.c_str()));
            }
            p.fileWidget = new qtwidget::FileWidget(extensions);

            p.orderComboBox = new QComboBox;

            auto layout = new QFormLayout;
            layout->addRow(tr("File name:"), p.fileWidget);
            layout->addRow(tr("Order:"), p.orderComboBox);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.fileWidget,
                &qtwidget::FileWidget::fileChanged,
                [this](const QString& value)
                {
                    timeline::LUTOptions options = _p->lutOptions;
                    options.fileName = value.toUtf8().data();
                    Q_EMIT lutOptionsChanged(options);
                });

            connect(
                p.orderComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    timeline::LUTOptions options = _p->lutOptions;
                    options.order = static_cast<timeline::LUTOrder>(value);
                    Q_EMIT lutOptionsChanged(options);
                });
        }

        LUTWidget::~LUTWidget()
        {}

        void LUTWidget::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            _widgetUpdate();
        }

        void LUTWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.fileWidget);
                p.fileWidget->setFile(QString::fromUtf8(p.lutOptions.fileName.c_str()));
            }
            {
                QSignalBlocker blocker(_p->orderComboBox);
                _p->orderComboBox->clear();
                for (const auto& i : timeline::getLUTOrderEnums())
                {
                    std::stringstream ss;
                    ss << i;
                    _p->orderComboBox->addItem(QString::fromUtf8(ss.str().c_str()));
                }
                _p->orderComboBox->setCurrentIndex(static_cast<int>(p.lutOptions.order));
            }
        }

        struct ColorControlsWidget::Private
        {
            bool enabled = false;
            timeline::Color color;

            QCheckBox* enabledCheckBox = nullptr;
            qtwidget::FloatSlider* addSlider = nullptr;
            qtwidget::FloatSlider* brightnessSlider = nullptr;
            qtwidget::FloatSlider* contrastSlider = nullptr;
            qtwidget::FloatSlider* saturationSlider = nullptr;
            qtwidget::FloatSlider* tintSlider = nullptr;
            QCheckBox* invertCheckBox = nullptr;
        };

        ColorControlsWidget::ColorControlsWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.addSlider = new qtwidget::FloatSlider;
            p.addSlider->setRange(math::FloatRange(-1.F, 1.F));
            p.addSlider->setDefaultValue(0.F);

            p.brightnessSlider = new qtwidget::FloatSlider;
            p.brightnessSlider->setRange(math::FloatRange(0.F, 4.F));
            p.brightnessSlider->setDefaultValue(1.F);

            p.contrastSlider = new qtwidget::FloatSlider;
            p.contrastSlider->setRange(math::FloatRange(0.F, 4.F));
            p.contrastSlider->setDefaultValue(1.F);

            p.saturationSlider = new qtwidget::FloatSlider;
            p.saturationSlider->setRange(math::FloatRange(0.F, 4.F));
            p.saturationSlider->setDefaultValue(1.F);

            p.tintSlider = new qtwidget::FloatSlider;
            p.tintSlider->setDefaultValue(0.F);

            p.invertCheckBox = new QCheckBox(tr("Invert"));

            auto layout = new QFormLayout;
            layout->addRow(p.enabledCheckBox);
            layout->addRow(tr("Add:"), p.addSlider);
            layout->addRow(tr("Brightness:"), p.brightnessSlider);
            layout->addRow(tr("Contrast:"), p.contrastSlider);
            layout->addRow(tr("Saturation:"), p.saturationSlider);
            layout->addRow(tr("Tint:"), p.tintSlider);
            layout->addRow(p.invertCheckBox);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    Q_EMIT colorEnabledChanged(value);
                });

            connect(
                p.addSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](const float value)
                {
                    timeline::Color color = _p->color;
                    color.add.x = color.add.y = color.add.z = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });

            connect(
                p.brightnessSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Color color = _p->color;
                    color.brightness.x = color.brightness.y = color.brightness.z = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });

            connect(
                p.contrastSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Color color = _p->color;
                    color.contrast.x = color.contrast.y = color.contrast.z = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });

            connect(
                p.saturationSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Color color = _p->color;
                    color.saturation.x = color.saturation.y = color.saturation.z = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });

            connect(
                p.tintSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Color color = _p->color;
                    color.tint = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });

            connect(
                p.invertCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    timeline::Color color = _p->color;
                    color.invert = value;
                    Q_EMIT colorEnabledChanged(true);
                    Q_EMIT colorChanged(color);
                });
        }

        ColorControlsWidget::~ColorControlsWidget()
        {}

        void ColorControlsWidget::setColorEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.enabled)
                return;
            p.enabled = value;
            _widgetUpdate();
        }

        void ColorControlsWidget::setColor(const timeline::Color& value)
        {
            TLRENDER_P();
            if (value == p.color)
                return;
            p.color = value;
            _widgetUpdate();
        }

        void ColorControlsWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.enabledCheckBox);
                p.enabledCheckBox->setChecked(p.enabled);
            }
            {
                QSignalBlocker signalBlocker(p.addSlider);
                p.addSlider->setValue(p.color.add.x);
            }
            {
                QSignalBlocker signalBlocker(p.brightnessSlider);
                p.brightnessSlider->setValue(p.color.brightness.x);
            }
            {
                QSignalBlocker signalBlocker(p.contrastSlider);
                p.contrastSlider->setValue(p.color.contrast.x);
            }
            {
                QSignalBlocker signalBlocker(p.saturationSlider);
                p.saturationSlider->setValue(p.color.saturation.x);
            }
            {
                QSignalBlocker signalBlocker(p.tintSlider);
                p.tintSlider->setValue(p.color.tint);
            }
            {
                QSignalBlocker signalBlocker(p.invertCheckBox);
                p.invertCheckBox->setChecked(p.color.invert);
            }
        }

        struct LevelsWidget::Private
        {
            bool enabled = false;
            timeline::Levels levels;

            QCheckBox* enabledCheckBox = nullptr;
            qtwidget::FloatSlider* inLowSlider = nullptr;
            qtwidget::FloatSlider* inHighSlider = nullptr;
            qtwidget::FloatSlider* gammaSlider = nullptr;
            qtwidget::FloatSlider* outLowSlider = nullptr;
            qtwidget::FloatSlider* outHighSlider = nullptr;
        };

        LevelsWidget::LevelsWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.inLowSlider = new qtwidget::FloatSlider;
            p.inLowSlider->setDefaultValue(0.F);

            p.inHighSlider = new qtwidget::FloatSlider;
            p.inHighSlider->setDefaultValue(1.F);

            p.gammaSlider = new qtwidget::FloatSlider;
            p.gammaSlider->setRange(math::FloatRange(.1F, 4.F));
            p.gammaSlider->setDefaultValue(1.F);

            p.outLowSlider = new qtwidget::FloatSlider;
            p.outLowSlider->setDefaultValue(0.F);

            p.outHighSlider = new qtwidget::FloatSlider;
            p.outHighSlider->setDefaultValue(1.F);

            auto layout = new QFormLayout;
            layout->addRow(p.enabledCheckBox);
            layout->addRow(tr("In low:"), p.inLowSlider);
            layout->addRow(tr("In high:"), p.inHighSlider);
            layout->addRow(tr("Gamma:"), p.gammaSlider);
            layout->addRow(tr("Out low:"), p.outLowSlider);
            layout->addRow(tr("Out high:"), p.outHighSlider);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    Q_EMIT levelsEnabledChanged(value);
                });

            connect(
                p.inLowSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Levels levels = _p->levels;
                    levels.inLow = value;
                    Q_EMIT levelsChanged(levels);
                    Q_EMIT levelsEnabledChanged(true);
                });
            connect(
                p.inHighSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Levels levels = _p->levels;
                    levels.inHigh = value;
                    Q_EMIT levelsChanged(levels);
                    Q_EMIT levelsEnabledChanged(true);
                });

            connect(
                p.gammaSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Levels levels = _p->levels;
                    levels.gamma = value;
                    Q_EMIT levelsChanged(levels);
                    Q_EMIT levelsEnabledChanged(true);
                });

            connect(
                p.outLowSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Levels levels = _p->levels;
                    levels.outLow = value;
                    Q_EMIT levelsChanged(levels);
                    Q_EMIT levelsEnabledChanged(true);
                });
            connect(
                p.outHighSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Levels levels = _p->levels;
                    levels.outHigh = value;
                    Q_EMIT levelsChanged(levels);
                    Q_EMIT levelsEnabledChanged(true);
                });
        }

        LevelsWidget::~LevelsWidget()
        {}

        void LevelsWidget::setLevelsEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.enabled)
                return;
            p.enabled = value;
            _widgetUpdate();
        }

        void LevelsWidget::setLevels(const timeline::Levels& value)
        {
            TLRENDER_P();
            if (value == p.levels)
                return;
            p.levels = value;
            _widgetUpdate();
        }

        void LevelsWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.enabledCheckBox);
                p.enabledCheckBox->setChecked(p.enabled);
            }
            {
                QSignalBlocker signalBlocker(p.inLowSlider);
                p.inLowSlider->setValue(p.levels.inLow);
            }
            {
                QSignalBlocker signalBlocker(p.inHighSlider);
                p.inHighSlider->setValue(p.levels.inHigh);
            }
            {
                QSignalBlocker signalBlocker(p.gammaSlider);
                p.gammaSlider->setValue(p.levels.gamma);
            }
            {
                QSignalBlocker signalBlocker(p.outLowSlider);
                p.outLowSlider->setValue(p.levels.outLow);
            }
            {
                QSignalBlocker signalBlocker(p.outHighSlider);
                p.outHighSlider->setValue(p.levels.outHigh);
            }
        }

        struct EXRDisplayWidget::Private
        {
            bool enabled = false;
            timeline::EXRDisplay exrDisplay;

            QCheckBox* enabledCheckBox = nullptr;
            qtwidget::FloatSlider* exposureSlider = nullptr;
            qtwidget::FloatSlider* defogSlider = nullptr;
            qtwidget::FloatSlider* kneeLowSlider = nullptr;
            qtwidget::FloatSlider* kneeHighSlider = nullptr;
        };

        EXRDisplayWidget::EXRDisplayWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.exposureSlider = new qtwidget::FloatSlider;
            p.exposureSlider->setRange(math::FloatRange(-10.F, 10.F));
            p.exposureSlider->setDefaultValue(0.F);

            p.defogSlider = new qtwidget::FloatSlider;
            p.defogSlider->setRange(math::FloatRange(0.F, .1F));
            p.defogSlider->setDefaultValue(0.F);

            p.kneeLowSlider = new qtwidget::FloatSlider;
            p.kneeLowSlider->setRange(math::FloatRange(-3.F, 3.F));
            p.kneeLowSlider->setDefaultValue(0.F);

            p.kneeHighSlider = new qtwidget::FloatSlider;
            p.kneeHighSlider->setRange(math::FloatRange(3.5F, 7.5F));
            p.kneeHighSlider->setDefaultValue(5.F);

            auto layout = new QFormLayout;
            layout->addRow(p.enabledCheckBox);
            layout->addRow(tr("Exposure:"), p.exposureSlider);
            layout->addRow(tr("Defog:"), p.defogSlider);
            layout->addRow(tr("Knee low:"), p.kneeLowSlider);
            layout->addRow(tr("Knee high:"), p.kneeHighSlider);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    Q_EMIT exrDisplayEnabledChanged(value);
                });

            connect(
                p.exposureSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::EXRDisplay exrDisplay = _p->exrDisplay;
                    exrDisplay.exposure = value;
                    Q_EMIT exrDisplayChanged(exrDisplay);
                    Q_EMIT exrDisplayEnabledChanged(true);
                });

            connect(
                p.defogSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::EXRDisplay exrDisplay = _p->exrDisplay;
                    exrDisplay.defog = value;
                    Q_EMIT exrDisplayChanged(exrDisplay);
                    Q_EMIT exrDisplayEnabledChanged(true);
                });

            connect(
                p.kneeLowSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::EXRDisplay exrDisplay = _p->exrDisplay;
                    exrDisplay.kneeLow = value;
                    Q_EMIT exrDisplayChanged(exrDisplay);
                    Q_EMIT exrDisplayEnabledChanged(true);
                });
            connect(
                p.kneeHighSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::EXRDisplay exrDisplay = _p->exrDisplay;
                    exrDisplay.kneeHigh = value;
                    Q_EMIT exrDisplayChanged(exrDisplay);
                    Q_EMIT exrDisplayEnabledChanged(true);
                });

        }

        EXRDisplayWidget::~EXRDisplayWidget()
        {}

        void EXRDisplayWidget::setEXRDisplayEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.enabled)
                return;
            p.enabled = value;
            _widgetUpdate();
        }

        void EXRDisplayWidget::setEXRDisplay(const timeline::EXRDisplay& value)
        {
            TLRENDER_P();
            if (value == p.exrDisplay)
                return;
            p.exrDisplay = value;
            _widgetUpdate();
        }

        void EXRDisplayWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.enabledCheckBox);
                p.enabledCheckBox->setChecked(p.enabled);
            }
            {
                QSignalBlocker signalBlocker(p.exposureSlider);
                p.exposureSlider->setValue(p.exrDisplay.exposure);
            }
            {
                QSignalBlocker signalBlocker(p.defogSlider);
                p.defogSlider->setValue(p.exrDisplay.defog);
            }
            {
                QSignalBlocker signalBlocker(p.kneeLowSlider);
                p.kneeLowSlider->setValue(p.exrDisplay.kneeLow);
            }
            {
                QSignalBlocker signalBlocker(p.kneeHighSlider);
                p.kneeHighSlider->setValue(p.exrDisplay.kneeHigh);
            }
        }

        struct SoftClipWidget::Private
        {
            bool enabled = false;
            float softClip = 0.F;

            QCheckBox* enabledCheckBox = nullptr;
            qtwidget::FloatSlider* softClipSlider = nullptr;
        };

        SoftClipWidget::SoftClipWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.softClipSlider = new qtwidget::FloatSlider;
            p.softClipSlider->setDefaultValue(0.F);

            auto layout = new QFormLayout;
            layout->addRow(p.enabledCheckBox);
            layout->addRow(tr("Soft clip:"), p.softClipSlider);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    Q_EMIT softClipEnabledChanged(value);
                });

            connect(
                p.softClipSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    Q_EMIT softClipChanged(value);
                    Q_EMIT softClipEnabledChanged(true);
                });
        }

        SoftClipWidget::~SoftClipWidget()
        {}

        void SoftClipWidget::setSoftClipEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.enabled)
                return;
            p.enabled = value;
            _widgetUpdate();
        }

        void SoftClipWidget::setSoftClip(float value)
        {
            TLRENDER_P();
            if (value == p.softClip)
                return;
            p.softClip = value;
            _widgetUpdate();
        }

        void SoftClipWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.enabledCheckBox);
                p.enabledCheckBox->setChecked(p.enabled);
            }
            {
                QSignalBlocker signalBlocker(p.softClipSlider);
                p.softClipSlider->setValue(p.softClip);
            }
        }

        struct ColorTool::Private
        {
            timeline::LUTOptions lutOptions;
            timeline::DisplayOptions displayOptions;

            ConfigWidget* configWidget = nullptr;
            LUTWidget* lutWidget = nullptr;
            ColorControlsWidget* colorControlsWidget = nullptr;
            LevelsWidget* levelsWidget = nullptr;
            EXRDisplayWidget* exrDisplayWidget = nullptr;
            SoftClipWidget* softClipWidget = nullptr;
        };

        ColorTool::ColorTool(
            const std::shared_ptr<ColorModel>& colorModel,
            QWidget* parent) :
            ToolWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.configWidget = new ConfigWidget(colorModel);
            p.lutWidget = new LUTWidget;
            p.colorControlsWidget = new ColorControlsWidget;
            p.levelsWidget = new LevelsWidget;
            p.exrDisplayWidget = new EXRDisplayWidget;
            p.softClipWidget = new SoftClipWidget;

            addBellows(tr("Configuration"), p.configWidget);
            addBellows(tr("LUT"), p.lutWidget);
            addBellows(tr("Color Controls"), p.colorControlsWidget);
            addBellows(tr("Levels"), p.levelsWidget);
            addBellows(tr("EXR Display"), p.exrDisplayWidget);
            addBellows(tr("Soft Clip"), p.softClipWidget);
            addStretch();

            connect(
                p.lutWidget,
                &LUTWidget::lutOptionsChanged,
                [this](const timeline::LUTOptions& value)
                {
                    Q_EMIT lutOptionsChanged(value);
                });

            connect(
                p.colorControlsWidget,
                &ColorControlsWidget::colorEnabledChanged,
                [this](bool value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.colorEnabled = value;
                    Q_EMIT displayOptionsChanged(displayOptions);
                });
            connect(
                p.colorControlsWidget,
                &ColorControlsWidget::colorChanged,
                [this](const timeline::Color& value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.color = value;
                    Q_EMIT displayOptionsChanged(displayOptions);
                });

            connect(
                p.levelsWidget,
                &LevelsWidget::levelsEnabledChanged,
                [this](bool value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.levelsEnabled = value;
                    Q_EMIT displayOptionsChanged(displayOptions);
                });
            connect(
                p.levelsWidget,
                &LevelsWidget::levelsChanged,
                [this](const timeline::Levels& value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.levels = value;
                    Q_EMIT displayOptionsChanged(displayOptions);
                });

            connect(
                p.exrDisplayWidget,
                &EXRDisplayWidget::exrDisplayEnabledChanged,
                [this](bool value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.exrDisplayEnabled = value;
                    Q_EMIT displayOptionsChanged(displayOptions);
                });
            connect(
                p.exrDisplayWidget,
                &EXRDisplayWidget::exrDisplayChanged,
                [this](const timeline::EXRDisplay& value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.exrDisplay = value;
                    Q_EMIT displayOptionsChanged(displayOptions);
                });

            connect(
                p.softClipWidget,
                &SoftClipWidget::softClipEnabledChanged,
                [this](bool value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.softClipEnabled = value;
                    Q_EMIT displayOptionsChanged(displayOptions);
                });
            connect(
                p.softClipWidget,
                &SoftClipWidget::softClipChanged,
                [this](float value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.softClip = value;
                    Q_EMIT displayOptionsChanged(displayOptions);
                });
        }

        ColorTool::~ColorTool()
        {}

        void ColorTool::setLUTOptions(const timeline::LUTOptions & lutOptions)
        {
            TLRENDER_P();
            if (lutOptions == p.lutOptions)
                return;
            p.lutOptions = lutOptions;
            _widgetUpdate();
        }

        void ColorTool::setDisplayOptions(const timeline::DisplayOptions& displayOptions)
        {
            TLRENDER_P();
            if (displayOptions == p.displayOptions)
                return;
            p.displayOptions = displayOptions;
            _widgetUpdate();
        }

        void ColorTool::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.lutWidget);
                p.lutWidget->setLUTOptions(p.lutOptions);
            }
            {
                QSignalBlocker blocker(p.colorControlsWidget);
                p.colorControlsWidget->setColorEnabled(p.displayOptions.colorEnabled);
                p.colorControlsWidget->setColor(p.displayOptions.color);
            }
            {
                QSignalBlocker blocker(p.levelsWidget);
                p.levelsWidget->setLevelsEnabled(p.displayOptions.levelsEnabled);
                p.levelsWidget->setLevels(p.displayOptions.levels);
            }
            {
                QSignalBlocker blocker(p.exrDisplayWidget);
                p.exrDisplayWidget->setEXRDisplayEnabled(p.displayOptions.exrDisplayEnabled);
                p.exrDisplayWidget->setEXRDisplay(p.displayOptions.exrDisplay);
            }
            {
                QSignalBlocker blocker(p.softClipWidget);
                p.softClipWidget->setSoftClipEnabled(p.displayOptions.softClipEnabled);
                p.softClipWidget->setSoftClip(p.displayOptions.softClip);
            }
        }

        ColorDockWidget::ColorDockWidget(
            ColorTool* colorTool,
            QWidget* parent)
        {
            setObjectName("ColorTool");
            setWindowTitle(tr("Color"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("COLOR"));
            dockTitleBar->setIcon(QIcon(":/Icons/Color.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(colorTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Color.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F3));
            toggleViewAction()->setToolTip(tr("Show color controls"));
        }
    }
}
