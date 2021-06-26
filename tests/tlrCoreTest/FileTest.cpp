// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCoreTest/FileTest.h>

#include <tlrCore/Assert.h>
#include <tlrCore/File.h>
#include <tlrCore/FileIO.h>

#include <limits>
#include <sstream>

using namespace tlr::file;

namespace tlr
{
    namespace CoreTest
    {
        FileTest::FileTest() :
            ITest("CoreTest::FileTest"),
            _fileName("file.txt"),
            _text("Hello"),
            _text2("world!")
        {}

        std::shared_ptr<FileTest> FileTest::create()
        {
            return std::shared_ptr<FileTest>(new FileTest);
        }

        void FileTest::run()
        {
            _func();
            _io();
        }

        void FileTest::_func()
        {
            {
                TLR_ASSERT(isAbsolute("/"));
                TLR_ASSERT(isAbsolute("/tmp"));
                TLR_ASSERT(isAbsolute("\\"));
                TLR_ASSERT(isAbsolute("C:"));
                TLR_ASSERT(isAbsolute("C:\\tmp"));
                TLR_ASSERT(!isAbsolute(""));
                TLR_ASSERT(!isAbsolute("../.."));
                TLR_ASSERT(!isAbsolute("..\\.."));
            }
            {
                TLR_ASSERT(normalize("/tmp/file.txt") == "/tmp/file.txt");
                TLR_ASSERT(normalize("\\tmp\\file.txt") == "/tmp/file.txt");
            }
            {
                std::string path;
                std::string baseName;
                std::string number;
                std::string extension;
                split("", &path, &baseName, &number, &extension);
                TLR_ASSERT(path.empty());
                TLR_ASSERT(baseName.empty());
                TLR_ASSERT(number.empty());
                TLR_ASSERT(extension.empty());
            }
            {
                std::string path;
                std::string baseName;
                std::string number;
                std::string extension;
                split("/tmp/file.txt", &path, &baseName, &number, &extension);
                TLR_ASSERT("/tmp/" == path);
                TLR_ASSERT("file" == baseName);
                TLR_ASSERT(number.empty());
                TLR_ASSERT(".txt" == extension);
            }
            {
                std::string path;
                std::string baseName;
                std::string number;
                std::string extension;
                split("/tmp/render.0001.exr", &path, &baseName, &number, &extension);
                TLR_ASSERT("/tmp/" == path);
                TLR_ASSERT("render." == baseName);
                TLR_ASSERT("0001" == number);
                TLR_ASSERT(".exr" == extension);
            }
            {
                std::stringstream ss;
                ss << "Temp dir:" << createTempDir();
                _print(ss.str());
            }
        }

        void FileTest::_io()
        {
            {
                auto io = FileIO::create();
                TLR_ASSERT(!io->isOpen());
                TLR_ASSERT(io->getFileName().empty());
                TLR_ASSERT(0 == io->getSize());
                TLR_ASSERT(0 == io->getPos());
                TLR_ASSERT(io->isEOF());
                const std::string fileName = createTempDir() + '/' + _fileName;
                io->open(fileName, Mode::Write);
                TLR_ASSERT(io->isOpen());
                TLR_ASSERT(io->getFileName() == fileName);
            }
            
            {
                auto io = FileIO::create();
                io->openTemp();
                TLR_ASSERT(io->isOpen());
            }

            {
                const int8_t   i8 = std::numeric_limits<int8_t>::max();
                const uint8_t  u8 = std::numeric_limits<uint8_t>::max();
                const int16_t  i16 = std::numeric_limits<int16_t>::max();
                const uint16_t u16 = std::numeric_limits<uint16_t>::max();
                const int32_t  i32 = std::numeric_limits<int32_t>::max();
                const uint32_t u32 = std::numeric_limits<uint32_t>::max();
                const float    f = std::numeric_limits<float>::max();
                const std::string fileName = createTempDir() + '/' + _fileName;
                auto io = FileIO::create();
                io->open(fileName, Mode::Write);
                io->write8(i8);
                io->writeU8(u8);
                io->write16(i16);
                io->writeU16(u16);
                io->write32(i32);
                io->writeU32(u32);
                io->writeF32(f);

                io->open(fileName, Mode::Read);
                int8_t   _i8 = 0;
                uint8_t  _u8 = 0;
                int16_t  _i16 = 0;
                uint16_t _u16 = 0;
                int32_t  _i32 = 0;
                uint32_t _u32 = 0;
                float    _f = 0.F;
                io->read8(&_i8);
                io->readU8(&_u8);
                io->read16(&_i16);
                io->readU16(&_u16);
                io->read32(&_i32);
                io->readU32(&_u32);
                io->readF32(&_f);
                TLR_ASSERT(i8 == _i8);
                TLR_ASSERT(u8 == _u8);
                TLR_ASSERT(i16 == _i16);
                TLR_ASSERT(u16 == _u16);
                TLR_ASSERT(i32 == _i32);
                TLR_ASSERT(u32 == _u32);
                TLR_ASSERT(f == _f);
            }

            {
                auto io = FileIO::create();
                const std::string fileName = createTempDir() + '/' + _fileName;
                io->open(fileName, Mode::Write);
                io->write(_text + " ");
                io->open(fileName, Mode::Append);
                io->seek(io->getSize());
                io->write(_text2);

                io->open(fileName, Mode::Read);
                std::string buf = readContents(io);
                _print(buf);
                TLR_ASSERT((_text + " " + _text2) == buf);
                io->setPos(0);
                TLR_ASSERT(0 == io->getPos());
            }

            {
                const std::string fileName = createTempDir() + '/' + _fileName;
                writeLines(
                    fileName,
                    {
                        "# This is a comment",
                        _text + " " + _text2
                    });

                auto io = FileIO::create();
                io->open(fileName, Mode::Read);
                char buf[string::cBufferSize];
                readWord(io, buf);
                _print(buf);
                TLR_ASSERT(_text == buf);
                readWord(io, buf);
                _print(buf);
                TLR_ASSERT(_text2 == buf);
            }

            {
                const std::string fileName = createTempDir() + '/' + _fileName;
                auto io = FileIO::create();
                io->open(fileName, Mode::Write);
                io->write(_text + "\n" + _text2);

                io->open(fileName, Mode::Read);
                char buf[string::cBufferSize];
                readLine(io, buf);
                _print(buf);
                TLR_ASSERT(_text == buf);
                readLine(io, buf);
                _print(buf);
                TLR_ASSERT(_text2 == buf);
            }

            {
                const std::string fileName = createTempDir() + '/' + _fileName;
                writeLines(
                    fileName,
                    {
                        _text,
                        "# This is a comment",
                        _text2
                    });

                const auto lines = readLines(fileName);
                for (const auto& i : lines)
                {
                    _print(i);
                }
                TLR_ASSERT(_text == lines[0]);
                TLR_ASSERT(_text2 == lines[2]);
            }
            
            {
                const std::string fileName = createTempDir() + '/' + _fileName;
                auto io = FileIO::create();
                io->open(fileName, Mode::Write);
                TLR_ASSERT(!io->hasEndianConversion());
                io->setEndianConversion(true);
                TLR_ASSERT(io->hasEndianConversion());
                uint32_t u32 = 0;
                uint8_t* p = reinterpret_cast<uint8_t*>(&u32);
                p[0] = 0;
                p[1] = 1;
                p[2] = 2;
                p[2] = 3;
                io->writeU32(u32);
                for (auto mode : { Mode::Read, Mode::ReadWrite })
                {
                    io->open(fileName, mode);
                    io->setEndianConversion(false);
                    uint32_t _u32 = 0;
                    io->readU32(&_u32);
                    uint8_t* p2 = reinterpret_cast<uint8_t*>(&_u32);
                    TLR_ASSERT(p[0] == p2[3]);
                    TLR_ASSERT(p[1] == p2[2]);
                    TLR_ASSERT(p[2] == p2[1]);
                    TLR_ASSERT(p[3] == p2[0]);
                    io->setEndianConversion(true);
                    io->setPos(0);
                    io->readU32(&_u32);
                    TLR_ASSERT(p[0] == p2[0]);
                    TLR_ASSERT(p[1] == p2[1]);
                    TLR_ASSERT(p[2] == p2[2]);
                    TLR_ASSERT(p[3] == p2[3]);
                }
            }

            for (auto mode : { Mode::Read, Mode::Write, Mode::ReadWrite, Mode::Append })
            {
                try
                {
                    auto io = FileIO::create();
                    io->open(std::string(), mode);
                    TLR_ASSERT(false);
                }
                catch (const std::exception& e)
                {
                    _print(e.what());
                }
            }

            try
            {
                const std::string fileName = createTempDir() + '/' + _fileName;
                auto io = FileIO::create();
                io->open(fileName, Mode::Write);
                io->open(fileName, Mode::Read);
                uint8_t buf[16];
                io->read(buf, 16, 1);
                TLR_ASSERT(false);
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }

#if !defined(_WINDOWS)
            try
            {
                const std::string fileName = createTempDir() + '/' + _fileName;
                auto io = FileIO::create();
                io->open(fileName, Mode::Write);
                io->open(fileName, Mode::ReadWrite);
                uint8_t buf[16];
                //! \bug FileIO::read() doesn't fail here on Windows?
                io->read(buf, 16, 1);
                TLR_ASSERT(false);
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }
#endif // _WINDOWS

            try
            {
                auto io = FileIO::create();
                uint8_t buf[16];
                io->write(buf, 16, 1);
                TLR_ASSERT(false);
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }

            try
            {
                auto io = FileIO::create();
                io->setPos(16);
                TLR_ASSERT(false);
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }

            try
            {
                auto io = FileIO::create();
                io->seek(16);
                TLR_ASSERT(false);
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }
        }
    }
}
