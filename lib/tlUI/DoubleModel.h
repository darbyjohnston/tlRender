// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <dtk/core/ObservableValue.h>
#include <dtk/core/Range.h>

namespace dtk
{
    class Context;
}

namespace tl
{
    namespace ui
    {
        //! Double precision floating point value model.
        class DoubleModel : public std::enable_shared_from_this<DoubleModel>
        {
            DTK_NON_COPYABLE(DoubleModel);

            void _init(const std::shared_ptr<dtk::Context>&);

        protected:
            DoubleModel();

        public:
            ~DoubleModel();

            //! Create a new model.
            static std::shared_ptr<DoubleModel> create(
                const std::shared_ptr<dtk::Context>&);

            //! \name Value
            ///@{

            //! Get the value.
            double getValue() const;

            //! Set the value.
            void setValue(double);

            //! Observe the value.
            std::shared_ptr<dtk::IObservableValue<double> > observeValue() const;

            ///@}

            //! \name Range
            ///@{

            //! Get the range.
            const dtk::RangeD& getRange() const;

            //! Set the range.
            void setRange(const dtk::RangeD&);

            //! Observe the range.
            std::shared_ptr<dtk::IObservableValue<dtk::RangeD> > observeRange() const;

            ///@}

            //! \name Increment
            ///@{

            //! Get the increment step.
            double getStep() const;

            //! Set the increment step.
            void setStep(double);

            //! Increment the value by a step.
            void incrementStep();

            //! Decrement the value by a step.
            void decrementStep();

            //! Get the increment large step.
            double getLargeStep() const;

            //! Set the increment large step.
            void setLargeStep(double);

            //! Increment the value by a large step.
            void incrementLargeStep();

            //! Decrement the value by a large step.
            void decrementLargeStep();

            ///@}

            //! \name Default Value
            ///@{

            //! Get whether there is a default value.
            bool hasDefaultValue() const;

            //! Observe the default value.
            std::shared_ptr<dtk::IObservableValue<bool> > observeHasDefaultValue() const;

            //! Get the default value.
            double getDefaultValue() const;

            //! Set the default value.
            void setDefaultValue(double);

            //! Set the value to the default value.
            void setDefaultValue();

            //! Clear the default value.
            void clearDefaultValue();

            ///@}

        private:
            DTK_PRIVATE();
        };
    }
}
