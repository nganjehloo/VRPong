#version 410 core

uniform mat4 ProjectionMatrix = mat4(1);
uniform mat4 CameraMatrix = mat4(1);
uniform mat4 toWorld;
uniform vec3 color;


layout(location = 0) in vec4 Position;
layout(location = 1) in vec3 Normal;


out vec3 vertNormal;
out vec3 Color;

out vec3 fpos;
void main(void) {
  
   mat4 ViewXfm = CameraMatrix  * toWorld; // InstanceTransform;
   //mat4 ViewXfm = CameraMatrix;
   vertNormal = Normal;// * mat3(transpose(inverse(toWorld));
   Color = color;
   gl_Position = ProjectionMatrix * ViewXfm * Position;
  
}