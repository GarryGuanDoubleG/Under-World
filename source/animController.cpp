#include "game.hpp"


AnimController::AnimController()
	: m_numAnims(0)
{

}

void AnimController::LoadAnimation(const string &filepath, const string &animName)
{
	m_importers.push_back(new Assimp::Importer);
	m_scenes.push_back(m_importers[m_numAnims]->ReadFile(filepath, aiProcessPreset_TargetRealtime_Quality));

	glm::mat4 globalInverseTrans(1.0f);

	MathUtil::AssimpToGLMMat4(&m_scenes[m_numAnims]->mRootNode->mTransformation.Inverse(), globalInverseTrans);
	m_GlobalInverseTransforms.push_back(globalInverseTrans);

	m_animIDMap.insert(pair<string, int>(animName, m_numAnims));

	m_numAnims++;
}


void AnimController::BoneTransform(float timeInSecs)
{
	glm::mat4 identity(1.0f);

	//float deltaTime = timeInSecs - m_playTime;
	//assimp is row major, glm column major
	float ticksPerSecond = m_scenes[m_currAnim]->mAnimations[0]->mTicksPerSecond;
	float timeInTicks = timeInSecs * ticksPerSecond;
	float animTime = fmod(timeInTicks, m_scenes[m_currAnim]->mAnimations[0]->mDuration);

	m_finalTransforms.resize(m_boneTransforms.size());

	CalculateHeiarchyTransform(animTime, m_scenes[m_currAnim]->mRootNode, identity);
}

const aiNodeAnim* AnimController::FindNodeAnim(const aiAnimation* anim, const string NodeName)
{
	for (int i = 0; i < anim->mNumChannels; i++) {
		const aiNodeAnim* animNode = anim->mChannels[i];

		if(string(animNode->mNodeName.data) == NodeName) {
			return animNode;
		}
	}

	return NULL;
}

void AnimController::InterpolateScaling(aiVector3D &out, float animTime, const aiNodeAnim *animNode)
{
	if(animNode->mNumScalingKeys == 0)
	{
		out = aiVector3D(1, 1, 1);
		return;
	}
	if(animNode->mNumScalingKeys == 1) {
		out = animNode->mScalingKeys[0].mValue;
		return;
	}

	int scalingIndex = animNode->mNumScalingKeys - 2;
	int nextScalingIndex = 1;

	for (int i = 0; i < animNode->mNumScalingKeys - 1; i++)
	{
		if(animTime < (float)animNode->mScalingKeys[i + 1].mTime)
		{
			scalingIndex = i;
			break;
		}
	}


	nextScalingIndex = scalingIndex + 1;

	float delaTime = (float)(animNode->mScalingKeys[nextScalingIndex].mTime - animNode->mScalingKeys[scalingIndex].mTime);
	float factor = (animTime - (float)animNode->mScalingKeys[scalingIndex].mTime) / delaTime;

	const aiVector3D& Start = animNode->mScalingKeys[scalingIndex].mValue;
	const aiVector3D& End = animNode->mScalingKeys[nextScalingIndex].mValue;

	aiVector3D Delta = End - Start;

	out = Start + factor * Delta;
}

void AnimController::InterpolateRotation(aiQuaternion & out, float animTime, const aiNodeAnim* animNode)
{
	if(animNode->mNumRotationKeys == 0)
	{
		cout << "num keys = 0\n";
		assert(0);
		return;
	}


	// we need at least two values to interpolate...
	if(animNode->mNumRotationKeys == 1) {
		out = animNode->mRotationKeys[0].mValue;
		return;
	}

	int rotationIndex;
	int nextRotationIndex;

	for (int i = 0; i < animNode->mNumRotationKeys - 1; i++)
	{
		if(animTime < (float)animNode->mRotationKeys[i + 1].mTime) {
			rotationIndex = i;
			break;
		}
	}

	nextRotationIndex = (rotationIndex + 1);

	float delaTime = (float)(animNode->mRotationKeys[nextRotationIndex].mTime - animNode->mRotationKeys[rotationIndex].mTime);
	float factor = (animTime - (float)animNode->mRotationKeys[rotationIndex].mTime) / delaTime;

	const aiQuaternion& StartRotationQ = animNode->mRotationKeys[rotationIndex].mValue;
	const aiQuaternion& EndRotationQ = animNode->mRotationKeys[nextRotationIndex].mValue;

	aiQuaternion::Interpolate(out, StartRotationQ, EndRotationQ, factor);
	out = out.Normalize();
}

void AnimController::InterpolatePosition(aiVector3D& out, float animTime, const aiNodeAnim* animNode)
{
	if(animNode->mNumPositionKeys == 0)
	{
		assert(0);
		return;
	}
	if(animNode->mNumPositionKeys == 1) {
		out = animNode->mPositionKeys[0].mValue;
		return;
	}

	int positionIndex = animNode->mNumPositionKeys - 2;

	for (int i = 0; i < animNode->mNumPositionKeys - 1; i++) {

		if(animTime < animNode->mPositionKeys[i + 1].mTime) {
			positionIndex = i;
			break;
		}
	}

	int nextPositionIndex = (positionIndex + 1);

	float delaTime = (float)(animNode->mPositionKeys[nextPositionIndex].mTime - animNode->mPositionKeys[positionIndex].mTime);
	float factor = (animTime - (float)animNode->mPositionKeys[positionIndex].mTime) / delaTime;

	const aiVector3D& start = animNode->mPositionKeys[positionIndex].mValue;
	const aiVector3D& end = animNode->mPositionKeys[nextPositionIndex].mValue;

	aiVector3D delta = end - start;
	out = start + factor * delta;
}


void AnimController::CalculateHeiarchyTransform(float animTime, aiNode *node, glm::mat4 parentTransform)
{
	string nodeName = node->mName.data;

	const aiAnimation *animation = m_scenes[m_currAnim]->mAnimations[0];
	const aiNodeAnim *animNode = FindNodeAnim(animation, nodeName);

	glm::mat4 nodeTransform(1.0f);

	if(animNode)
	{
		aiVector3D scale;
		InterpolateScaling(scale, animTime, animNode);

		aiQuaternion rot;
		InterpolateRotation(rot, animTime, animNode);
		glm::quat rotQuat(rot.w, rot.x, rot.y, rot.z);


		aiVector3D translation;
		InterpolatePosition(translation, animTime, animNode);

		glm::mat4 translate(1.0f);
		translate = glm::translate(translate, glm::vec3(translation.x, translation.y, translation.z));

		nodeTransform = glm::scale(glm::mat4(1.0f), glm::vec3(scale.x, scale.y, scale.z))
			* glm::toMat4(rotQuat)
			* translate;
	}
	else
	{
		MathUtil::AssimpToGLMMat4(&node->mTransformation, nodeTransform);
	}

	glm::mat4 globalTransform = parentTransform *nodeTransform;

	if(m_boneMap.find(nodeName) != m_boneMap.end())
	{
		int boneIndex = m_boneMap[nodeName];
		m_finalTransforms[boneIndex] = m_GlobalInverseTransforms[m_currAnim] * globalTransform * m_boneTransforms[boneIndex];
	}

	for (int i = 0; i < node->mNumChildren; i++) {
		CalculateHeiarchyTransform(animTime, node->mChildren[i], globalTransform);
	}
}

void AnimController::SetAnimation(float currTime, const string &name)
{
	m_playTime = currTime;
	m_currAnim = m_animIDMap[name];

}