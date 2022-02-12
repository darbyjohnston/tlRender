// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include "ColorModel.h"

#include <tlCore/OS.h>

#include <QApplication>
#include <QPalette>

namespace OCIO = OCIO_NAMESPACE;

namespace tl
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
        try
        {
            _ocioConfig = OCIO::Config::CreateFromFile(value.fileName.c_str());
            if (_ocioConfig)
            {
                _config->setIfChanged(value);
                _configUpdate();
            }
        }
        catch (const std::exception& e)
        {
            if (const auto context = _context.lock())
            {
                context->log(std::string(), e.what(), core::LogType::Error);
            }
        }
    }

    void ColorModel::setConfig(const std::string& fileName)
    {
        try
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
        catch (const std::exception& e)
        {
            if (const auto context = _context.lock())
            {
                context->log(std::string(), e.what(), core::LogType::Error);
            }
        }
    }

    std::shared_ptr<observer::IValue<ColorModelData> > ColorModel::observeData() const
    {
        return _data;
    }

    void ColorModel::setInputIndex(size_t value)
    {
        const auto& inputs = _data->get().inputs;
        if (value >= 0 && value < inputs.size())
        {
            imaging::ColorConfig config = _config->get();
            config.input = value > 0 ? inputs[value] : std::string();
            _config->setIfChanged(config);
            _configUpdate();
        }
    }

    void ColorModel::setDisplayIndex(size_t value)
    {
        const auto& displays = _data->get().displays;
        if (value >= 0 && value < displays.size())
        {
            imaging::ColorConfig config = _config->get();
            config.display = value > 0 ? displays[value] : std::string();
            _config->setIfChanged(config);
            _configUpdate();
        }
    }

    void ColorModel::setViewIndex(size_t value)
    {
        const auto& views = _data->get().views;
        if (value >= 0 && value < views.size())
        {
            imaging::ColorConfig config = _config->get();
            config.view = value > 0 ? views[value] : std::string();
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
            data.inputs.push_back("None");
            for (int i = 0; i < _ocioConfig->getNumColorSpaces(); ++i)
            {
                data.inputs.push_back(_ocioConfig->getColorSpaceNameByIndex(i));
            }
            auto j = std::find(data.inputs.begin(), data.inputs.end(), config.input);
            if (j != data.inputs.end())
            {
                data.inputIndex = j - data.inputs.begin();
            }

            data.displays.push_back("None");
            for (int i = 0; i < _ocioConfig->getNumDisplays(); ++i)
            {
                data.displays.push_back(_ocioConfig->getDisplay(i));
            }
            j = std::find(data.displays.begin(), data.displays.end(), config.display);
            if (j != data.displays.end())
            {
                data.displayIndex = j - data.displays.begin();
            }

            data.views.push_back("None");
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

    ColorInputListModel::ColorInputListModel(
        const std::shared_ptr<ColorModel>& colorModel,
        QObject* parent) :
        QAbstractListModel(parent)
    {
        _dataObserver = observer::ValueObserver<ColorModelData>::create(
            colorModel->observeData(),
            [this](const ColorModelData& value)
            {
                beginResetModel();
                _inputs = value.inputs;
                _inputIndex = value.inputIndex;
                endResetModel();
            });
    }

    int ColorInputListModel::rowCount(const QModelIndex& parent) const
    {
        return _inputs.size();
    }

    QVariant ColorInputListModel::data(const QModelIndex& index, int role) const
    {
        QVariant out;
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _inputs.size() &&
            index.column() >= 0 &&
            index.column() < 2)
        {
            switch (role)
            {
            case Qt::DisplayRole:
                out.setValue(QString::fromUtf8(_inputs[index.row()].c_str()));
                break;
            case Qt::BackgroundRole:
                if (index.row() == _inputIndex)
                {
                    out.setValue(
                        QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                }
                break;
            case Qt::ForegroundRole:
                if (index.row() == _inputIndex)
                {
                    out.setValue(
                        QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                }
                break;
            default: break;
            }
        }
        return out;
    }

    ColorDisplayListModel::ColorDisplayListModel(
        const std::shared_ptr<ColorModel>& colorModel,
        QObject* parent) :
        QAbstractListModel(parent)
    {
        _dataObserver = observer::ValueObserver<ColorModelData>::create(
            colorModel->observeData(),
            [this](const ColorModelData& value)
            {
                beginResetModel();
                _displays = value.displays;
                _displayIndex = value.displayIndex;
                endResetModel();
            });
    }

    int ColorDisplayListModel::rowCount(const QModelIndex& parent) const
    {
        return _displays.size();
    }

    QVariant ColorDisplayListModel::data(const QModelIndex& index, int role) const
    {
        QVariant out;
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _displays.size() &&
            index.column() >= 0 &&
            index.column() < 2)
        {
            switch (role)
            {
            case Qt::DisplayRole:
                out.setValue(QString::fromUtf8(_displays[index.row()].c_str()));
                break;
            case Qt::BackgroundRole:
                if (index.row() == _displayIndex)
                {
                    out.setValue(
                        QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                }
                break;
            case Qt::ForegroundRole:
                if (index.row() == _displayIndex)
                {
                    out.setValue(
                        QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                }
                break;
            default: break;
            }
        }
        return out;
    }

    ColorViewListModel::ColorViewListModel(
        const std::shared_ptr<ColorModel>& colorModel,
        QObject* parent) :
        QAbstractListModel(parent)
    {
        _dataObserver = observer::ValueObserver<ColorModelData>::create(
            colorModel->observeData(),
            [this](const ColorModelData& value)
            {
                beginResetModel();
                _views = value.views;
                _viewIndex = value.viewIndex;
                endResetModel();
            });
    }

    int ColorViewListModel::rowCount(const QModelIndex& parent) const
    {
        return _views.size();
    }

    QVariant ColorViewListModel::data(const QModelIndex& index, int role) const
    {
        QVariant out;
        if (index.isValid() &&
            index.row() >= 0 &&
            index.row() < _views.size() &&
            index.column() >= 0 &&
            index.column() < 2)
        {
            switch (role)
            {
            case Qt::DisplayRole:
                out.setValue(QString::fromUtf8(_views[index.row()].c_str()));
                break;
            case Qt::BackgroundRole:
                if (index.row() == _viewIndex)
                {
                    out.setValue(
                        QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                }
                break;
            case Qt::ForegroundRole:
                if (index.row() == _viewIndex)
                {
                    out.setValue(
                        QBrush(qApp->palette().color(QPalette::ColorRole::HighlightedText)));
                }
                break;
            default: break;
            }
        }
        return out;
    }
}
