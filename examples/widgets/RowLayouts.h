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
            //! Row layouts.
            class RowLayouts : public IExampleWidget
            {
                TLRENDER_NON_COPYABLE(RowLayouts);

            protected:
                void _init(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                RowLayouts();

            public:
                ~RowLayouts();

                static std::shared_ptr<RowLayouts> create(
                    const std::shared_ptr<dtk::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const math::Box2i&) override;
                void sizeHintEvent(const ui::SizeHintEvent&) override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
