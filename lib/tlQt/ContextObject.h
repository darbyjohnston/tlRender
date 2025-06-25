// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <feather-tk/core/Util.h>

#include <QObject>

#include <memory>

namespace feather_tk
{
    class Context;
}

namespace tl
{
    namespace qt
    {
        //! Context object.
        class ContextObject : public QObject
        {
            Q_OBJECT

        public:
            ContextObject(
                const std::shared_ptr<feather_tk::Context>&,
                QObject* parent = nullptr);

            virtual ~ContextObject();

            //! Get the context.
            const std::shared_ptr<feather_tk::Context>& context() const;

        private:
            void _timerCallback();

            FEATHER_TK_PRIVATE();
        };
    }
}
