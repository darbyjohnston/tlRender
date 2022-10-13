// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlCore/FileIO.h>

#include <tlCore/Error.h>
#include <tlCore/Memory.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <cstring>
#include <exception>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <sys/stat.h>
#include <windows.h>

namespace tl
{
    namespace file
    {
        namespace
        {
            enum class ErrorType
            {
                Open,
                OpenTemp,
                MemoryMap,
                Close,
                CloseMemoryMap,
                Read,
                ReadMemoryMap,
                Write,
                Seek,
                SeekMemoryMap
            };

            std::string getErrorMessage(
                ErrorType          type,
                const std::string& fileName,
                const std::string& message = std::string())
            {
                std::string out;
                switch (type)
                {
                case ErrorType::Open:
                    out = string::Format("{0}: Cannot open file").arg(fileName);
                    break;
                case ErrorType::OpenTemp:
                    out = string::Format("Cannot open temporary file");
                    break;
                case ErrorType::MemoryMap:
                    out = string::Format("{0}: Cannot memory map").arg(fileName);
                    break;
                case ErrorType::Close:
                    out = string::Format("{0}: Cannot close").arg(fileName);
                    break;
                case ErrorType::CloseMemoryMap:
                    out = string::Format("{0}: Cannot unmap").arg(fileName);
                    break;
                case ErrorType::Read:
                    out = string::Format("{0}: Cannot read").arg(fileName);
                    break;
                case ErrorType::ReadMemoryMap:
                    out = string::Format("{0}: Cannot read memory map").arg(fileName);
                    break;
                case ErrorType::Write:
                    out = string::Format("{0}: Cannot write").arg(fileName);
                    break;
                case ErrorType::Seek:
                    out = string::Format("{0}: Cannot seek").arg(fileName);
                    break;
                case ErrorType::SeekMemoryMap:
                    out = string::Format("{0}: Cannot seek memory map").arg(fileName);
                    break;
                default: break;
                }
                if (!message.empty())
                {
                    out = string::Format("{0}: {1}").arg(out).arg(message);
                }
                return out;
            }

        } // namespace

        struct FileIO::Private
        {
            void setPos(size_t, bool seek);

            std::string    fileName;
            Mode           mode = Mode::First;
            size_t         pos = 0;
            size_t         size = 0;
            bool           endianConversion = false;
            HANDLE         f = INVALID_HANDLE_VALUE;
#if defined(TLRENDER_ENABLE_MMAP)
            void* mMap = nullptr;
            const uint8_t* mMapStart = nullptr;
            const uint8_t* mMapEnd = nullptr;
            const uint8_t* mMapP = nullptr;
#endif // TLRENDER_ENABLE_MMAP
        };

        FileIO::FileIO() :
            _p(new Private)
        {}

        FileIO::~FileIO()
        {
            close();
        }

        void FileIO::open(const std::string& fileName, Mode mode)
        {
            TLRENDER_P();

            close();

            const std::wstring fileNameW = string::toWide(fileName);

            // Open the file.
            DWORD desiredAccess = 0;
            DWORD shareMode = 0;
            DWORD disposition = 0;
            DWORD flags =
                //FILE_ATTRIBUTE_NORMAL;
                FILE_FLAG_SEQUENTIAL_SCAN;
            switch (mode)
            {
            case Mode::Read:
                desiredAccess = GENERIC_READ;
                shareMode = FILE_SHARE_READ;
                disposition = OPEN_EXISTING;
                break;
            case Mode::Write:
                desiredAccess = GENERIC_WRITE;
                disposition = CREATE_ALWAYS;
                break;
            case Mode::ReadWrite:
                desiredAccess = GENERIC_READ | GENERIC_WRITE;
                shareMode = FILE_SHARE_READ;
                disposition = OPEN_EXISTING;
                break;
            case Mode::Append:
                desiredAccess = GENERIC_WRITE;
                disposition = OPEN_EXISTING;
                break;
            default: break;
            }
            try
            {
                p.f = CreateFileW(
                    fileNameW.c_str(),
                    desiredAccess,
                    shareMode,
                    0,
                    disposition,
                    flags,
                    0);
            }
            catch (const std::exception&)
            {
                p.f = INVALID_HANDLE_VALUE;
            }
            if (INVALID_HANDLE_VALUE == p.f)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Open, fileName, error::getLastError()));
            }
            p.fileName = fileName;
            p.mode = mode;
            p.pos = 0;
            struct _stati64 info;
            memset(&info, 0, sizeof(struct _stati64));
            if (_wstati64(fileNameW.c_str(), &info) != 0)
            {
                throw std::runtime_error(
                    string::Format("{0}: Cannot get file size").arg(fileName));
            }
            p.size = info.st_size;

#if defined(TLRENDER_ENABLE_MMAP)
            // Memory mapping.
            if (Mode::Read == p.mode && p.size > 0)
            {
                p.mMap = CreateFileMapping(p.f, 0, PAGE_READONLY, 0, 0, 0);
                if (!p.mMap)
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::MemoryMap, fileName, error::getLastError()));
                }

                p.mMapStart = reinterpret_cast<const uint8_t*>(MapViewOfFile(p.mMap, FILE_MAP_READ, 0, 0, 0));
                if (!p.mMapStart)
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::MemoryMap, fileName));
                }

                p.mMapEnd = p.mMapStart + p.size;
                p.mMapP = p.mMapStart;
            }
#endif // TLRENDER_ENABLE_MMAP
        }

        void FileIO::openTemp()
        {
            WCHAR path[MAX_PATH];
            DWORD r = GetTempPathW(MAX_PATH, path);
            if (!r)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::OpenTemp, std::string(), error::getLastError()));
            }
            WCHAR buf[MAX_PATH];
            if (GetTempFileNameW(path, L"", 0, buf))
            {
                std::string fileName;
                try
                {
                    fileName = string::fromWide(buf);
                }
                catch (const std::exception&)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::OpenTemp, fileName));
                }
                open(fileName, Mode::ReadWrite);
            }
            else
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::OpenTemp, std::string(), error::getLastError()));
            }
        }

        bool FileIO::close(std::string* error)
        {
            TLRENDER_P();

            bool out = true;

            p.fileName = std::string();

#if defined(TLRENDER_ENABLE_MMAP)
            if (p.mMapStart != 0)
            {
                if (!::UnmapViewOfFile((void*)p.mMapStart))
                {
                    out = false;
                    if (error)
                    {
                        *error = getErrorMessage(
                            ErrorType::CloseMemoryMap, p.fileName, error::getLastError());
                    }
                }
                p.mMapStart = 0;
            }
            if (p.mMap != 0)
            {
                if (!::CloseHandle(p.mMap))
                {
                    out = false;
                    if (error)
                    {
                        *error = getErrorMessage(
                            ErrorType::Close, p.fileName, error::getLastError());
                    }
                }
                p.mMap = 0;
            }
            p.mMapEnd = 0;
            p.mMapP = 0;
#endif // TLRENDER_ENABLE_MMAP

            if (p.f != INVALID_HANDLE_VALUE)
            {
                CloseHandle(p.f);
                p.f = INVALID_HANDLE_VALUE;
            }

            p.mode = Mode::First;
            p.pos = 0;
            p.size = 0;

            return out;
        }

        bool FileIO::isOpen() const
        {
            return _p->f != INVALID_HANDLE_VALUE;
        }

        const std::string& FileIO::getFileName() const
        {
            return _p->fileName;
        }

        size_t FileIO::getSize() const
        {
            return _p->size;
        }

        size_t FileIO::getPos() const
        {
            return _p->pos;
        }

        void FileIO::setPos(size_t in)
        {
            _p->setPos(in, false);
        }

        void FileIO::seek(size_t in)
        {
            _p->setPos(in, true);
        }

#if defined(TLRENDER_ENABLE_MMAP)
        const uint8_t* FileIO::getMMapP() const
        {
            return _p->mMapP;
        }

        const uint8_t* FileIO::getMMapEnd() const
        {
            return _p->mMapEnd;
        }
#endif // TLRENDER_ENABLE_MMAP

        bool FileIO::hasEndianConversion() const
        {
            return _p->endianConversion;
        }

        void FileIO::setEndianConversion(bool in)
        {
            _p->endianConversion = in;
        }

        bool FileIO::isEOF() const
        {
            TLRENDER_P();
            return
                p.f == INVALID_HANDLE_VALUE ||
                (p.size ? p.pos >= p.size : true);
        }

        void FileIO::read(void* in, size_t size, size_t wordSize)
        {
            TLRENDER_P();

            if (!p.f)
            {
                throw std::runtime_error(getErrorMessage(ErrorType::Read, p.fileName));
            }

            switch (p.mode)
            {
            case Mode::Read:
            {
#if defined(TLRENDER_ENABLE_MMAP)
                const uint8_t* mMapP = p.mMapP + size * wordSize;
                if (mMapP > p.mMapEnd)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::ReadMemoryMap, p.fileName));
                }
                if (p.endianConversion && wordSize > 1)
                {
                    memory::endian(p.mMapP, in, size, wordSize);
                }
                else
                {
                    std::memcpy(in, p.mMapP, size * wordSize);
                }
                p.mMapP = mMapP;
#else // TLRENDER_ENABLE_MMAP
                DWORD n;
                if (!::ReadFile(p.f, in, static_cast<DWORD>(size * wordSize), &n, 0))
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Read, p.fileName, error::getLastError()));
                }
                if (p.endianConversion && wordSize > 1)
                {
                    memory::endian(in, size, wordSize);
                }
#endif // TLRENDER_ENABLE_MMAP
                break;
            }
            case Mode::ReadWrite:
            {
                DWORD n;
                if (!::ReadFile(p.f, in, static_cast<DWORD>(size * wordSize), &n, 0))
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::Read, p.fileName, error::getLastError()));
                }
                if (p.endianConversion && wordSize > 1)
                {
                    memory::endian(in, size, wordSize);
                }
                break;
            }
            default: break;
            }
            p.pos += size * wordSize;
        }

        void FileIO::write(const void* in, size_t size, size_t wordSize)
        {
            TLRENDER_P();

            if (!p.f)
            {
                throw std::runtime_error(getErrorMessage(ErrorType::Write, p.fileName));
            }

            const uint8_t* inP = reinterpret_cast<const uint8_t*>(in);
            std::vector<uint8_t> tmp;
            if (p.endianConversion && wordSize > 1)
            {
                tmp.resize(size * wordSize);
                memory::endian(in, tmp.data(), size, wordSize);
                inP = tmp.data();
            }

            DWORD n = 0;
            if (!::WriteFile(p.f, inP, static_cast<DWORD>(size * wordSize), &n, 0))
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Write, p.fileName, error::getLastError()));
            }
            p.pos += size * wordSize;
            p.size = std::max(p.pos, p.size);
        }

        void FileIO::Private::setPos(size_t value, bool seek)
        {
            switch (mode)
            {
            case Mode::Read:
            {
#if defined(TLRENDER_ENABLE_MMAP)
                if (!seek)
                {
                    mMapP = reinterpret_cast<const uint8_t*>(mMapStart) + value;
                }
                else
                {
                    mMapP += value;
                }
                if (mMapP > mMapEnd)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::SeekMemoryMap, fileName));
                }
#else // TLRENDER_ENABLE_MMAP
                LARGE_INTEGER v;
                v.QuadPart = value;
                if (!::SetFilePointerEx(
                    f,
                    static_cast<LARGE_INTEGER>(v),
                    0,
                    !seek ? FILE_BEGIN : FILE_CURRENT))
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Seek, fileName, error::getLastError()));
                }
#endif // TLRENDER_ENABLE_MMAP
                break;
            }
            case Mode::Write:
            case Mode::ReadWrite:
            case Mode::Append:
            {
                LARGE_INTEGER v;
                v.QuadPart = value;
                if (!::SetFilePointerEx(
                    f,
                    static_cast<LARGE_INTEGER>(v),
                    0,
                    !seek ? FILE_BEGIN : FILE_CURRENT))
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::Seek, fileName, error::getLastError()));
                }
                break;
            }
            default: break;
            }

            if (!seek)
            {
                pos = value;
            }
            else
            {
                pos += value;
            }
        }

        void truncate(const std::string& fileName, size_t size)
        {
            HANDLE h = INVALID_HANDLE_VALUE;
            try
            {
                h = CreateFileW(
                    string::toWide(fileName).c_str(),
                    GENERIC_WRITE,
                    0,
                    0,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    0);
            }
            catch (const std::exception&)
            {
                h = INVALID_HANDLE_VALUE;
            }
            if (INVALID_HANDLE_VALUE == h)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Open, fileName, error::getLastError()));
            }
            LARGE_INTEGER v;
            v.QuadPart = size;
            if (!::SetFilePointerEx(
                h,
                static_cast<LARGE_INTEGER>(v),
                0,
                FILE_BEGIN))
            {
                CloseHandle(h);
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Seek, fileName, error::getLastError()));
            }
            if (!::SetEndOfFile(h))
            {
                CloseHandle(h);
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Write, fileName, error::getLastError()));
            }
            CloseHandle(h);
        }
    }
}
