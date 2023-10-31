#version 330 core

layout (location = 0) in vec3 aPos;

out float Height;
out vec4 vertexColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	gl_Position = proj * view * model *vec4(aPos,1.0);
	Height = aPos.y;
	vertexColor = vec4(0.9f,0.5f,0.1f,1.0f);
}