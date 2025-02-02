// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "IExampleWidget.h"

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            void IExampleWidget::_init(
                const std::string& exampleName,
                const std::string& objectName,
                const std::shared_ptr<dtk::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IWidget::_init(objectName, context, parent);
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
