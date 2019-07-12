#include "PCH.h"
#include "Renderer.h"
#include <chrono>

void CRenderer::Update()
{
	static std::chrono::high_resolution_clock clock;
	static auto t0 = clock.now();

	m_PSConstantBufferData.numFrames++;

	auto t1 = clock.now();
	auto deltaTime = t1 - t0;

	m_PSConstantBufferData.elapsedTime += deltaTime.count() * 1e-9;

	// Update for next frame
	t0 = t1;
}

void CRenderer::SetCursorPosition( unsigned int x, unsigned int y )
{
	m_PSConstantBufferData.cursorPos[0] = x;
	m_PSConstantBufferData.cursorPos[1] = y;
}
