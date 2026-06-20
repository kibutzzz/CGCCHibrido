#version 410
in vec3 worldNormal;
in vec3 fragmentPosition;
in vec2 interpolatedTextureCoordinate;
out vec4 color;

uniform vec3 cameraPosition;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float ns;
uniform sampler2D textureBuffer;
uniform vec3 lightPosition;
uniform vec3 lightColor;

void main() {
  vec3 normal = normalize(worldNormal);
  vec3 lightDirection = normalize(lightPosition - fragmentPosition);
  vec3 viewDirection = normalize(cameraPosition - fragmentPosition);
  vec3 reflectionDirection = reflect(-lightDirection, normal);

  vec3 ambient = ka * lightColor;
  vec3 diffuse = kd * max(dot(normal, lightDirection), 0.0) * lightColor;
  vec3 specular =
      ks * pow(max(dot(reflectionDirection, viewDirection), 0.0), max(ns, 1.0)) *
      lightColor;

  vec4 textureColor = texture(textureBuffer, interpolatedTextureCoordinate);
  color = vec4((ambient + diffuse + specular) * vec3(textureColor), textureColor.a);
}
