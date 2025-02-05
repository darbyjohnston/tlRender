// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/ISystem.h>

namespace tl
{
    namespace time
    {
        //! Timer.
        class Timer : public std::enable_shared_from_this<Timer>
        {
            DTK_NON_COPYABLE(Timer);

        protected:
            void _init(const std::shared_ptr<dtk::Context>&);

            Timer();

        public:
            ~Timer();

            //! Create a new timer.
            static std::shared_ptr<Timer> create(
                const std::shared_ptr<dtk::Context>&);

            //! Does the timer repeat?
            bool isRepeating() const;

            //! Set whether the timer repeats.
            void setRepeating(bool);

            //! Start the timer.
            void start(
                const std::chrono::microseconds&,
                const std::function<void(void)>&);

            //! Start the timer.
            void start(
                const std::chrono::microseconds&,
                const std::function<void(
                    const std::chrono::steady_clock::time_point&,
                    const std::chrono::microseconds&)>&);

            //! Stop the timer.
            void stop();

            //! Is the timer active?
            bool isActive() const;

            //! Get the timeout.
            const std::chrono::microseconds& getTimeout() const;

            void tick();

        private:
            DTK_PRIVATE();
        };

        //! Timer system.
        class TimerSystem : public system::ISystem
        {
            DTK_NON_COPYABLE(TimerSystem);

        protected:
            TimerSystem(const std::shared_ptr<dtk::Context>&);

        public:
            virtual ~TimerSystem();

            static std::shared_ptr<TimerSystem> create(
                const std::shared_ptr<dtk::Context>&);

            void addTimer(const std::shared_ptr<Timer>&);

            void tick() override;
            std::chrono::milliseconds getTickTime() const override;

        private:
            DTK_PRIVATE();
        };
    }
}
