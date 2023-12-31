#version 330 core

out vec4 FragColor;

in float Height;
in vec4 vertexColor;

void main()
{
	float h = (Height + 16)/32.0f;
    FragColor = vec4(h, h, h, 1.0);
}