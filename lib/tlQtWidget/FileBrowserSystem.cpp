// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlQtWidget/FileBrowserSystem.h>

#include <tlTimeline/Util.h>

#include <dtk/core/Context.h>

#include <QFileDialog>

#include <filesystem>

namespace tl
{
    namespace qtwidget
    {
        struct FileBrowserSystem::Private
        {
            std::string path;
            QStringList extensions;
        };

        FileBrowserSystem::FileBrowserSystem(const std::shared_ptr<dtk::Context>& context) :
            ISystem(context, "tl::qtwidget::FileBrowserSystem"),
            _p(new Private)
        {
            DTK_P();

            p.path = std::filesystem::current_path().u8string();

            std::vector<std::string> extensions;
            for (const auto& i : timeline::getExtensions(
                context,
                static_cast<int>(io::FileType::Movie) |
                static_cast<int>(io::FileType::Sequence) |
                static_cast<int>(io::FileType::Audio)))
            {
                p.extensions.push_back(QString::fromUtf8(i.c_str()));
            }

#if defined(TLRENDER_NFD)
            NFD::Init();
#endif // TLRENDER_NFD
        }

        FileBrowserSystem::~FileBrowserSystem()
        {
#if defined(TLRENDER_NFD)
            NFD::Quit();
#endif // TLRENDER_NFD
        }

        std::shared_ptr<FileBrowserSystem> FileBrowserSystem::create(const std::shared_ptr<dtk::Context>& context)
        {
            auto out = context->getSystem<FileBrowserSystem>();
            if (!out)
            {
                out = std::shared_ptr<FileBrowserSystem>(new FileBrowserSystem(context));
                context->addSystem(out);
            }
            return out;
        }

        void FileBrowserSystem::open(
            QWidget* window,
            const std::function<void(const file::Path&)>& callback)
        {
            DTK_P();
            QString filter;
            if (!_p->extensions.isEmpty())
            {
                filter.append(QObject::tr("Files"));
                filter.append(" (");
                QStringList extensions;
                Q_FOREACH(QString i, _p->extensions)
                {
                    extensions.push_back(QString("*%1").arg(i));
                }
                filter.append(extensions.join(' '));
                filter.append(")");
            }
            const auto fileName = QFileDialog::getOpenFileName(
                window,
                QObject::tr("Open"),
                QString::fromUtf8(p.path.c_str()));
            if (callback)
            {
                callback(file::Path(fileName.toUtf8().data()));
            }
        }

        const std::string& FileBrowserSystem::getPath() const
        {
            return _p->path;
        }

        void FileBrowserSystem::setPath(const std::string& value)
        {
            _p->path = value;
        }
    }
}
