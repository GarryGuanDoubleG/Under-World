#version 430 core

out vec4 FragColor;
in vec2 UV;

//uniform sampler3D tex;
uniform sampler2D tex;

void main()
{       
	//FragColor = vec4(texture(tex, UV).rgb, 1.0f);
	FragColor = vec4(.5f, .5f, .5f, 1.0f);
	//FragColor = vec4(texture(tex, vec3(UV, 1.0f)).rgb, 1.0f);
	//FragColor = getColor();
}