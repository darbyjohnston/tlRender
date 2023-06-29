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
            //! Scroll areas.
            class ScrollAreas : public ui::IWidget
            {
                TLRENDER_NON_COPYABLE(ScrollAreas);

            protected:
                void _init(const std::shared_ptr<system::Context>&);

                ScrollAreas();

            public:
                ~ScrollAreas();

                static std::shared_ptr<ScrollAreas> create(
                    const std::shared_ptr<system::Context>&);

                void setGeometry(const math::BBox2i&) override;

            private:
                TLRENDER_PRIVATE();
            };
        }
    }
}
