// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Bellows widget.
        class Bellows : public IWidget
        {
            DTK_NON_COPYABLE(Bellows);

        protected:
            void _init(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Bellows();

        public:
            virtual ~Bellows();

            //! Create a new widget.
            static std::shared_ptr<Bellows> create(
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Create a new widget.
            static std::shared_ptr<Bellows> create(
                const std::string& text,
                const std::shared_ptr<dtk::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the text.
            void setText(const std::string&);

            //! Set the widget.
            void setWidget(const std::shared_ptr<IWidget>&);

            //! Get whether the bellows is open.
            bool isOpen() const;

            //! Set whether the bellows is open.
            void setOpen(bool);

            void setGeometry(const dtk::Box2I&) override;
            void sizeHintEvent(const SizeHintEvent&) override;

        private:
            DTK_PRIVATE();
        };
    }
}
