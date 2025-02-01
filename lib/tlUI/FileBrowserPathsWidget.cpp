// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlUI/FileBrowserPrivate.h>

#include <tlUI/Bellows.h>
#include <tlUI/ButtonGroup.h>
#include <tlUI/DrivesModel.h>
#include <tlUI/ListButton.h>
#include <tlUI/RecentFilesModel.h>
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
            std::shared_ptr<RecentFilesModel> recentFilesModel;
            std::vector<file::Path> recent;
            std::vector<std::string> paths;
            std::vector<std::shared_ptr<ListButton> > buttons;
            std::shared_ptr<ButtonGroup> buttonGroup;
            std::map<std::string, std::shared_ptr<Bellows> > bellows;
            std::map<std::string, std::shared_ptr<VerticalLayout> > layouts;
            std::shared_ptr<VerticalLayout> layout;
            std::function<void(const std::string&)> callback;
            std::shared_ptr<dtk::ListObserver<std::string> > drivesObserver;
            std::shared_ptr<dtk::ListObserver<file::Path> > recentObserver;
        };

        void PathsWidget::_init(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::ui::PathsWidget", context, parent);
            TLRENDER_P();

            setBackgroundRole(ColorRole::Base);

            p.drivesModel = DrivesModel::create(context);

            p.buttonGroup = ButtonGroup::create(ButtonGroupType::Click, context);

            p.layout = VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(SizeRole::None);

            p.bellows["Drives"] = Bellows::create("Drives", context, p.layout);
            p.bellows["Drives"]->setOpen(true);
            p.layouts["Drives"] = VerticalLayout::create(context);
            p.layouts["Drives"]->setSpacingRole(SizeRole::None);
            p.bellows["Drives"]->setWidget(p.layouts["Drives"]);

            p.bellows["Shortcuts"] = Bellows::create("Shortcuts", context, p.layout);
            p.bellows["Shortcuts"]->setOpen(true);
            p.layouts["Shortcuts"] = VerticalLayout::create(context);
            p.layouts["Shortcuts"]->setSpacingRole(SizeRole::None);
            p.bellows["Shortcuts"]->setWidget(p.layouts["Shortcuts"]);

            p.bellows["Recent"] = Bellows::create("Recent", context, p.layout);
            p.bellows["Recent"]->setOpen(true);
            p.layouts["Recent"] = VerticalLayout::create(context);
            p.layouts["Recent"]->setSpacingRole(SizeRole::None);
            p.bellows["Recent"]->setWidget(p.layouts["Recent"]);

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

            p.drivesObserver = dtk::ListObserver<std::string>::create(
                p.drivesModel->observeDrives(),
                [this](const std::vector<std::string>& value)
                {
                    _p->drives = value;
                    _pathsUpdate();
                });
        }

        PathsWidget::PathsWidget() :
            _p(new Private)
        {}

        PathsWidget::~PathsWidget()
        {}

        std::shared_ptr<PathsWidget> PathsWidget::create(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<PathsWidget>(new PathsWidget);
            out->_init(context, parent);
            return out;
        }

        void PathsWidget::setCallback(const std::function<void(const std::string&)>& value)
        {
            _p->callback = value;
        }

        void PathsWidget::setRecentFilesModel(const std::shared_ptr<RecentFilesModel>& value)
        {
            TLRENDER_P();
            p.recentObserver.reset();
            p.recentFilesModel = value;
            if (p.recentFilesModel)
            {
                p.recentObserver = dtk::ListObserver<file::Path>::create(
                    p.recentFilesModel->observeRecent(),
                    [this](const std::vector<file::Path>& value)
                    {
                        _p->recent = value;
                        _pathsUpdate();
                    });
            }
        }

        void PathsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void PathsWidget::sizeHintEvent(const SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void PathsWidget::_createButton(
            const std::string& text,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            TLRENDER_P();
            auto button = ListButton::create(context);
            button->setText(text);
            button->setParent(parent);
            p.buttons.push_back(button);
            p.buttonGroup->addButton(button);
        }

        void PathsWidget::_pathsUpdate()
        {
            TLRENDER_P();
            
            p.paths.clear();
            for (auto layout : p.layouts)
            {
                auto children = layout.second->getChildren();
                for (auto i : children)
                {
                    i->setParent(nullptr);
                }
            }
            p.buttons.clear();
            p.buttonGroup->clearButtons();

            if (auto context = _context.lock())
            {
                for (const auto& i : p.drives)
                {
                    _createButton(i, context, p.layouts["Drives"]);
                    p.paths.push_back(i);
                }

                _createButton("Current", context, p.layouts["Shortcuts"]);
                p.paths.push_back(file::getCWD());
                for (auto userPath : file::getUserPathEnums())
                {
                    const std::string path = file::getUserPath(userPath);
                    _createButton(file::Path(path).getBaseName(), context, p.layouts["Shortcuts"]);
                    p.paths.push_back(path);
                }

                for (auto recent : p.recent)
                {
                    const std::string label = recent.get(-1, file::PathType::FileName);
                    _createButton(label, context, p.layouts["Recent"]);
                    p.paths.push_back(recent.getDirectory());
                }
            }
        }
    }
}
