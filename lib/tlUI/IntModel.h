// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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

            int getValue() const;

            void setValue(int);

            std::shared_ptr<observer::IValue<int> > observeValue() const;

            ///@}

            //! \name Range
            ///@{

            const math::IntRange& getRange() const;

            void setRange(const math::IntRange&);

            std::shared_ptr<observer::IValue<math::IntRange> > observeRange() const;

            ///@}

            //! \name Increment
            ///@{

            int getStep() const;

            void setStep(int);

            void incrementStep();
            void decrementStep();

            int getLargeStep() const;

            void setLargeStep(int);

            void incrementLargeStep();
            void decrementLargeStep();

            ///@}

        private:
            TLRENDER_PRIVATE();
        };
    }
}
