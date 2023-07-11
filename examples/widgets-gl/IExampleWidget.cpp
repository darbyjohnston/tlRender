// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include "IExampleWidget.h"

namespace tl
{
    namespace examples
    {
        namespace widgets_gl
        {
            void IExampleWidget::_init(
                const std::string& exampleName,
                const std::string& name,
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init(name, context, parent);
                _exampleName = exampleName;
            }

            IExampleWidget::IExampleWidget()
            {}

            IExampleWidget::~IExampleWidget()
            {}

            const std::string& IExampleWidget::getExampleName() const
            {
                return _exampleName;
            }
        }
    }
}
