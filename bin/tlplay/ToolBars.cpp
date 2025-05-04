// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "ToolBars.h"

#include "CompareActions.h"
#include "FileActions.h"
#include "ViewActions.h"
#include "WindowActions.h"

#include <dtk/ui/Divider.h>

namespace tl
{
    namespace play
    {
        void FileToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<FileActions>& fileActions,
            const std::shared_ptr<IWidget>& parent)
        {
            ToolBar::_init(context, dtk::Orientation::Horizontal, parent);
            auto actions = fileActions->getActions();
            addAction(actions["Open"]);
            addAction(actions["Close"]);
            addAction(actions["CloseAll"]);
            addAction(actions["Reload"]);
        }

        FileToolBar::~FileToolBar()
        {
        }

        std::shared_ptr<FileToolBar> FileToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<FileActions>& fileActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FileToolBar>(new FileToolBar);
            out->_init(context, fileActions, parent);
            return out;
        }

        void CompareToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<CompareActions>& compareActions,
            const std::shared_ptr<IWidget>& parent)
        {
            ToolBar::_init(context, dtk::Orientation::Horizontal, parent);
            auto actions = compareActions->getActions();
            for (const auto& label : timeline::getCompareLabels())
            {
                addAction(actions[label]);
            }
        }

        CompareToolBar::~CompareToolBar()
        {
        }

        std::shared_ptr<CompareToolBar> CompareToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<CompareActions>& compareActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CompareToolBar>(new CompareToolBar);
            out->_init(context, compareActions, parent);
            return out;
        }

        void ViewToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<IWidget>& parent)
        {
            ToolBar::_init(context, dtk::Orientation::Horizontal, parent);
            auto actions = viewActions->getActions();
            addAction(actions["Frame"]);
            addAction(actions["ZoomReset"]);
            addAction(actions["ZoomIn"]);
            addAction(actions["ZoomOut"]);
        }

        ViewToolBar::~ViewToolBar()
        {
        }

        std::shared_ptr<ViewToolBar> ViewToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewToolBar>(new ViewToolBar);
            out->_init(context, viewActions, parent);
            return out;
        }

        void WindowToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<WindowActions>& windowActions,
            const std::shared_ptr<IWidget>& parent)
        {
            ToolBar::_init(context, dtk::Orientation::Horizontal, parent);
            auto actions = windowActions->getActions();
            addAction(actions["FullScreen"]);
            addAction(actions["Settings"]);
        }

        WindowToolBar::~WindowToolBar()
        {
        }

        std::shared_ptr<WindowToolBar> WindowToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<WindowActions>& windowActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<WindowToolBar>(new WindowToolBar);
            out->_init(context, windowActions, parent);
            return out;
        }

        void ToolBars::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<FileActions>& fileActions,
            const std::shared_ptr<CompareActions>& compareActions,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<WindowActions>& windowActions,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(context, "ToolBars", parent);
            _layout = dtk::HorizontalLayout::create(context, shared_from_this());
            _layout->setSpacingRole(dtk::SizeRole::SpacingSmall);
            FileToolBar::create(context, fileActions, _layout);
            dtk::Divider::create(context, dtk::Orientation::Horizontal, _layout);
            CompareToolBar::create(context, compareActions, _layout);
            dtk::Divider::create(context, dtk::Orientation::Horizontal, _layout);
            ViewToolBar::create(context, viewActions, _layout);
            dtk::Divider::create(context, dtk::Orientation::Horizontal, _layout);
            WindowToolBar::create(context, windowActions, _layout);
        }

        ToolBars::~ToolBars()
        {
        }

        std::shared_ptr<ToolBars> ToolBars::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::shared_ptr<FileActions>& fileActions,
            const std::shared_ptr<CompareActions>& compareActions,
            const std::shared_ptr<ViewActions>& viewActions,
            const std::shared_ptr<WindowActions>& windowActions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolBars>(new ToolBars);
            out->_init(
                context,
                fileActions,
                compareActions,
                viewActions,
                windowActions,
                parent);
            return out;
        }

        void ToolBars::setGeometry(const dtk::Box2I& value)
        {
            IWidget::setGeometry(value);
            _layout->setGeometry(value);
        }

        void ToolBars::sizeHintEvent(const dtk::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _setSizeHint(_layout->getSizeHint());
        }
    }
}