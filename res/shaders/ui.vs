#version 300 es

in vec2 aPos;
in vec2 aTexCoord;

uniform vec2 uScreenSize;

out vec2 vTexCoord;

void main() {
    vec2 real_coordinates = aPos / uScreenSize;
    real_coordinates *= vec2(2.0);
    real_coordinates -= vec2(1.0);
    gl_Position = vec4(real_coordinates, 0.0, 1.0);

    vTexCoord = aTexCoord;
}
