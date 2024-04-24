#version 330 core
out vec4 frag_color;

float near = 0.1; 
float far  = 100.0; 
  
float linearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main() {
    float depth = .1 - linearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    frag_color = vec4(vec3(depth), 1.0);
}