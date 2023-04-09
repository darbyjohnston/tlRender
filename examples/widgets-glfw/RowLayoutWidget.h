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
            //! Row layout widget.
            class RowLayoutWidget : public ui::IWidget
            {
                TLRENDER_NON_COPYABLE(RowLayoutWidget);

            protected:
                void _init(const std::shared_ptr<system::Context>&);

                RowLayoutWidget();

            public:
                ~RowLayoutWidget();

                static std::shared_ptr<RowLayoutWidget> create(
                    const std::shared_ptr<system::Context>&);

                void setGeometry(const math::BBox2i&) override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
