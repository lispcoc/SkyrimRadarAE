#include "MinimapUtil.h"

namespace Util
{
	double GetPercentage(Actor* actor, UInt32 valueID)
	{
		return (double)(actor->actorValueOwner.GetCurrent(valueID) / actor->actorValueOwner.GetMaximum(valueID));
	}
}