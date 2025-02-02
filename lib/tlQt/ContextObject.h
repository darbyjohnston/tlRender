// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QObject>

#include <memory>

namespace dtk
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
                const std::shared_ptr<dtk::Context>&,
                QObject* parent = nullptr);

            virtual ~ContextObject();

            //! Get the context.
            const std::shared_ptr<dtk::Context>& context() const;

        private:
            void _timerCallback();

            TLRENDER_PRIVATE();
        };
    }
}
