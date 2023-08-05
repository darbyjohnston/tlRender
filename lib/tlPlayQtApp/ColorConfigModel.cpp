// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/ColorConfigModel.h>

#include <tlCore/OS.h>

#include <QApplication>
#include <QPalette>

#if defined(TLRENDER_OCIO)
#include <OpenColorIO/OpenColorIO.h>
#endif // TLRENDER_OCIO

#if defined(TLRENDER_OCIO)
namespace OCIO = OCIO_NAMESPACE;
#endif // TLRENDER_OCIO

namespace tl
{
    namespace play_qt
    {
        bool ColorConfigModelData::operator == (const ColorConfigModelData& other) const
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

        struct ColorConfigModel::Private
        {
            std::weak_ptr<system::Context> context;
#if defined(TLRENDER_OCIO)
            OCIO_NAMESPACE::ConstConfigRcPtr ocioConfig;
#endif // TLRENDER_OCIO
            std::shared_ptr<observer::Value<timeline::ColorConfigOptions> > configOptions;
            std::shared_ptr<observer::Value<ColorConfigModelData> > data;
        };

        void ColorConfigModel::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.context = context;

            p.configOptions = observer::Value<timeline::ColorConfigOptions>::create();
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
                        timeline::ColorConfigOptions configOptions;
                        configOptions.fileName = env;
                        const char* display = p.ocioConfig->getDefaultDisplay();
                        configOptions.display = display;
                        configOptions.view = p.ocioConfig->getDefaultView(display);
                        p.configOptions->setIfChanged(configOptions);
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
            return _p->configOptions;
        }

        void ColorConfigModel::setConfigOptions(const timeline::ColorConfigOptions& value)
        {
            TLRENDER_P();
#if defined(TLRENDER_OCIO)
            try
            {
                p.ocioConfig.reset();
                p.ocioConfig = OCIO::Config::CreateFromFile(value.fileName.c_str());
            }
            catch (const std::exception& e)
            {}
#endif // TLRENDER_OCIO
            p.configOptions->setIfChanged(value);
            _configUpdate();
        }

        void ColorConfigModel::setConfig(const std::string& fileName)
        {
            TLRENDER_P();
#if defined(TLRENDER_OCIO)
            try
            {
                p.ocioConfig.reset();
                p.ocioConfig = OCIO::Config::CreateFromFile(fileName.c_str());
            }
            catch (const std::exception&)
            {}
#endif // TLRENDER_OCIO
            timeline::ColorConfigOptions configOptions;
            configOptions.fileName = fileName;
#if defined(TLRENDER_OCIO)
            if (p.ocioConfig)
            {
                const char* display = p.ocioConfig->getDefaultDisplay();
                configOptions.display = display;
                configOptions.view = p.ocioConfig->getDefaultView(display);
            }
#endif // TLRENDER_OCIO
            p.configOptions->setIfChanged(configOptions);
            _configUpdate();
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
                timeline::ColorConfigOptions configOptions = p.configOptions->get();
                configOptions.input = value > 0 ? inputs[value] : std::string();
                p.configOptions->setIfChanged(configOptions);
                _configUpdate();
            }
        }

        void ColorConfigModel::setDisplayIndex(size_t value)
        {
            TLRENDER_P();
            const auto& displays = p.data->get().displays;
            if (value >= 0 && value < displays.size())
            {
                timeline::ColorConfigOptions configOptions = p.configOptions->get();
                configOptions.display = value > 0 ? displays[value] : std::string();
                p.configOptions->setIfChanged(configOptions);
                _configUpdate();
            }
        }

        void ColorConfigModel::setViewIndex(size_t value)
        {
            TLRENDER_P();
            const auto& views = p.data->get().views;
            if (value >= 0 && value < views.size())
            {
                timeline::ColorConfigOptions configOptions = p.configOptions->get();
                configOptions.view = value > 0 ? views[value] : std::string();
                p.configOptions->setIfChanged(configOptions);
                _configUpdate();
            }
        }

        void ColorConfigModel::_configUpdate()
        {
            TLRENDER_P();
            ColorConfigModelData data;
            const auto& configOptions = p.configOptions->get();
            data.fileName = configOptions.fileName;
#if defined(TLRENDER_OCIO)
            if (p.ocioConfig)
            {
                data.inputs.push_back("None");
                for (int i = 0; i < p.ocioConfig->getNumColorSpaces(); ++i)
                {
                    data.inputs.push_back(p.ocioConfig->getColorSpaceNameByIndex(i));
                }
                auto j = std::find(data.inputs.begin(), data.inputs.end(), configOptions.input);
                if (j != data.inputs.end())
                {
                    data.inputIndex = j - data.inputs.begin();
                }

                data.displays.push_back("None");
                for (int i = 0; i < p.ocioConfig->getNumDisplays(); ++i)
                {
                    data.displays.push_back(p.ocioConfig->getDisplay(i));
                }
                j = std::find(data.displays.begin(), data.displays.end(), configOptions.display);
                if (j != data.displays.end())
                {
                    data.displayIndex = j - data.displays.begin();
                }

                data.views.push_back("None");
                const std::string display = p.configOptions->get().display;
                for (int i = 0; i < p.ocioConfig->getNumViews(display.c_str()); ++i)
                {
                    data.views.push_back(p.ocioConfig->getView(display.c_str(), i));
                }
                j = std::find(data.views.begin(), data.views.end(), configOptions.view);
                if (j != data.views.end())
                {
                    data.viewIndex = j - data.views.begin();
                }
            }
#endif // TLRENDER_OCIO
            p.data->setIfChanged(data);
        }

        struct ColorInputListModel::Private
        {
            std::vector<std::string> inputs;
            size_t inputIndex = 0;
            std::shared_ptr<observer::ValueObserver<ColorConfigModelData> > dataObserver;
        };

        ColorInputListModel::ColorInputListModel(
            const std::shared_ptr<ColorConfigModel>& colorConfigModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver = observer::ValueObserver<ColorConfigModelData>::create(
                colorConfigModel->observeData(),
                [this](const ColorConfigModelData& value)
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
            std::shared_ptr<observer::ValueObserver<ColorConfigModelData> > dataObserver;
        };

        ColorDisplayListModel::ColorDisplayListModel(
            const std::shared_ptr<ColorConfigModel>& colorConfigModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver = observer::ValueObserver<ColorConfigModelData>::create(
                colorConfigModel->observeData(),
                [this](const ColorConfigModelData& value)
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
            std::shared_ptr<observer::ValueObserver<ColorConfigModelData> > dataObserver;
        };

        ColorViewListModel::ColorViewListModel(
            const std::shared_ptr<ColorConfigModel>& colorConfigModel,
            QObject* parent) :
            QAbstractListModel(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.dataObserver = observer::ValueObserver<ColorConfigModelData>::create(
                colorConfigModel->observeData(),
                [this](const ColorConfigModelData& value)
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
