// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
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
                FTK_NON_COPYABLE(ITestPattern);

            protected:
                void _init(
                    const std::shared_ptr<ftk::Context>&,
                    const std::string&,
                    const ftk::Size2I&);

                ITestPattern();

            public:
                virtual ~ITestPattern() = 0;

                const std::string& getName() const;

                virtual void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const OTIO_NS::RationalTime&) = 0;

            protected:
                std::weak_ptr<ftk::Context> _context;
                std::string _name;
                ftk::Size2I _size;
            };

            class CountTestPattern : public ITestPattern
            {
            protected:
                void _init(
                    const std::shared_ptr<ftk::Context>&,
                    const ftk::Size2I&);

            public:
                virtual ~CountTestPattern();

                static std::string getClassName();

                static std::shared_ptr<CountTestPattern> create(
                    const std::shared_ptr<ftk::Context>&,
                    const ftk::Size2I&);

                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const OTIO_NS::RationalTime&) override;

            private:
                ftk::FontInfo _secondsFontInfo;
                ftk::FontMetrics _secondsFontMetrics;
                ftk::FontInfo _framesFontInfo;
                ftk::FontMetrics _framesFontMetrics;
            };

            class SwatchesTestPattern : public ITestPattern
            {
            protected:
                void _init(
                    const std::shared_ptr<ftk::Context>&,
                    const ftk::Size2I&);

            public:
                virtual ~SwatchesTestPattern();

                static std::string getClassName();

                static std::shared_ptr<SwatchesTestPattern> create(
                    const std::shared_ptr<ftk::Context>&,
                    const ftk::Size2I&);

                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const OTIO_NS::RationalTime&) override;

            private:
                std::shared_ptr<ftk::Image> _gradient;
            };

            class GridTestPattern : public ITestPattern
            {
            public:
                virtual ~GridTestPattern();

                static std::string getClassName();

                static std::shared_ptr<GridTestPattern> create(
                    const std::shared_ptr<ftk::Context>&,
                    const ftk::Size2I&);

                void render(
                    const std::shared_ptr<timeline::IRender>&,
                    const OTIO_NS::RationalTime&) override;
            };

            class TestPatternFactory
            {
            public:
                static std::shared_ptr<ITestPattern> create(
                    const std::shared_ptr<ftk::Context>&,
                    const std::string& name,
                    const ftk::Size2I&);
            };
        }
    }
}
