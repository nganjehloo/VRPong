#version 410 core



in vec3 Color;
in vec3 vertNormal;
in vec3 fpos;
in vec3 viewPos;
in vec3 viewDir;

out vec4 fragColor;

void main(void) {

   
    fragColor = vec4( Color ,1.0f);
}