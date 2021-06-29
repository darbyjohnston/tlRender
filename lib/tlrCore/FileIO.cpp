// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#include <tlrCore/FileIO.h>

#include <tlrCore/Assert.h>
#include <tlrCore/Error.h>

#include <algorithm>
#include <iostream>

namespace tlr
{
    namespace file
    {
        TLR_ENUM_IMPL(
            Mode,
            "Read",
            "Write",
            "ReadWrite",
            "Append");

        FileIO::FileIO()
        {}

        FileIO::~FileIO()
        {
            close();
        }

        std::shared_ptr<FileIO> FileIO::create()
        {
            return std::shared_ptr<FileIO>(new FileIO);
        }

        void FileIO::setPos(size_t in)
        {
            _setPos(in, false);
        }

        void FileIO::seek(size_t in)
        {
            _setPos(in, true);
        }

        void FileIO::read8(int8_t* value, size_t size)
        {
            return read(value, size, 1);
        }

        void FileIO::readU8(uint8_t* value, size_t size)
        {
            return read(value, size, 1);
        }

        void FileIO::read16(int16_t* value, size_t size)
        {
            return read(value, size, 2);
        }

        void FileIO::readU16(uint16_t* value, size_t size)
        {
            return read(value, size, 2);
        }

        void FileIO::read32(int32_t* value, size_t size)
        {
            return read(value, size, 4);
        }

        void FileIO::readU32(uint32_t* value, size_t size)
        {
            return read(value, size, 4);
        }

        void FileIO::readF32(float* value, size_t size)
        {
            return read(value, size, 4);
        }

        void FileIO::write8(const int8_t* value, size_t size)
        {
            write(value, size, 1);
        }

        void FileIO::writeU8(const uint8_t* value, size_t size)
        {
            write(value, size, 1);
        }

        void FileIO::write16(const int16_t* value, size_t size)
        {
            write(value, size, 2);
        }

        void FileIO::writeU16(const uint16_t* value, size_t size)
        {
            write(value, size, 2);
        }

        void FileIO::write32(const int32_t* value, size_t size)
        {
            return write(value, size, 4);
        }

        void FileIO::writeU32(const uint32_t* value, size_t size)
        {
            return write(value, size, 4);
        }

        void FileIO::writeF32(const float* value, size_t size)
        {
            write(value, size, 4);
        }

        void FileIO::write8(int8_t value)
        {
            write8(&value, 1);
        }

        void FileIO::writeU8(uint8_t value)
        {
            writeU8(&value, 1);
        }

        void FileIO::write16(int16_t value)
        {
            write16(&value, 1);
        }

        void FileIO::writeU16(uint16_t value)
        {
            writeU16(&value, 1);
        }

        void FileIO::write32(int32_t value)
        {
            write32(&value, 1);
        }

        void FileIO::writeU32(uint32_t value)
        {
            writeU32(&value, 1);
        }

        void FileIO::writeF32(float value)
        {
            writeF32(&value, 1);
        }

        void FileIO::write(const std::string& value)
        {
            write8(reinterpret_cast<const int8_t*>(value.c_str()), value.size());
        }

        std::string readContents(const std::shared_ptr<FileIO>& io)
        {
#ifdef TLR_ENABLE_MMAP
            const uint8_t* p = io->mmapP();
            const uint8_t* end = io->mmapEnd();
            return std::string(reinterpret_cast<const char*>(p), end - p);
#else // TLR_ENABLE_MMAP
            const size_t fileSize = io->getSize();
            std::string out;
            out.resize(fileSize);
            io->read(reinterpret_cast<void*>(&out[0]), fileSize);
            return out;
#endif // TLR_ENABLE_MMAP
        }

        void readWord(const std::shared_ptr<FileIO>& io, char* out, size_t maxLen)
        {
            TLR_ASSERT(maxLen);
            out[0] = 0;
            enum class Parse
            {
                End,
                Word,
                Comment
            };
            Parse parse = Parse::Word;
            size_t i = 0;
            while (parse != Parse::End && !io->isEOF())
            {
                // Get the next character.
                uint8_t c = 0;
                io->read(&c, 1);

                switch (c)
                {
                case '#':
                    // Start of a comment.
                    parse = Parse::Comment;
                    break;
                case '\0':
                case '\n':
                case '\r':
                    // End of a comment or word.
                    parse = Parse::Word;
                case ' ':
                case '\t':
                    if (out[0])
                    {
                        parse = Parse::End;
                    }
                    break;
                default:
                    // Add the character to the word.
                    if (Parse::Word == parse && i < (maxLen - 1))
                    {
                        out[i++] = c;
                    }
                    break;
                }
            }
            out[i] = 0;
        }

        void readLine(const std::shared_ptr<FileIO>& io, char* out, size_t maxLen)
        {
            TLR_ASSERT(maxLen);
            size_t i = 0;
            if (!io->isEOF())
            {
                char c = 0;
                do
                {
                    io->read(&c, 1);
                    if (
                        c != '\n' &&
                        c != '\r')
                    {
                        out[i++] = c;
                    }
                } while (
                    c != '\n' &&
                    c != '\r' &&
                    !io->isEOF() &&
                    i < (maxLen - 1));
            }
            out[i] = 0;
        }

        std::vector<std::string> readLines(const std::string& fileName)
        {
            std::vector<std::string> out;
            auto io = FileIO::create();
            io->open(fileName, Mode::Read);
            while (!io->isEOF())
            {
                char buf[string::cBufferSize] = "";
                readLine(io, buf, string::cBufferSize);
                out.push_back(buf);
            }
            return out;
        }

        void writeLines(const std::string& fileName, const std::vector<std::string>& lines)
        {
            auto io = FileIO::create();
            io->open(fileName, Mode::Write);
            for (const auto& line : lines)
            {
                io->write(line);
                io->write8('\n');
            }
        }
    }

    TLR_ENUM_SERIALIZE_IMPL(file, Mode);
}
