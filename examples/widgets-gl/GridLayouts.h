// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/IWidget.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_gl
        {
            //! Grid layouts.
            class GridLayouts : public ui::IWidget
            {
                TLRENDER_NON_COPYABLE(GridLayouts);

            protected:
                void _init(const std::shared_ptr<system::Context>&);

                GridLayouts();

            public:
                ~GridLayouts();

                static std::shared_ptr<GridLayouts> create(
                    const std::shared_ptr<system::Context>&);

                void setGeometry(const math::BBox2i&) override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
