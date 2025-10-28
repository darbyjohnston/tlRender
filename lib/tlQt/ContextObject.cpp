// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQt/ContextObject.h>

#include <ftk/Core/Context.h>

#include <QTimer>

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
            std::shared_ptr<ftk::Context> context;
            std::unique_ptr<QTimer> timer;
        };

        ContextObject::ContextObject(
            const std::shared_ptr<ftk::Context>& context,
            QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            FTK_P();

            p.context = context;

            p.timer.reset(new QTimer);
            p.timer->setTimerType(Qt::PreciseTimer);
            connect(p.timer.get(), &QTimer::timeout, this, &ContextObject::_timerCallback);
            p.timer->start(timeout);
        }

        ContextObject::~ContextObject()
        {}

        const std::shared_ptr<ftk::Context>& ContextObject::context() const
        {
            return _p->context;
        }

        void ContextObject::_timerCallback()
        {
            if (_p && _p->context)
            {
                _p->context->tick();
            }
        }
    }
}
