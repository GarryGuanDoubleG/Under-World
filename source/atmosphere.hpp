#pragma once

class Atmosphere
{
	int m_skyW, m_skyH;
	int m_transW, m_transH;
	int m_inscatterW, m_inscatterH, m_inscatterD;

	Texture *irradiance;
	Texture *inscatter;
	Texture *transmittance;

	GLuint quadVao;
	GLuint m_FBO;
	Texture m_outputTex;
public:
	Atmosphere();
	Atmosphere(GLuint quadVao);

	void Precompute();
	void PrecomputeTransmittance();
	void PrecomputeDeltaE(Texture & deltaE);
	void PrecomputeDeltaSMSR(Texture & deltaSR, Texture & deltaSM);
	void CopyIrradiance(Texture & deltaE);
	void CopyInscatter(Texture & deltaSM, Texture & deltaSR);
	void PrecomputeDeltaJ(Texture & deltaJ, Texture & deltaSR, Texture & deltaSM, Texture & deltaE, GLboolean first);
	void PrecomputeIrradiance_N(Texture & deltaJ, Texture & deltaSR, Texture & deltaSM, Texture & deltaE, GLboolean first);
	void PrecomputeDeltaSR(Texture & deltaSR, Texture & deltaJ);
	void AddDeltas(Texture & deltaE, Texture & deltaSR);

	void RenderToQuad();
	Texture Render(DeferredBuffer & gbuffer, Texture *scene);
private:

};