// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace io
    {
        inline const std::string& IIO::getFileName() const
        {
            return _fileName;
        }

        inline const Info& IIO::getInfo() const
        {
            return _info;
        }

        inline std::queue<VideoFrame>& IRead::getVideoQueue()
        {
            return _videoQueue;
        }
    }
}