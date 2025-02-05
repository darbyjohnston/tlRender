// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "IExampleWidget.h"

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            //! MDI example.
            class MDI : public IExampleWidget
            {
                TLRENDER_NON_COPYABLE(MDI);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                MDI();

            public:
                ~MDI();

                static std::shared_ptr<MDI> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const dtk::Box2I&) override;
                void sizeHintEvent(const ui::SizeHintEvent&) override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
