#version 400 core
in vec2 UV;
out vec4 color;

uniform sampler2D screenTexture;
uniform sampler2D alphaMask;
//uniform sampler2D heightmap;


void main()
{ 
    vec4 minimap = texture(screenTexture, UV);

	float alpha = texture(alphaMask, UV).r;

	color = vec4(minimap.rgb, minimap.a * alpha);
}