// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/PPM.h>

#include <tlrCore/Error.h>
#include <tlrCore/String.h>

#include <array>
#include <sstream>

namespace tlr
{
    namespace ppm
    {
        TLR_ENUM_IMPL(
            Data,
            "ASCII",
            "Binary");
        TLR_ENUM_SERIALIZE_IMPL(Data);

        size_t getScanlineByteCount(
            int    width,
            size_t channelCount,
            size_t bitDepth)
        {
            size_t chars = 0;
            switch (bitDepth)
            {
            case  8: chars = 3; break;
            case 16: chars = 5; break;
            default: break;
            }
            return (chars + 1) * width * channelCount + 1;
        }

        namespace
        {
            template<typename T>
            void _readASCII(
                const std::shared_ptr<file::FileIO>& io,
                uint8_t*                             out,
                size_t                               size)
            {
                char tmp[string::cBufferSize] = "";
                T* outP = reinterpret_cast<T*>(out);
                for (int i = 0; i < size; ++i)
                {
                    file::readWord(io, tmp, string::cBufferSize);
                    int value = 0;
                    string::fromString(tmp, string::cBufferSize, value);
                    outP[i] = value;
                }
            }

        } // namespace

        void readASCII(
            const std::shared_ptr<file::FileIO>& io,
            uint8_t*                             out,
            size_t                               size,
            size_t                               bitDepth)
        {
            switch (bitDepth)
            {
            case  8: _readASCII<uint8_t>(io, out, size); break;
            case 16: _readASCII<uint16_t>(io, out, size); break;
            default: break;
            }
        }

        namespace
        {
            template<typename T>
            size_t _writeASCII(
                const uint8_t* in,
                char*          out,
                size_t         size)
            {
                char* outP = out;
                const T* inP = reinterpret_cast<const T*>(in);
                for (size_t i = 0; i < size; ++i)
                {
                    const std::string s = std::to_string(static_cast<unsigned int>(inP[i]));
                    const char* c = s.c_str();
                    for (size_t j = 0; j < s.size(); ++j)
                    {
                        *outP++ = c[j];
                    }
                    *outP++ = ' ';
                }
                *outP++ = '\n';
                return outP - out;
            }

        } // namespace

        size_t writeASCII(
            const uint8_t* in,
            char*          out,
            size_t         size,
            size_t         bitDepth)
        {
            switch (bitDepth)
            {
            case  8: return _writeASCII<uint8_t>(in, out, size);
            case 16: return _writeASCII<uint16_t>(in, out, size);
            default: break;
            }
            return 0;
        }

        Plugin::Plugin()
        {}

        std::shared_ptr<Plugin> Plugin::create(const std::shared_ptr<core::LogSystem>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(
                "PPM",
                { ".ppm" },
                logSystem);
            return out;
        }

        std::shared_ptr<avio::IRead> Plugin::read(
            const file::Path& path,
            const avio::Options& options)
        {
            return Read::create(path, avio::merge(options, _options), _logSystem);
        }

        std::vector<imaging::PixelType> Plugin::getWritePixelTypes() const
        {
            return
            {
                imaging::PixelType::L_U8,
                imaging::PixelType::RGB_U8,
                imaging::PixelType::L_U16,
                imaging::PixelType::RGB_U16
            };
        }

        std::shared_ptr<avio::IWrite> Plugin::write(
            const file::Path& path,
            const avio::Info& info,
            const avio::Options& options)
        {
            return !info.video.empty() && _isWriteCompatible(info.video[0]) ?
                Write::create(path, info, avio::merge(options, _options), _logSystem) :
                nullptr;
        }
    }
}
