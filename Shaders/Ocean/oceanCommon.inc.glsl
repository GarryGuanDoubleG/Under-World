
uniform float radius;
uniform mat4 cameraToOcean;
uniform mat4 screenToCamera;
uniform mat4 cameraToScreen;
uniform mat3 oceanToCamera;
uniform mat4 oceanToWorld;
uniform vec3 oceanCameraPos;
uniform vec3 oceanSunDir;
uniform vec3 horizon1;
uniform vec3 horizon2;
uniform vec2 gridSize;
uniform vec4 GRID_SIZES;

uniform sampler2DArray fftWavesSampler;
uniform sampler3D slopeVarianceSampler;
uniform float heightOffset;

uniform vec3 seaColor; // sea bottom color

const float PI = 3.141592657;
const float CHOPPY_FACTOR = 5.0;