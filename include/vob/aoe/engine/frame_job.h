#pragma once


namespace vob::aoeng
{
	struct VOB_AOE_API IFrameJob
	{
		virtual ~IFrameJob() = default;
		virtual void prepare() {}
		virtual void execute() = 0;
		virtual void cleanup() {}
	};
}
