// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "ColorModel.h"

#include <tlrCore/OS.h>

namespace OCIO = OCIO_NAMESPACE;

namespace tlr
{
    bool ColorModelData::operator == (const ColorModelData& other) const
    {
        return
            fileName == other.fileName &&
            inputs == other.inputs &&
            inputIndex == other.inputIndex &&
            displays == other.displays &&
            displayIndex == other.displayIndex &&
            views == other.views &&
            viewIndex == other.viewIndex;
    }

    void ColorModel::_init(const std::shared_ptr<core::Context>& context)
    {
        _context = context;

        _config = observer::Value<imaging::ColorConfig>::create();
        _data = observer::Value<ColorModelData>::create();
        
        std::string env;
        if (os::getEnv("OCIO", env) && !env.empty())
        {
            _ocioConfig = OCIO::Config::CreateFromEnv();
            if (_ocioConfig)
            {
                imaging::ColorConfig config;
                config.fileName = env;
                const char* display = _ocioConfig->getDefaultDisplay();
                config.display = display;
                config.view = _ocioConfig->getDefaultView(display);
                _config->setIfChanged(config);
                _configUpdate();
            }
        }
    }

    ColorModel::ColorModel()
    {}

    ColorModel::~ColorModel()
    {}

    std::shared_ptr<ColorModel> ColorModel::create(const std::shared_ptr<core::Context>& context)
    {
        auto out = std::shared_ptr<ColorModel>(new ColorModel);
        out->_init(context);
        return out;
    }

    std::shared_ptr<observer::IValue<imaging::ColorConfig> > ColorModel::observeConfig() const
    {
        return _config;
    }

    void ColorModel::setConfig(const imaging::ColorConfig& value)
    {
        _ocioConfig = OCIO::Config::CreateFromFile(value.fileName.c_str());
        if (_ocioConfig)
        {
            _config->setIfChanged(value);
            _configUpdate();
        }
    }

    void ColorModel::setConfig(const std::string& fileName)
    {
        _ocioConfig = OCIO::Config::CreateFromFile(fileName.c_str());
        if (_ocioConfig)
        {
            imaging::ColorConfig config;
            config.fileName = fileName;
            const char* display = _ocioConfig->getDefaultDisplay();
            config.display = display;
            config.view = _ocioConfig->getDefaultView(display);
            _config->setIfChanged(config);
            _configUpdate();
        }
    }

    std::shared_ptr<observer::IValue<ColorModelData> > ColorModel::observeData() const
    {
        return _data;
    }

    void ColorModel::setInputIndex(int value)
    {
        const auto& inputs = _data->get().inputs;
        if (value >= 0 && value < inputs.size())
        {
            imaging::ColorConfig config = _config->get();
            config.input = inputs[value];
            _config->setIfChanged(config);
            _configUpdate();
        }
    }

    void ColorModel::setDisplayIndex(int value)
    {
        const auto& displays = _data->get().displays;
        if (value >= 0 && value < displays.size())
        {
            imaging::ColorConfig config = _config->get();
            config.display = displays[value];
            _config->setIfChanged(config);
            _configUpdate();
        }
    }

    void ColorModel::setViewIndex(int value)
    {
        const auto& views = _data->get().views;
        if (value >= 0 && value < views.size())
        {
            imaging::ColorConfig config = _config->get();
            config.view = views[value];
            _config->setIfChanged(config);
            _configUpdate();
        }
    }

    void ColorModel::_configUpdate()
    {
        ColorModelData data;
        const auto& config = _config->get();
        data.fileName = config.fileName;
        if (_ocioConfig)
        {
            for (int i = 0; i < _ocioConfig->getNumColorSpaces(); ++i)
            {
                data.inputs.push_back(_ocioConfig->getColorSpaceNameByIndex(i));
            }
            auto j = std::find(data.inputs.begin(), data.inputs.end(), config.input);
            if (j != data.inputs.end())
            {
                data.inputIndex = j - data.inputs.begin();
            }

            for (int i = 0; i < _ocioConfig->getNumDisplays(); ++i)
            {
                data.displays.push_back(_ocioConfig->getDisplay(i));
            }
            j = std::find(data.displays.begin(), data.displays.end(), config.display);
            if (j != data.displays.end())
            {
                data.displayIndex = j - data.displays.begin();
            }

            const std::string display = _config->get().display;
            for (int i = 0; i < _ocioConfig->getNumViews(display.c_str()); ++i)
            {
                data.views.push_back(_ocioConfig->getView(display.c_str(), i));
            }
            j = std::find(data.views.begin(), data.views.end(), config.view);
            if (j != data.views.end())
            {
                data.viewIndex = j - data.views.begin();
            }
        }
        _data->setIfChanged(data);
    }
}
