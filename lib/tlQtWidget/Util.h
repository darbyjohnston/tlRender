// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <QFont>

#include <memory>

namespace tl
{
    namespace system
    {
        class Context;
    }

    //! Qt QWidget support.
    namespace qtwidget
    {
        //! Initialize the library. This needs to be called before the Qt
        //! application is created.
        void init(const std::shared_ptr<system::Context>&);

        //! Get a font.
        QFont font(const QString&);
    }
}
