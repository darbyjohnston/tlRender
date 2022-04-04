// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>
#include <tlCore/OCIO.h>
#include <tlCore/ValueObserver.h>

#include <QAbstractListModel>

namespace tl
{
    namespace play
    {
        //! Color model data.
        struct ColorModelData
        {
            std::string fileName;
            std::vector<std::string> inputs;
            size_t inputIndex = 0;
            std::vector<std::string> displays;
            size_t displayIndex = 0;
            std::vector<std::string> views;
            size_t viewIndex = 0;

            bool operator == (const ColorModelData&) const;
        };

        //! Color model.
        class ColorModel : public std::enable_shared_from_this<ColorModel>
        {
            TLRENDER_NON_COPYABLE(ColorModel);

        protected:
            void _init(const std::shared_ptr<system::Context>&);
            ColorModel();

        public:
            ~ColorModel();

            //! Create a new color model.
            static std::shared_ptr<ColorModel> create(const std::shared_ptr<system::Context>&);

            //! Observe the configuration.
            std::shared_ptr<observer::IValue<imaging::ColorConfig> > observeConfig() const;

            //! Set the configuration.
            void setConfig(const imaging::ColorConfig&);

            //! Set the configuration.
            void setConfig(const std::string& fileName);

            //! Observe the model data.
            std::shared_ptr<observer::IValue<ColorModelData> > observeData() const;

            //! Set the input index.
            void setInputIndex(size_t);

            //! Set the display index.
            void setDisplayIndex(size_t);

            //! Set the view index.
            void setViewIndex(size_t);

        private:
            void _configUpdate();

            TLRENDER_PRIVATE();
        };

        //! Color input list model.
        class ColorInputListModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            ColorInputListModel(
                const std::shared_ptr<ColorModel>&,
                QObject* parent = nullptr);

            ~ColorInputListModel() override;

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };

        //! Color display list model.
        class ColorDisplayListModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            ColorDisplayListModel(
                const std::shared_ptr<ColorModel>&,
                QObject* parent = nullptr);

            ~ColorDisplayListModel() override;

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };

        //! Color view list model.
        class ColorViewListModel : public QAbstractListModel
        {
            Q_OBJECT

        public:
            ColorViewListModel(
                const std::shared_ptr<ColorModel>&,
                QObject* parent = nullptr);

            ~ColorViewListModel() override;

            int rowCount(const QModelIndex& parent = QModelIndex()) const override;
            QVariant data(const QModelIndex&, int role = Qt::DisplayRole) const override;

        protected:
            TLRENDER_PRIVATE();
        };
    }
}
