// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Icon widget.
        class Icon : public IWidget
        {
            TLRENDER_NON_COPYABLE(Icon);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            Icon();

        public:
            ~Icon() override;

            //! Create a new widget.
            static std::shared_ptr<Icon> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Create a new widget.
            static std::shared_ptr<Icon> create(
                const std::string& icon,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the icon.
            void setIcon(const std::string&);

            //! Set the margin role.
            void setMarginRole(SizeRole);

            void tickEvent(
                bool,
                bool,
                const TickEvent&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(
                const math::BBox2i&,
                bool,
                const ClipEvent&) override;
            void drawEvent(
                const math::BBox2i&,
                const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
