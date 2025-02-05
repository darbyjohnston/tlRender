// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#include <tlPlay/Settings.h>

#include <dtk/core/Context.h>
#include <dtk/core/FileIO.h>
#include <dtk/core/LogSystem.h>

#include <dtk/core/Format.h>

#include <filesystem>
#include <iostream>

namespace tl
{
    namespace play
    {
        void Settings::_init(
            const std::shared_ptr<dtk::Context>& context,
            const std::string fileName,
            bool reset)
        {
            _context = context;
            _fileName = fileName;
            _observer = dtk::ObservableValue<std::string>::create();
            if (!reset)
            {
                _read();
            }
        }

        Settings::Settings() 
        {}

        Settings::~Settings()
        {
            _write();
        }

        std::shared_ptr<Settings> Settings::create(
            const std::shared_ptr<dtk::Context>& context,
            const std::string fileName,
            bool reset)
        {
            auto out = std::shared_ptr<Settings>(new Settings);
            out->_init(context, fileName, reset);
            return out;
        }

        std::shared_ptr<dtk::IObservableValue<std::string> > Settings::observeValues() const
        {
            return _observer;
        }

        void Settings::reset()
        {
            for (auto i = _defaultValues.begin(); i != _defaultValues.end(); ++i)
            {
                _values[i.key()] = i.value();
                _observer->setAlways(i.key());
            }
        }

        void Settings::_read()
        {
            if (std::filesystem::exists(std::filesystem::u8path(_fileName)))
            {
                try
                {
                    auto io = dtk::FileIO::create(_fileName, dtk::FileMode::Read);
                    const std::string contents = dtk::read(io);
                    const auto values = nlohmann::json::parse(contents);
                    for (auto i = values.begin(); i != values.end(); ++i)
                    {
                        _values[i.key()] = i.value();
                    }
                }
                catch (const std::exception& e)
                {
                    if (auto context = _context.lock())
                    {
                        context->log(
                            "tl::play::Settings",
                            dtk::Format("Cannot read settings file: {0}: {1}").
                            arg(_fileName).
                            arg(e.what()),
                            dtk::LogType::Error);
                    }
                }
            }
        }

        void Settings::_write()
        {
            try
            {
                auto io = dtk::FileIO::create(_fileName, dtk::FileMode::Write);
                const std::string contents = _values.dump(4);
                io->write(contents.c_str(), contents.size());
            }
            catch (const std::exception& e)
            {
                if (auto context = _context.lock())
                {
                    context->log(
                        "tl::play::Settings",
                        dtk::Format("Cannot write settings file: {0}: {1}").
                        arg(_fileName).
                        arg(e.what()),
                        dtk::LogType::Error);
                }
            }
        }
    }
}
