#pragma once

#include "Misc/AutomationTest.h"
#include "CoreMinimal.h"

namespace EasySyncTest
{
	class FDelayedCallbackLatentCommand : public IAutomationLatentCommand
	{
	public:
		FDelayedCallbackLatentCommand(TFunction<void()>&& InCallback, float InDelay = 0.1f)
			: Callback(MoveTemp(InCallback)), Delay(InDelay)
		{
			//Set the start time
			StartTime = FPlatformTime::Seconds();
		}

		virtual bool Update() override
		{
			double NewTime = FPlatformTime::Seconds();
			if (NewTime - StartTime >= static_cast<double>(Delay))
			{
				Callback();
				return true;
			}
			return false;
		}

	private:
		TFunction<void()> Callback;
		float Delay;
	};
}
