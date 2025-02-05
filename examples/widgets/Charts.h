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
            //! Charts.
            class Charts : public IExampleWidget
            {
                DTK_NON_COPYABLE(Charts);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                Charts();

            public:
                ~Charts();

                static std::shared_ptr<Charts> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const dtk::Box2I&) override;
                void sizeHintEvent(const ui::SizeHintEvent&) override;

            private:
                DTK_PRIVATE();
            };
        }
    }
}
