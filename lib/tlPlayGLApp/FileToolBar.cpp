// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/FileToolBar.h>

#include <tlPlayGLApp/App.h>

#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_gl
    {
        struct FileToolBar::Private
        {
            std::weak_ptr<App> app;

            std::map<std::string, std::shared_ptr<ui::ToolButton> > buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<observer::ListObserver<std::shared_ptr<play::FilesModelItem> > > filesObserver;
        };

        void FileToolBar::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            IWidget::_init("tl::examples::play_gl::FileToolBar", context);
            TLRENDER_P();

            p.app = app;

            p.buttons["Open"] = ui::ToolButton::create(context);
            p.buttons["Open"]->setIcon("FileOpen");
            p.buttons["OpenSeparateAudio"] = ui::ToolButton::create(context);
            p.buttons["OpenSeparateAudio"]->setIcon("FileOpenSeparateAudio");
            p.buttons["OpenSeparateAudio"]->setEnabled(false);
            p.buttons["Close"] = ui::ToolButton::create(context);
            p.buttons["Close"]->setIcon("FileClose");
            p.buttons["CloseAll"] = ui::ToolButton::create(context);
            p.buttons["CloseAll"]->setIcon("FileCloseAll");

            p.layout = ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.buttons["Open"]->setParent(p.layout);
            p.buttons["OpenSeparateAudio"]->setParent(p.layout);
            p.buttons["Close"]->setParent(p.layout);
            p.buttons["CloseAll"]->setParent(p.layout);

            auto appWeak = std::weak_ptr<App>(app);
            p.buttons["Open"]->setClickedCallback(
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->openDialog();
                    }
                });
            p.buttons["OpenSeparateAudio"]->setClickedCallback(
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                    }
                });
            p.buttons["Close"]->setClickedCallback(
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->close();
                    }
                });
            p.buttons["CloseAll"]->setClickedCallback(
                [appWeak]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->closeAll();
                    }
                });

            p.filesObserver = observer::ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                app->getFilesModel()->observeFiles(),
                [this](const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
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
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<FileToolBar>(new FileToolBar);
            out->_init(app, context);
            return out;
        }

        void FileToolBar::setGeometry(const math::BBox2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void FileToolBar::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void FileToolBar::_filesUpdate(
            const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            TLRENDER_P();
            p.buttons["Close"]->setEnabled(!value.empty());
            p.buttons["CloseAll"]->setEnabled(!value.empty());
        }
    }
}
