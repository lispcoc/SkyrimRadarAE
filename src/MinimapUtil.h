#pragma once

namespace Util
{
	double GetPercentage(RE::Actor* actor, RE::ActorValue valueID);

	template<typename T>
	void ProperArray(RE::BSTArray<T>* _array, T* dst)
	{
		std::uint32_t len = _array->size(), i;

		for (i = 0; i < len; ++i)
		{
			dst[i] = *_array[i].data();
		}
	}
}