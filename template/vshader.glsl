#version 120

attribute vec4 vPosition;
attribute vec2 vTexCoord;
varying vec2 texCoord;

uniform mat4 ctm;
uniform mat4 model_view;
uniform mat4 projection;

void main()
{
	texCoord = vTexCoord;
	gl_Position = ctm * vPosition;
	gl_Position = projection * model_view * ctm * vPosition;
}
