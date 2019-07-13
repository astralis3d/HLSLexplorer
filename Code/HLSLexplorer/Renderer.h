#pragma once

#include "IRenderer.h"

class CRenderer : public IRenderer
{
public:
	CRenderer() {}
	virtual ~CRenderer() { }

	virtual void Update();
	virtual void ResizeViewport(unsigned int newWidth, unsigned int newHeight);
	virtual void SetCursorPosition( unsigned int x, unsigned int y );

	// constant buffer
	struct SConstantBuffer
	{
		SConstantBuffer()
		{
			elapsedTime = 0.0f;
			numFrames = 0;
		}

		float elapsedTime;
		unsigned int numFrames;
		unsigned cursorPos[2];

		float viewportX;
		float viewportY;
		float viewportInvX;
		float viewportInvY;
	};

protected:
	SConstantBuffer m_PSConstantBufferData;
};