#pragma once


namespace vob::aoeph
{
	struct Material
	{
		float elasticityOver100 = 80'000'0.0f;
		float restitution = 0.01f;
		float friction = 0.5f;
	};
}
