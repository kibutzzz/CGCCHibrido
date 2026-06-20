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
uniform int numLights;
uniform vec3 lightPositions[8];
uniform vec3 lightColors[8];
uniform float selectionBrightness;

void main() {
  vec3 normal = normalize(worldNormal);
  vec3 viewDirection = normalize(cameraPosition - fragmentPosition);

  vec3 ambient = vec3(0.0);
  vec3 diffuse = vec3(0.0);
  vec3 specular = vec3(0.0);

  for (int i = 0; i < 3; i++) {
    vec3 lightDirection = normalize(lightPositions[i] - fragmentPosition);
    vec3 reflectionDirection = reflect(-lightDirection, normal);

    ambient += ka * lightColors[i];
    diffuse += kd * max(dot(normal, lightDirection), 0.0) * lightColors[i];
    specular += ks * pow(max(dot(reflectionDirection, viewDirection), 0.0), max(ns, 1.0)) * lightColors[i];
  }

  vec4 textureColor = texture(textureBuffer, interpolatedTextureCoordinate);
  color = vec4((ambient + diffuse + specular) * selectionBrightness * vec3(textureColor), textureColor.a);
}
