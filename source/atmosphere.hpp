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

	//imgui values
	//uniform shader values
	float m_HR;
	glm::vec3 m_betaR;
	
	//mie factors
	float m_HM;
	glm::vec3 m_betaMSca;
	float m_mieG;
public:

	bool m_showImGUI;
public:
	Atmosphere();
	Atmosphere(GLuint quadVao);

	void SetUniforms(Shader *shader);

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
	void RenderImGUI();
	void BindScatteringTextures(Shader * shader);
	Texture Render(DeferredBuffer & gbuffer, Texture *scene);
private:

};