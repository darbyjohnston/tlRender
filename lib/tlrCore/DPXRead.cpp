// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/DPX.h>

#include <tlrCore/StringFormat.h>

#include <sstream>

namespace tlr
{
    namespace dpx
    {
        namespace
        {
            class File
            {
            public:
                File(const std::string& fileName)
                {
                }

                ~File()
                {
                }

                const imaging::Info& getInfo() const
                {
                    return _info;
                }

                avio::VideoFrame read(
                    const std::string& fileName,
                    const otime::RationalTime& time)
                {
                    avio::VideoFrame out;
                    return out;
                }

            private:
                imaging::Info _info;
            };
        }

        void Read::_init(
            const std::string& fileName,
            const avio::Options& options)
        {
            ISequenceRead::_init(fileName, options);
        }

        Read::Read()
        {}

        Read::~Read()
        {}

        std::shared_ptr<Read> Read::create(
            const std::string& fileName,
            const avio::Options& options)
        {
            auto out = std::shared_ptr<Read>(new Read);
            out->_init(fileName, options);
            return out;
        }

        avio::Info Read::_getInfo(const std::string& fileName)
        {
            avio::Info out;
            avio::VideoInfo videoInfo;
            videoInfo.info = std::unique_ptr<File>(new File(fileName))->getInfo();
            videoInfo.duration = _defaultSpeed;
            out.video.push_back(videoInfo);
            return out;
        }

        avio::VideoFrame Read::_readVideoFrame(
            const std::string& fileName,
            const otime::RationalTime& time)
        {
            return std::unique_ptr<File>(new File(fileName))->read(fileName, time);
        }
    }
}
