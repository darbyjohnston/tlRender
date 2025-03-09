// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Widgets/FileToolBar.h>

#include <tlPlayApp/App.h>

#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace play
    {
        struct FileToolBar::Private
        {
            std::map<std::string, std::shared_ptr<dtk::Action> > actions;
            std::map<std::string, std::shared_ptr<dtk::ToolButton> > buttons;
            std::shared_ptr<dtk::HorizontalLayout> layout;

            std::shared_ptr<dtk::ListObserver<std::shared_ptr<FilesModelItem> > > filesObserver;
        };

        void FileToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(
                context,
                "tl::play_app::FileToolBar",
                parent);
            DTK_P();

            p.actions = actions;

            p.buttons["Open"] = dtk::ToolButton::create(context, p.actions["Open"]);
            p.buttons["OpenSeparateAudio"] = dtk::ToolButton::create(context, p.actions["OpenSeparateAudio"]);
            p.buttons["Close"] = dtk::ToolButton::create(context, p.actions["Close"]);
            p.buttons["CloseAll"] = dtk::ToolButton::create(context, p.actions["CloseAll"]);

            p.layout = dtk::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(dtk::SizeRole::None);
            p.buttons["Open"]->setParent(p.layout);
            p.buttons["OpenSeparateAudio"]->setParent(p.layout);
            p.buttons["Close"]->setParent(p.layout);
            p.buttons["CloseAll"]->setParent(p.layout);

            p.filesObserver = dtk::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<FilesModelItem> >& value)
                {
                    _filesUpdate(value);
                });
        }

        FileToolBar::FileToolBar() :
            _p(new Private)
        {}

        FileToolBar::~FileToolBar()
        {}

        std::shared_ptr<FileToolBar> FileToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<App>& app,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileToolBar>(new FileToolBar);
            out->_init(context, app, actions, parent);
            return out;
        }

        void FileToolBar::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileToolBar::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_p->layout->getSizeHint());
        }

        void FileToolBar::_filesUpdate(
            const std::vector<std::shared_ptr<FilesModelItem> >& value)
        {
            DTK_P();
            p.buttons["Close"]->setEnabled(!value.empty());
            p.buttons["CloseAll"]->setEnabled(!value.empty());
        }
    }
}
