#version 300 es

in vec3 aPos;
in vec2 aTexCoord;

uniform mat4 uWorldMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

out vec2 vTexCoord;

void main() {
  gl_Position = uProjectionMatrix * uViewMatrix * uWorldMatrix * vec4(aPos, 1.0);
  vTexCoord = aTexCoord;
}
