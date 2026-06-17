#version 410
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vNormal;
out vec3 fragPos;
out vec2 texCoord;

void main() {
  vec4 worldPos = model * vec4(position, 1.0);
  gl_Position = projection * view * worldPos;
  fragPos = vec3(worldPos);
  vNormal = mat3(transpose(inverse(model))) * normal;
  texCoord = tex_coord;
}
