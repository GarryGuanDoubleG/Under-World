#pragma once


namespace MathUtil
{
	float lerp(float a, float b, float f);

	void AssimpToGLMMat4(const aiMatrix4x4 *from, glm::mat4 &to);
}
