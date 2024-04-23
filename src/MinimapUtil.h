#pragma once
#include "skse64/GameData.h"
#include "skse64/GameReferences.h"
#include "skse64/GameForms.h"
#include "skse64/PapyrusArgs.h"

namespace Util
{
	double GetPercentage(Actor* actor, UInt32 valueID);

	template<typename T>
	void ProperArray(VMArray<T>* _array, T* dst)
	{
		UInt32 len = _array->Length(), i;

		for (i = 0; i < len; ++i)
		{
			_array->Get(&dst[i], i);
		}
	}
}