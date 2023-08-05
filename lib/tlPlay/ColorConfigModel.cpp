// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlay/ColorConfigModel.h>

#include <tlCore/Context.h>
#include <tlCore/OS.h>

#if defined(TLRENDER_OCIO)
#include <OpenColorIO/OpenColorIO.h>
#endif // TLRENDER_OCIO

#if defined(TLRENDER_OCIO)
namespace OCIO = OCIO_NAMESPACE;
#endif // TLRENDER_OCIO

namespace tl
{
    namespace play
    {
        bool ColorConfigModelData::operator == (const ColorConfigModelData& other) const
        {
            return
                enabled == other.enabled &&
                fileName == other.fileName &&
                inputs == other.inputs &&
                inputIndex == other.inputIndex &&
                displays == other.displays &&
                displayIndex == other.displayIndex &&
                views == other.views &&
                viewIndex == other.viewIndex;
        }

        bool ColorConfigModelData::operator != (const ColorConfigModelData& other) const
        {
            return !(*this == other);
        }

        struct ColorConfigModel::Private
        {
            std::weak_ptr<system::Context> context;
#if defined(TLRENDER_OCIO)
            OCIO_NAMESPACE::ConstConfigRcPtr ocioConfig;
#endif // TLRENDER_OCIO
            std::shared_ptr<observer::Value<timeline::ColorConfigOptions> > options;
            std::shared_ptr<observer::Value<ColorConfigModelData> > data;
        };

        void ColorConfigModel::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;

            p.options = observer::Value<timeline::ColorConfigOptions>::create();
            p.data = observer::Value<ColorConfigModelData>::create();

#if defined(TLRENDER_OCIO)
            std::string env;
            if (os::getEnv("OCIO", env) && !env.empty())
            {
                try
                {
                    p.ocioConfig.reset();
                    p.ocioConfig = OCIO::Config::CreateFromEnv();
                    if (p.ocioConfig)
                    {
                        timeline::ColorConfigOptions options;
                        options.fileName = env;
                        const char* display = p.ocioConfig->getDefaultDisplay();
                        options.display = display;
                        options.view = p.ocioConfig->getDefaultView(display);
                        p.options->setIfChanged(options);
                        _configUpdate();
                    }
                }
                catch (const std::exception& e)
                {
                    if (const auto context = p.context.lock())
                    {
                        context->log(std::string(), e.what(), log::Type::Error);
                    }
                }
            }
#endif // TLRENDER_OCIO
        }

        ColorConfigModel::ColorConfigModel() :
            _p(new Private)
        {}

        ColorConfigModel::~ColorConfigModel()
        {}

        std::shared_ptr<ColorConfigModel> ColorConfigModel::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<ColorConfigModel>(new ColorConfigModel);
            out->_init(context);
            return out;
        }

        std::shared_ptr<observer::IValue<timeline::ColorConfigOptions> > ColorConfigModel::observeConfigOptions() const
        {
            return _p->options;
        }

        void ColorConfigModel::setConfigOptions(const timeline::ColorConfigOptions& value)
        {
            TLRENDER_P();
            const bool changed = value.fileName != p.options->get().fileName;
#if defined(TLRENDER_OCIO)
            if (changed)
            {
                try
                {
                    p.ocioConfig.reset();
                    p.ocioConfig = OCIO::Config::CreateFromFile(value.fileName.c_str());
                }
                catch (const std::exception& e)
                {
                }
            }
#endif // TLRENDER_OCIO
            auto options = value;
#if defined(TLRENDER_OCIO)
            if (changed && p.ocioConfig)
            {
                const char* display = p.ocioConfig->getDefaultDisplay();
                options.display = display;
                options.view = p.ocioConfig->getDefaultView(display);
            }
#endif // TLRENDER_OCIO
            if (p.options->setIfChanged(options))
            {
                _configUpdate();
            }
        }

        void ColorConfigModel::setEnabled(bool value)
        {
            TLRENDER_P();
            auto options = p.options->get();
            options.enabled = value;
            if (p.options->setIfChanged(options))
            {
                _configUpdate();
            }
        }

        void ColorConfigModel::setConfig(const std::string& fileName)
        {
            TLRENDER_P();
            const bool changed = fileName != p.options->get().fileName;
#if defined(TLRENDER_OCIO)
            if (changed)
            {
                try
                {
                    p.ocioConfig.reset();
                    p.ocioConfig = OCIO::Config::CreateFromFile(fileName.c_str());
                }
                catch (const std::exception&)
                {
                }
            }
#endif // TLRENDER_OCIO
            timeline::ColorConfigOptions options;
            options.enabled = true;
            options.fileName = fileName;
#if defined(TLRENDER_OCIO)
            if (changed && p.ocioConfig)
            {
                const char* display = p.ocioConfig->getDefaultDisplay();
                options.display = display;
                options.view = p.ocioConfig->getDefaultView(display);
            }
#endif // TLRENDER_OCIO
            if (p.options->setIfChanged(options))
            {
                _configUpdate();
            }
        }

        std::shared_ptr<observer::IValue<ColorConfigModelData> > ColorConfigModel::observeData() const
        {
            return _p->data;
        }

        void ColorConfigModel::setInputIndex(size_t value)
        {
            TLRENDER_P();
            const auto& inputs = p.data->get().inputs;
            if (value >= 0 && value < inputs.size())
            {
                auto options = p.options->get();
                options.enabled = true;
                options.input = value > 0 ? inputs[value] : std::string();
                if (p.options->setIfChanged(options))
                {
                    _configUpdate();
                }
            }
        }

        void ColorConfigModel::setDisplayIndex(size_t value)
        {
            TLRENDER_P();
            const auto& displays = p.data->get().displays;
            if (value >= 0 && value < displays.size())
            {
                auto options = p.options->get();
                options.enabled = true;
                options.display = value > 0 ? displays[value] : std::string();
                if (p.options->setIfChanged(options))
                {
                    _configUpdate();
                }
            }
        }

        void ColorConfigModel::setViewIndex(size_t value)
        {
            TLRENDER_P();
            const auto& views = p.data->get().views;
            if (value >= 0 && value < views.size())
            {
                auto options = p.options->get();
                options.enabled = true;
                options.view = value > 0 ? views[value] : std::string();
                if (p.options->setIfChanged(options))
                {
                    _configUpdate();
                }
            }
        }

        void ColorConfigModel::_configUpdate()
        {
            TLRENDER_P();
            ColorConfigModelData data;
            const auto& options = p.options->get();
            data.enabled = options.enabled;
            data.fileName = options.fileName;
#if defined(TLRENDER_OCIO)
            if (p.ocioConfig)
            {
                data.inputs.push_back("None");
                for (int i = 0; i < p.ocioConfig->getNumColorSpaces(); ++i)
                {
                    data.inputs.push_back(p.ocioConfig->getColorSpaceNameByIndex(i));
                }
                auto j = std::find(data.inputs.begin(), data.inputs.end(), options.input);
                if (j != data.inputs.end())
                {
                    data.inputIndex = j - data.inputs.begin();
                }

                data.displays.push_back("None");
                for (int i = 0; i < p.ocioConfig->getNumDisplays(); ++i)
                {
                    data.displays.push_back(p.ocioConfig->getDisplay(i));
                }
                j = std::find(data.displays.begin(), data.displays.end(), options.display);
                if (j != data.displays.end())
                {
                    data.displayIndex = j - data.displays.begin();
                }

                data.views.push_back("None");
                const std::string display = p.options->get().display;
                for (int i = 0; i < p.ocioConfig->getNumViews(display.c_str()); ++i)
                {
                    data.views.push_back(p.ocioConfig->getView(display.c_str(), i));
                }
                j = std::find(data.views.begin(), data.views.end(), options.view);
                if (j != data.views.end())
                {
                    data.viewIndex = j - data.views.begin();
                }
            }
#endif // TLRENDER_OCIO
            p.data->setIfChanged(data);
        }
    }
}
