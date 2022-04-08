// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ColorTool.h>

#include <tlPlayApp/ColorModel.h>

#include <tlQtWidget/FloatSlider.h>

#include <tlCore/Path.h>

#include <QBoxLayout>
#include <QCheckBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListView>
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
            QLineEdit* fileNameLineEdit = nullptr;
            QListView* inputListView = nullptr;
            QListView* displayListView = nullptr;
            QListView* viewListView = nullptr;
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

            auto tabWidget = new QTabWidget;
            tabWidget->addTab(p.inputListView, tr("Input"));
            tabWidget->addTab(p.displayListView, tr("Display"));
            tabWidget->addTab(p.viewListView, tr("View"));

            auto layout = new QVBoxLayout;
            auto hLayout = new QHBoxLayout;
            hLayout->addWidget(p.fileNameLineEdit);
            hLayout->addWidget(fileNameButton);
            layout->addLayout(hLayout);
            layout->addWidget(tabWidget);
            setLayout(layout);

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

        ConfigWidget::~ConfigWidget()
        {}

        void ConfigWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.fileNameLineEdit);
                p.fileNameLineEdit->setText(QString::fromUtf8(p.data.fileName.c_str()));
            }
        }

        struct ColorWidget::Private
        {
            bool colorEnabled = false;
            timeline::Color color;

            QCheckBox* colorEnabledCheckBox = nullptr;
            qtwidget::FloatSlider* addSlider = nullptr;
            qtwidget::FloatSlider* brightnessSlider = nullptr;
            qtwidget::FloatSlider* contrastSlider = nullptr;
            qtwidget::FloatSlider* saturationSlider = nullptr;
            qtwidget::FloatSlider* tintSlider = nullptr;
            QCheckBox* invertCheckBox = nullptr;
        };

        ColorWidget::ColorWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.colorEnabledCheckBox = new QCheckBox(tr("Enabled"));

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
            layout->addRow(p.colorEnabledCheckBox);
            layout->addRow(tr("Add:"), p.addSlider);
            layout->addRow(tr("Brightness:"), p.brightnessSlider);
            layout->addRow(tr("Contrast:"), p.contrastSlider);
            layout->addRow(tr("Saturation:"), p.saturationSlider);
            layout->addRow(tr("Tint:"), p.tintSlider);
            layout->addRow(p.invertCheckBox);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.colorEnabledCheckBox,
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

        ColorWidget::~ColorWidget()
        {}

        void ColorWidget::setColorEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.colorEnabled)
                return;
            p.colorEnabled = value;
            _widgetUpdate();
        }

        void ColorWidget::setColor(const timeline::Color& value)
        {
            TLRENDER_P();
            if (value == p.color)
                return;
            p.color = value;
            _widgetUpdate();
        }

        void ColorWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.colorEnabledCheckBox);
                p.colorEnabledCheckBox->setChecked(p.colorEnabled);
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
            bool levelsEnabled = false;
            timeline::Levels levels;

            QCheckBox* levelsEnabledCheckBox = nullptr;
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

            p.levelsEnabledCheckBox = new QCheckBox(tr("Enabled"));

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
            layout->addRow(p.levelsEnabledCheckBox);
            layout->addRow(tr("In low:"), p.inLowSlider);
            layout->addRow(tr("In high:"), p.inHighSlider);
            layout->addRow(tr("Gamma:"), p.gammaSlider);
            layout->addRow(tr("Out low:"), p.outLowSlider);
            layout->addRow(tr("Out high:"), p.outHighSlider);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.levelsEnabledCheckBox,
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
            if (value == p.levelsEnabled)
                return;
            p.levelsEnabled = value;
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
                QSignalBlocker signalBlocker(p.levelsEnabledCheckBox);
                p.levelsEnabledCheckBox->setChecked(p.levelsEnabled);
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

        struct ExposureWidget::Private
        {
            bool exposureEnabled = false;
            timeline::Exposure exposure;

            QCheckBox* exposureEnabledCheckBox = nullptr;
            qtwidget::FloatSlider* exposureSlider = nullptr;
            qtwidget::FloatSlider* defogSlider = nullptr;
            qtwidget::FloatSlider* kneeLowSlider = nullptr;
            qtwidget::FloatSlider* kneeHighSlider = nullptr;
        };

        ExposureWidget::ExposureWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.exposureEnabledCheckBox = new QCheckBox(tr("Enabled"));

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
            layout->addRow(p.exposureEnabledCheckBox);
            layout->addRow(tr("Exposure:"), p.exposureSlider);
            layout->addRow(tr("Defog:"), p.defogSlider);
            layout->addRow(tr("Knee low:"), p.kneeLowSlider);
            layout->addRow(tr("Knee high:"), p.kneeHighSlider);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.exposureEnabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    Q_EMIT exposureEnabledChanged(value);
                });

            connect(
                p.exposureSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Exposure exposure = _p->exposure;
                    exposure.exposure = value;
                    Q_EMIT exposureChanged(exposure);
                    Q_EMIT exposureEnabledChanged(true);
                });

            connect(
                p.defogSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Exposure exposure = _p->exposure;
                    exposure.defog = value;
                    Q_EMIT exposureChanged(exposure);
                    Q_EMIT exposureEnabledChanged(true);
                });

            connect(
                p.kneeLowSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Exposure exposure = _p->exposure;
                    exposure.kneeLow = value;
                    Q_EMIT exposureChanged(exposure);
                    Q_EMIT exposureEnabledChanged(true);
                });
            connect(
                p.kneeHighSlider,
                &qtwidget::FloatSlider::valueChanged,
                [this](float value)
                {
                    timeline::Exposure exposure = _p->exposure;
                    exposure.kneeHigh = value;
                    Q_EMIT exposureChanged(exposure);
                    Q_EMIT exposureEnabledChanged(true);
                });

        }

        ExposureWidget::~ExposureWidget()
        {}

        void ExposureWidget::setExposureEnabled(bool value)
        {
            TLRENDER_P();
            if (value == p.exposureEnabled)
                return;
            p.exposureEnabled = value;
            _widgetUpdate();
        }

        void ExposureWidget::setExposure(const timeline::Exposure& value)
        {
            TLRENDER_P();
            if (value == p.exposure)
                return;
            p.exposure = value;
            _widgetUpdate();
        }

        void ExposureWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.exposureEnabledCheckBox);
                p.exposureEnabledCheckBox->setChecked(p.exposureEnabled);
            }
            {
                QSignalBlocker signalBlocker(p.exposureSlider);
                p.exposureSlider->setValue(p.exposure.exposure);
            }
            {
                QSignalBlocker signalBlocker(p.defogSlider);
                p.defogSlider->setValue(p.exposure.defog);
            }
            {
                QSignalBlocker signalBlocker(p.kneeLowSlider);
                p.kneeLowSlider->setValue(p.exposure.kneeLow);
            }
            {
                QSignalBlocker signalBlocker(p.kneeHighSlider);
                p.kneeHighSlider->setValue(p.exposure.kneeHigh);
            }
        }

        struct SoftClipWidget::Private
        {
            bool softClipEnabled = false;
            float softClip = 0.F;

            QCheckBox* softClipEnabledCheckBox = nullptr;
            qtwidget::FloatSlider* softClipSlider = nullptr;
        };

        SoftClipWidget::SoftClipWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.softClipEnabledCheckBox = new QCheckBox(tr("Enabled"));

            p.softClipSlider = new qtwidget::FloatSlider;
            p.softClipSlider->setDefaultValue(0.F);

            auto layout = new QFormLayout;
            layout->addRow(p.softClipEnabledCheckBox);
            layout->addRow(tr("Soft clip:"), p.softClipSlider);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.softClipEnabledCheckBox,
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
            if (value == p.softClipEnabled)
                return;
            p.softClipEnabled = value;
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
                QSignalBlocker signalBlocker(p.softClipEnabledCheckBox);
                p.softClipEnabledCheckBox->setChecked(p.softClipEnabled);
            }
            {
                QSignalBlocker signalBlocker(p.softClipSlider);
                p.softClipSlider->setValue(p.softClip);
            }
        }

        struct ColorTool::Private
        {
            timeline::DisplayOptions displayOptions;

            ConfigWidget* configWidget = nullptr;
            ColorWidget* colorWidget = nullptr;
            LevelsWidget* levelsWidget = nullptr;
            ExposureWidget* exposureWidget = nullptr;
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
            p.colorWidget = new ColorWidget;
            p.levelsWidget = new LevelsWidget;
            p.exposureWidget = new ExposureWidget;
            p.softClipWidget = new SoftClipWidget;

            addBellows(tr("Configuration"), p.configWidget);
            addBellows(tr("Color"), p.colorWidget);
            addBellows(tr("Levels"), p.levelsWidget);
            addBellows(tr("Exposure"), p.exposureWidget);
            addBellows(tr("Soft Clip"), p.softClipWidget);
            addStretch();

            connect(
                p.colorWidget,
                &ColorWidget::colorEnabledChanged,
                [this](bool value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.colorEnabled = value;
                    Q_EMIT displayOptionsChanged(displayOptions);
                });
            connect(
                p.colorWidget,
                &ColorWidget::colorChanged,
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
                p.exposureWidget,
                &ExposureWidget::exposureEnabledChanged,
                [this](bool value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.exposureEnabled = value;
                    Q_EMIT displayOptionsChanged(displayOptions);
                });
            connect(
                p.exposureWidget,
                &ExposureWidget::exposureChanged,
                [this](const timeline::Exposure& value)
                {
                    timeline::DisplayOptions displayOptions = _p->displayOptions;
                    displayOptions.exposure = value;
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
                QSignalBlocker blocker(p.colorWidget);
                p.colorWidget->setColorEnabled(p.displayOptions.colorEnabled);
                p.colorWidget->setColor(p.displayOptions.color);
            }
            {
                QSignalBlocker blocker(p.levelsWidget);
                p.levelsWidget->setLevelsEnabled(p.displayOptions.levelsEnabled);
                p.levelsWidget->setLevels(p.displayOptions.levels);
            }
            {
                QSignalBlocker blocker(p.exposureWidget);
                p.exposureWidget->setExposureEnabled(p.displayOptions.exposureEnabled);
                p.exposureWidget->setExposure(p.displayOptions.exposure);
            }
            {
                QSignalBlocker blocker(p.softClipWidget);
                p.softClipWidget->setSoftClipEnabled(p.displayOptions.softClipEnabled);
                p.softClipWidget->setSoftClip(p.displayOptions.softClip);
            }
        }
    }
}
