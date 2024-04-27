#version 330 core
out vec4 frag_color;
  
in vec2 tex_coord;

uniform sampler2D screen_texture;

const float offset = STRIDE;

void main()
{
    OFFSETS;

    float kernel[OFFSET_SZ] = KERNEL;
    
    vec3 sample_tex[OFFSET_SZ];
    for(int i = 0; i < OFFSET_SZ; i++) {
        sample_tex[i] = vec3(texture(screen_texture, tex_coord.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < OFFSET_SZ; i++)
        col += sample_tex[i] * kernel[i];
    
    frag_color = vec4(col, 1.0);
}