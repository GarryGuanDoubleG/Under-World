#pragma once

class AnimController
{

public:
	vector<glm::mat4> m_boneTransforms; /**<container for joints used in skeletal animations*/
	vector<glm::mat4> m_finalTransforms;

	map<string, GLuint>m_boneMap; /**<map of bone ids*/
	GLuint m_boneCount;

private:
	string m_modelPath;

	float m_animationTime0;
	float m_animationTime1;

	float m_playTime;

	int m_numAnims;
	int m_currAnim;

	vector<Assimp::Importer *> m_importers;
	vector<const aiScene *> m_scenes;
	vector<glm::mat4> m_GlobalInverseTransforms;

	map<string, int> m_animIDMap;

public:
	AnimController();

	void SetAnimation(float currTime, const string &name);
	void BoneTransform(float time);
	void LoadAnimation(const string &filepath, const string &animName);

private:

	const aiNodeAnim* FindNodeAnim(const aiAnimation* anim, const string NodeName);

	void InterpolateScaling(aiVector3D &out, float animTime, const aiNodeAnim *animNode);
	void InterpolateRotation(aiQuaternion & out, float animTime, const aiNodeAnim* animNode);
	void InterpolatePosition(aiVector3D& out, float animTime, const aiNodeAnim* animNode);

	void CalculateHeiarchyTransform(float animTime, aiNode *node, glm::mat4 transform);

	//void CalculateHeiarchyTransform(const aiScene *scene0, const aiScene scene1, float animTime0, float animTime1, aiNode *node0, aiNode *node1, glm::mat4 parentTransform);
};