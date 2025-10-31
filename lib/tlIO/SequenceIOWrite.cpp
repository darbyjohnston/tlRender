// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/SequenceIO.h>

#include <cstring>
#include <sstream>

namespace tl
{
    namespace io
    {
        struct ISequenceWrite::Private
        {
            std::string path;
            std::string baseName;
            std::string number;
            int pad = 0;
            std::string extension;

            float defaultSpeed = SequenceOptions().defaultSpeed;
        };

        void ISequenceWrite::_init(
            const file::Path& path,
            const Info& info,
            const Options& options,
            const std::shared_ptr<ftk::LogSystem>& logSystem)
        {
            IWrite::_init(path, options, info, logSystem);

            FTK_P();

            const auto i = options.find("SequenceIO/DefaultSpeed");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.defaultSpeed;
            }
        }

        ISequenceWrite::ISequenceWrite() :
            _p(new Private)
        {}

        ISequenceWrite::~ISequenceWrite()
        {}

        void ISequenceWrite::writeVideo(
            const OTIO_NS::RationalTime& time,
            const std::shared_ptr<ftk::Image>& image,
            const Options& options)
        {
            _writeVideo(
                _path.get(static_cast<int>(time.value())),
                time,
                image,
                merge(options, _options));
        }
    }
}
