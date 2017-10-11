#version 400 core
out vec4 FragColor;
in vec2 UV;

uniform sampler2D tex;
uniform sampler3D tex3D;
uniform float near_plane;
uniform float far_plane;

// required when using a perspective projection matrix
float LinearizeDepth(float depth)
{
    float z = depth * 2.0 - 1.0; // Back to NDC 
    return (2.0 * near_plane * far_plane) / (far_plane + near_plane - z * (far_plane - near_plane));	
}

void main()
{           
	FragColor = vec4(0);
	FragColor += vec4(texture(tex, UV).rgb, 1.0f);
	FragColor += vec4(vec3(texture(tex3D, vec3(UV, 0.5f)).rgb), 1.0f);
	//FragColor = vec4(vec3(FragColor.g), 1.0f);
	//FragColor = vec4(1.f, .1f, .1f, 1.0f);
}