#include "Minimap.h"

using namespace RE;

#define DYNAMIC_CAST(a, b, c) dynamic_cast<c*>(dynamic_cast<b*>(a))

namespace Minimap
{
	std::uint32_t factionIDs[] = { 6, 17, 9, 9, 7, 8, 3, 10, 11, 21, 20, 27, 25, 26, 28, 23, 24, 29, 0, 0, 22 };

	int numQueuedAdds;
	std::vector<RE::TESFaction*> factions;
	std::vector<TrackedType*> tracked;
	std::vector<MapTween> all_active_tweens;
	std::vector<int> queuedRemovals;
	std::vector<std::uint32_t> trackedIDs;
	std::vector<SetStruct> queuedSets;

	BSSpinLock          arrayLock, setLock, tweenLock;
	GFxValue*			widget = NULL;
	GFxValue			widgetValue;
	TrackedType*		lastActor;
	signed char queuedVisibility = -1;
	bool	queuedClear = false;
	bool	queuedUpdate = false;
	bool	queuedAppear = false;
	Actor*	playerHorse = NULL;
	std::uint32_t	lastFactionID;

	float	rwidth;
	float	height_to_width_scale;
	bool	showBars = false;
	bool	isCircle = true;
	bool	turnedOn = true;
	bool	allowInside = true;
	bool	playerIsInside = false;
	float	checkAbove;
	float	checkBelow;
	std::uint32_t	maxActors = 128;
	bool*	toggles;
	float*	floats;

	float	  zoomProgress;
	ZoomState zoomState;
	float     fadeProgress;
	FadeState minimapState;

	float setting_fade_in_speed = 1 / 1.25f;
	float setting_fade_out_speed = 1 / 1.25f;
	float setting_visible_speed = 1 / 10.8f;
	float setting_attribute_threshold = -0.1f;
	bool  setting_context_fade_enabled = false;
	float inside_radius = -1.0f;
	float radius_extra;
	float inside_fade_threshold = 0.7f;

	float	 actualWidth;
	float	 actualHeight;
	double	 actualScale;
	std::uint32_t	 barColor;

	GFxValue actors;
	GFxValue player_arrow;
	GFxValue north;
	GFxValue* north_indicator;

	TESNPC* ToNPC(Actor* actor)
	{
		if (!actor)
		{
			return NULL;
		}

		TESNPC* result = dynamic_cast<TESNPC*>(dynamic_cast<TESActorBase*>(actor));

		if (!result)
		{
			_DMESSAGE("(ActorBase -> NPC) conversion failed.");
			return NULL;
		}

		return result;
	}

	bool Add(StaticFunctionTag *, Actor* actor)
	{
		if (!actor)
		{
			_DMESSAGE("ADD: Actor is null!");
			return false;
		}

		TESObjectREFR* object = dynamic_cast<RE::TESObjectREFR*>(actor);

		if (!object)
		{
			_DMESSAGE("ADD: Failed (Actor -> object) conversion!");
			return false;
		}

		arrayLock.Lock();

		if (tracked.size() >= maxActors)
		{
			arrayLock.Unlock();
			return false;
		}

		tracked.push_back(object);
		trackedIDs.push_back(0); // Synchronous.
		++numQueuedAdds;

		if (MINIMAP_DEBUG)
		{
			TESNPC* npc = dynamic_cast<TESNPC*>(dynamic_cast<TESForm*>(actor));

			if (!npc)
			{
				_DMESSAGE("ADD: Failed (Actor -> NPC) conversion!");
			}
			else
			{
				_DMESSAGE("Add: %s", npc->fullName.data());
			}
		}

		arrayLock.Unlock();
		return true;
	}

	std::int32_t Remove(RE::StaticFunctionTag *, RE::Actor* actor)
	{
		if (!actor)
		{
			_DMESSAGE("REMOVE: Actor is null!");
			return -1;
		}

		TESObjectREFR* object = DYNAMIC_CAST(actor, Actor, TESObjectREFR);

		if (!object)
		{
			_DMESSAGE("REMOVE: Failed (Actor -> Object) conversion!");
			return -1;
		}

		std::int32_t index = -1, i;
		arrayLock.Lock();
		size_t len = tracked.size();

		for (i = 0; i < len; ++i)
		{
			if (tracked.at(i) == object)
			{
				if (MINIMAP_DEBUG)
				{
					TESNPC* npc = DYNAMIC_CAST(tracked.at(i), TESForm, TESNPC);
					RE::ConsoleLog::GetSingleton()->Print("Remove: %s", npc->fullName.data());
				}

				tracked.erase(tracked.begin() + i);
				trackedIDs.erase(trackedIDs.begin() + i);
				index = i;
				break;
			}
		}

		if (index >= 0)
		{
			queuedRemovals.push_back(index);
		}

		arrayLock.Unlock();
		return index;
	}

	void Clear(StaticFunctionTag *)
	{
		arrayLock.Lock();

		if (!widget)
		{
			arrayLock.Unlock();
			return;
		}

		widget->Invoke("clear", NULL, NULL, 0);
		tracked.clear();
		all_active_tweens.clear();
		trackedIDs.clear();
		queuedRemovals.clear();
		numQueuedAdds = 0;

		minimapState = VISIBLE;
		fadeProgress = 0.0f;

		setLock.Lock();
		queuedSets.clear();
		setLock.Unlock();

		arrayLock.Unlock();
	}

	void Set(StaticFunctionTag *, Actor* actor, std::uint32_t ID)
	{
		/*if (!actor)
		{
			_DMESSAGE("SET: Failed (Actor -> Object) conversion!");
			return;
		}*/

		setLock.Lock();
		TESObjectREFR* object = DYNAMIC_CAST(actor, Actor, TESObjectREFR);

		if (!object)
		{
			_DMESSAGE("SET: Failed (Actor -> Object) conversion!");
			return;
		}

		if (MINIMAP_DEBUG)
		{
			TESNPC* npc = DYNAMIC_CAST(object, TESForm, TESNPC);
			RE::ConsoleLog::GetSingleton()->Print("Set: %s", npc->fullName.data());
		}

		queuedSets.push_back(SetStruct{ object, ID });
		setLock.Unlock();
	}

	std::uint32_t GetIconFiltered(Actor* actor)
	{
		return FilterIcon(GetIcon(actor), actor);
	}

	std::uint32_t FilterIcon(std::uint32_t icon, Actor* actor)
	{
		if (icon == ICON_HIDDEN)
		{
			return icon;
		}

		static std::uint32_t list[] = { 99, 99, 2, 6, 4, 0, 14, 1, 5, 12, 17, 7, 8, 13, 21, 23, 22, 24, 99, 99, 99, 99, 99, 3, 10, 11, 20, 27, 25, 26, 28, 29, 0, 0, 30 };
		static std::uint32_t len = arrsize(list), i;

		for (i = 2; i < len; ++i)
		{
			if (i == 18)
			{
				i = 22; // 23 - 1
				continue;
			}

			if (toggles[i] == false && list[i] == icon)
			{
				return ICON_HIDDEN;
			}
		}

		// The horse icon.
		if (icon == 17)
		{
			TESNPC* npc = ToNPC(actor);

			if (npc)
			{
				std::string name(npc->fullName.data());

				// Frost has a unique icon.
				if (name.find("Frost") == 0)
				{
					return 18;
				}
			}
		}

		if (toggles[kType_SimpleMode] &&
			(icon == 0 || (icon != 4 && icon != 1 && icon != 6 && icon != VICTIM && icon != CORPSE)))
		{
			return 0;
		}

		return icon;
	}

	std::uint32_t GetIconFiltered(TESObjectREFR* obj)
	{
		Actor* object = DYNAMIC_CAST(obj, TESObjectREFR, Actor);
		return GetIconFiltered(object);
	}

	std::uint32_t GetIcon(TESObjectREFR* obj)
	{
		Actor* object = DYNAMIC_CAST(obj, TESObjectREFR, Actor);
		return GetIcon(object);
	}

	std::uint32_t GetIcon(Actor* actor)
	{
		static TESForm* playerForm = DYNAMIC_CAST(PlayerCharacter::GetSingleton(), PlayerCharacter, TESForm);
		static Actor*   playerActor = DYNAMIC_CAST(PlayerCharacter::GetSingleton(), PlayerCharacter, Actor);

		if (!actor)
		{
			_DMESSAGE("Actor for GetIcon() is null!");
			return ICON_HIDDEN;
		}

		std::uint32_t result = 0;
		//TESForm* actorForm = DYNAMIC_CAST(actor, Actor, TESForm);
		TESNPC*  actorNPC = ToNPC(actor);

		if (!actorNPC)
		{
			return ICON_HIDDEN;
		}

		std::string name(actorNPC->fullName.data());
		stl::enumeration<BGSRelationship::RELATIONSHIP_LEVEL, std::uint8_t>  rank = BGSRelationship::GetRelationship(actorNPC, dynamic_cast<TESNPC*>(playerForm))->level;
		bool   hostileToPlayer = actor->IsHostileToActor(playerActor);

		if (actor->IsDead(1))
		{
			result = 5;
		}
		else if (/*rank <= -2 ||*/ (actor->IsInCombat() && hostileToPlayer))
		{
			result = 1;
		}
		else if (//(actor->flags1 & Actor::kFlags_IsPlayerTeammate) &&
			hostileToPlayer == false &&
			(name.find("Familiar") == 0 || name.find("Flaming Familiar") == 0 ||
				name.find("Storm Atronach") == 0 || name.find("Flame Atronach") == 0 || name.find("Frost Atronach") == 0 ||
				name.find("Storm Atronach") == 0 || name.find("Seeker") == 0))
		{
			// _DMESSAGE("Found familiar: %s", name.c_str());
			result = 30;
		}
		else if (rank >= BGSRelationship::RELATIONSHIP_LEVEL::kAlly)
		{
			result = 4;
		}
		else if (actor->GetActorRuntimeData().race->IsChildRace())
		{
			result = 14;
		}
		else if (name.find("Soldier") != std::string::npos
			|| name.find("Guard") != std::string::npos
			|| name.find("Legate ") != std::string::npos)
		{
			result = 2;
		}
		else if (actor == playerHorse)
		{
			result = 17;
		}
		else if (name.find("Chicken") == 0 || name.find("Cow") == 0 ||
			name.find("Goat") == 0 || name.find("Horse") == 0)
		{
			result = 13;
		}
		else
		{
			auto visitor = [](TESFaction*, std::int8_t){
				//virtual bool Accept(TESFaction* faction, SInt8 rank)
				//{
				//	int len = factions.size(), i;
				//
				//	// Reversed to allow the Skeever faction to come before the Prey faction.
				//	for (i = len - 1; i >= 0; --i)
				//	{
				//		if (faction->formID == factions.at(i)->formID)
				//		{
				//			// at i == 0, faction == "CurrentFollowerFaction"
				//			if (i == 0)
				//			{
				//				// _DMESSAGE("Follower rank: %d", rank);
				//				// rank < 0 means not yet a real follower.
				//				if (rank < 0)
				//				{
				//					continue;
				//				}
				//			}
				//
				//			lastFactionID = factionIDs[i];
				//			return true;
				//		}
				//	}
				//
				//	return false;
				//}
				return false;
			};

			if (actor->VisitFactions(visitor))
			{
				result = lastFactionID;
			}

			if (result == 0)
			{
				if (name.find("Fox") == 0 || name.find("Snow Fox") == 0)
				{
					result = 26;
				}
			}
		}

		return result;
	}

	void SetFactions(StaticFunctionTag *, BSTArray<TESFaction*> facs)
	{
		std::uint32_t i;
		TESFaction *fac;
		std::uint32_t len = facs.size();
		factions.clear();
		factions.reserve(len);

		for (i = 0; i < len; ++i)
		{
			fac = facs[i];

			if (fac == NULL)
			{
				continue;
			}

			//_DMESSAGE("Faction: %s", fac->fullName.data());
			factions.push_back(fac);
		}
	}

	void UpdateSettings(StaticFunctionTag *, BSTArray<float> _floatSettings, BSTArray<bool> _settings, float r, float g, float b)
	{
		arrayLock.Lock();
		queuedUpdate = true;
		Util::ProperArray<float>(&_floatSettings, floats);
		Util::ProperArray<bool>(&_settings, toggles);
		// barColor = _barsColor; //_barsColor->abgr;
		barColor = (((std::uint32_t)r) << 16) | (((std::uint32_t)g) << 8) | ((std::uint32_t)b);
		arrayLock.Unlock();
	}

	void SetVisible(StaticFunctionTag *, bool visible)
	{
		arrayLock.Lock();
		queuedVisibility = visible ? 1 : 0;
		arrayLock.Unlock();
	}

	void StartZoomOut(StaticFunctionTag *)
	{
		arrayLock.Lock();

		if (zoomState == ZOOMED)
		{
			zoomProgress = 0.0f;
		}
		else
		{
			zoomState = ZOOM_IN;
		}
		arrayLock.Unlock();
	}

	float GetRectRatio(StaticFunctionTag *)
	{
		arrayLock.Lock();
		float result = isCircle ? 1.0f : height_to_width_scale;
		arrayLock.Unlock();
		return result;
	}

	void ToggleVisible(StaticFunctionTag* )
	{
		arrayLock.Lock();

		if (widget == NULL)
		{
			arrayLock.Unlock();
			return;
		}

		GFxValue::DisplayInfo info;

		if (widget->GetDisplayInfo(&info))
		{
			queuedVisibility = !info.GetVisible();
		}

		arrayLock.Unlock();
	}

	void SetInside(StaticFunctionTag* , bool inside)
	{
		arrayLock.Lock();
		playerIsInside = inside;
		queuedAppear = true;
		arrayLock.Unlock();
	}

	double lerp(double start, double end, double time)
	{
		return (start * (1.0f - time)) + (end * time);
	}

	double rlerp(double f1, double f2, double f3)
	{
		return (f3 - f1) / (f2 - f1);
	}

	bool RegisterFuncs(BSScript::IVirtualMachine* registry)
	{
		int i;
		all_active_tweens = std::vector<MapTween>();
		tracked = std::vector<TrackedType*>();
		trackedIDs = std::vector<std::uint32_t>();
		factions = std::vector<TESFaction*>();
		queuedSets = std::vector<SetStruct>();
		queuedRemovals = std::vector<int>();

		lastActor = NULL;
		tracked.reserve(32);
		trackedIDs.reserve(32);
		queuedSets.reserve(12);
		queuedRemovals.reserve(32);

		toggles = new bool[NUM_TOGGLES];
		floats = new float[NUM_FLOATS];

		for (i = 0; i < NUM_TOGGLES; ++i)
		{
			toggles[i] = false;
		}

		for (i = 0; i < NUM_FLOATS; ++i)
		{
			floats[i] = 0.0f;
		}

		registry->RegisterFunction("MinimapAdd", "MinimapSKSE", Minimap::Add);
		registry->RegisterFunction("MinimapRemove", "MinimapSKSE", Minimap::Remove);
		registry->RegisterFunction("MinimapClear", "MinimapSKSE", Minimap::Clear);
		registry->RegisterFunction("MinimapSet", "MinimapSKSE", Minimap::Set, registry);
		registry->RegisterFunction("MinimapSetVisible", "MinimapSKSE", Minimap::SetVisible, registry);
		registry->RegisterFunction("MinimapToggleVisible", "MinimapSKSE", Minimap::ToggleVisible, registry);
		registry->RegisterFunction("MinimapSetInside", "MinimapSKSE", Minimap::SetInside, registry);
		registry->RegisterFunction("MinimapGetRectRatio", "MinimapSKSE", Minimap::GetRectRatio, registry);
		registry->RegisterFunction("MinimapSetFactions", "MinimapSKSE", Minimap::SetFactions, registry);
		registry->RegisterFunction("MinimapUpdateSettings", "MinimapSKSE", Minimap::UpdateSettings, registry);
		registry->RegisterFunction("MinimapStartZoom", "MinimapSKSE", Minimap::StartZoomOut, registry);

		//registry->SetFunctionFlags("MinimapSKSE", "MinimapAdd", VMClassRegistry::kFunctionFlag_NoWait);
		//registry->SetFunctionFlags("MinimapSKSE", "MinimapRemove", VMClassRegistry::kFunctionFlag_NoWait);
		//registry->SetFunctionFlags("MinimapSKSE", "MinimapClear", VMClassRegistry::kFunctionFlag_NoWait);
		//registry->SetFunctionFlags("MinimapSKSE", "MinimapSet", VMClassRegistry::kFunctionFlag_NoWait);
		//registry->SetFunctionFlags("MinimapSKSE", "MinimapSetVisible", VMClassRegistry::kFunctionFlag_NoWait);
		//registry->SetFunctionFlags("MinimapSKSE", "MinimapToggleVisible", VMClassRegistry::kFunctionFlag_NoWait);
		//registry->SetFunctionFlags("MinimapSKSE", "MinimapSetInside", VMClassRegistry::kFunctionFlag_NoWait);
		//registry->SetFunctionFlags("MinimapSKSE", "MinimapSetFactions", VMClassRegistry::kFunctionFlag_NoWait);
		//registry->SetFunctionFlags("MinimapSKSE", "MinimapUpdateSettings", VMClassRegistry::kFunctionFlag_NoWait);
		//registry->SetFunctionFlags("MinimapSKSE", "MinimapStartZoom", VMClassRegistry::kFunctionFlag_NoWait);
		//registry->SetFunctionFlags("MinimapSKSE", "MinimapGetRectRatio", VMClassRegistry::kFunctionFlag_NoWait);
		return true;
	}

}