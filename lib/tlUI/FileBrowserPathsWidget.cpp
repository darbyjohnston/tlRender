// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FileBrowserPrivate.h>

#include <tlUI/ButtonGroup.h>
#include <tlUI/Divider.h>
#include <tlUI/DrivesModel.h>
#include <tlUI/Label.h>
#include <tlUI/ListButton.h>
#include <tlUI/RowLayout.h>

#include <tlCore/File.h>

namespace tl
{
    namespace ui
    {
        struct PathsWidget::Private
        {
            std::shared_ptr<DrivesModel> drivesModel;
            std::vector<std::string> drives;
            std::shared_ptr<RecentFilesModel> recentModel;
            std::vector<file::Path> recent;
            std::vector<std::string> paths;
            std::vector<std::shared_ptr<ListButton> > buttons;
            std::shared_ptr<ButtonGroup> buttonGroup;
            std::shared_ptr<VerticalLayout> layout;
            std::function<void(const std::string&)> callback;
            std::shared_ptr<observer::ListObserver<std::string> > drivesObserver;
            std::shared_ptr<observer::ListObserver<file::Path> > recentObserver;
        };

        void PathsWidget::_init(
            const std::shared_ptr<RecentFilesModel>& recentModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::PathsWidget", context, parent);
            TLRENDER_P();

            p.drivesModel = DrivesModel::create(context);

            p.recentModel = recentModel;

            p.buttonGroup = ButtonGroup::create(ButtonGroupType::Click, context);

            p.layout = VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);

            _pathsUpdate();

            p.buttonGroup->setClickedCallback(
                [this](int value)
                {
                    TLRENDER_P();
                    if (value >= 0 && value < p.paths.size())
                    {
                        const std::string& path = p.paths[value];
                        if (p.callback)
                        {
                            p.callback(path);
                        }
                    }
                });

            p.drivesObserver = observer::ListObserver<std::string>::create(
                p.drivesModel->observeDrives(),
                [this](const std::vector<std::string>& value)
                {
                    _p->drives = value;
                    _pathsUpdate();
                });

            p.recentObserver = observer::ListObserver<file::Path>::create(
                p.recentModel->observeRecent(),
                [this](const std::vector<file::Path>& value)
                {
                    _p->recent = value;
                    _pathsUpdate();
                });
        }

        PathsWidget::PathsWidget() :
            _p(new Private)
        {}

        PathsWidget::~PathsWidget()
        {}

        std::shared_ptr<PathsWidget> PathsWidget::create(
            const std::shared_ptr<RecentFilesModel>& recentFilesModel,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PathsWidget>(new PathsWidget);
            out->_init(recentFilesModel, context, parent);
            return out;
        }

        void PathsWidget::setCallback(const std::function<void(const std::string&)>& value)
        {
            _p->callback = value;
        }

        void PathsWidget::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void PathsWidget::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void PathsWidget::_createLabel(
            const std::string& text,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            auto label = Label::create(context);
            label->setText(text);
            label->setMarginRole(SizeRole::MarginSmall);
            label->setParent(p.layout);
        }

        void PathsWidget::_createButton(
            const std::string& text,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            auto button = ListButton::create(context);
            button->setText(text);
            button->setParent(p.layout);
            p.buttons.push_back(button);
            p.buttonGroup->addButton(button);
        }

        void PathsWidget::_pathsUpdate()
        {
            TLRENDER_P();
            
            p.paths.clear();
            auto children = p.layout->getChildren();
            for (auto i : children)
            {
                i->setParent(nullptr);
            }
            p.buttons.clear();
            p.buttonGroup->clearButtons();

            if (auto context = _context.lock())
            {
                _createLabel("Drives:", context);
                for (const auto& i : p.drives)
                {
                    _createButton(i, context);
                    p.paths.push_back(i);
                }

                auto divider = Divider::create(Orientation::Vertical, context, p.layout);

                _createLabel("Shortcuts:", context);
                _createButton("Current", context);
                p.paths.push_back(file::getCWD());
                for (auto userPath : file::getUserPathEnums())
                {
                    const std::string path = file::getUserPath(userPath);
                    _createButton(file::Path(path).getBaseName(), context);
                    p.paths.push_back(path);
                }

                divider = Divider::create(Orientation::Vertical, context, p.layout);

                _createLabel("Recent:", context);
                for (auto recent : p.recent)
                {
                    const std::string label = file::Path(
                        file::removeEndSeparator(recent.getDirectory())).
                        getBaseName();
                    _createButton(label, context);
                    p.paths.push_back(recent.getDirectory());
                }
            }
        }
    }
}
