#version 300 es

precision mediump float;

in vec2 vTexCoord;
uniform sampler2D uTexture;

out vec4 fColor;

void main() {
    vec2 texcoord = vec2(0.5, 0.5);
  fColor = texture(uTexture, texcoord);
}

