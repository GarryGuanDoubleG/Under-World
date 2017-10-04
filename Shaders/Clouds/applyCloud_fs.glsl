#version 430

in vec2 UV;

layout(binding = 0) uniform sampler2D CloudsTex;
layout(binding = 1) uniform sampler2D ShadedScene;

out vec4 result;

void main() {
    vec4 scene_color = textureLod(ShadedScene, UV, 0);
    vec4 cloud_color = textureLod(CloudsTex, UV, 0);

    //result = scene_color * (1 - cloud_color.w) + cloud_color;
	result.rgb = mix(scene_color, cloud_color, 0.5f).rgb;
    result.w = scene_color.w;
	//result = scene_color;
}