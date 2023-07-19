// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayGLApp/Settings.h>

namespace tl
{
    namespace play_gl
    {
        struct Settings::Private
        {
            std::shared_ptr<observer::Map<std::string, std::string> > data;
        };

        void Settings::_init(
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.data = observer::Map<std::string, std::string>::create();
        }

        Settings::Settings() :
            _p(new Private)
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

        const std::map<std::string, std::string>& Settings::getData() const
        {
            return _p->data->get();
        }

        std::shared_ptr<observer::IMap<std::string, std::string> > Settings::observeData() const
        {
            return _p->data;
        }

        void Settings::setData(const std::string& key, const std::string& value)
        {
            _p->data->setItemOnlyIfChanged(key, value);
        }
    }
}
