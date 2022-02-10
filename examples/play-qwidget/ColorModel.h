// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Context.h>
#include <tlrCore/ListObserver.h>
#include <tlrCore/OCIO.h>

#include <QAbstractTableModel>

#include <OpenColorIO/OpenColorIO.h>

#include <QAbstractListModel>

namespace tlr
{
    //! Color model data.
    struct ColorModelData
    {
        std::string fileName;
        std::vector<std::string> inputs;
        int inputIndex = -1;
        std::vector<std::string> displays;
        int displayIndex = -1;
        std::vector<std::string> views;
        int viewIndex = -1;

        bool operator == (const ColorModelData&) const;
    };

    //! Color model.
    class ColorModel : public std::enable_shared_from_this<ColorModel>
    {
        TLR_NON_COPYABLE(ColorModel);

    protected:
        void _init(const std::shared_ptr<core::Context>&);
        ColorModel();

    public:
        ~ColorModel();

        //! Create a new color model.
        static std::shared_ptr<ColorModel> create(const std::shared_ptr<core::Context>&);

        //! Observe the configuration.
        std::shared_ptr<observer::IValue<imaging::ColorConfig> > observeConfig() const;

        //! Set the configuration.
        void setConfig(const imaging::ColorConfig&);

        //! Set the configuration.
        void setConfig(const std::string& fileName);

        //! Observe the model data.
        std::shared_ptr<observer::IValue<ColorModelData> > observeData() const;

        //! Set the input index.
        void setInputIndex(int);

        //! Set the display index.
        void setDisplayIndex(int);

        //! Set the view index.
        void setViewIndex(int);

    private:
        void _configUpdate();

        std::weak_ptr<core::Context> _context;
        OCIO_NAMESPACE::ConstConfigRcPtr _ocioConfig;
        std::shared_ptr<observer::Value<imaging::ColorConfig> > _config;
        std::shared_ptr<observer::Value<ColorModelData> > _data;
    };
}
