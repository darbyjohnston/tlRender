// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QObject>

#include <memory>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace qt
    {
        //! Context object.
        class ContextObject : public QObject
        {
            Q_OBJECT

        public:
            ContextObject(
                const std::shared_ptr<system::Context>&,
                QObject* parent = nullptr);

            //! Get the context.
            const std::shared_ptr<system::Context>& context() const;

        protected:
            void timerEvent(QTimerEvent*) override;

        private:
            TLRENDER_PRIVATE();
        };
    }
}
