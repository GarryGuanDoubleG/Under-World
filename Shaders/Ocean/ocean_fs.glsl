/*
 * Proland: a procedural landscape rendering library.
 * Copyright (c) 2008-2011 INRIA
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Proland is distributed under a dual-license scheme.
 * You can obtain a specific license from Inria: proland-licensing@inria.fr.
 */

/*
 * Authors: Eric Bruneton, Antoine Begault, Guillaume Piolat.
 */

/**
 * Real-time Realistic Ocean Lighting using Seamless Transitions from Geometry to BRDF
 * Copyright (c) 2009 INRIA
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * Author: Eric Bruneton
 */

#extension GL_EXT_gpu_shader4 : enable
#version 450 core

#pragma include "oceanCommon.inc.glsl"
#pragma include "Scattering/scatterFunctions.inc.glsl"
//#include "atmosphereShader.glhl"


in vec2 oceanU;
in vec3 oceanP;
layout(location=0) out vec4 data;

vec3 getWorldCameraPos();
vec3 getWorldSunDir();
vec3 hdr(vec3 L);

void sunRadianceAndSkyIrradiance(vec3 worldP, vec3 worldN, vec3 worldS, out vec3 sunL, out vec3 skyE)
{
    float r = length(worldP);
    if (r < 0.9 * PLANET_RADIUS) {
        worldP.z += PLANET_RADIUS;
        r = length(worldP);
    }
    vec3 worldV = worldP / r; // vertical vector
    float muS = dot(worldV, worldS);
    sunL = sunRadiance(r, muS);
    skyE = skyIrradiance(r, muS) * (1.0 + dot(worldV, worldN));
}

// ---------------------------------------------------------------------------
// REFLECTED SUN RADIANCE
// ---------------------------------------------------------------------------

// assumes x>0
float erfc(float x) {
    return 2.0 * exp(-x * x) / (2.319 * x + sqrt(4.0 + 1.52 * x * x));
}

float Lambda(float cosTheta, float sigmaSq) {
    float v = cosTheta / sqrt((1.0 - cosTheta * cosTheta) * (2.0 * sigmaSq));
    return max(0.0, (exp(-v * v) - v * sqrt(PI) * erfc(v)) / (2.0 * v * sqrt(PI)));
    //return (exp(-v * v)) / (2.0 * v * sqrt(PI));
}

// L, V, N in world space
float reflectedSunRadiance(vec3 L, vec3 V, vec3 N, float sigmaSq) {
    vec3 H = normalize(L + V);
    vec3 Ty = normalize(cross(N, vec3(1.0, 0.0, 0.0)));
    vec3 Tx = cross(Ty, N);
    float zetax = dot(H, Tx) / dot(H, N);
    float zetay = dot(H, Ty) / dot(H, N);

    float zL = dot(L, N); // cos of source zenith angle
    float zV = dot(V, N); // cos of receiver zenith angle
    float zH = dot(H, N); // cos of facet normal zenith angle
    float zH2 = zH * zH;

    float p = exp(-0.5 * (zetax * zetax + zetay * zetay) / sigmaSq) / (2.0 * PI * sigmaSq);

    float fresnel = 0.02 + 0.98 * pow(1.0 - dot(V, H), 5.0);

    zL = max(zL,0.01);
    zV = max(zV,0.01);

    return fresnel * p / ((1.0 + Lambda(zL, sigmaSq) + Lambda(zV, sigmaSq)) * zV * zH2 * zH2 * 4.0);
}

// L, V, N in world space
float wardReflectedSunRadiance(vec3 L, vec3 V, vec3 N, float sigmaSq) {
    vec3 H = normalize(L + V);

    float hn = dot(H, N);
    float p = exp(-2.0 * ((1.0 - hn * hn) / sigmaSq) / (1.0 + hn)) / (4.0 * PI * sigmaSq);

    float c = 1.0 - dot(V, H);
    float c2 = c * c;
    float fresnel = 0.02 + 0.98 * c2 * c2 * c;

    float zL = dot(L, N);
    float zV = dot(V, N);
    zL = max(zL,0.01);
    zV = max(zV,0.01);

    // brdf times cos(thetaL)
    return zL <= 0.0 ? 0.0 : max(fresnel * p * sqrt(abs(zL / zV)), 0.0);
}

// ---------------------------------------------------------------------------
// REFLECTED SKY RADIANCE
// ---------------------------------------------------------------------------

uniform sampler2DArray skymapSampler;

vec3 skymapRadiance(vec2 tau, vec2 dtaudx, vec2 dtaudy, vec3 sunDir)
{
    vec2 sun = normalize(sunDir.xy);
    mat2 rot = mat2(sun.x, -sun.y, sun.y, sun.x);
    vec2 uv = vec2(0.5 / 255.0)+ ((rot * tau) * 0.5 / 1.1 + 0.5) * (255.0 / 256.0);
    vec2 duvdx = rot * dtaudx;
    vec2 duvdy = rot * dtaudy;

    float l = acos(sunDir.z) * (28.0 / (PI / 2.0));
    float layer = floor(l);
    float a = l - layer;
    vec3 l0 = texture2DArrayGrad(skymapSampler, vec3(uv, layer), duvdx, duvdy).rgb;
    vec3 l1 = texture2DArrayGrad(skymapSampler, vec3(uv, layer + 1.0), duvdx, duvdy).rgb;
    return (l0 * (1.0 -  a) + l1 * a) * 100.0;
}

float meanFresnel(float cosThetaV, float sigmaV) {
    return pow(1.0 - cosThetaV, 5.0 * exp(-2.69 * sigmaV)) / (1.0 + 22.7 * pow(sigmaV, 1.5));
}

float meanFresnel(vec3 V, vec3 N, float sigmaSq) {
    return meanFresnel(dot(V, N), sqrt(sigmaSq));
}

vec2 U(vec2 zeta, vec3 V) {
    vec3 F = normalize(vec3(-zeta, 1.0));
    vec3 R = 2.0 * dot(F, V) * F - V;
    return -R.xy / (1.0 + R.z);
}

// V, N, sunDir in world space
vec3 reflectedSkyRadiance(vec3 V, vec3 N, float sigmaSq, vec3 sunDir) {
    vec3 result = vec3(0.0);

    vec2 zeta0 = - N.xy / N.z;
    vec2 tau0 = U(zeta0, V);

    const float n = 1.0 / 1.1;
    vec2 JX = (U(zeta0 + vec2(0.01, 0.0), V) - tau0) / 0.01 * n * sqrt(sigmaSq);
    vec2 JY = (U(zeta0 + vec2(0.0, 0.01), V) - tau0) / 0.01 * n * sqrt(sigmaSq);

    result = skymapRadiance(tau0, JX, JY, sunDir);

    result *= 0.02 + 0.98 * meanFresnel(V, N, sigmaSq);
    return result;
}

vec3 reflectedSkyRadianceW(vec3 V, vec3 N, float sigmaSq, vec3 sunDir) {
    vec3 NX = normalize(vec3(-N.y, N.x, 0.0));
    vec3 NY = cross(N, NX);
    vec3 tV = vec3(dot(V, NX), dot(V, NY), dot(V, N));
    vec3 tSun = vec3(dot(sunDir, NX), dot(sunDir, NY), dot(sunDir, N));
    return reflectedSkyRadiance(tV, vec3(0.0, 0.0, 1.0), sigmaSq, tSun);
}

// ---------------------------------------------------------------------------
// REFRACTED SEA RADIANCE
// ---------------------------------------------------------------------------

float refractedSeaRadiance(vec3 V, vec3 N, float sigmaSq) {
    return 0.98 * (1.0 - meanFresnel(V, N, sigmaSq));
}

void main() {
    vec3 WSD = getWorldSunDir();
    vec3 WCP = getWorldCameraPos();
    vec2 u = oceanU;

    vec3 earthCamera = vec3(0.0, 0.0, oceanCameraPos.z + radius);
    vec3 earthP = radius > 0.0 ? normalize(oceanP + vec3(0.0, 0.0, radius)) * (radius + 10.0) : oceanP;

    vec3 oceanCamera = vec3(0.0, 0.0, oceanCameraPos.z);
    vec3 V = normalize(oceanCamera - oceanP);

    vec2 slopes = texture2DArray(fftWavesSampler, vec3(u / GRID_SIZES.x, 1.0)).xy;
    slopes += texture2DArray(fftWavesSampler, vec3(u / GRID_SIZES.y, 1.0)).zw;
    slopes += texture2DArray(fftWavesSampler, vec3(u / GRID_SIZES.z, 2.0)).xy;
    slopes += texture2DArray(fftWavesSampler, vec3(u / GRID_SIZES.w, 2.0)).zw;
    if (radius > 0.0) {
        slopes -= oceanP.xy / (radius + oceanP.z);
    }
    vec3 N = normalize(vec3(-slopes.x, -slopes.y, 1.0));
    if (dot(V, N) < 0.0) {
        N = reflect(N, V); // reflects backfacing normals
    }

    float Jxx = dFdx(u.x);
    float Jxy = dFdy(u.x);
    float Jyx = dFdx(u.y);
    float Jyy = dFdy(u.y);
    float A = Jxx * Jxx + Jyx * Jyx;
    float B = Jxx * Jxy + Jyx * Jyy;
    float C = Jxy * Jxy + Jyy * Jyy;
    const float SCALE = 10.0;
    float ua = pow(A / SCALE, 0.25);
    float ub = 0.5 + 0.5 * B / sqrt(A * C);
    float uc = pow(C / SCALE, 0.25);
    float sigmaSq = texture(slopeVarianceSampler, vec3(ua, ub, uc)).x;

    sigmaSq = max(sigmaSq, 2e-5);

    vec3 sunL;
    vec3 skyE;
    vec3 extinction;
    sunRadianceAndSkyIrradiance(earthP, N, oceanSunDir, sunL, skyE);

    //vec3 Lsun = reflectedSunRadiance(oceanSunDir, V, N, sigmaSq) * sunL;
    //vec3 Lsky = reflectedSkyRadiance(V, N, sigmaSq, oceanSunDir);
    vec3 Lsun = wardReflectedSunRadiance(oceanSunDir, V, N, sigmaSq) * sunL;
    vec3 Lsky = meanFresnel(V, N, sigmaSq) * skyE / PI;
    vec3 Lsea = refractedSeaRadiance(V, N, sigmaSq) * seaColor * skyE / PI;

    vec3 surfaceColor = Lsun + Lsky + Lsea;

    // aerial perspective
    vec3 inscatter = inScattering(earthCamera, earthP, oceanSunDir, extinction, 0.0);
    vec3 finalColor = surfaceColor * extinction + inscatter;

    data.rgb = hdr(finalColor);
    data.a = 1.0;
}
