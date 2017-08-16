#version 400 core

layout (location = 0) in vec3 verts;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec3 worldPos;


out vec2 UV;

//uniforms
uniform mat4 view;
uniform mat4 projection;
uniform mat4 model;

uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform vec2 billboardSize;

uniform float time;

void main(void)
{
	mat4 modelMat = model;

	modelMat[3][0] = worldPos.x;
	modelMat[3][1] = worldPos.y + .5f;
	modelMat[3][2] = worldPos.z;
	modelMat[3][3] = 1.0f;

	mat4 modelView = view * modelMat;
	modelView[0][0] = 1.0; 
	modelView[0][1] = 0.0; 
	modelView[0][2] = 0.0; 

	// Thrid colunm.
	modelView[2][0] = 0.0; 
	modelView[2][1] = 0.0; 
	modelView[2][2] = 1.0; 

	vec4 P = modelView * vec4(verts, 1.0f);
	gl_Position = projection * P;
	
	UV = uv;
}

