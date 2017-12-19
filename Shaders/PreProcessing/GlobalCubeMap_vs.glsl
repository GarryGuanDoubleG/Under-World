#version 450 core

layout (location = 0) in vec3 position;

uniform mat4 projection;
uniform mat4 view;

out vec3 outLocalPos;

void main()
{
	outLocalPos = position;

	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * view * vec4(position, 1.0f);

	gl_Position = clipPos.xyww;
}