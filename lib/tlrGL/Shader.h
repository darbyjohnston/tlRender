// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrCore/Util.h>

#include <tlrGlad/gl.h>

#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#include <memory>
#include <string>
#include <vector>

namespace tlr
{
    namespace imaging
    {
        class Color4f;
    }

    namespace gl
    {
        //! OpenGL shader.
        class Shader : public std::enable_shared_from_this<Shader>
        {
            TLR_NON_COPYABLE(Shader);

        protected:
            void _init();
            Shader();

        public:
            ~Shader();

            //! Create a new shader.
            static std::shared_ptr<Shader> create(
                const std::string& vertexSource,
                const std::string& fragmentSource);

            //! Get the vertex shader source.
            const std::string& getVertexSource() const;

            //! Get the fragment shader source.
            const std::string& getFragmentSource() const;

            //! Get the OpenGL shader program.
            GLuint getProgram() const;

            //! Bind the shader.
            void bind();

            //! \name Uniforms
            //! Set uniform values.
            ///@{

            void setUniform(GLint, int);
            void setUniform(GLint, float);
            void setUniform(GLint, const glm::vec2&);
            void setUniform(GLint, const glm::vec3&);
            void setUniform(GLint, const glm::vec4&);
            void setUniform(GLint, const glm::mat3x3&);
            void setUniform(GLint, const glm::mat4x4&);
            void setUniform(GLint, const imaging::Color4f&);
            void setUniform(GLint, const float[4]);

            void setUniform(GLint, const std::vector<int>&);
            void setUniform(GLint, const std::vector<float>&);
            void setUniform(GLint, const std::vector<glm::vec3>&);
            void setUniform(GLint, const std::vector<glm::vec4>&);

            void setUniform(const std::string&, int);
            void setUniform(const std::string&, float);
            void setUniform(const std::string&, const glm::vec2&);
            void setUniform(const std::string&, const glm::vec3&);
            void setUniform(const std::string&, const glm::vec4&);
            void setUniform(const std::string&, const glm::mat3x3&);
            void setUniform(const std::string&, const glm::mat4x4&);
            void setUniform(const std::string&, const imaging::Color4f&);
            void setUniform(const std::string&, const float[4]);

            void setUniform(const std::string&, const std::vector<int>&);
            void setUniform(const std::string&, const std::vector<float>&);
            void setUniform(const std::string&, const std::vector<glm::vec3>&);
            void setUniform(const std::string&, const std::vector<glm::vec4>&);

            ///@}

        private:
            std::string _vertexSource;
            std::string _fragmentSource;
            GLuint _vertex = 0;
            GLuint _fragment = 0;
            GLuint _program = 0;
        };
    }
}
