#version 330 core
out vec4 frag_color;

in vec2 frag_pos;  
in vec2 tex_coord;

uniform float time;

const int max_iteration = 1000;

void main() {
    vec2 z0 = vec2(1.235*frag_pos.x-.53, 1.12*frag_pos.y);
    vec2 z = vec2(0);
    int i = 0;
    while (dot(z, z) <= 16 && i < max_iteration) {
        float tmp = z.x * z.x - z.y * z.y + z0.x;
        z.y = 2 * z.x * z.y + z0.y;
        z.x = tmp;
        i+=1;
    }

    frag_color  = vec4(float(i<max_iteration)*(1-exp(time-float(i))), 0.0, 0.0, 1.0);
}  