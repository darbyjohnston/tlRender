// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/FilesToolWidget.h>

#include <tlPlayGLApp/App.h>

#include <tlUI/Bellows.h>
#include <tlUI/ButtonGroup.h>
#include <tlUI/ComboBox.h>
#include <tlUI/Divider.h>
#include <tlUI/FloatEditSlider.h>
#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_gl
    {
        struct FilesToolWidget::Private
        {
            std::shared_ptr<ui::ButtonGroup> aButtonGroup;
            std::shared_ptr<ui::ButtonGroup> bButtonGroup;
            std::map<std::shared_ptr<play::FilesModelItem>, std::shared_ptr<ui::ToolButton> > aButtons;
            std::map<std::shared_ptr<play::FilesModelItem>, std::shared_ptr<ui::ToolButton> > bButtons;
            std::vector<std::shared_ptr<ui::ComboBox> > layerComboBoxes;
            std::vector<std::shared_ptr<ui::IWidget> > widgets;
            std::shared_ptr<ui::FloatEditSlider> wipeXSlider;
            std::shared_ptr<ui::FloatEditSlider> wipeYSlider;
            std::shared_ptr<ui::FloatEditSlider> wipeRotationSlider;
            std::shared_ptr<ui::FloatEditSlider> overlaySlider;
            std::shared_ptr<ui::VerticalLayout> widgetLayout;
            std::shared_ptr<ui::VerticalLayout> layout;
            std::shared_ptr<ui::ScrollWidget> scrollWidget;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<observer::ValueObserver<std::shared_ptr<play::FilesModelItem> > > aObserver;
            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > bObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ValueObserver<float> > wipeXObserver;
            std::shared_ptr<observer::ValueObserver<float> > wipeYObserver;
            std::shared_ptr<observer::ValueObserver<float> > wipeRotationObserver;
            std::shared_ptr<observer::ValueObserver<float> > overlayObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> > compareObserver;
        };

        void FilesToolWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Files,
                "tl::play_gl::FilesToolWidget",
                app,
                context,
                parent);
            TLRENDER_P();

            p.aButtonGroup = ui::ButtonGroup::create(ui::ButtonGroupType::Radio, context);
            p.bButtonGroup = ui::ButtonGroup::create(ui::ButtonGroupType::Check, context);

            p.wipeXSlider = ui::FloatEditSlider::create(context);
            p.wipeYSlider = ui::FloatEditSlider::create(context);
            p.wipeRotationSlider = ui::FloatEditSlider::create(context);
            p.wipeRotationSlider->getModel()->setRange(math::FloatRange(0.F, 360.F));
            p.wipeRotationSlider->getModel()->setStep(1.F);
            p.wipeRotationSlider->getModel()->setLargeStep(10.F);

            p.overlaySlider = ui::FloatEditSlider::create(context);

            p.layout = ui::VerticalLayout::create(context);
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.widgetLayout = ui::VerticalLayout::create(context, p.layout);
            p.widgetLayout->setMarginRole(ui::SizeRole::MarginSmall);
            p.widgetLayout->setSpacingRole(ui::SizeRole::None);

            auto vLayout = ui::VerticalLayout::create(context, p.layout);
            vLayout->setSpacingRole(ui::SizeRole::None);
            ui::Divider::create(ui::Orientation::Horizontal, context, vLayout);
            auto bellows = ui::Bellows::create(context, vLayout);
            bellows->setText("Wipe");
            auto gridLayout = ui::GridLayout::create(context);
            gridLayout->setMarginRole(ui::SizeRole::MarginSmall);
            auto label = ui::Label::create("X:", context, gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.wipeXSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.wipeXSlider, 0, 1);
            label = ui::Label::create("Y:", context, gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.wipeYSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.wipeYSlider, 1, 1);
            label = ui::Label::create("Rotation:", context, gridLayout);
            gridLayout->setGridPos(label, 2, 0);
            p.wipeRotationSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.wipeRotationSlider, 2, 1);
            bellows->setWidget(gridLayout);

            ui::Divider::create(ui::Orientation::Horizontal, context, vLayout);
            
            bellows = ui::Bellows::create(context, vLayout);
            bellows->setText("Overlay");
            gridLayout = ui::GridLayout::create(context);
            gridLayout->setMarginRole(ui::SizeRole::MarginSmall);
            p.overlaySlider->setParent(gridLayout);
            gridLayout->setGridPos(p.overlaySlider, 0, 0);
            bellows->setWidget(gridLayout);
            ui::Divider::create(ui::Orientation::Horizontal, context, vLayout);

            p.scrollWidget = ui::ScrollWidget::create(context, ui::ScrollType::Both);
            p.scrollWidget->setWidget(p.layout);
            _setWidget(p.scrollWidget);

            auto appWeak = std::weak_ptr<App>(app);
            p.aButtonGroup->setCheckedCallback(
                [appWeak](int index, bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setA(index);
                    }
                });

            p.bButtonGroup->setCheckedCallback(
                [appWeak](int index, bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setB(index, value);
                    }
                });

            p.filesObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _filesUpdate(value);
                });

            p.aObserver = observer::ValueObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->getFilesModel()->observeA(),
                [this](const std::shared_ptr<play::FilesModelItem>& value)
                {
                    _aUpdate(value);
                });

            p.bObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->getFilesModel()->observeB(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _bUpdate(value);
                });

            p.layersObserver = observer::ListObserver<int>::create(
                app->getFilesModel()->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    _layersUpdate(value);
                });

            p.wipeXObserver = observer::ValueObserver<float>::create(
                p.wipeXSlider->getModel()->observeValue(),
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.wipeCenter.x = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                },
                observer::CallbackAction::Suppress);

            p.wipeYObserver = observer::ValueObserver<float>::create(
                p.wipeYSlider->getModel()->observeValue(),
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.wipeCenter.y = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                },
                observer::CallbackAction::Suppress);

            p.wipeRotationObserver = observer::ValueObserver<float>::create(
                p.wipeRotationSlider->getModel()->observeValue(),
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.wipeRotation = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                },
                observer::CallbackAction::Suppress);

            p.overlayObserver = observer::ValueObserver<float>::create(
                p.overlaySlider->getModel()->observeValue(),
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.overlay = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                },
                observer::CallbackAction::Suppress);

            p.compareObserver = observer::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _compareUpdate(value);
                });
        }

        FilesToolWidget::FilesToolWidget() :
            _p(new Private)
        {}

        FilesToolWidget::~FilesToolWidget()
        {}

        std::shared_ptr<FilesToolWidget> FilesToolWidget::create(
            const std::shared_ptr<App>&app,
            const std::shared_ptr<system::Context>&context,
            const std::shared_ptr<IWidget>&parent)
        {
            auto out = std::shared_ptr<FilesToolWidget>(new FilesToolWidget);
            out->_init(app, context, parent);
            return out;
        }

        void FilesToolWidget::_filesUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            TLRENDER_P();
            p.aButtonGroup->clearButtons();
            p.bButtonGroup->clearButtons();
            p.layerComboBoxes.clear();
            for (const auto& widget : p.widgets)
            {
                widget->setParent(nullptr);
            }
            p.widgets.clear();
            auto appWeak = _app;
            if (auto app = appWeak.lock())
            {
                const auto& a = app->getFilesModel()->getA();
                const auto& b = app->getFilesModel()->getB();
                if (auto context = _context.lock())
                {
                    for (const auto& item : value)
                    {
                        auto label = ui::Label::create(context);
                        label->setText(item->path.get(-1, false));
                        label->setTextWidth(32);
                        label->setHStretch(ui::Stretch::Expanding);

                        auto aButton = ui::ToolButton::create(context);
                        aButton->setText("A");
                        aButton->setChecked(item == a);
                        p.aButtons[item] = aButton;
                        p.aButtonGroup->addButton(aButton);

                        auto bButton = ui::ToolButton::create(context);
                        bButton->setText("B");
                        const auto i = std::find(b.begin(), b.end(), item);
                        bButton->setChecked(i != b.end());
                        p.bButtons[item] = bButton;
                        p.bButtonGroup->addButton(bButton);

                        auto layerComboBox = ui::ComboBox::create(context);
                        layerComboBox->setItems(item->videoLayers);
                        layerComboBox->setCurrentIndex(item->videoLayer);
                        p.layerComboBoxes.push_back(layerComboBox);

                        auto layout = ui::HorizontalLayout::create(context);
                        layout->setSpacingRole(ui::SizeRole::SpacingSmall);
                        label->setParent(layout);
                        auto hLayout = ui::HorizontalLayout::create(context, layout);
                        hLayout->setSpacingRole(ui::SizeRole::None);
                        aButton->setParent(hLayout);
                        bButton->setParent(hLayout);
                        layerComboBox->setParent(layout);
                        p.widgets.push_back(layout);
                        layout->setParent(p.widgetLayout);

                        layerComboBox->setIndexCallback(
                            [appWeak, item](int value)
                            {
                                if (auto app = appWeak.lock())
                                {
                                    app->getFilesModel()->setLayer(item, value);
                                }
                            });
                    }
                }
            }
        }

        void FilesToolWidget::_aUpdate(const std::shared_ptr<play::FilesModelItem>& value)
        {
            TLRENDER_P();
            for (const auto& button : p.aButtons)
            {
                button.second->setChecked(button.first == value);
            }
        }

        void FilesToolWidget::_bUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            TLRENDER_P();
            for (const auto& button : p.bButtons)
            {
                const auto i = std::find(value.begin(), value.end(), button.first);
                button.second->setChecked(i != value.end());
            }
        }

        void FilesToolWidget::_layersUpdate(const std::vector<int>& value)
        {
            TLRENDER_P();
            for (size_t i = 0; i < value.size() && i < p.layerComboBoxes.size(); ++i)
            {
                p.layerComboBoxes[i]->setCurrentIndex(value[i]);
            }
        }

        void FilesToolWidget::_compareUpdate(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            p.wipeXSlider->getModel()->setValue(value.wipeCenter.x);
            p.wipeYSlider->getModel()->setValue(value.wipeCenter.y);
            p.wipeRotationSlider->getModel()->setValue(value.wipeRotation);
            p.overlaySlider->getModel()->setValue(value.overlay);
        }
    }
}
