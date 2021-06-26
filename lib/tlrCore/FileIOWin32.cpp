// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/FileIO.h>

#include <tlrCore/Error.h>
#include <tlrCore/Memory.h>
#include <tlrCore/StringFormat.h>

#include <codecvt>
#include <locale>
#include <exception>

using namespace tlr::core;

namespace tlr
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

            std::string getErrorMessage(ErrorType type, const std::string& fileName)
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
                const std::string last = getLastError();
                if (!last.empty())
                {
                    out = string::Format("{0}: {1}").arg(out).arg(last);
                }
                return out;
            }

        } // namespace

        void FileIO::open(const std::string& fileName, Mode mode)
        {
            close();

#if defined(TLR_ENABLE_MMAP)
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
                disposition = CREATE_ALWAYS;
                break;
            case Mode::Append:
                desiredAccess = GENERIC_WRITE;
                disposition = OPEN_EXISTING;
                break;
            default: break;
            }
            try
            {
                std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> utf16;
                _f = CreateFileW(utf16.from_bytes(fileName).c_str(), desiredAccess, shareMode, 0, disposition, flags, 0);
            }
            catch (const std::exception&)
            {
                _f = INVALID_HANDLE_VALUE;
            }
            if (INVALID_HANDLE_VALUE == _f)
            {
                throw std::runtime_error(getErrorMessage(ErrorType::Open, fileName));
            }
            _fileName = fileName;
            _mode = mode;
            _pos = 0;
            _size = GetFileSize(_f, 0);

            // Memory mapping.
            if (Mode::Read == _mode && _size > 0)
            {
                _mmap = CreateFileMapping(_f, 0, PAGE_READONLY, 0, 0, 0);
                if (!_mmap)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::MemoryMap, fileName));
                }

                _mmapStart = reinterpret_cast<const uint8_t*>(MapViewOfFile(_mmap, FILE_MAP_READ, 0, 0, 0));
                if (!_mmapStart)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::MemoryMap, fileName));
                }

                _mmapEnd = _mmapStart + _size;
                _mmapP = _mmapStart;
            }
#else // TLR_ENABLE_MMAP
            std::string modeStr;
            switch (mode)
            {
            case Mode::Read:
                modeStr = "r";
                break;
            case Mode::Write:
                modeStr = "w";
                break;
            case Mode::ReadWrite:
                modeStr = "r+";
                break;
            case Mode::Append:
                modeStr = "a";
                break;
            default: break;
            }
            _f = fopen(fileName.c_str(), modeStr.c_str());
            if (!_f)
            {
                throw std::runtime_error(getErrorMessage(ErrorType::Open, fileName));
            }
            _fileName = fileName;
            _mode = mode;
            _pos = 0;
            if (fseek(_f, 0, SEEK_END) != 0)
            {
                throw std::runtime_error(getErrorMessage(ErrorType::Open, fileName));
            }
            _size = ftell(_f);
            if (fseek(_f, 0, SEEK_SET) != 0)
            {
                throw std::runtime_error(getErrorMessage(ErrorType::Open, fileName));
            }
#endif // TLR_ENABLE_MMAP
        }

        void FileIO::openTemp()
        {
            WCHAR path[MAX_PATH];
            DWORD r = GetTempPathW(MAX_PATH, path);
            if (!r)
            {
                throw std::runtime_error(getErrorMessage(ErrorType::OpenTemp, std::string()));
            }
            WCHAR buf[MAX_PATH];
            if (GetTempFileNameW(path, L"", 0, buf))
            {
                std::string fileName;
                try
                {
                    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> utf16;
                    fileName = utf16.to_bytes(buf);
                }
                catch (const std::exception&)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::OpenTemp, fileName));
                }
                open(fileName, Mode::ReadWrite);
            }
            else
            {
                throw std::runtime_error(getErrorMessage(ErrorType::OpenTemp, std::string()));
            }
        }

        bool FileIO::close(std::string* error)
        {
            bool out = true;

            _fileName = std::string();

#if defined(TLR_ENABLE_MMAP)
            if (_mmapStart != 0)
            {
                if (!::UnmapViewOfFile((void*)_mmapStart))
                {
                    out = false;
                    if (error)
                    {
                        *error = getErrorMessage(ErrorType::CloseMemoryMap, _fileName);
                    }
                }
                _mmapStart = 0;
            }
            if (_mmap != 0)
            {
                if (!::CloseHandle(_mmap))
                {
                    out = false;
                    if (error)
                    {
                        *error = getErrorMessage(ErrorType::Close, _fileName);
                    }
                }
                _mmap = 0;
            }
            _mmapEnd = 0;
            _mmapP = 0;

            if (_f != INVALID_HANDLE_VALUE)
            {
                CloseHandle(_f);
                _f = INVALID_HANDLE_VALUE;
            }
#else // TLR_ENABLE_MMAP
            if (_f)
            {
                fclose(_f);
                _f = nullptr;
            }
#endif // TLR_ENABLE_MMAP

            _mode = Mode::First;
            _pos = 0;
            _size = 0;

            return out;
        }

        bool FileIO::isOpen() const
        {
#if defined(TLR_ENABLE_MMAP)
            return _f != INVALID_HANDLE_VALUE;
#else // TLR_ENABLE_MMAP
            return _f != nullptr;
#endif // TLR_ENABLE_MMAP
        }

        bool FileIO::isEOF() const
        {
#if defined(TLR_ENABLE_MMAP)
            return
                _f == INVALID_HANDLE_VALUE ||
                (_size ? _pos >= _size : true);
#else // TLR_ENABLE_MMAP
            return
                !_f ||
                (_size ? _pos >= _size : true);
#endif // TLR_ENABLE_MMAP
        }

        void FileIO::read(void* in, size_t size, size_t wordSize)
        {
            if (!_f)
            {
                throw std::runtime_error(getErrorMessage(ErrorType::Read, _fileName));
            }

            switch (_mode)
            {
            case Mode::Read:
            {
#if defined(TLR_ENABLE_MMAP)
                const uint8_t* p = _mmapP + size * wordSize;
                if (p > _mmapEnd)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::ReadMemoryMap, _fileName));
                }
                if (_endianConversion && wordSize > 1)
                {
                    memory::endian(_mmapP, in, size, wordSize);
                }
                else
                {
                    memcpy(in, _mmapP, size * wordSize);
                }
                _mmapP = p;
#else // TLR_ENABLE_MMAP
                /*DWORD n;
                if (!::ReadFile(_f, in, static_cast<DWORD>(size * wordSize), &n, 0))
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Read, _fileName));
                }*/
                size_t r = fread(in, 1, size * wordSize, _f);
                if (r != size * wordSize)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Read, _fileName));
                }
                if (_endianConversion && wordSize > 1)
                {
                    memory::endian(in, size, wordSize);
                }
#endif // TLR_ENABLE_MMAP
                break;
            }
            case Mode::ReadWrite:
            {
#if defined(TLR_ENABLE_MMAP)
                DWORD n;
                if (!::ReadFile(_f, in, static_cast<DWORD>(size * wordSize), &n, 0))
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Read, _fileName));
                }
#else // TLR_ENABLE_MMAP
                size_t r = fread(in, 1, size * wordSize, _f);
                if (r != size * wordSize)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Read, _fileName));
                }
#endif // TLR_ENABLE_MMAP
                if (_endianConversion && wordSize > 1)
                {
                    memory::endian(in, size, wordSize);
                }
                break;
            }
            default: break;
            }
            _pos += size * wordSize;
        }

        void FileIO::write(const void* in, size_t size, size_t wordSize)
        {
            if (!_f)
            {
                throw std::runtime_error(getErrorMessage(ErrorType::Write, _fileName));
            }

            const uint8_t* inP = reinterpret_cast<const uint8_t*>(in);

            std::vector<uint8_t> tmp;
            if (_endianConversion && wordSize > 1)
            {
                tmp.resize(size * wordSize);
                memory::endian(in, tmp.data(), size, wordSize);
                in = tmp.data();
            }

#if defined(TLR_ENABLE_MMAP)
            DWORD n = 0;
            if (!::WriteFile(_f, inP, static_cast<DWORD>(size * wordSize), &n, 0))
            {
                throw std::runtime_error(getErrorMessage(ErrorType::Write, _fileName));
            }
#else // TLR_ENABLE_MMAP
            size_t r = fwrite(in, 1, size * wordSize, _f);
            if (r != size * wordSize)
            {
                throw std::runtime_error(getErrorMessage(ErrorType::Write, _fileName));
            }
#endif // TLR_ENABLE_MMAP
            _pos += size * wordSize;
            _size = std::max(_pos, _size);
        }

        void FileIO::_setPos(size_t value, bool seek)
        {
            switch (_mode)
            {
            case Mode::Read:
            {
#if defined(TLR_ENABLE_MMAP)
                if (!seek)
                {
                    _mmapP = reinterpret_cast<const uint8_t*>(_mmapStart) + value;
                }
                else
                {
                    _mmapP += value;
                }
                if (_mmapP > _mmapEnd)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::SeekMemoryMap, _fileName));
                }
#else // TLR_ENABLE_MMAP
                /*LARGE_INTEGER v;
                v.QuadPart = value;
                if (!::SetFilePointerEx(
                    _f,
                    static_cast<LARGE_INTEGER>(v),
                    0,
                    !seek ? FILE_BEGIN : FILE_CURRENT))
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Seek, _fileName));
                }*/
                if (fseek(_f, value, !seek ? SEEK_SET : SEEK_CUR) != 0)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Seek, _fileName));
                }
#endif // TLR_ENABLE_MMAP
                break;
            }
            case Mode::Write:
            case Mode::ReadWrite:
            case Mode::Append:
            {
#if defined(TLR_ENABLE_MMAP)
                LARGE_INTEGER v;
                v.QuadPart = value;
                if (!::SetFilePointerEx(
                    _f,
                    static_cast<LARGE_INTEGER>(v),
                    0,
                    !seek ? FILE_BEGIN : FILE_CURRENT))
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Seek, _fileName));
                }
#else // TLR_ENABLE_MMAP
                if (fseek(_f, value, !seek ? SEEK_SET : SEEK_CUR) != 0)
                {
                    throw std::runtime_error(getErrorMessage(ErrorType::Seek, _fileName));
                }
#endif // TLR_ENABLE_MMAP
                break;
            }
            default: break;
            }

            if (!seek)
            {
                _pos = value;
            }
            else
            {
                _pos += value;
            }
        }
    }
}
