// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/Settings.h>

#include <tlCore/Context.h>
#include <tlCore/File.h>
#include <tlCore/FileIO.h>
#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_gl
    {
        void Settings::_init(
            const std::shared_ptr<system::Context>& context)
        {
            _context = context;
            _observer = observer::Value<std::string>::create();
        }

        Settings::Settings() 
        {}

        Settings::~Settings()
        {}

        std::shared_ptr<Settings> Settings::create(
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<Settings>(new Settings);
            out->_init(context);
            return out;
        }

        std::shared_ptr<observer::IValue<std::string> > Settings::observeValues() const
        {
            return _observer;
        }

        void Settings::read(const std::string& fileName)
        {
            if (file::exists(fileName))
            {
                try
                {
                    auto io = file::FileIO::create(fileName, file::Mode::Read);
                    const std::string contents = file::readContents(io);
                    _values = nlohmann::json::parse(contents);
                }
                catch (const std::exception& e)
                {
                    if (auto context = _context.lock())
                    {
                        context->log(
                            "tl::play_gl::Settings",
                            string::Format("Cannot read settings file: {0}: {1}").
                            arg(fileName).
                            arg(e.what()),
                            log::Type::Error);
                    }
                }
            }
        }

        void Settings::write(const std::string& fileName)
        {
            try
            {
                auto io = file::FileIO::create(fileName, file::Mode::Write);
                const std::string contents = _values.dump(4);
                io->write(contents.c_str(), contents.size());
            }
            catch (const std::exception& e)
            {
                if (auto context = _context.lock())
                {
                    context->log(
                        "tl::play_gl::Settings",
                        string::Format("Cannot write settings file: {0}: {1}").
                            arg(fileName).
                            arg(e.what()),
                        log::Type::Error);
                }
            }
        }
    }
}
