// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/ContextObject.h>

#include <tlApp/IApp.h>

#include <QApplication>

namespace tl
{
    namespace examples
    {
        //! Example showing the timeline widget.
        namespace timeline_qtwidget
        {
            //! Application.
            class App : public QApplication, public app::IApp
            {
                Q_OBJECT

            public:
                App(
                    int& argc,
                    char** argv,
                    const std::shared_ptr<system::Context>&);

            private:
                std::string _input;
                qt::ContextObject* _contextObject = nullptr;
            };
        }
    }
}
