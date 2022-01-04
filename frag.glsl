#version 330 core

out vec4 FragColor;

uniform vec2 lowerLeft;
uniform vec2 upperRight;
uniform vec2 viewportDimensions;
uniform int maxIterations;
uniform sampler1D gradient;

vec3 hsv2rgb(vec3 c) {
  vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
  vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
  return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main() {
 vec2 c = mix(lowerLeft, upperRight, gl_FragCoord.xy / viewportDimensions.xy);
 float x0, y0, x, y, x2, y2;
 x = y = x2 = y2 = 0;
 x0 = c.x, y0 = c.y;
 float iteration = 0;
 while (x2 + y2 <= (1<<16) && iteration < maxIterations) {
   y = (x + x) * y + y0;
   x = x2 - y2 + x0;
   x2 = x * x;
   y2 = y * y;
   ++iteration;
 }
 if (iteration < maxIterations) {
   iteration += 1 - log(log(sqrt(x2 + y2))) / log(2.0f);
   //FragColor = vec4(hsv2rgb(vec3(iteration/float(maxIterations),1, 1)), 1);
   FragColor = texture(gradient, iteration/float(maxIterations));
 }  else {
   FragColor = vec4(0, 0, 0, 1);
 }
}