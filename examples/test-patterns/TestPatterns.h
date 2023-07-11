// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlTimeline/IRender.h>

#include <tlCore/Time.h>

namespace tl
{
    namespace examples
    {
        namespace test_patterns
        {
            class ITestPattern : public std::enable_shared_from_this<ITestPattern>
            {
                TLRENDER_NON_COPYABLE(ITestPattern);

            protected:
                void _init(
                    const std::string&,
                    const imaging::Size&,
                    const std::shared_ptr<system::Context>&);

                ITestPattern();

            public:
                virtual ~ITestPattern() = 0;

                const std::string& getName() const;

                virtual void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const otime::RationalTime&) = 0;

            protected:
                std::weak_ptr<system::Context> _context;
                std::string _name;
                imaging::Size _size;
            };

            class CountTestPattern : public ITestPattern
            {
            protected:
                void _init(
                    const imaging::Size&,
                    const std::shared_ptr<system::Context>&);

            public:
                ~CountTestPattern() override;

                static std::string getClassName();

                static std::shared_ptr<CountTestPattern> create(
                    const imaging::Size&,
                    const std::shared_ptr<system::Context>&);

                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const otime::RationalTime&) override;

            private:
                imaging::FontInfo _secondsFontInfo;
                imaging::FontMetrics _secondsFontMetrics;
                imaging::FontInfo _framesFontInfo;
                imaging::FontMetrics _framesFontMetrics;
            };

            class SwatchesTestPattern : public ITestPattern
            {
            protected:
                void _init(
                    const imaging::Size&,
                    const std::shared_ptr<system::Context>&);

            public:
                ~SwatchesTestPattern() override;

                static std::string getClassName();

                static std::shared_ptr<SwatchesTestPattern> create(
                    const imaging::Size&,
                    const std::shared_ptr<system::Context>&);

                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const otime::RationalTime&) override;

            private:
                std::shared_ptr<imaging::Image> _gradient;
            };

            class GridTestPattern : public ITestPattern
            {
            public:
                ~GridTestPattern() override;

                static std::string getClassName();

                static std::shared_ptr<GridTestPattern> create(
                    const imaging::Size&,
                    const std::shared_ptr<system::Context>&);

                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const otime::RationalTime&) override;
            };

            class TestPatternFactory
            {
            public:
                static std::shared_ptr<ITestPattern> create(
                    const std::string& name,
                    const imaging::Size&,
                    const std::shared_ptr<system::Context>&);
            };
        }
    }
}
