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

#if defined(TLRENDER_ENABLE_MMAP)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif // NOMINMAX
#include <windows.h>
#else // TLRENDER_ENABLE_MMAP
#include <stdio.h>
#endif // TLRENDER_ENABLE_MMAP

namespace tl
{
    namespace core
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
#if defined(TLRENDER_ENABLE_MMAP)
                HANDLE         f = INVALID_HANDLE_VALUE;
#else // TLRENDER_ENABLE_MMAP
                FILE* f = nullptr;
#endif // TLRENDER_ENABLE_MMAP
#if defined(TLRENDER_ENABLE_MMAP)
                void* mmap = nullptr;
                const uint8_t* mmapStart = nullptr;
                const uint8_t* mmapEnd = nullptr;
                const uint8_t* mmapP = nullptr;
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

#if defined(TLRENDER_ENABLE_MMAP)
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
                    p.f = CreateFileW(string::toWide(fileName).c_str(), desiredAccess, shareMode, 0, disposition, flags, 0);
                }
                catch (const std::exception&)
                {
                    p.f = INVALID_HANDLE_VALUE;
                }
                if (INVALID_HANDLE_VALUE == p.f)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Open, fileName, getLastError()));
                }
                p.fileName = fileName;
                p.mode = mode;
                p.pos = 0;
                p.size = GetFileSize(p.f, 0);

                // Memory mapping.
                if (Mode::Read == p.mode && p.size > 0)
                {
                    p.mmap = CreateFileMapping(p.f, 0, PAGE_READONLY, 0, 0, 0);
                    if (!p.mmap)
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::MemoryMap, fileName, getLastError()));
                    }

                    p.mmapStart = reinterpret_cast<const uint8_t*>(MapViewOfFile(p.mmap, FILE_MAP_READ, 0, 0, 0));
                    if (!p.mmapStart)
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::MemoryMap, fileName));
                    }

                    p.mmapEnd = p.mmapStart + p.size;
                    p.mmapP = p.mmapStart;
                }
#else // TLRENDER_ENABLE_MMAP
                std::wstring modeStr;
                switch (mode)
                {
                case Mode::Read:
                    modeStr = L"r";
                    break;
                case Mode::Write:
                    modeStr = L"w";
                    break;
                case Mode::ReadWrite:
                    modeStr = L"r+";
                    break;
                case Mode::Append:
                    modeStr = L"a";
                    break;
                default: break;
                }
                if (_wfopen_s(&p.f, string::toWide(fileName).c_str(), modeStr.c_str()) != 0)
                {
                    p.f = nullptr;
                }
                if (!p.f)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Open, fileName));
                }
                p.fileName = fileName;
                p.mode = mode;
                p.pos = 0;
                if (fseek(p.f, 0, SEEK_END) != 0)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Open, fileName));
                }
                p.size = ftell(p.f);
                if (fseek(p.f, 0, SEEK_SET) != 0)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Open, fileName));
                }
#endif // TLRENDER_ENABLE_MMAP
            }

            void FileIO::openTemp()
            {
                WCHAR path[MAX_PATH];
                DWORD r = GetTempPathW(MAX_PATH, path);
                if (!r)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::OpenTemp, std::string(), getLastError()));
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
                    throw std::runtime_error(getErrorMessage(ErrorType::OpenTemp, std::string(), getLastError()));
                }
            }

            bool FileIO::close(std::string* error)
            {
                TLRENDER_P();

                bool out = true;

                p.fileName = std::string();

#if defined(TLRENDER_ENABLE_MMAP)
                if (p.mmapStart != 0)
                {
                    if (!::UnmapViewOfFile((void*)p.mmapStart))
                    {
                        out = false;
                        if (error)
                        {
                            *error = getErrorMessage(ErrorType::CloseMemoryMap, p.fileName, getLastError());
                        }
                    }
                    p.mmapStart = 0;
                }
                if (p.mmap != 0)
                {
                    if (!::CloseHandle(p.mmap))
                    {
                        out = false;
                        if (error)
                        {
                            *error = getErrorMessage(ErrorType::Close, p.fileName, getLastError());
                        }
                    }
                    p.mmap = 0;
                }
                p.mmapEnd = 0;
                p.mmapP = 0;

                if (p.f != INVALID_HANDLE_VALUE)
                {
                    CloseHandle(p.f);
                    p.f = INVALID_HANDLE_VALUE;
                }
#else // TLRENDER_ENABLE_MMAP
                if (p.f)
                {
                    fclose(p.f);
                    p.f = nullptr;
                }
#endif // TLRENDER_ENABLE_MMAP

                p.mode = Mode::First;
                p.pos = 0;
                p.size = 0;

                return out;
            }

            bool FileIO::isOpen() const
            {
#if defined(TLRENDER_ENABLE_MMAP)
                return _p->f != INVALID_HANDLE_VALUE;
#else // TLRENDER_ENABLE_MMAP
                return _p->f != nullptr;
#endif // TLRENDER_ENABLE_MMAP
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
            const uint8_t* FileIO::mmapP() const
            {
                return _p->mmapP;
            }

            const uint8_t* FileIO::mmapEnd() const
            {
                return _p->mmapEnd;
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
#if defined(TLRENDER_ENABLE_MMAP)
                return
                    p.f == INVALID_HANDLE_VALUE ||
                    (p.size ? p.pos >= p.size : true);
#else // TLRENDER_ENABLE_MMAP
                return
                    !p.f ||
                    (p.size ? p.pos >= p.size : true);
#endif // TLRENDER_ENABLE_MMAP
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
                    const uint8_t* mmapP = p.mmapP + size * wordSize;
                    if (mmapP > p.mmapEnd)
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::ReadMemoryMap, p.fileName));
                    }
                    if (p.endianConversion && wordSize > 1)
                    {
                        memory::endian(p.mmapP, in, size, wordSize);
                    }
                    else
                    {
                        std::memcpy(in, p.mmapP, size * wordSize);
                    }
                    p.mmapP = mmapP;
#else // TLRENDER_ENABLE_MMAP
                    /*DWORD n;
                    if (!::ReadFile(p.f, in, static_cast<DWORD>(size * wordSize), &n, 0))
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::Read, p.fileName, getLastError()));
                    }*/
                    size_t r = fread(in, 1, size * wordSize, p.f);
                    if (r != size * wordSize)
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::Read, p.fileName));
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
#if defined(TLRENDER_ENABLE_MMAP)
                    DWORD n;
                    if (!::ReadFile(p.f, in, static_cast<DWORD>(size * wordSize), &n, 0))
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::Read, p.fileName, getLastError()));
                    }
#else // TLRENDER_ENABLE_MMAP
                    size_t r = fread(in, 1, size * wordSize, p.f);
                    if (r != size * wordSize)
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::Read, p.fileName));
                    }
#endif // TLRENDER_ENABLE_MMAP
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

#if defined(TLRENDER_ENABLE_MMAP)
                DWORD n = 0;
                if (!::WriteFile(p.f, inP, static_cast<DWORD>(size * wordSize), &n, 0))
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Write, p.fileName, getLastError()));
                }
#else // TLRENDER_ENABLE_MMAP
                size_t r = fwrite(in, 1, size * wordSize, p.f);
                if (r != size * wordSize)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Write, p.fileName));
                }
#endif // TLRENDER_ENABLE_MMAP
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
                        mmapP = reinterpret_cast<const uint8_t*>(mmapStart) + value;
                    }
                    else
                    {
                        mmapP += value;
                    }
                    if (mmapP > mmapEnd)
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::SeekMemoryMap, fileName));
                    }
#else // TLRENDER_ENABLE_MMAP
                    /*LARGE_INTEGER v;
                    v.QuadPart = value;
                    if (!::SetFilePointerEx(
                        f,
                        static_cast<LARGE_INTEGER>(v),
                        0,
                        !seek ? FILE_BEGIN : FILE_CURRENT))
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::Seek, fileName, getLastError()));
                    }*/
                    if (fseek(f, value, !seek ? SEEK_SET : SEEK_CUR) != 0)
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::Seek, fileName));
                    }
#endif // TLRENDER_ENABLE_MMAP
                    break;
                }
                case Mode::Write:
                case Mode::ReadWrite:
                case Mode::Append:
                {
#if defined(TLRENDER_ENABLE_MMAP)
                    LARGE_INTEGER v;
                    v.QuadPart = value;
                    if (!::SetFilePointerEx(
                        f,
                        static_cast<LARGE_INTEGER>(v),
                        0,
                        !seek ? FILE_BEGIN : FILE_CURRENT))
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::Seek, fileName, getLastError()));
                    }
#else // TLRENDER_ENABLE_MMAP
                    if (fseek(f, value, !seek ? SEEK_SET : SEEK_CUR) != 0)
                    {
                        throw std::runtime_error(getErrorMessage(ErrorType::Seek, fileName));
                    }
#endif // TLRENDER_ENABLE_MMAP
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
                    throw std::runtime_error(getErrorMessage(ErrorType::Open, fileName, getLastError()));
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
                    throw std::runtime_error(getErrorMessage(ErrorType::Seek, fileName, getLastError()));
                }
                if (!::SetEndOfFile(h))
                {
                    CloseHandle(h);
                    throw std::runtime_error(getErrorMessage(ErrorType::Write, fileName, getLastError()));
                }
                CloseHandle(h);
            }
        }
    }
}
