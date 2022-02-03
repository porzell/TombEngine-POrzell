#pragma once
#include <vector>
#include <SimpleMath.h>

struct MESH_INFO;

namespace TEN::Renderer
{
	struct RendererItem;
	struct RendererBucket;
	struct RendererLight;
	struct RendererEffect;
	struct RendererTransparentFace;

	struct RendererRoom
	{
		bool Visited;
		int Distance;
		short RoomNumber;
		Vector4 AmbientLight;
		std::vector<RendererBucket> Buckets;
		std::vector<RendererLight> Lights;
		std::vector<RendererItem*> ItemsToDraw;
		std::vector<RendererEffect*> EffectsToDraw;
		std::vector<MESH_INFO*> StaticsToDraw;
		std::vector<RendererTransparentFace> TransparentFacesToDraw;
	};
}
