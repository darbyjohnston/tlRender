// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/ColorTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/ColorConfigModel.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlQtWidget/FileWidget.h>
#include <tlQtWidget/FloatEditSlider.h>
#include <tlQtWidget/SearchWidget.h>

#include <tlPlay/ColorModel.h>

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
    namespace play_qt
    {
        struct ConfigWidget::Private
        {
            std::shared_ptr<play::ColorConfigModel> colorConfigModel;

            QCheckBox* enabledCheckBox = nullptr;
            qtwidget::FileWidget* fileWidget = nullptr;

            std::shared_ptr<observer::ValueObserver<timeline::ColorConfigOptions> > configOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::ColorConfigOptions> > configOptionsObserver2;
            std::shared_ptr<observer::ValueObserver<play::ColorConfigModelData> > dataObserver;
        };

        ConfigWidget::ConfigWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.colorConfigModel = play::ColorConfigModel::create(app->getContext());

            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            auto inputListModel = new ColorInputListModel(p.colorConfigModel, this);
            auto inputListProxyModel = new QSortFilterProxyModel(this);
            inputListProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            inputListProxyModel->setFilterKeyColumn(-1);
            inputListProxyModel->setSourceModel(inputListModel);

            auto displayListModel = new ColorDisplayListModel(p.colorConfigModel, this);
            auto displayListProxyModel = new QSortFilterProxyModel(this);
            displayListProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            displayListProxyModel->setFilterKeyColumn(-1);
            displayListProxyModel->setSourceModel(displayListModel);

            auto viewListModel = new ColorViewListModel(p.colorConfigModel, this);
            auto viewListProxyModel = new QSortFilterProxyModel(this);
            viewListProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            viewListProxyModel->setFilterKeyColumn(-1);
            viewListProxyModel->setSourceModel(viewListModel);

            p.fileWidget = new qtwidget::FileWidget(app->getContext());

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
            formLayout->addRow(p.enabledCheckBox);
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
                p.enabledCheckBox,
                &QCheckBox::toggled,
                [this](bool value)
                {
                    _p->colorConfigModel->setEnabled(value);
                });

            connect(
                p.fileWidget,
                &qtwidget::FileWidget::fileChanged,
                [this](const QString& value)
                {
                    _p->colorConfigModel->setConfig(value.toUtf8().data());
                });

            connect(
                inputListView,
                &QAbstractItemView::activated,
                [this, inputListProxyModel](const QModelIndex& index)
                {
                    auto sourceIndex = inputListProxyModel->mapToSource(index);
                    _p->colorConfigModel->setInputIndex(sourceIndex.row());
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
                    _p->colorConfigModel->setDisplayIndex(sourceIndex.row());
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
                    _p->colorConfigModel->setViewIndex(sourceIndex.row());
                });

            connect(
                viewSearchWidget,
                SIGNAL(searchChanged(const QString&)),
                viewListProxyModel,
                SLOT(setFilterFixedString(const QString&)));

            p.configOptionsObserver = observer::ValueObserver<timeline::ColorConfigOptions>::create(
                p.colorConfigModel->observeConfigOptions(),
                [app](const timeline::ColorConfigOptions& value)
                {
                    app->colorModel()->setColorConfigOptions(value);
                });

            p.configOptionsObserver2 = observer::ValueObserver<timeline::ColorConfigOptions>::create(
                app->colorModel()->observeColorConfigOptions(),
                [this](const timeline::ColorConfigOptions& value)
                {
                    _p->colorConfigModel->setConfigOptions(value);
                });

            p.dataObserver = observer::ValueObserver<play::ColorConfigModelData>::create(
                p.colorConfigModel->observeData(),
                [this](const play::ColorConfigModelData& value)
                {
                    _widgetUpdate(value);
                });
        }

        ConfigWidget::~ConfigWidget()
        {}

        void ConfigWidget::_widgetUpdate(const play::ColorConfigModelData& value)
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

        struct ColorControlsWidget::Private
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

        ColorControlsWidget::ColorControlsWidget(App* app, QWidget* parent) :
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
                    auto options = app->colorModel()->getDisplayOptions();
                    options.color.enabled = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.addSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.add.x = value;
                    options.color.add.y = value;
                    options.color.add.z = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.brightnessSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.brightness.x = value;
                    options.color.brightness.y = value;
                    options.color.brightness.z = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.contrastSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.contrast.x = value;
                    options.color.contrast.y = value;
                    options.color.contrast.z = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.saturationSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.saturation.x = value;
                    options.color.saturation.y = value;
                    options.color.saturation.z = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.tintSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.tint = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.invertCheckBox,
                &QCheckBox::toggled,
                [app](bool value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.color.enabled = true;
                    options.color.invert = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            p.displayObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->colorModel()->observeDisplayOptions(),
                [this](const timeline::DisplayOptions& value)
                {
                    _widgetUpdate(value);
                });
        }

        ColorControlsWidget::~ColorControlsWidget()
        {}

        void ColorControlsWidget::_widgetUpdate(const timeline::DisplayOptions& value)
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
                    auto options = app->colorModel()->getDisplayOptions();
                    options.levels.enabled = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.inLowSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.levels.enabled = true;
                    options.levels.inLow = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.inHighSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.levels.enabled = true;
                    options.levels.inHigh = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.gammaSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.levels.enabled = true;
                    options.levels.gamma = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.outLowSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.levels.enabled = true;
                    options.levels.outLow = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.outHighSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.levels.enabled = true;
                    options.levels.outHigh = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            p.displayObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->colorModel()->observeDisplayOptions(),
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
                    auto options = app->colorModel()->getDisplayOptions();
                    options.exrDisplay.enabled = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.exposureSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.exrDisplay.enabled = true;
                    options.exrDisplay.exposure = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.defogSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.exrDisplay.enabled = true;
                    options.exrDisplay.defog = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.kneeLowSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.exrDisplay.enabled = true;
                    options.exrDisplay.kneeLow = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.kneeHighSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.exrDisplay.enabled = true;
                    options.exrDisplay.kneeHigh = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            p.displayObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->colorModel()->observeDisplayOptions(),
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
                    auto options = app->colorModel()->getDisplayOptions();
                    options.softClip.enabled = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            connect(
                p.softClipSlider,
                &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                {
                    auto options = app->colorModel()->getDisplayOptions();
                    options.softClip.enabled = true;
                    options.softClip.value = value;
                    app->colorModel()->setDisplayOptions(options);
                });

            p.displayObserver = observer::ValueObserver<timeline::DisplayOptions>::create(
                app->colorModel()->observeDisplayOptions(),
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
            ConfigWidget* configWidget = nullptr;
            LUTWidget* lutWidget = nullptr;
            ColorControlsWidget* colorControlsWidget = nullptr;
            LevelsWidget* levelsWidget = nullptr;
            EXRDisplayWidget* exrDisplayWidget = nullptr;
            SoftClipWidget* softClipWidget = nullptr;
        };

        ColorTool::ColorTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.configWidget = new ConfigWidget(app);
            p.lutWidget = new LUTWidget(app);
            p.colorControlsWidget = new ColorControlsWidget(app);
            p.levelsWidget = new LevelsWidget(app);
            p.exrDisplayWidget = new EXRDisplayWidget(app);
            p.softClipWidget = new SoftClipWidget(app);

            addBellows(tr("Configuration"), p.configWidget);
            addBellows(tr("LUT"), p.lutWidget);
            addBellows(tr("Color Controls"), p.colorControlsWidget);
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
