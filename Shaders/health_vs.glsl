#version 400 core

layout (location = 0) in vec3 verts;
layout (location = 1) in vec2 uv;

out vec2 UV;

//uniforms
uniform mat4 view;
uniform mat4 projection;

uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform vec2 billboardSize;
uniform vec3 billboardCenter;

void main(void)
{
	vec3 vertPositive = vec3((verts.x + 1.0f)* .5f, verts.yz);
	vec3 worldSpaceVert = billboardCenter + cameraRight * vertPositive.x * billboardSize.x 
						  + cameraUp * vertPositive.y * billboardSize.y;
	
	gl_Position = projection * view * vec4(worldSpaceVert, 1.0f);

	UV = uv;
}

