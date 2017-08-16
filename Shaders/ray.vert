#version 400 core

layout (location = 0) in vec3 position;


//uniforms
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

void main(void)
{
	gl_Position = projection * view * model * vec4(position,1.0f);
	//gl_Position = model * vec4(position, 1.0f);
	//gl_Position = vec4(position,1.0f);
}
