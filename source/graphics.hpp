#pragma once
#include "SDL.h"

class Graphics
{
public:
	Graphics(int winWidth, int winHeight);

	bool InitializeGraphics(int winWidth, int winHeight);

	void Render();
	void Cleanup();
private:
	void CheckSDLError(int line = -1);
};