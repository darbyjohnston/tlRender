// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Widgets/FileToolBar.h>

namespace tl
{
    namespace play
    {
        void FileToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            ToolBar::_init(context, dtk::Orientation::Horizontal, parent);
            auto tmp = actions;
            addAction(tmp["Open"]);
            addAction(tmp["OpenSeparateAudio"]);
            addAction(tmp["Close"]);
            addAction(tmp["CloseAll"]);
            addAction(tmp["Reload"]);
        }

        FileToolBar::~FileToolBar()
        {}

        std::shared_ptr<FileToolBar> FileToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileToolBar>(new FileToolBar);
            out->_init(context, actions, parent);
            return out;
        }
    }
}
