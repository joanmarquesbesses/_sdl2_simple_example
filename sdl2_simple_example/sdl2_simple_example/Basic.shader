#shader vertex
#version 330 core

layout(location = 0) in vec4 position;
layout(location = 1) in vec4 color;

out vec4 v_color;

void main()
{
	gl_Position = position;
	v_color = color;
};

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

in vec4 v_color;

void main()
{
	color = vec4(255, 0, 0, 255);
};