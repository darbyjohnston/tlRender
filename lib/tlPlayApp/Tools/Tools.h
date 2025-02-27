// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/ui/Event.h>
#include <dtk/core/ObservableValue.h>

namespace tl
{
    namespace play
    {
        //! Tools.
        enum class Tool
        {
            None,
            Files,
            Export,
            View,
            ColorPicker,
            ColorControls,
            Info,
            Audio,
            Devices,
            Settings,
            Messages,
            SystemLog,

            Count,
            First = None
        };
        DTK_ENUM(Tool);

        //! Get the tool text.
        std::string getText(Tool);

        //! Get the tool icon.
        std::string getIcon(Tool);

        //! Get the tool keyboard shortcut.
        dtk::Key getShortcut(Tool);

        //! Get the tool tooltip.
        std::string getTooltip(Tool);

        //! Get the tools shown in the toolbar.
        std::vector<Tool> getToolsInToolbar();

        //! Tools model.
        class ToolsModel : public std::enable_shared_from_this<ToolsModel>
        {
            DTK_NON_COPYABLE(ToolsModel);

        protected:
            void _init();

            ToolsModel();

        public:
            ~ToolsModel();

            //! Create a new model.
            static std::shared_ptr<ToolsModel> create();

            //! Get the active tool.
            Tool getActiveTool() const;

            //! Observe the active tool.
            std::shared_ptr<dtk::ObservableValue<Tool> > observeActiveTool() const;

            //! Set the active tool.
            void setActiveTool(Tool);

        private:
            DTK_PRIVATE();
        };
    }
}
