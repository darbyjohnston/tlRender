// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <ftk/Core/Util.h>

#include <QObject>

#include <memory>

namespace ftk
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
                const std::shared_ptr<ftk::Context>&,
                QObject* parent = nullptr);

            virtual ~ContextObject();

            //! Get the context.
            const std::shared_ptr<ftk::Context>& context() const;

        private:
            void _timerCallback();

            FTK_PRIVATE();
        };
    }
}
