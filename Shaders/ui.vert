#version 400 core
layout(location = 0) in vec3 verts;
layout (location = 1) in vec2 verts_uv;

out vec2 UV;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;


void main()
{
	 //// Declare a hard-coded array of positions     
	 //const vec4 vertices[3] = vec4[3](vec4(0.25, -0.25, 0.5, 1.0),                                     
		//							  vec4(-0.25, -0.25, 0.5, 1.0),                                      
		//							  vec4(0.25, 0.25, 0.5, 1.0));     
	  
	 // // Index into our array using gl_VertexID     
	 // gl_Position = vertices[gl_VertexID];


	 gl_Position = proj * vec4(verts, 1.0f);
	 UV = verts_uv;
}