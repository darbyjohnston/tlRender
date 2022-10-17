// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlApp/IApp.h>

#include <tlCore/Time.h>

#include <opentimelineio/timeline.h>

#include <QApplication>

namespace tl
{
    namespace qt
    {
        class TimeObject;
    }

    //! View application.
    namespace view
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

            ~App() override;

            //! Get the time object.
            qt::TimeObject* timeObject() const;

        public Q_SLOTS:
            //! Open a file.
            void open(const QString&);

            //! Open a file dialog.
            void openDialog();

        Q_SIGNALS:
            //! This signal is emitted when the timeline is changed.
            void timelineChanged(tl::otio::Timeline*);

        private Q_SLOTS:

        private:
            TLRENDER_PRIVATE();
        };
    }
}
