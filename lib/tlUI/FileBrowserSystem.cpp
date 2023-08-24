// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlUI/FileBrowser.h>

#include <tlUI/RecentFilesModel.h>

#include <tlCore/File.h>

#if defined(TLRENDER_NFD)
#include <nfd.hpp>
#endif // TLRENDER_NFD

namespace tl
{
    namespace ui
    {
        struct FileBrowserSystem::Private
        {
            bool native = true;
            std::string path;
            FileBrowserOptions options;
            std::shared_ptr<FileBrowser> fileBrowser;
            std::shared_ptr<RecentFilesModel> recentFilesModel;
        };

        void FileBrowserSystem::_init(const std::shared_ptr<system::Context>& context)
        {
            ISystem::_init("tl::ui::FileBrowserSystem", context);
            TLRENDER_P();

            p.path = file::getCWD();
            p.options.list.sequence = false;
            p.recentFilesModel = RecentFilesModel::create(context);

#if defined(TLRENDER_NFD)
            NFD::Init();
#endif // TLRENDER_NFD
        }

        FileBrowserSystem::FileBrowserSystem() :
            _p(new Private)
        {}

        FileBrowserSystem::~FileBrowserSystem()
        {
#if defined(TLRENDER_NFD)
            NFD::Quit();
#endif // TLRENDER_NFD
        }

        std::shared_ptr<FileBrowserSystem> FileBrowserSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<FileBrowserSystem>(new FileBrowserSystem);
            out->_init(context);
            return out;
        }

        void FileBrowserSystem::open(
            const std::shared_ptr<EventLoop>& eventLoop,
            const std::function<void(const file::FileInfo&)>& callback)
        {
            TLRENDER_P();
            bool native = p.native;
#if defined(TLRENDER_NFD)
            if (native)
            {
                nfdu8char_t* outPath = nullptr;
                NFD::OpenDialog(outPath);
                if (outPath)
                {
                    callback(file::FileInfo(file::Path(outPath)));
                    NFD::FreePath(outPath);
                }
            }
#else  // TLRENDER_NFD
            native = false;
#endif  // TLRENDER_NFD
            if (!native)
            {
                if (auto context = _context.lock())
                {
                    p.fileBrowser = FileBrowser::create(p.path, context);
                    p.fileBrowser->setOptions(p.options);
                    p.fileBrowser->setRecentFilesModel(p.recentFilesModel);
                    p.fileBrowser->open(eventLoop);
                    p.fileBrowser->setCallback(
                        [this, callback](const file::FileInfo& value)
                        {
                            callback(value);
                            _p->fileBrowser->close();
                        });
                    p.fileBrowser->setCloseCallback(
                        [this]
                        {
                            _p->path = _p->fileBrowser->getPath();
                            _p->options = _p->fileBrowser->getOptions();
                            _p->fileBrowser.reset();
                        });
                }
            }
        }

        bool FileBrowserSystem::isNativeFileDialog() const
        {
            return _p->native;
        }

        void FileBrowserSystem::setNativeFileDialog(bool value)
        {
            _p->native = value;
        }

        const std::string& FileBrowserSystem::getPath() const
        {
            return _p->path;
        }

        void FileBrowserSystem::setPath(const std::string& value)
        {
            _p->path = value;
        }

        const FileBrowserOptions& FileBrowserSystem::getOptions() const
        {
            return _p->options;
        }

        void FileBrowserSystem::setOptions(const FileBrowserOptions& options)
        {
            _p->options = options;
        }

        const std::shared_ptr<RecentFilesModel>& FileBrowserSystem::getRecentFilesModel() const
        {
            return _p->recentFilesModel;
        }
    }
}
