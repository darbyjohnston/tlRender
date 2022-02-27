// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlAppPlay/ColorModel.h>

#include <tlCore/OS.h>

#include <QApplication>
#include <QPalette>

#include <OpenColorIO/OpenColorIO.h>

namespace OCIO = OCIO_NAMESPACE;

using namespace tl::core;

namespace tl
{
    namespace app
    {
        namespace play
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

            struct ColorModel::Private
            {
                std::weak_ptr<Context> context;
                OCIO_NAMESPACE::ConstConfigRcPtr ocioConfig;
                std::shared_ptr<observer::Value<imaging::ColorConfig> > config;
                std::shared_ptr<observer::Value<ColorModelData> > data;
            };

            void ColorModel::_init(const std::shared_ptr<Context>& context)
            {
                TLRENDER_P();

                p.context = context;

                p.config = observer::Value<imaging::ColorConfig>::create();
                p.data = observer::Value<ColorModelData>::create();

                std::string env;
                if (os::getEnv("OCIO", env) && !env.empty())
                {
                    p.ocioConfig = OCIO::Config::CreateFromEnv();
                    if (p.ocioConfig)
                    {
                        imaging::ColorConfig config;
                        config.fileName = env;
                        const char* display = p.ocioConfig->getDefaultDisplay();
                        config.display = display;
                        config.view = p.ocioConfig->getDefaultView(display);
                        p.config->setIfChanged(config);
                        _configUpdate();
                    }
                }
            }

            ColorModel::ColorModel() :
                _p(new Private)
            {}

            ColorModel::~ColorModel()
            {}

            std::shared_ptr<ColorModel> ColorModel::create(const std::shared_ptr<Context>& context)
            {
                auto out = std::shared_ptr<ColorModel>(new ColorModel);
                out->_init(context);
                return out;
            }

            std::shared_ptr<observer::IValue<imaging::ColorConfig> > ColorModel::observeConfig() const
            {
                return _p->config;
            }

            void ColorModel::setConfig(const imaging::ColorConfig& value)
            {
                TLRENDER_P();
                try
                {
                    p.ocioConfig = OCIO::Config::CreateFromFile(value.fileName.c_str());
                    if (p.ocioConfig)
                    {
                        p.config->setIfChanged(value);
                        _configUpdate();
                    }
                }
                catch (const std::exception& e)
                {
                    if (const auto context = p.context.lock())
                    {
                        context->log(std::string(), e.what(), LogType::Error);
                    }
                }
            }

            void ColorModel::setConfig(const std::string& fileName)
            {
                TLRENDER_P();
                try
                {
                    p.ocioConfig = OCIO::Config::CreateFromFile(fileName.c_str());
                    if (p.ocioConfig)
                    {
                        imaging::ColorConfig config;
                        config.fileName = fileName;
                        const char* display = p.ocioConfig->getDefaultDisplay();
                        config.display = display;
                        config.view = p.ocioConfig->getDefaultView(display);
                        p.config->setIfChanged(config);
                        _configUpdate();
                    }
                }
                catch (const std::exception& e)
                {
                    if (const auto context = p.context.lock())
                    {
                        context->log(std::string(), e.what(), LogType::Error);
                    }
                }
            }

            std::shared_ptr<observer::IValue<ColorModelData> > ColorModel::observeData() const
            {
                return _p->data;
            }

            void ColorModel::setInputIndex(size_t value)
            {
                TLRENDER_P();
                const auto& inputs = p.data->get().inputs;
                if (value >= 0 && value < inputs.size())
                {
                    imaging::ColorConfig config = p.config->get();
                    config.input = value > 0 ? inputs[value] : std::string();
                    p.config->setIfChanged(config);
                    _configUpdate();
                }
            }

            void ColorModel::setDisplayIndex(size_t value)
            {
                TLRENDER_P();
                const auto& displays = p.data->get().displays;
                if (value >= 0 && value < displays.size())
                {
                    imaging::ColorConfig config = p.config->get();
                    config.display = value > 0 ? displays[value] : std::string();
                    p.config->setIfChanged(config);
                    _configUpdate();
                }
            }

            void ColorModel::setViewIndex(size_t value)
            {
                TLRENDER_P();
                const auto& views = p.data->get().views;
                if (value >= 0 && value < views.size())
                {
                    imaging::ColorConfig config = p.config->get();
                    config.view = value > 0 ? views[value] : std::string();
                    p.config->setIfChanged(config);
                    _configUpdate();
                }
            }

            void ColorModel::_configUpdate()
            {
                TLRENDER_P();
                ColorModelData data;
                const auto& config = p.config->get();
                data.fileName = config.fileName;
                if (p.ocioConfig)
                {
                    data.inputs.push_back("None");
                    for (int i = 0; i < p.ocioConfig->getNumColorSpaces(); ++i)
                    {
                        data.inputs.push_back(p.ocioConfig->getColorSpaceNameByIndex(i));
                    }
                    auto j = std::find(data.inputs.begin(), data.inputs.end(), config.input);
                    if (j != data.inputs.end())
                    {
                        data.inputIndex = j - data.inputs.begin();
                    }

                    data.displays.push_back("None");
                    for (int i = 0; i < p.ocioConfig->getNumDisplays(); ++i)
                    {
                        data.displays.push_back(p.ocioConfig->getDisplay(i));
                    }
                    j = std::find(data.displays.begin(), data.displays.end(), config.display);
                    if (j != data.displays.end())
                    {
                        data.displayIndex = j - data.displays.begin();
                    }

                    data.views.push_back("None");
                    const std::string display = p.config->get().display;
                    for (int i = 0; i < p.ocioConfig->getNumViews(display.c_str()); ++i)
                    {
                        data.views.push_back(p.ocioConfig->getView(display.c_str(), i));
                    }
                    j = std::find(data.views.begin(), data.views.end(), config.view);
                    if (j != data.views.end())
                    {
                        data.viewIndex = j - data.views.begin();
                    }
                }
                p.data->setIfChanged(data);
            }

            struct ColorInputListModel::Private
            {
                std::vector<std::string> inputs;
                size_t inputIndex = 0;
                std::shared_ptr<observer::ValueObserver<ColorModelData> > dataObserver;
            };

            ColorInputListModel::ColorInputListModel(
                const std::shared_ptr<ColorModel>& colorModel,
                QObject* parent) :
                QAbstractListModel(parent),
                _p(new Private)
            {
                TLRENDER_P();

                p.dataObserver = observer::ValueObserver<ColorModelData>::create(
                    colorModel->observeData(),
                    [this](const ColorModelData& value)
                    {
                        beginResetModel();
                        _p->inputs = value.inputs;
                        _p->inputIndex = value.inputIndex;
                        endResetModel();
                    });
            }

            ColorInputListModel::~ColorInputListModel()
            {}

            int ColorInputListModel::rowCount(const QModelIndex& parent) const
            {
                return _p->inputs.size();
            }

            QVariant ColorInputListModel::data(const QModelIndex& index, int role) const
            {
                TLRENDER_P();
                QVariant out;
                if (index.isValid() &&
                    index.row() >= 0 &&
                    index.row() < p.inputs.size() &&
                    index.column() >= 0 &&
                    index.column() < 2)
                {
                    switch (role)
                    {
                    case Qt::DisplayRole:
                        out.setValue(QString::fromUtf8(p.inputs[index.row()].c_str()));
                        break;
                    case Qt::BackgroundRole:
                        if (index.row() == p.inputIndex)
                        {
                            out.setValue(
                                QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                        }
                        break;
                    case Qt::ForegroundRole:
                        if (index.row() == p.inputIndex)
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

            struct ColorDisplayListModel::Private
            {
                std::vector<std::string> displays;
                size_t displayIndex = 0;
                std::shared_ptr<observer::ValueObserver<ColorModelData> > dataObserver;
            };

            ColorDisplayListModel::ColorDisplayListModel(
                const std::shared_ptr<ColorModel>& colorModel,
                QObject* parent) :
                QAbstractListModel(parent),
                _p(new Private)
            {
                TLRENDER_P();

                p.dataObserver = observer::ValueObserver<ColorModelData>::create(
                    colorModel->observeData(),
                    [this](const ColorModelData& value)
                    {
                        beginResetModel();
                        _p->displays = value.displays;
                        _p->displayIndex = value.displayIndex;
                        endResetModel();
                    });
            }

            ColorDisplayListModel::~ColorDisplayListModel()
            {}

            int ColorDisplayListModel::rowCount(const QModelIndex& parent) const
            {
                return _p->displays.size();
            }

            QVariant ColorDisplayListModel::data(const QModelIndex& index, int role) const
            {
                TLRENDER_P();
                QVariant out;
                if (index.isValid() &&
                    index.row() >= 0 &&
                    index.row() < p.displays.size() &&
                    index.column() >= 0 &&
                    index.column() < 2)
                {
                    switch (role)
                    {
                    case Qt::DisplayRole:
                        out.setValue(QString::fromUtf8(p.displays[index.row()].c_str()));
                        break;
                    case Qt::BackgroundRole:
                        if (index.row() == p.displayIndex)
                        {
                            out.setValue(
                                QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                        }
                        break;
                    case Qt::ForegroundRole:
                        if (index.row() == p.displayIndex)
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

            struct ColorViewListModel::Private
            {
                std::vector<std::string> views;
                size_t viewIndex = 0;
                std::shared_ptr<observer::ValueObserver<ColorModelData> > dataObserver;
            };

            ColorViewListModel::ColorViewListModel(
                const std::shared_ptr<ColorModel>& colorModel,
                QObject* parent) :
                QAbstractListModel(parent),
                _p(new Private)
            {
                TLRENDER_P();

                p.dataObserver = observer::ValueObserver<ColorModelData>::create(
                    colorModel->observeData(),
                    [this](const ColorModelData& value)
                    {
                        beginResetModel();
                        _p->views = value.views;
                        _p->viewIndex = value.viewIndex;
                        endResetModel();
                    });
            }

            ColorViewListModel::~ColorViewListModel()
            {}

            int ColorViewListModel::rowCount(const QModelIndex& parent) const
            {
                return _p->views.size();
            }

            QVariant ColorViewListModel::data(const QModelIndex& index, int role) const
            {
                TLRENDER_P();
                QVariant out;
                if (index.isValid() &&
                    index.row() >= 0 &&
                    index.row() < p.views.size() &&
                    index.column() >= 0 &&
                    index.column() < 2)
                {
                    switch (role)
                    {
                    case Qt::DisplayRole:
                        out.setValue(QString::fromUtf8(p.views[index.row()].c_str()));
                        break;
                    case Qt::BackgroundRole:
                        if (index.row() == p.viewIndex)
                        {
                            out.setValue(
                                QBrush(qApp->palette().color(QPalette::ColorRole::Highlight)));
                        }
                        break;
                    case Qt::ForegroundRole:
                        if (index.row() == p.viewIndex)
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
    }
}
