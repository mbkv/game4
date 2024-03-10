#version 330 core

layout(location = 0) in vec2 aPos; 
layout(location = 1) in vec2 aTexCoord;

uniform vec2 uScreenSize;

out vec2 vTexCoord;

void main() {
	vec2 real_coordinates = (aPos / uScreenSize) * vec2(2) - vec2(1);
	real_coordinates.y *= -1;
  gl_Position = vec4(real_coordinates, 0.0, 1.0);
  vTexCoord = aTexCoord;
}


