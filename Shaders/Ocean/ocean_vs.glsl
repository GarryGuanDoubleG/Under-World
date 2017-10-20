#version 450 core

#pragma include "oceanCommon.inc.glsl"
#pragma include "Scattering/scatterFunctions.inc.glsl"

layout(location=0) in vec3 vertex;
out float oceanTIn;
out vec2 oceanU; // coordinates in ocean space used to compute P(u)
out vec3 oceanP; // wave point P(u) in ocean space

vec2 oceanPos(vec3 vertex, out float t, out vec3 cameraDir, out vec3 oceanDir) {
    float horizon = horizon1.x + horizon1.y * vertex.x - sqrt(horizon2.x + (horizon2.y + horizon2.z * vertex.x) * vertex.x);
    cameraDir = normalize((screenToCamera * vec4(vertex.x, min(vertex.y, horizon), 0.0, 1.0)).xyz);
    oceanDir = (cameraToOcean * vec4(cameraDir, 0.0)).xyz;
    float cz = oceanCameraPos.z;
    float dz = oceanDir.z;
    if (radius == 0.0) {
        t = (heightOffset + 5.0 - cz) / dz;
    } else {
        float b = dz * (cz + radius);
        float c = cz * (cz + 2.0 * radius);
        float tSphere = - b - sqrt(max(b * b - c, 0.0));
        float tApprox = - cz / dz * (1.0 + cz / (2.0 * radius) * (1.0 - dz * dz));
        t = abs((tApprox - tSphere) * dz) < 1.0 ? tApprox : tSphere;
    }
    return oceanCameraPos.xy + t * oceanDir.xy;
}

vec2 oceanPos(vec3 vertex) {
    float t;
    vec3 cameraDir;
    vec3 oceanDir;
    return oceanPos(vertex, t, cameraDir, oceanDir);
}

void main() {
    float t;
    vec3 cameraDir;
    vec3 oceanDir;
    vec2 u = oceanPos(vertex, t, cameraDir, oceanDir);
    vec2 dux = oceanPos(vertex + vec3(gridSize.x, 0.0, 0.0)) - u;
    vec2 duy = oceanPos(vertex + vec3(0.0, gridSize.y, 0.0)) - u;
    vec3 dP = vec3(0.0, 0.0, heightOffset + (radius > 0.0 ? 0.0 : 5.0));
    if (duy.x != 0.0 || duy.y != 0.0) {
        dP.z += texture2DArrayGrad(fftWavesSampler, vec3(u / GRID_SIZES.x, 0.0), dux / GRID_SIZES.x, duy / GRID_SIZES.x).x;
        dP.z += texture2DArrayGrad(fftWavesSampler, vec3(u / GRID_SIZES.y, 0.0), dux / GRID_SIZES.y, duy / GRID_SIZES.y).y;
        dP.z += texture2DArrayGrad(fftWavesSampler, vec3(u / GRID_SIZES.z, 0.0), dux / GRID_SIZES.z, duy / GRID_SIZES.z).z;
        dP.z += texture2DArrayGrad(fftWavesSampler, vec3(u / GRID_SIZES.w, 0.0), dux / GRID_SIZES.w, duy / GRID_SIZES.w).w;
        dP.xy += CHOPPY_FACTOR * texture2DArrayGrad(fftWavesSampler, vec3(u / GRID_SIZES.x, 3.0), dux / GRID_SIZES.x, duy / GRID_SIZES.x).xy;
        dP.xy += CHOPPY_FACTOR * texture2DArrayGrad(fftWavesSampler, vec3(u / GRID_SIZES.y, 3.0), dux / GRID_SIZES.y, duy / GRID_SIZES.y).zw;
        dP.xy += CHOPPY_FACTOR * texture2DArrayGrad(fftWavesSampler, vec3(u / GRID_SIZES.z, 4.0), dux / GRID_SIZES.z, duy / GRID_SIZES.z).xy;
        dP.xy += CHOPPY_FACTOR * texture2DArrayGrad(fftWavesSampler, vec3(u / GRID_SIZES.w, 4.0), dux / GRID_SIZES.w, duy / GRID_SIZES.w).zw;
    }
    gl_Position = cameraToScreen * vec4(t * cameraDir + oceanToCamera * dP, 1.0);
    oceanU = u;
    oceanP = vec3(0.0, 0.0, oceanCameraPos.z) + t * oceanDir + dP;
}

