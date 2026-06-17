#version 410
in vec3 vNormal;
in vec3 fragPos;
in vec2 texCoord;
out vec4 color;

uniform vec3 camPos;
uniform vec3 ka;
uniform vec3 kd;
uniform vec3 ks;
uniform float ns;
uniform sampler2D tex_buffer;
uniform vec3 lightPos;
uniform vec3 lightColor;

void main() {
  vec3 N = normalize(vNormal);
  vec3 L = normalize(lightPos - fragPos);
  vec3 V = normalize(camPos - fragPos);
  vec3 R = reflect(-L, N);

  vec3 ambient = ka * lightColor;
  vec3 diffuse = kd * max(dot(N, L), 0.0) * lightColor;
  vec3 specular = ks * pow(max(dot(R, V), 0.0), max(ns, 1.0)) * lightColor;

  vec4 texColor = texture(tex_buffer, texCoord);
  color = vec4((ambient + diffuse + specular) * vec3(texColor), texColor.a);
}
