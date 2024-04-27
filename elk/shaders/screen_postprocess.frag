#version 330 core
out vec4 frag_color;
  
in vec2 tex_coord;

uniform sampler2D screen_texture;

const float offset = 1.0;

void main()
{
    vec2 offsets[9] = vec2[](
		vec2(-1*offset, 1*offset),
		vec2(0, 1*offset),
		vec2(1*offset, 1*offset),
		vec2(-1*offset, 0),
		vec2(0, 0),
		vec2(1*offset, 0),
		vec2(-1*offset, -1*offset),
		vec2(0, -1*offset),
		vec2(1*offset, -1*offset)
	);

    float kernel[9] = float[](
        -1.0, -1.0, -1.0,
        -1.0, 9.0, -1.0,
        -1.0, -1.0, -1.0
    );
    
    vec3 sample_tex[9];
    for(int i = 0; i < 9; i++) {
        sample_tex[i] = vec3(texture(screen_texture, tex_coord.st + offsets[i]));
    }
    vec3 col = vec3(0.0);
    for(int i = 0; i < 9; i++)
        col += sample_tex[i] * kernel[i];
    
    frag_color = vec4(col, 1.0);
}