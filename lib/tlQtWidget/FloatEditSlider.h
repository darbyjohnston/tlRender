// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <dtk/core/Range.h>

#include <QWidget>

#include <memory>

namespace tl
{
    namespace qtwidget
    {
        //! Floating point value editor and slider.
        class FloatEditSlider : public QWidget
        {
            Q_OBJECT
            Q_PROPERTY(
                dtk::RangeF range
                READ range
                WRITE setRange
                NOTIFY rangeChanged)
            Q_PROPERTY(
                float value
                READ value
                WRITE setValue
                NOTIFY valueChanged)
            Q_PROPERTY(
                float defaultValue
                READ defaultValue
                WRITE setDefaultValue)
            Q_PROPERTY(
                float singleStep
                READ singleStep
                WRITE setSingleStep)
            Q_PROPERTY(
                float pageStep
                READ pageStep
                WRITE setPageStep)
            Q_PROPERTY(
                Qt::Orientation orientation
                READ orientation
                WRITE setOrientation)

        public:
            FloatEditSlider(
                Qt::Orientation = Qt::Horizontal,
                QWidget* parent = nullptr);

            virtual ~FloatEditSlider();

            //! Get the range.
            const dtk::RangeF& range() const;

            //! Get the value.
            float value() const;

            //! Get the default value.
            float defaultValue() const;

            //! Get the single step.
            float singleStep() const;

            //! Get the path step.
            float pageStep() const;

            //! Get the orientation.
            Qt::Orientation orientation() const;

        public slots:
            //! Set the range.
            void setRange(const dtk::RangeF&);

            //! Set the value.
            void setValue(float);

            //! Set the default value.
            void setDefaultValue(float);

            //! Set the single step.
            void setSingleStep(float);

            //! Set the page step.
            void setPageStep(float);

            //! Set the orientation.
            void setOrientation(Qt::Orientation);

        Q_SIGNALS:
            //! This signal is emitted when the range is changed.
            void rangeChanged(const dtk::RangeF&);

            //! This signal is emitted when the value is changed.
            void valueChanged(float);

        private:
            void _layoutUpdate();
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    }
}
