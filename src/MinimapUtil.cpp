#include "MinimapUtil.h"

namespace Util
{
	double GetPercentage(RE::Actor* actor, RE::ActorValue valueID)
	{
		return (double)(actor->AsActorValueOwner()->GetActorValue(valueID) / actor->AsActorValueOwner()->GetBaseActorValue(valueID));
	}
}