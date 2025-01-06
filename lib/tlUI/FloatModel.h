// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>
#include <tlCore/Range.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace ui
    {
        //! Floating point value model.
        class FloatModel : public std::enable_shared_from_this<FloatModel>
        {
            TLRENDER_NON_COPYABLE(FloatModel);

            void _init(const std::shared_ptr<system::Context>&);

        protected:
            FloatModel();

        public:
            ~FloatModel();

            //! Create a new model.
            static std::shared_ptr<FloatModel> create(
                const std::shared_ptr<system::Context>&);

            //! \name Value
            ///@{

            //! Get the value.
            float getValue() const;

            //! Set the value.
            void setValue(float);

            //! Observe the value.
            std::shared_ptr<observer::IValue<float> > observeValue() const;

            ///@}

            //! \name Range
            ///@{

            //! Get the range.
            const math::FloatRange& getRange() const;

            //! Set the range.
            void setRange(const math::FloatRange&);

            //! Observe the range.
            std::shared_ptr<observer::IValue<math::FloatRange> > observeRange() const;

            ///@}

            //! \name Increment
            ///@{

            //! Get the increment step.
            float getStep() const;

            //! Set the increment step.
            void setStep(float);

            //! Increment the value by a step.
            void incrementStep();

            //! Decrement the value by a step.
            void decrementStep();

            //! Get the increment large step.
            float getLargeStep() const;

            //! Set the increment large step.
            void setLargeStep(float);

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
            std::shared_ptr<observer::IValue<bool> > observeHasDefaultValue() const;

            //! Get the default value.
            float getDefaultValue() const;

            //! Set the default value.
            void setDefaultValue(float);

            //! Set the value to the default value.
            void setDefaultValue();

            //! Clear the default value.
            void clearDefaultValue();

            ///@}

        private:
            TLRENDER_PRIVATE();
        };
    }
}
