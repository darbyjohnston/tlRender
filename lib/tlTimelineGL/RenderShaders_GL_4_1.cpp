// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimelineGL/RenderPrivate.h>

#include <ftk/Core/Format.h>

namespace tl
{
    namespace timeline_gl
    {
        std::string vertexSource()
        {
            return
                "#version 410\n"
                "\n"
                "in vec3 vPos;\n"
                "in vec2 vTexture;\n"
                "out vec2 fTexture;\n"
                "\n"
                "struct Transform\n"
                "{\n"
                "    mat4 mvp;\n"
                "};\n"
                "\n"
                "uniform Transform transform;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                "    fTexture = vTexture;\n"
                "}\n";
        }

        std::string meshFragmentSource()
        {
            return
                "#version 410\n"
                "\n"
                "out vec4 outColor;\n"
                "\n"
                "uniform vec4 color;\n"
                "\n"
                "void main()\n"
                "{\n"
                "\n"
                "    outColor = color;\n"
                "}\n";
        }

        std::string textureFragmentSource()
        {
            return
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 outColor;\n"
                "\n"
                "uniform vec4 color;\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    outColor = texture(textureSampler, fTexture) * color;\n"
                "}\n";
        }

        namespace
        {
            const std::string videoLevels =
                "// enum ftk::VideoLevels\n"
                "const uint VideoLevels_FullRange  = 0;\n"
                "const uint VideoLevels_LegalRange = 1;\n";
        }

        std::string displayFragmentSource(
            const std::string& ocioDef,
            const std::string& ocio,
            const std::string& lutDef,
            const std::string& lut,
            timeline::LUTOrder lutOrder)
        {
            std::vector<std::string> args;
            args.push_back(videoLevels);
            args.push_back(ocioDef);
            args.push_back(lutDef);
            switch (lutOrder)
            {
            case timeline::LUTOrder::PreConfig:
                args.push_back(lut);
                args.push_back(ocio);
                break;
            case timeline::LUTOrder::PostConfig:
                args.push_back(ocio);
                args.push_back(lut);
                break;
            default: break;
            }
            return ftk::Format(
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 outColor;\n"
                "\n"
                "// enum tl::ftk::ChannelDisplay\n"
                "const uint Channels_Color = 0;\n"
                "const uint Channels_Red   = 1;\n"
                "const uint Channels_Green = 2;\n"
                "const uint Channels_Blue  = 3;\n"
                "const uint Channels_Alpha = 4;\n"
                "\n"
                "struct Levels\n"
                "{\n"
                "    float inLow;\n"
                "    float inHigh;\n"
                "    float gamma;\n"
                "    float outLow;\n"
                "    float outHigh;\n"
                "};\n"
                "\n"
                "struct EXRDisplay\n"
                "{\n"
                "    float v;\n"
                "    float d;\n"
                "    float k;\n"
                "    float f;\n"
                "    float g;\n"
                "};\n"
                "\n"
                "{0}\n"
                "\n"
                "uniform sampler2D textureSampler;\n"
                "\n"
                "uniform int        channels;\n"
                "uniform int        mirrorX;\n"
                "uniform int        mirrorY;\n"
                "uniform bool       colorEnabled;\n"
                "uniform vec3       colorAdd;\n"
                "uniform mat4       colorMatrix;\n"
                "uniform bool       colorInvert;\n"
                "uniform bool       levelsEnabled;\n"
                "uniform Levels     levels;\n"
                "uniform bool       exrDisplayEnabled;\n"
                "uniform EXRDisplay exrDisplay;\n"
                "uniform float      softClip;\n"
                "uniform int        videoLevels;\n"
                "\n"
                "vec4 colorFunc(vec4 value, vec3 add, mat4 m)\n"
                "{\n"
                "    vec4 tmp;\n"
                "    tmp[0] = value[0] + add[0];\n"
                "    tmp[1] = value[1] + add[1];\n"
                "    tmp[2] = value[2] + add[2];\n"
                "    tmp[3] = 1.0;\n"
                "    tmp = tmp * m;\n"
                "    tmp[3] = value[3];\n"
                "    return tmp;\n"
                "}\n"
                "\n"
                "vec4 levelsFunc(vec4 value, Levels data)\n"
                "{\n"
                "    vec4 tmp;\n"
                "    tmp[0] = (value[0] - data.inLow) / data.inHigh;\n"
                "    tmp[1] = (value[1] - data.inLow) / data.inHigh;\n"
                "    tmp[2] = (value[2] - data.inLow) / data.inHigh;\n"
                "    if (tmp[0] >= 0.0)\n"
                "        tmp[0] = pow(tmp[0], data.gamma);\n"
                "    if (tmp[1] >= 0.0)\n"
                "        tmp[1] = pow(tmp[1], data.gamma);\n"
                "    if (tmp[2] >= 0.0)\n"
                "        tmp[2] = pow(tmp[2], data.gamma);\n"
                "    value[0] = tmp[0] * data.outHigh + data.outLow;\n"
                "    value[1] = tmp[1] * data.outHigh + data.outLow;\n"
                "    value[2] = tmp[2] * data.outHigh + data.outLow;\n"
                "    return value;\n"
                "}\n"
                "\n"
                "vec4 softClipFunc(vec4 value, float softClip)\n"
                "{\n"
                "    float tmp = 1.0 - softClip;\n"
                "    if (value[0] > tmp)\n"
                "        value[0] = tmp + (1.0 - exp(-(value[0] - tmp) / softClip)) * softClip;\n"
                "    if (value[1] > tmp)\n"
                "        value[1] = tmp + (1.0 - exp(-(value[1] - tmp) / softClip)) * softClip;\n"
                "    if (value[2] > tmp)\n"
                "        value[2] = tmp + (1.0 - exp(-(value[2] - tmp) / softClip)) * softClip;\n"
                "    return value;\n"
                "}\n"
                "\n"
                "float knee(float value, float f)\n"
                "{\n"
                "    return log(value * f + 1.0) / f;\n"
                "}\n"
                "\n"
                "vec4 exrDisplayFunc(vec4 value, EXRDisplay data)\n"
                "{\n"
                "    value[0] = max(0.0, value[0] - data.d) * data.v;\n"
                "    value[1] = max(0.0, value[1] - data.d) * data.v;\n"
                "    value[2] = max(0.0, value[2] - data.d) * data.v;\n"
                "    if (value[0] > data.k)\n"
                "        value[0] = data.k + knee(value[0] - data.k, data.f);\n"
                "    if (value[1] > data.k)\n"
                "        value[1] = data.k + knee(value[1] - data.k, data.f);\n"
                "    if (value[2] > data.k)\n"
                "        value[2] = data.k + knee(value[2] - data.k, data.f);\n"
                "    if (value[0] > 0.0) value[0] = pow(value[0], data.g);\n"
                "    if (value[1] > 0.0) value[1] = pow(value[1], data.g);\n"
                "    if (value[2] > 0.0) value[2] = pow(value[2], data.g);\n"
                "    float s = pow(2, -3.5 * data.g);\n"
                "    value[0] *= s;\n"
                "    value[1] *= s;\n"
                "    value[2] *= s;\n"
                "    return value;\n"
                "}\n"
                "\n"
                "{1}\n"
                "\n"
                "{2}\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec2 t = fTexture;\n"
                "    if (1 == mirrorX)\n"
                "    {\n"
                "        t.x = 1.0 - t.x;\n"
                "    }\n"
                "    if (1 == mirrorY)\n"
                "    {\n"
                "        t.y = 1.0 - t.y;\n"
                "    }\n"
                "\n"
                "    outColor = texture(textureSampler, t);\n"
                "\n"
                "    // Video levels.\n"
                "    if (VideoLevels_LegalRange == videoLevels)\n"
                "    {\n"
                "        const float scale = (940.0 - 64.0) / 1023.0;\n"
                "        const float offset = 64.0 / 1023.0;\n"
                "        outColor.r = outColor.r * scale + offset;\n"
                "        outColor.g = outColor.g * scale + offset;\n"
                "        outColor.b = outColor.b * scale + offset;\n"
                "    }\n"
                "\n"
                "    // Apply color transformations.\n"
                "    if (colorEnabled)\n"
                "    {\n"
                "        outColor = colorFunc(outColor, colorAdd, colorMatrix);\n"
                "    }\n"
                "    if (colorInvert)\n"
                "    {\n"
                "        outColor.r = 1.0 - outColor.r;\n"
                "        outColor.g = 1.0 - outColor.g;\n"
                "        outColor.b = 1.0 - outColor.b;\n"
                "    }\n"
                "    if (levelsEnabled)\n"
                "    {\n"
                "        outColor = levelsFunc(outColor, levels);\n"
                "    }\n"
                "    if (exrDisplayEnabled)\n"
                "    {\n"
                "        outColor = exrDisplayFunc(outColor, exrDisplay);\n"
                "    }\n"
                "    if (softClip > 0.0)\n"
                "    {\n"
                "        outColor = softClipFunc(outColor, softClip);\n"
                "    }\n"
                "\n"
                "    // Apply color management.\n"
                "    {3}\n"
                "    {4}\n"
                "\n"
                "    // Swizzle for the channels display.\n"
                "    if (Channels_Red == channels)\n"
                "    {\n"
                "        outColor.g = outColor.r;\n"
                "        outColor.b = outColor.r;\n"
                "    }\n"
                "    else if (Channels_Green == channels)\n"
                "    {\n"
                "        outColor.r = outColor.g;\n"
                "        outColor.b = outColor.g;\n"
                "    }\n"
                "    else if (Channels_Blue == channels)\n"
                "    {\n"
                "        outColor.r = outColor.b;\n"
                "        outColor.g = outColor.b;\n"
                "    }\n"
                "    else if (Channels_Alpha == channels)\n"
                "    {\n"
                "        outColor.r = outColor.a;\n"
                "        outColor.g = outColor.a;\n"
                "        outColor.b = outColor.a;\n"
                "    }\n"
                "}\n").
                arg(args[0]).
                arg(args[1]).
                arg(args[2]).
                arg(args[3]).
                arg(args[4]);
        }

        std::string dissolveFragmentSource()
        {
            return
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 outColor;\n"
                "\n"
                "uniform float     dissolve;\n"
                "uniform sampler2D textureSampler;\n"
                "uniform sampler2D textureSampler2;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 c = texture(textureSampler, fTexture);\n"
                "    vec4 c2 = texture(textureSampler2, fTexture);\n"
                "    float idissolve = 1.0 - dissolve;\n"
                "    outColor.r = c.r * idissolve + c2.r * dissolve;\n"
                "    outColor.g = c.g * idissolve + c2.g * dissolve;\n"
                "    outColor.b = c.b * idissolve + c2.b * dissolve;\n"
                "    outColor.a = c.a * idissolve + c2.a * dissolve;\n"
                "}\n";
        }

        std::string differenceFragmentSource()
        {
            return
                "#version 410\n"
                "\n"
                "in vec2 fTexture;\n"
                "out vec4 outColor;\n"
                "\n"
                "uniform sampler2D textureSampler;\n"
                "uniform sampler2D textureSamplerB;\n"
                "\n"
                "void main()\n"
                "{\n"
                "    vec4 c = texture(textureSampler, fTexture);\n"
                "    vec4 cB = texture(textureSamplerB, fTexture);\n"
                "    outColor.r = abs(c.r - cB.r);\n"
                "    outColor.g = abs(c.g - cB.g);\n"
                "    outColor.b = abs(c.b - cB.b);\n"
                "    outColor.a = max(c.a, cB.a);\n"
                "}\n";
        }
    }
}
