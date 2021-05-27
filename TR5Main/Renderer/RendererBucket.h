#pragma once
#include "RenderEnums.h"
#include "RendererVertex.h"
#include "RendererPolygon.h"
#include <vector>
namespace T5M::Renderer {
	struct RendererBucket {
		size_t texture;
		bool animated;
		BLEND_MODES blendMode;
		std::vector<RendererVertex> Vertices;
		std::vector<int> Indices;
		std::vector<RendererPolygon> Polygons;
		int StartVertex;
		int StartIndex;

	};
}