// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <tlRenderGL/Shader.h>

#include <tlCore/Color.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <iostream>

using namespace tl::core;

namespace tl
{
    namespace gl
    {
        void Shader::_init()
        {
            _vertex = glCreateShader(GL_VERTEX_SHADER);
            if (!_vertex)
            {
                throw std::runtime_error("Cannot create vertex shader");
            }
            const char* src = _vertexSource.c_str();
            glShaderSource(_vertex, 1, &src, NULL);
            glCompileShader(_vertex);
            int success = 0;
            glGetShaderiv(_vertex, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                char infoLog[string::cBufferSize];
                glGetShaderInfoLog(_vertex, string::cBufferSize, NULL, infoLog);
                auto lines = string::split(_vertexSource, { '\n', '\r' }, true);
                for (size_t i = 0; i < lines.size(); ++i)
                {
                    lines[i].insert(0, string::Format("{0}: ").arg(i));
                }
                lines.push_back(infoLog);
                throw std::runtime_error(string::join(lines, '\n'));
            }

            _fragment = glCreateShader(GL_FRAGMENT_SHADER);
            if (!_fragment)
            {
                throw std::runtime_error("Cannot create fragment shader");
            }
            src = _fragmentSource.c_str();
            glShaderSource(_fragment, 1, &src, NULL);
            glCompileShader(_fragment);
            glGetShaderiv(_fragment, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                char infoLog[string::cBufferSize];
                glGetShaderInfoLog(_fragment, string::cBufferSize, NULL, infoLog);
                auto lines = string::split(_fragmentSource, { '\n', '\r' }, true);
                for (size_t i = 0; i < lines.size(); ++i)
                {
                    lines[i].insert(0, string::Format("{0}: ").arg(i));
                }
                lines.push_back(infoLog);
                throw std::runtime_error(string::join(lines, '\n'));
            }

            _program = glCreateProgram();
            glAttachShader(_program, _vertex);
            glAttachShader(_program, _fragment);
            glLinkProgram(_program);
            glGetProgramiv(_program, GL_LINK_STATUS, &success);
            if (!success)
            {
                char infoLog[string::cBufferSize];
                glGetProgramInfoLog(_program, string::cBufferSize, NULL, infoLog);
                throw std::runtime_error(infoLog);
            }
        }

        Shader::Shader()
        {}

        Shader::~Shader()
        {
            if (_program)
            {
                glDeleteProgram(_program);
                _program = 0;
            }
            if (_vertex)
            {
                glDeleteShader(_vertex);
                _vertex = 0;
            }
            if (_fragment)
            {
                glDeleteShader(_fragment);
                _fragment = 0;
            }
        }

        std::shared_ptr<Shader> Shader::create(
            const std::string& vertexSource,
            const std::string& fragmentSource)
        {
            auto out = std::shared_ptr<Shader>(new Shader);
            out->_vertexSource = vertexSource;
            out->_fragmentSource = fragmentSource;
            out->_init();
            return out;
        }

        const std::string& Shader::getVertexSource() const
        {
            return _vertexSource;
        }

        const std::string& Shader::getFragmentSource() const
        {
            return _fragmentSource;
        }

        GLuint Shader::getProgram() const
        {
            return _program;
        }

        void Shader::bind()
        {
            glUseProgram(_program);
        }

        void Shader::setUniform(GLint location, int value)
        {
            glUniform1i(location, value);
        }

        void Shader::setUniform(GLint location, float value)
        {
            glUniform1f(location, value);
        }

        void Shader::setUniform(GLint location, const math::Vector2f& value)
        {
            glUniform2fv(location, 1, &value.x);
        }

        void Shader::setUniform(GLint location, const math::Vector3f& value)
        {
            glUniform3fv(location, 1, &value.x);
        }

        void Shader::setUniform(GLint location, const math::Vector4f& value)
        {
            glUniform4fv(location, 1, &value.x);
        }

        void Shader::setUniform(GLint location, const math::Matrix3x3f& value)
        {
            glUniformMatrix3fv(location, 1, GL_FALSE, value.e);
        }

        void Shader::setUniform(GLint location, const math::Matrix4x4f& value)
        {
            glUniformMatrix4fv(location, 1, GL_FALSE, value.e);
        }

        void Shader::setUniform(GLint location, const imaging::Color4f& value)
        {
            glUniform4fv(location, 1, &value.r);
        }

        void Shader::setUniform(GLint location, const float value[4])
        {
            glUniform4fv(location, 1, value);
        }

        void Shader::setUniform(GLint location, const std::vector<int>& value)
        {
            glUniform1iv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(GLint location, const std::vector<float>& value)
        {
            glUniform1fv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(GLint location, const std::vector<math::Vector3f>& value)
        {
            glUniform3fv(location, value.size(), &value[0].x);
        }

        void Shader::setUniform(GLint location, const std::vector<math::Vector4f>& value)
        {
            glUniform4fv(location, value.size(), &value[0].x);
        }

        void Shader::setUniform(const std::string& name, int value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniform1i(location, value);
        }

        void Shader::setUniform(const std::string& name, float value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniform1f(location, value);
        }

        void Shader::setUniform(const std::string& name, const math::Vector2f& value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniform2fv(location, 1, &value.x);
        }

        void Shader::setUniform(const std::string& name, const math::Vector3f& value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniform3fv(location, 1, &value.x);
        }

        void Shader::setUniform(const std::string& name, const math::Vector4f& value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniform4fv(location, 1, &value.x);
        }

        void Shader::setUniform(const std::string& name, const math::Matrix3x3f& value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniformMatrix3fv(location, 1, GL_FALSE, value.e);
        }

        void Shader::setUniform(const std::string& name, const math::Matrix4x4f& value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniformMatrix4fv(location, 1, GL_FALSE, value.e);
        }
        
        void Shader::setUniform(const std::string& name, const imaging::Color4f& value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniform4fv(location, 1, &value.r);
        }

        void Shader::setUniform(const std::string& name, const float value[4])
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniform4fv(location, 1, value);
        }

        void Shader::setUniform(const std::string& name, const std::vector<int>& value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniform1iv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(const std::string& name, const std::vector<float>& value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniform1fv(location, value.size(), &value[0]);
        }

        void Shader::setUniform(const std::string& name, const std::vector<math::Vector3f>& value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniform3fv(location, value.size(), &value[0].x);
        }

        void Shader::setUniform(const std::string& name, const std::vector<math::Vector4f>& value)
        {
            const GLint location = glGetUniformLocation(_program, name.c_str());
            glUniform4fv(location, value.size(), &value[0].x);
        }
    }
}
