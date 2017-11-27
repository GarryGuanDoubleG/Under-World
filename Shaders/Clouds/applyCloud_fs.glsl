#version 450 core

in vec2 UV;

layout(binding = 0) uniform sampler2D cloudsTex;
layout(binding = 1) uniform sampler2D shadedScene;

out vec4 result;

void main() {
    vec4 scene_color = textureLod(shadedScene, UV, 0);
    vec4 cloud_color = textureLod(cloudsTex, UV, 0);

    result = scene_color * (1 - cloud_color.w) + cloud_color;
	//result.rgb = mix(scene_color, cloud_color, cloud_color.a).rgb;
    //result.w = scene_color.w;
	result.w = 1.0f;
	//result = scene_color;
	//result = cloud_color;
}