// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlViewApp/App.h>

#include <tlViewApp/MainWindow.h>

#include <tlQtWidget/Style.h>

#include <tlQt/ContextObject.h>
#include <tlQt/TimeObject.h>

#include <tlTimeline/Timeline.h>

#include <QFileDialog>

namespace tl
{
    namespace view
    {
        namespace
        {
            struct Options
            {
                std::string fileName;
            };
        }

        struct App::Private
        {
            Options options;

            qt::ContextObject* contextObject = nullptr;
            qt::TimeObject* timeObject = nullptr;

            std::string fileName;
            otio::SerializableObject::Retainer<otio::Timeline> timeline;

            MainWindow* mainWindow = nullptr;
        };

        App::App(
            int& argc,
            char** argv,
            const std::shared_ptr<system::Context>& context) :
            QApplication(argc, argv),
            _p(new Private)
        {
            TLRENDER_P();

            IApp::_init(
                argc,
                argv,
                context,
                "tlview",
                "View timelines.",
                {
                    app::CmdLineValueArg<std::string>::create(
                        p.options.fileName,
                        "input",
                        "Timeline or folder.",
                        true)
                });
            const int exitCode = getExit();
            if (exitCode != 0)
            {
                exit(exitCode);
                return;
            }

            // Initialize Qt.
            QCoreApplication::setOrganizationName("tlRender");
            QCoreApplication::setApplicationName("tlview");
            setStyle("Fusion");
            setPalette(qtwidget::darkStyle());
            setStyleSheet(qtwidget::styleSheet());

            // Create objects.
            p.contextObject = new qt::ContextObject(context, this);
            p.timeObject = new qt::TimeObject(this);

            // Create the main window.
            p.mainWindow = new MainWindow(this);

            // Open the input files.
            if (!p.options.fileName.empty())
            {
                open(QString::fromUtf8(p.options.fileName.c_str()));
            }

            p.mainWindow->show();
        }

        App::~App()
        {
            TLRENDER_P();

            delete p.mainWindow;
            p.mainWindow = nullptr;
        }

        qt::TimeObject* App::timeObject() const
        {
            return _p->timeObject;
        }

        void App::open(const QString& fileName)
        {
            TLRENDER_P();

            p.fileName = fileName.toUtf8().data();

            otio::ErrorStatus errorStatus;
            std::string error;
            p.timeline = timeline::read(p.fileName, &errorStatus);
            if (otio::is_error(errorStatus))
            {
                p.timeline = nullptr;
                error = errorStatus.full_description;
            }
            else if (!p.timeline)
            {
                error = "Cannot read timeline";
            }
            if (!error.empty())
            {
                _log(error, log::Type::Error);
            }

            Q_EMIT timelineChanged(p.timeline);
        }

        void App::openDialog()
        {
            TLRENDER_P();

            std::vector<std::string> extensions;
			extensions.push_back(".otio");

            QString dir;
            if (!p.fileName.empty())
            {
                dir = QString::fromUtf8(file::Path(p.fileName).getDirectory().c_str());
            }

            const auto fileName = QFileDialog::getOpenFileName(
                p.mainWindow,
                tr("Open"),
                dir,
                tr("Files") + " (" + QString::fromUtf8(string::join(extensions, " ").c_str()) + ")");
            if (!fileName.isEmpty())
            {
                open(fileName);
            }
        }
    }
}
