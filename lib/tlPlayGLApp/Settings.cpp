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
            std::map<std::string, std::string> defaultValues;
            std::shared_ptr<observer::Map<std::string, std::string> > values;
        };

        void Settings::_init(
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();
            p.values = observer::Map<std::string, std::string>::create();
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

        const std::map<std::string, std::string>& Settings::getValues() const
        {
            return _p->values->get();
        }

        std::string Settings::getValue(const std::string& value) const
        {
            TLRENDER_P();
            std::string out;
            if (p.values->hasKey(value))
            {
                out = p.values->getItem(value);
            }
            return out;
        }

        std::shared_ptr<observer::IMap<std::string, std::string> > Settings::observeValues() const
        {
            return _p->values;
        }

        void Settings::setValue(const std::string& key, const std::string& value)
        {
            TLRENDER_P();
            if (!p.values->hasKey(key))
            {
                p.defaultValues[key] = value;
            }
            p.values->setItemOnlyIfChanged(key, value);
        }
    }
}
