// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021 Darby Johnston
// All rights reserved.

namespace tlr
{
    namespace render
    {
        inline const std::string& Shader::getVertexSource() const
        {
            return _vertexSource;
        }

        inline const std::string& Shader::getFragmentSource() const
        {
            return _fragmentSource;
        }

        inline GLuint Shader::getProgram() const
        {
            return _program;
        }
    }
}
