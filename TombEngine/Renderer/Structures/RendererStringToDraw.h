#pragma once
#include <SimpleMath.h>

namespace TEN::Renderer
{
	struct RendererStringToDraw
	{
		std::shared_ptr<SpriteFont> Font;
		float X;
		float Y;
		int Flags;
		std::wstring String;
		Vector3 Color;
		float Scale;
	};
}