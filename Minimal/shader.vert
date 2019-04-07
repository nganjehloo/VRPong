#version 410 core

uniform mat4 ProjectionMatrix = mat4(1);
uniform mat4 CameraMatrix = mat4(1);
uniform mat4 toWorld;
uniform int isLaser;

layout(location = 0) in vec4 Position;
layout(location = 1) in vec3 Normal;
layout (location = 2) in vec2 TexCoords;

out vec3 vertNormal;
out vec3 Color;
out vec2 Texcoords;
out vec3 fpos;


void main(void) {
  
   mat4 ViewXfm = CameraMatrix  * toWorld; // InstanceTransform;
   vertNormal =  mat3(transpose(inverse(toWorld))) * Normal;
   gl_Position = ProjectionMatrix * ViewXfm * Position;
   fpos = vec3(toWorld * Position);
   Texcoords = TexCoords;
  
}