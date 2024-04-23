#pragma once

namespace Util
{
	double GetPercentage(RE::Actor* actor, RE::ActorValue valueID);

	template<typename T>
	void ProperArray(RE::BSTArray<T>* _array, T* dst)
	{
		std::uint32_t len = _array->Length(), i;

		for (i = 0; i < len; ++i)
		{
			_array->Get(&dst[i], i);
		}
	}
}