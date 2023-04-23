// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IWidget.h>

namespace tl
{
    namespace examples
    {
        namespace ui_glfw
        {
            //! Numeric widgets.
            class NumericWidgets : public ui::IWidget
            {
                TLRENDER_NON_COPYABLE(NumericWidgets);

            protected:
                void _init(const std::shared_ptr<system::Context>&);

                NumericWidgets();

            public:
                ~NumericWidgets();

                static std::shared_ptr<NumericWidgets> create(
                    const std::shared_ptr<system::Context>&);

                void setGeometry(const math::BBox2i&) override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
