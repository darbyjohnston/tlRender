// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace file
    {
        inline const std::string& FileIO::getFileName() const
        {
            return _fileName;
        }

        inline size_t FileIO::getSize() const
        {
            return _size;
        }

        inline size_t FileIO::getPos() const
        {
            return _pos;
        }

#if defined(TLR_ENABLE_MMAP)
        inline const uint8_t* FileIO::mmapP() const
        {
            return _mmapP;
        }

        inline const uint8_t* FileIO::mmapEnd() const
        {
            return _mmapEnd;
        }
#endif // TLR_ENABLE_MMAP

        inline bool FileIO::hasEndianConversion() const
        {
            return _endianConversion;
        }

        inline void FileIO::setEndianConversion(bool in)
        {
            _endianConversion = in;
        }
    }
}
