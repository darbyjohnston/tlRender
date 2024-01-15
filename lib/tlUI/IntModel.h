// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>
#include <tlCore/Range.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace ui
    {
        //! Integer value model.
        class IntModel : public std::enable_shared_from_this<IntModel>
        {
            TLRENDER_NON_COPYABLE(IntModel);

            void _init(const std::shared_ptr<system::Context>&);

        protected:
            IntModel();

        public:
            ~IntModel();

            //! Create a new model.
            static std::shared_ptr<IntModel> create(
                const std::shared_ptr<system::Context>&);

            //! \name Value
            ///@{

            //! Get the value.
            int getValue() const;

            //! Set the value.
            void setValue(int);

            //! Observe the value.
            std::shared_ptr<observer::IValue<int> > observeValue() const;

            ///@}

            //! \name Range
            ///@{

            //! Get the range.
            const math::IntRange& getRange() const;

            //! Set the range.
            void setRange(const math::IntRange&);

            //! Observe the range.
            std::shared_ptr<observer::IValue<math::IntRange> > observeRange() const;

            ///@}

            //! \name Increment
            ///@{

            //! Get the increment step.
            int getStep() const;

            //! Set the increment step.
            void setStep(int);

            //! Increment the value by a step.
            void incrementStep();

            //! Decrement the value by a step.
            void decrementStep();

            //! Get the increment large step.
            int getLargeStep() const;

            //! Set the increment large step.
            void setLargeStep(int);

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
            int getDefaultValue() const;

            //! Set the default value.
            void setDefaultValue(int);

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
