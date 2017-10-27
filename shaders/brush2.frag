﻿#version 330 core
// for sampler bindings.
#extension GL_ARB_shading_language_420pack: require

in vec3 texcoord;
in float light;
in vec3 ws_pos;
in vec3 ws_norm;

layout(binding=0) uniform sampler2DArray s_albedo;

layout(location=0) out vec4 color;

void main(void)
{
    color = vec4(0.2, 1.0, 0.7, 0.5);
}

