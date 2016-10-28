#version 330
uniform mat4 uProjectionMatrix;
uniform mat4 uModelviewMatrix;
uniform vec2 uZRange;
in vec3 ivPosition;
in vec3 ivNormal;
in vec3 ivInstanceOffset;
in vec3 ivInstanceDirection;
out vec3 vfPosition;
out vec3 vfNormal;
out vec3 vfColor;

mat3 matrixFromDirection(vec3 direction) {
  float c = direction.z;
  float s = length(direction.xy);
  if (s == 0.0) {
    s = 1.0;
  }
  float x = -direction.y / s;
  float y = direction.x / s;
  mat3 matrix;
  matrix[0][0] = x*x*(1.0-c)+c;
  matrix[0][1] = y*x*(1.0-c);
  matrix[0][2] = -y*s;
  matrix[1][0] = x*y*(1.0-c);
  matrix[1][1] = y*y*(1.0-c)+c;
  matrix[1][2] = x*s;
  matrix[2][0] = y*s;
  matrix[2][1] = -x*s;
  matrix[2][2] = c;
  return matrix;
}

vec3 colormap(vec3 direction);

void main(void) {
  vfColor = colormap(normalize(ivInstanceDirection));
  mat3 instanceMatrix = matrixFromDirection(ivInstanceDirection);
  vfNormal = (uModelviewMatrix * vec4(instanceMatrix*ivNormal, 0.0)).xyz;
  vfPosition = (uModelviewMatrix * vec4(instanceMatrix*ivPosition+ivInstanceOffset, 1.0)).xyz;
  if (ivInstanceDirection.z >= uZRange.x && ivInstanceDirection.z <= uZRange.y) {
    gl_Position = uProjectionMatrix * vec4(vfPosition, 1.0);
  } else {
    gl_Position = vec4(2.0, 2.0, 2.0, 0.0);
  }
}
