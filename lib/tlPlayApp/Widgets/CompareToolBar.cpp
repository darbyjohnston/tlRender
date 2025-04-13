// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlayApp/Widgets/CompareToolBar.h>

#include <tlTimeline/CompareOptions.h>

#include <dtk/ui/RowLayout.h>
#include <dtk/ui/ToolButton.h>

namespace tl
{
    namespace play
    {
        void CompareToolBar::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            ToolBar::_init(context, dtk::Orientation::Horizontal, parent);

            auto tmp = actions;
            const auto labels = timeline::getCompareLabels();
            for (size_t i = 0; i < labels.size(); ++i)
            {
                addAction(tmp[labels[i]]);
            }
        }

        CompareToolBar::CompareToolBar()
        {}

        CompareToolBar::~CompareToolBar()
        {}

        std::shared_ptr<CompareToolBar> CompareToolBar::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::map<std::string, std::shared_ptr<dtk::Action> >& actions,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CompareToolBar>(new CompareToolBar);
            out->_init(context, actions, parent);
            return out;
        }
    }
}
