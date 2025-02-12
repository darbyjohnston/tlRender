// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FilesToolPrivate.h>

#include <tlPlayApp/App.h>

#include <dtk/ui/Bellows.h>
#include <dtk/ui/ButtonGroup.h>
#include <dtk/ui/ComboBox.h>
#include <dtk/ui/FloatEditSlider.h>
#include <dtk/ui/GridLayout.h>
#include <dtk/ui/Label.h>
#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ScrollWidget.h>
#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace play_app
    {
        struct FilesTool::Private
        {
            std::shared_ptr<dtk::ButtonGroup> aButtonGroup;
            std::shared_ptr<dtk::ButtonGroup> bButtonGroup;
            std::map<std::shared_ptr<play::FilesModelItem>, std::shared_ptr<FileButton> > aButtons;
            std::map<std::shared_ptr<play::FilesModelItem>, std::shared_ptr<dtk::ToolButton> > bButtons;
            std::vector<std::shared_ptr<dtk::ComboBox> > layerComboBoxes;
            std::shared_ptr<dtk::FloatEditSlider> wipeXSlider;
            std::shared_ptr<dtk::FloatEditSlider> wipeYSlider;
            std::shared_ptr<dtk::FloatEditSlider> wipeRotationSlider;
            std::shared_ptr<dtk::FloatEditSlider> overlaySlider;
            std::shared_ptr<dtk::GridLayout> widgetLayout;

            std::shared_ptr<dtk::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
            std::shared_ptr<dtk::ValueObserver<std::shared_ptr<play::FilesModelItem> > > aObserver;
            std::shared_ptr<dtk::ListObserver<std::shared_ptr<play::FilesModelItem> > > bObserver;
            std::shared_ptr<dtk::ListObserver<int> > layersObserver;
            std::shared_ptr<dtk::ValueObserver<timeline::CompareOptions> > compareObserver;
        };

        void FilesTool::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                context,
                app,
                Tool::Files,
                "tl::play_app::FilesTool",
                parent);
            DTK_P();

            p.aButtonGroup = dtk::ButtonGroup::create(context, dtk::ButtonGroupType::Radio);
            p.bButtonGroup = dtk::ButtonGroup::create(context, dtk::ButtonGroupType::Check);

            p.wipeXSlider = dtk::FloatEditSlider::create(context);
            p.wipeXSlider->setDefaultValue(.5F);
            p.wipeYSlider = dtk::FloatEditSlider::create(context);
            p.wipeYSlider->setDefaultValue(.5F);
            p.wipeRotationSlider = dtk::FloatEditSlider::create(context);
            p.wipeRotationSlider->setRange(dtk::RangeF(0.F, 360.F));
            p.wipeRotationSlider->setStep(1.F);
            p.wipeRotationSlider->setLargeStep(10.F);
            p.wipeRotationSlider->setDefaultValue(0.F);

            p.overlaySlider = dtk::FloatEditSlider::create(context);
            p.overlaySlider->setDefaultValue(.5F);

            auto layout = dtk::VerticalLayout::create(context);
            layout->setSpacingRole(dtk::SizeRole::None);

            p.widgetLayout = dtk::GridLayout::create(context, layout);
            p.widgetLayout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.widgetLayout->setSpacingRole(dtk::SizeRole::None);

            auto vLayout = dtk::VerticalLayout::create(context, layout);
            vLayout->setSpacingRole(dtk::SizeRole::None);
            auto bellows = dtk::Bellows::create(context, "Wipe", vLayout);
            auto gridLayout = dtk::GridLayout::create(context);
            gridLayout->setMarginRole(dtk::SizeRole::MarginSmall);
            auto label = dtk::Label::create(context, "X:", gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.wipeXSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.wipeXSlider, 0, 1);
            label = dtk::Label::create(context, "Y:", gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.wipeYSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.wipeYSlider, 1, 1);
            label = dtk::Label::create(context, "Rotation:", gridLayout);
            gridLayout->setGridPos(label, 2, 0);
            p.wipeRotationSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.wipeRotationSlider, 2, 1);
            bellows->setWidget(gridLayout);
            
            bellows = dtk::Bellows::create(context, "Overlay", vLayout);
            gridLayout = dtk::GridLayout::create(context);
            gridLayout->setMarginRole(dtk::SizeRole::MarginSmall);
            p.overlaySlider->setParent(gridLayout);
            gridLayout->setGridPos(p.overlaySlider, 0, 0);
            bellows->setWidget(gridLayout);

            auto scrollWidget = dtk::ScrollWidget::create(context, dtk::ScrollType::Both);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);

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

            p.wipeXSlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.wipeCenter.x = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            p.wipeYSlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.wipeCenter.y = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            p.wipeRotationSlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.wipeRotation = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            p.overlaySlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getFilesModel()->getCompareOptions();
                        options.overlay = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            p.filesObserver = dtk::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _filesUpdate(value);
                });

            p.aObserver = dtk::ValueObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->getFilesModel()->observeA(),
                [this](const std::shared_ptr<play::FilesModelItem>& value)
                {
                    _aUpdate(value);
                });

            p.bObserver = dtk::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->getFilesModel()->observeB(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
                {
                    _bUpdate(value);
                });

            p.layersObserver = dtk::ListObserver<int>::create(
                app->getFilesModel()->observeLayers(),
                [this](const std::vector<int>& value)
                {
                    _layersUpdate(value);
                });

            p.compareObserver = dtk::ValueObserver<timeline::CompareOptions>::create(
                app->getFilesModel()->observeCompareOptions(),
                [this](const timeline::CompareOptions& value)
                {
                    _compareUpdate(value);
                });
        }

        FilesTool::FilesTool() :
            _p(new Private)
        {}

        FilesTool::~FilesTool()
        {}

        std::shared_ptr<FilesTool> FilesTool::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FilesTool>(new FilesTool);
            out->_init(context, app, parent);
            return out;
        }

        void FilesTool::_filesUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            DTK_P();
            p.aButtonGroup->clearButtons();
            p.bButtonGroup->clearButtons();
            p.layerComboBoxes.clear();
            auto children = p.widgetLayout->getChildren();
            for (const auto& widget : children)
            {
                widget->setParent(nullptr);
            }
            children.clear();
            auto appWeak = _app;
            if (auto app = appWeak.lock())
            {
                const auto& a = app->getFilesModel()->getA();
                const auto& b = app->getFilesModel()->getB();
                if (auto context = getContext())
                {
                    size_t row = 0;
                    for (const auto& item : value)
                    {
                        auto aButton = FileButton::create(context, item);
                        aButton->setChecked(item == a);
                        aButton->setTooltip(item->path.get());
                        p.aButtons[item] = aButton;
                        p.aButtonGroup->addButton(aButton);
                        aButton->setParent(p.widgetLayout);
                        p.widgetLayout->setGridPos(aButton, row, 0);

                        auto bButton = dtk::ToolButton::create(context);
                        bButton->setText("B");
                        const auto i = std::find(b.begin(), b.end(), item);
                        bButton->setChecked(i != b.end());
                        bButton->setVAlign(dtk::VAlign::Center);
                        bButton->setTooltip("Set the B file(s)");
                        p.bButtons[item] = bButton;
                        p.bButtonGroup->addButton(bButton);
                        bButton->setParent(p.widgetLayout);
                        p.widgetLayout->setGridPos(bButton, row, 1);

                        auto layerComboBox = dtk::ComboBox::create(context);
                        layerComboBox->setItems(item->videoLayers);
                        layerComboBox->setCurrentIndex(item->videoLayer);
                        layerComboBox->setHAlign(dtk::HAlign::Left);
                        layerComboBox->setVAlign(dtk::VAlign::Center);
                        layerComboBox->setTooltip("Set the current layer");
                        p.layerComboBoxes.push_back(layerComboBox);
                        layerComboBox->setParent(p.widgetLayout);
                        p.widgetLayout->setGridPos(layerComboBox, row, 2);

                        layerComboBox->setIndexCallback(
                            [appWeak, item](int value)
                            {
                                if (auto app = appWeak.lock())
                                {
                                    app->getFilesModel()->setLayer(item, value);
                                }
                            });

                        ++row;
                    }
                    if (value.empty())
                    {
                        auto label = dtk::Label::create(context, "No files open", p.widgetLayout);
                        p.widgetLayout->setGridPos(label, 0, 0);
                    }
                }
            }
        }

        void FilesTool::_aUpdate(const std::shared_ptr<play::FilesModelItem>& value)
        {
            DTK_P();
            for (const auto& button : p.aButtons)
            {
                button.second->setChecked(button.first == value);
            }
        }

        void FilesTool::_bUpdate(const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            DTK_P();
            for (const auto& button : p.bButtons)
            {
                const auto i = std::find(value.begin(), value.end(), button.first);
                button.second->setChecked(i != value.end());
            }
        }

        void FilesTool::_layersUpdate(const std::vector<int>& value)
        {
            DTK_P();
            for (size_t i = 0; i < value.size() && i < p.layerComboBoxes.size(); ++i)
            {
                p.layerComboBoxes[i]->setCurrentIndex(value[i]);
            }
        }

        void FilesTool::_compareUpdate(const timeline::CompareOptions& value)
        {
            DTK_P();
            p.wipeXSlider->setValue(value.wipeCenter.x);
            p.wipeYSlider->setValue(value.wipeCenter.y);
            p.wipeRotationSlider->setValue(value.wipeRotation);
            p.overlaySlider->setValue(value.overlay);
        }
    }
}
