// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/ColorTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlQtWidget/FileWidget.h>
#include <tlQtWidget/FloatEditSlider.h>
#include <tlQtWidget/SearchWidget.h>

#include <tlPlay/ColorModel.h>
#include <tlPlay/ViewportModel.h>

#include <tlCore/Path.h>

#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QToolButton>

namespace tl
{
    namespace play_qt
    {
        struct OCIOWidget::Private
        {
            std::shared_ptr<play::OCIOModel> ocioModel;

            QCheckBox* enabledCheckBox = nullptr;
            qtwidget::FileWidget* fileWidget = nullptr;
            QComboBox* inputComboBox = nullptr;
            QComboBox* displayComboBox = nullptr;
            QComboBox* viewComboBox = nullptr;
            QComboBox* lookComboBox = nullptr;

            std::shared_ptr<observer::ValueObserver<timeline::OCIOOptions> > optionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::OCIOOptions> > optionsObserver2;
            std::shared_ptr<observer::ValueObserver<play::OCIOModelData> > dataObserver;
        };

        OCIOWidget::OCIOWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();
            
#if !defined(TLRENDER_OCIO)
            setEnabled(false);
#endif // TLRENDER_OCIO

            p.ocioModel = play::OCIOModel::create(app->getContext());

            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.fileWidget = new qtwidget::FileWidget(app->getContext());

            p.inputComboBox = new QComboBox;
            p.displayComboBox = new QComboBox;
            p.viewComboBox = new QComboBox;
            p.lookComboBox = new QComboBox;

            auto formLayout = new QFormLayout;
            formLayout->addRow(p.enabledCheckBox);
            formLayout->addRow(tr("File name:"), p.fileWidget);
            formLayout->addRow(tr("Input:"), p.inputComboBox);
            formLayout->addRow(tr("Display:"), p.displayComboBox);
            formLayout->addRow(tr("View:"), p.viewComboBox);
            formLayout->addRow(tr("Look:"), p.lookComboBox);
            setLayout(formLayout);

            connect(
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    _p->ocioModel->setEnabled(value);
                });

            connect(
                p.fileWidget,
                &qtwidget::FileWidget::fileChanged,
                [this](const QString& value)
                {
                    _p->ocioModel->setConfig(value.toUtf8().data());
                });

            connect(
                p.inputComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this](int index)
                {
                    _p->ocioModel->setInputIndex(index);
                });

            connect(
                p.displayComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this](int index)
                {
                    _p->ocioModel->setDisplayIndex(index);
                });

            connect(
                p.viewComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this](int index)
                {
                    _p->ocioModel->setViewIndex(index);
                });

            connect(
                p.lookComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                [this](int index)
                {
                    _p->ocioModel->setLookIndex(index);
                });

            p.optionsObserver = observer::ValueObserver<timeline::OCIOOptions>::create(
                p.ocioModel->observeOptions(),
                [app](const timeline::OCIOOptions& value)
                {
                    app->colorModel()->setOCIOOptions(value);
                });

            p.optionsObserver2 = observer::ValueObserver<timeline::OCIOOptions>::create(
                app->colorModel()->observeOCIOOptions(),
                [this](const timeline::OCIOOptions& value)
                {
                    _p->ocioModel->setOptions(value);
                });

            p.dataObserver = observer::ValueObserver<play::OCIOModelData>::create(
                p.ocioModel->observeData(),
                [this](const play::OCIOModelData& value)
                {
                    _widgetUpdate(value);
                });
        }

        OCIOWidget::~OCIOWidget()
        {}

        void OCIOWidget::_widgetUpdate(const play::OCIOModelData& value)
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.enabledCheckBox);
                p.enabledCheckBox->setChecked(value.enabled);
            }
            {
                QSignalBlocker blocker(p.fileWidget);
                p.fileWidget->setFile(QString::fromUtf8(value.fileName.c_str()));
            }
            {
                QSignalBlocker blocker(p.inputComboBox);
                p.inputComboBox->clear();
                for (const auto& input : value.inputs)
                {
                    p.inputComboBox->addItem(QString::fromUtf8(input.c_str()));
                }
                p.inputComboBox->setCurrentIndex(value.inputIndex);
            }
            {
                QSignalBlocker blocker(p.displayComboBox);
                p.displayComboBox->clear();
                for (const auto& display : value.displays)
                {
                    p.displayComboBox->addItem(QString::fromUtf8(display.c_str()));
                }
                p.displayComboBox->setCurrentIndex(value.displayIndex);
            }
            {
                QSignalBlocker blocker(p.viewComboBox);
                p.viewComboBox->clear();
                for (const auto& view : value.views)
                {
                    p.viewComboBox->addItem(QString::fromUtf8(view.c_str()));
                }
                p.viewComboBox->setCurrentIndex(value.viewIndex);
            }
            {
                QSignalBlocker blocker(p.lookComboBox);
                p.lookComboBox->clear();
                for (const auto& look : value.looks)
                {
                    p.lookComboBox->addItem(QString::fromUtf8(look.c_str()));
                }
                p.lookComboBox->setCurrentIndex(value.lookIndex);
            }
        }

        struct LUTWidget::Private
        {
            QCheckBox* enabledCheckBox = nullptr;
            qtwidget::FileWidget* fileWidget = nullptr;
            QComboBox* orderComboBox = nullptr;

            std::shared_ptr<observer::ValueObserver<timeline::LUTOptions> > lutObserver;
        };

        LUTWidget::LUTWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();
            
#if !defined(TLRENDER_OCIO)
            setEnabled(false);
#endif // TLRENDER_OCIO

            QStringList extensions;
            for (const auto& i : timeline::getLUTFormatExtensions())
            {
                extensions.push_back(QString::fromUtf8(i.c_str()));
            }

            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.fileWidget = new qtwidget::FileWidget(app->getContext());

            p.orderComboBox = new QComboBox;

            auto layout = new QFormLayout;
            layout->addRow(p.enabledCheckBox);
            layout->addRow(tr("File name:"), p.fileWidget);
            layout->addRow(tr("Order:"), p.orderComboBox);
            setLayout(layout);

            connect(
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [app](bool value)
                {
                    auto options = app->colorModel()->getLUTOptions();
                    options.enabled = value;
                    app->colorModel()->setLUTOptions(options);
                });

            connect(
                p.fileWidget,
                &qtwidget::FileWidget::fileChanged,
                [app](const QString& value)
                {
                    auto options = app->colorModel()->getLUTOptions();
                    options.enabled = true;
                    options.fileName = value.toUtf8().data();
                    app->colorModel()->setLUTOptions(options);
                });

            connect(
                p.orderComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [app](int value)
                {
                    auto options = app->colorModel()->getLUTOptions();
                    options.enabled = true;
                    options.order = static_cast<timeline::LUTOrder>(value);
                    app->colorModel()->setLUTOptions(options);
                });

            p.lutObserver = observer::ValueObserver<timeline::LUTOptions>::create(
                app->colorModel()->observeLUTOptions(),
                [this](const timeline::LUTOptions& value)
                {
                    _widgetUpdate(value);
                });
        }

        LUTWidget::~LUTWidget()
        {}

        void LUTWidget::_widgetUpdate(const tl::timeline::LUTOptions& value)
        {
            TLRENDER_P();
            {
                QSignalBlocker blocker(p.enabledCheckBox);
                p.enabledCheckBox->setChecked(value.enabled);
            }
            {
                QSignalBlocker blocker(p.fileWidget);
                p.fileWidget->setFile(QString::fromUtf8(value.fileName.c_str()));
            }
            {
                QSignalBlocker blocker(_p->orderComboBox);
                _p->orderComboBox->clear();
                for (const auto& label : timeline::getLUTOrderLabels())
                {
                    _p->orderComboBox->addItem(QString::fromUtf8(label.c_str()));
                }
                _p->orderComboBox->setCurrentIndex(static_cast<int>(value.order));
            }
        }

        struct ColorWidget::Private
        {
            QCheckBox* enabledCheckBox = nullptr;
            qtwidget::FloatEditSlider* addSlider = nullptr;
            qtwidget::FloatEditSlider* brightnessSlider = nullptr;
            qtwidget::FloatEditSlider* contrastSlider = nullptr;
            qtwidget::FloatEditSlider* saturationSlider = nullptr;
            qtwidget::FloatEditSlider* tintSlider = nullptr;
            QCheckBox* invertCheckBox = nullptr;

            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> > displayObserver;
        };

        ColorWidget::ColorWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.addSlider = new qtwidget::FloatEditSlider;
            p.addSlider->setRange(math::FloatRange(-1.F, 1.F));
            p.addSlider->setDefaultValue(0.F);

            p.brightnessSlider = new qtwidget::FloatEditSlider;
            p.brightnessSlider->setRange(math::FloatRange(0.F, 4.F));
            p.brightnessSlider->setDefaultValue(1.F);

            p.contrastSlider = new qtwidget::FloatEditSlider;
            p.contrastSlider->setRange(math::FloatRange(0.F, 4.F));
            p.contrastSlider->setDefaultValue(1.F);

            p.saturationSlider = new qtwidget::FloatEditSlider;
            p.saturationSlider->setRange(math::FloatRange(0.F, 4.F));
            p.saturationSlider->setDefaultValue(1.F);

            p.tintSlider = new qtwidget::FloatEditSlider;
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

            connect(
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [app](bool value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.color.enabled = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.addSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.add.x = value;
                    options.color.add.y = value;
                    options.color.add.z = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.brightnessSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.brightness.x = value;
                    options.color.brightness.y = value;
                    options.color.brightness.z = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.contrastSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.contrast.x = value;
                    options.color.contrast.y = value;
                    options.color.contrast.z = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.saturationSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.saturation.x = value;
                    options.color.saturation.y = value;
                    options.color.saturation.z = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.tintSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.tint = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.invertCheckBox,
                &QCheckBox::toggled,
                [app](bool value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.invert = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            p.displayObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->viewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _widgetUpdate(value);
                });
        }

        ColorWidget::~ColorWidget()
        {}

        void ColorWidget::_widgetUpdate(const timeline::DisplayOptions& value)
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.enabledCheckBox);
                p.enabledCheckBox->setChecked(value.color.enabled);
            }
            {
                QSignalBlocker signalBlocker(p.addSlider);
                p.addSlider->setValue(value.color.add.x);
            }
            {
                QSignalBlocker signalBlocker(p.brightnessSlider);
                p.brightnessSlider->setValue(value.color.brightness.x);
            }
            {
                QSignalBlocker signalBlocker(p.contrastSlider);
                p.contrastSlider->setValue(value.color.contrast.x);
            }
            {
                QSignalBlocker signalBlocker(p.saturationSlider);
                p.saturationSlider->setValue(value.color.saturation.x);
            }
            {
                QSignalBlocker signalBlocker(p.tintSlider);
                p.tintSlider->setValue(value.color.tint);
            }
            {
                QSignalBlocker signalBlocker(p.invertCheckBox);
                p.invertCheckBox->setChecked(value.color.invert);
            }
        }

        struct LevelsWidget::Private
        {
            QCheckBox* enabledCheckBox = nullptr;
            qtwidget::FloatEditSlider* inLowSlider = nullptr;
            qtwidget::FloatEditSlider* inHighSlider = nullptr;
            qtwidget::FloatEditSlider* gammaSlider = nullptr;
            qtwidget::FloatEditSlider* outLowSlider = nullptr;
            qtwidget::FloatEditSlider* outHighSlider = nullptr;

            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> > displayObserver;
        };

        LevelsWidget::LevelsWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.inLowSlider = new qtwidget::FloatEditSlider;
            p.inLowSlider->setDefaultValue(0.F);

            p.inHighSlider = new qtwidget::FloatEditSlider;
            p.inHighSlider->setDefaultValue(1.F);

            p.gammaSlider = new qtwidget::FloatEditSlider;
            p.gammaSlider->setRange(math::FloatRange(.1F, 4.F));
            p.gammaSlider->setDefaultValue(1.F);

            p.outLowSlider = new qtwidget::FloatEditSlider;
            p.outLowSlider->setDefaultValue(0.F);

            p.outHighSlider = new qtwidget::FloatEditSlider;
            p.outHighSlider->setDefaultValue(1.F);

            auto layout = new QFormLayout;
            layout->addRow(p.enabledCheckBox);
            layout->addRow(tr("In low:"), p.inLowSlider);
            layout->addRow(tr("In high:"), p.inHighSlider);
            layout->addRow(tr("Gamma:"), p.gammaSlider);
            layout->addRow(tr("Out low:"), p.outLowSlider);
            layout->addRow(tr("Out high:"), p.outHighSlider);
            setLayout(layout);

            connect(
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [app](bool value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.levels.enabled = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.inLowSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.levels.enabled = true;
                    options.levels.inLow = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.inHighSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.levels.enabled = true;
                    options.levels.inHigh = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.gammaSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.levels.enabled = true;
                    options.levels.gamma = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.outLowSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.levels.enabled = true;
                    options.levels.outLow = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.outHighSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.levels.enabled = true;
                    options.levels.outHigh = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            p.displayObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->viewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _widgetUpdate(value);
                });
        }

        LevelsWidget::~LevelsWidget()
        {}

        void LevelsWidget::_widgetUpdate(const timeline::DisplayOptions& value)
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.enabledCheckBox);
                p.enabledCheckBox->setChecked(value.levels.enabled);
            }
            {
                QSignalBlocker signalBlocker(p.inLowSlider);
                p.inLowSlider->setValue(value.levels.inLow);
            }
            {
                QSignalBlocker signalBlocker(p.inHighSlider);
                p.inHighSlider->setValue(value.levels.inHigh);
            }
            {
                QSignalBlocker signalBlocker(p.gammaSlider);
                p.gammaSlider->setValue(value.levels.gamma);
            }
            {
                QSignalBlocker signalBlocker(p.outLowSlider);
                p.outLowSlider->setValue(value.levels.outLow);
            }
            {
                QSignalBlocker signalBlocker(p.outHighSlider);
                p.outHighSlider->setValue(value.levels.outHigh);
            }
        }

        struct EXRDisplayWidget::Private
        {
            QCheckBox* enabledCheckBox = nullptr;
            qtwidget::FloatEditSlider* exposureSlider = nullptr;
            qtwidget::FloatEditSlider* defogSlider = nullptr;
            qtwidget::FloatEditSlider* kneeLowSlider = nullptr;
            qtwidget::FloatEditSlider* kneeHighSlider = nullptr;

            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> > displayObserver;
        };

        EXRDisplayWidget::EXRDisplayWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.exposureSlider = new qtwidget::FloatEditSlider;
            p.exposureSlider->setRange(math::FloatRange(-10.F, 10.F));
            p.exposureSlider->setDefaultValue(0.F);

            p.defogSlider = new qtwidget::FloatEditSlider;
            p.defogSlider->setRange(math::FloatRange(0.F, .1F));
            p.defogSlider->setDefaultValue(0.F);

            p.kneeLowSlider = new qtwidget::FloatEditSlider;
            p.kneeLowSlider->setRange(math::FloatRange(-3.F, 3.F));
            p.kneeLowSlider->setDefaultValue(0.F);

            p.kneeHighSlider = new qtwidget::FloatEditSlider;
            p.kneeHighSlider->setRange(math::FloatRange(3.5F, 7.5F));
            p.kneeHighSlider->setDefaultValue(5.F);

            auto layout = new QFormLayout;
            layout->addRow(p.enabledCheckBox);
            layout->addRow(tr("Exposure:"), p.exposureSlider);
            layout->addRow(tr("Defog:"), p.defogSlider);
            layout->addRow(tr("Knee low:"), p.kneeLowSlider);
            layout->addRow(tr("Knee high:"), p.kneeHighSlider);
            setLayout(layout);

            connect(
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [app](bool value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.exrDisplay.enabled = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.exposureSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.exrDisplay.enabled = true;
                    options.exrDisplay.exposure = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.defogSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.exrDisplay.enabled = true;
                    options.exrDisplay.defog = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.kneeLowSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.exrDisplay.enabled = true;
                    options.exrDisplay.kneeLow = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.kneeHighSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.exrDisplay.enabled = true;
                    options.exrDisplay.kneeHigh = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            p.displayObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->viewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _widgetUpdate(value);
                });
        }

        EXRDisplayWidget::~EXRDisplayWidget()
        {}

        void EXRDisplayWidget::_widgetUpdate(const timeline::DisplayOptions& value)
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.enabledCheckBox);
                p.enabledCheckBox->setChecked(value.exrDisplay.enabled);
            }
            {
                QSignalBlocker signalBlocker(p.exposureSlider);
                p.exposureSlider->setValue(value.exrDisplay.exposure);
            }
            {
                QSignalBlocker signalBlocker(p.defogSlider);
                p.defogSlider->setValue(value.exrDisplay.defog);
            }
            {
                QSignalBlocker signalBlocker(p.kneeLowSlider);
                p.kneeLowSlider->setValue(value.exrDisplay.kneeLow);
            }
            {
                QSignalBlocker signalBlocker(p.kneeHighSlider);
                p.kneeHighSlider->setValue(value.exrDisplay.kneeHigh);
            }
        }

        struct SoftClipWidget::Private
        {
            QCheckBox* enabledCheckBox = nullptr;
            qtwidget::FloatEditSlider* softClipSlider = nullptr;

            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> > displayObserver;
        };

        SoftClipWidget::SoftClipWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.softClipSlider = new qtwidget::FloatEditSlider;
            p.softClipSlider->setDefaultValue(0.F);

            auto layout = new QFormLayout;
            layout->addRow(p.enabledCheckBox);
            layout->addRow(tr("Soft clip:"), p.softClipSlider);
            setLayout(layout);

            connect(
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [app](bool value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.softClip.enabled = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            connect(
                p.softClipSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->viewportModel()->getDisplayOptions();
                    options.softClip.enabled = true;
                    options.softClip.value = value;
                    app->viewportModel()->setDisplayOptions(options);
                });

            p.displayObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->viewportModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _widgetUpdate(value);
                });
        }

        SoftClipWidget::~SoftClipWidget()
        {}

        void SoftClipWidget::_widgetUpdate(const timeline::DisplayOptions& value)
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.enabledCheckBox);
                p.enabledCheckBox->setChecked(value.softClip.enabled);
            }
            {
                QSignalBlocker signalBlocker(p.softClipSlider);
                p.softClipSlider->setValue(value.softClip.value);
            }
        }

        struct ColorTool::Private
        {
            OCIOWidget* ocioWidget = nullptr;
            LUTWidget* lutWidget = nullptr;
            ColorWidget* colorWidget = nullptr;
            LevelsWidget* levelsWidget = nullptr;
            EXRDisplayWidget* exrDisplayWidget = nullptr;
            SoftClipWidget* softClipWidget = nullptr;
        };

        ColorTool::ColorTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.ocioWidget = new OCIOWidget(app);
            p.lutWidget = new LUTWidget(app);
            p.colorWidget = new ColorWidget(app);
            p.levelsWidget = new LevelsWidget(app);
            p.exrDisplayWidget = new EXRDisplayWidget(app);
            p.softClipWidget = new SoftClipWidget(app);

            addBellows(tr("OCIO"), p.ocioWidget);
            addBellows(tr("LUT"), p.lutWidget);
            addBellows(tr("Color"), p.colorWidget);
            addBellows(tr("Levels"), p.levelsWidget);
            addBellows(tr("EXR Display"), p.exrDisplayWidget);
            addBellows(tr("Soft Clip"), p.softClipWidget);
            addStretch();
        }

        ColorTool::~ColorTool()
        {}

        ColorDockWidget::ColorDockWidget(
            ColorTool* colorTool,
            QWidget* parent)
        {
            setObjectName("ColorTool");
            setWindowTitle(tr("Color"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Color"));
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
