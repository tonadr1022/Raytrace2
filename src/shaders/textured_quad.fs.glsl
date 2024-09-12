#version 460 core

in vec2 tex_coords;
out vec4 o_color;

uniform sampler2D tex;

void main() {
    o_color = vec4(texture(tex, tex_coords).rgb, 1.0);
}
