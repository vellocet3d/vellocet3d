#pragma once


namespace vel
{
	struct VertexBoneData
	{
		unsigned int	ids[8] = {0,0,0,0,0,0,0,0}; // 8 bones allowed per vertex
		float			weights[8] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
	};
}