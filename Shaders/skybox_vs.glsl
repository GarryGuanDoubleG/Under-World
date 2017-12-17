#version 330 core
layout (location = 0) in vec3 pos;

out vec3 FragPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    FragPos = pos;  
	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView * vec4(FragPos, 1.0);

	gl_Position = clipPos.xyww;
}