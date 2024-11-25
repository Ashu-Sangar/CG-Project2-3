#version 120

attribute vec4 vPosition;
attribute vec2 vTexCoord;
attribute vec4 vColor;
attribute vec4 vNormal;

varying vec2 texCoord;
varying vec4 color;
varying vec4 N, V, L, H;

uniform mat4 ctm;
uniform mat4 model_view;
uniform mat4 projection;

uniform vec4 light_position;
uniform vec4 user_position;

uniform int flashlight;

void main()
{
	
	N = normalize(model_view * ctm * vNormal);
    if(flashlight == 0) L = normalize(model_view * (light_position - ctm * vPosition));
	else L = normalize(model_view * (user_position - ctm * vPosition));
    V = normalize(vec4(0, 0, 0, 1) - model_view * ctm * vPosition);
	H = normalize(L + V);
	
	texCoord = vTexCoord;
	gl_Position = projection * model_view * ctm * vPosition;
}
