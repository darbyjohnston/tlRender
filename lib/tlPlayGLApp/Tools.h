// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Event.h>

#include <tlCore/MapObserver.h>

namespace tl
{
    namespace play_gl
    {
        //! Tools.
        enum class Tool
        {
            Files,
            Compare,
            Color,
            Info,
            Audio,
            Devices,
            Settings,
            Messages,
            SystemLog,

            Count,
            First = Audio
        };
        TLRENDER_ENUM(Tool);
        TLRENDER_ENUM_SERIALIZE(Tool);

        //! Get the tool text.
        std::string getText(Tool);

        //! Get the tool icon.
        std::string getIcon(Tool);

        //! Get the tool keyboard shortcut.
        ui::Key getShortcut(Tool);

        //! Tools model.
        class ToolsModel : public std::enable_shared_from_this<ToolsModel>
        {
            TLRENDER_NON_COPYABLE(ToolsModel);

        protected:
            void _init();

            ToolsModel();

        public:
            ~ToolsModel();

            //! Create a new model.
            static std::shared_ptr<ToolsModel> create();

            //! Get tool visibility.
            const std::map<Tool, bool>& getToolsVisible() const;

            //! Observe tool visibility.
            std::shared_ptr<observer::IMap<Tool, bool> > observeToolsVisible() const;

            //! Set tool visibility.
            void setToolVisible(Tool, bool);

        private:
            TLRENDER_PRIVATE();
        };
    }
}
