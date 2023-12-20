// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlQt/ContextObject.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace qt
    {
        namespace
        {
            const size_t timeout = 5;
        }

        struct ContextObject::Private
        {
            std::shared_ptr<system::Context> context;
        };

        ContextObject::ContextObject(
            const std::shared_ptr<system::Context>& context,
            QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            _p->context = context;

            startTimer(timeout, Qt::PreciseTimer);
        }

        ContextObject::~ContextObject()
        {}

        const std::shared_ptr<system::Context>& ContextObject::context() const
        {
            return _p->context;
        }

        void ContextObject::timerEvent(QTimerEvent*)
        {
            _p->context->tick();
        }
    }
}
