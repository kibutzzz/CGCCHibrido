#version 410
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 textureCoordinate;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 worldNormal;
out vec3 fragmentPosition;
out vec2 interpolatedTextureCoordinate;

void main() {
  vec4 worldPosition = model * vec4(position, 1.0);
  gl_Position = projection * view * worldPosition;
  fragmentPosition = vec3(worldPosition);
  worldNormal = mat3(transpose(inverse(model))) * normal;
  interpolatedTextureCoordinate = textureCoordinate;
}
