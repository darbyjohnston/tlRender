// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

#pragma once

#include <tlrAV/Color.h>

#include <tlrCore/Matrix.h>
#include <tlrCore/Util.h>
#include <tlrCore/Vector.h>

#include <glad.h>

#include <memory>
#include <string>
#include <vector>

namespace tlr
{
    namespace render
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
            void setUniform(GLint, const math::Vector2f&);
            void setUniform(GLint, const math::Vector3f&);
            void setUniform(GLint, const math::Vector4f&);
            void setUniform(GLint, const math::Matrix3x3f&);
            void setUniform(GLint, const math::Matrix4x4f&);
            void setUniform(GLint, const imaging::Color4f&);
            void setUniform(GLint, const float [4]);

            void setUniform(GLint, const std::vector<int>&);
            void setUniform(GLint, const std::vector<float>&);
            void setUniform(GLint, const std::vector<math::Vector3f>&);
            void setUniform(GLint, const std::vector<math::Vector4f>&);

            void setUniform(const std::string&, int);
            void setUniform(const std::string&, float);
            void setUniform(const std::string&, const math::Vector2f&);
            void setUniform(const std::string&, const math::Vector3f&);
            void setUniform(const std::string&, const math::Vector4f&);
            void setUniform(const std::string&, const math::Matrix3x3f&);
            void setUniform(const std::string&, const math::Matrix4x4f&);
            void setUniform(const std::string&, const imaging::Color4f&);
            void setUniform(const std::string&, const float [4]);

            void setUniform(const std::string&, const std::vector<int>&);
            void setUniform(const std::string&, const std::vector<float>&);
            void setUniform(const std::string&, const std::vector<math::Vector3f>&);
            void setUniform(const std::string&, const std::vector<math::Vector4f>&);

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

#include <tlrRender/ShaderInline.h>
