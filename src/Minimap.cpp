#include "Minimap.h"
#include "skse64/GameData.h"
#include "skse64/GameReferences.h"
#include "skse64/GameForms.h"
#include "skse64/PapyrusAlias.h"
#include "skse64/PapyrusEvents.h"

namespace Minimap
{
	UInt32 factionIDs[] = { 6, 17, 9, 9, 7, 8, 3, 10, 11, 21, 20, 27, 25, 26, 28, 23, 24, 29, 0, 0, 22 };

	int numQueuedAdds;
	std::vector<TESFaction*> factions;
	std::vector<TrackedType*> tracked;
	std::vector<MapTween> all_active_tweens;
	std::vector<int> queuedRemovals;
	std::vector<UInt32> trackedIDs;
	std::vector<SetStruct> queuedSets;

	ICriticalSection	arrayLock, setLock, tweenLock;
	GFxValue*			widget = NULL;
	GFxValue			widgetValue;
	TrackedType*		lastActor;
	signed char queuedVisibility = -1;
	bool	queuedClear = false;
	bool	queuedUpdate = false;
	bool	queuedAppear = false;
	Actor*	playerHorse = NULL;
	UInt32	lastFactionID;

	float	rwidth;
	float	height_to_width_scale;
	bool	showBars = false;
	bool	isCircle = true;
	bool	turnedOn = true;
	bool	allowInside = true;
	bool	playerIsInside = false;
	float	checkAbove;
	float	checkBelow;
	UInt32	maxActors = 128;
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
	UInt32	 barColor;

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

		TESNPC* result = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);

		if (!result)
		{
			_DMESSAGE("(ActorBase -> NPC) conversion failed.");
			return NULL;
		}

		return result;
	}

	bool Add(StaticFunctionTag *base, Actor* actor)
	{
		if (!actor)
		{
			_DMESSAGE("ADD: Actor is null!");
			return false;
		}

		TESObjectREFR* object = DYNAMIC_CAST(actor, Actor, TESObjectREFR);

		if (!object)
		{
			_DMESSAGE("ADD: Failed (Actor -> object) conversion!");
			return false;
		}

		arrayLock.Enter();

		if (tracked.size() >= maxActors)
		{
			arrayLock.Leave();
			return false;
		}

		tracked.push_back(object);
		trackedIDs.push_back(0); // Synchronous.
		++numQueuedAdds;

		if (MINIMAP_DEBUG)
		{
			TESNPC* npc = DYNAMIC_CAST(actor->baseForm, TESForm, TESNPC);

			if (!npc)
			{
				_DMESSAGE("ADD: Failed (Actor -> NPC) conversion!");
			}
			else
			{
				_DMESSAGE("Add: %s", npc->fullName.name.data);
			}
		}

		arrayLock.Leave();
		return true;
	}

	SInt32 Remove(StaticFunctionTag *base, Actor* actor)
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

		SInt32 index = -1, i;
		arrayLock.Enter();
		UInt32 len = tracked.size();

		for (i = 0; i < len; ++i)
		{
			if (tracked.at(i) == object)
			{
				if (MINIMAP_DEBUG)
				{
					TESNPC* npc = DYNAMIC_CAST(tracked.at(i)->baseForm, TESForm, TESNPC);
					_MESSAGE("Remove: %s", npc->fullName.name.data);
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

		arrayLock.Leave();
		return index;
	}

	void Clear(StaticFunctionTag *base)
	{
		arrayLock.Enter();

		if (!widget)
		{
			arrayLock.Leave();
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

		setLock.Enter();
		queuedSets.clear();
		setLock.Leave();

		arrayLock.Leave();
	}

	void Set(StaticFunctionTag *base, Actor* actor, UInt32 ID)
	{
		/*if (!actor)
		{
			_DMESSAGE("SET: Failed (Actor -> Object) conversion!");
			return;
		}*/

		setLock.Enter();
		TESObjectREFR* object = DYNAMIC_CAST(actor, Actor, TESObjectREFR);

		if (!object)
		{
			_DMESSAGE("SET: Failed (Actor -> Object) conversion!");
			return;
		}

		if (MINIMAP_DEBUG)
		{
			TESNPC* npc = DYNAMIC_CAST(object->baseForm, TESForm, TESNPC);
			_MESSAGE("Set: %s", npc->fullName.name.data);
		}

		queuedSets.push_back(SetStruct{ object, ID });
		setLock.Leave();
	}

	UInt32 GetIconFiltered(Actor* actor)
	{
		return FilterIcon(GetIcon(actor), actor);
	}

	UInt32 FilterIcon(UInt32 icon, Actor* actor)
	{
		if (icon == ICON_HIDDEN)
		{
			return icon;
		}

		static UInt32 list[] = { 99, 99, 2, 6, 4, 0, 14, 1, 5, 12, 17, 7, 8, 13, 21, 23, 22, 24, 99, 99, 99, 99, 99, 3, 10, 11, 20, 27, 25, 26, 28, 29, 0, 0, 30 };
		static UInt32 len = arrsize(list), i;

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
				std::string name(npc->fullName.name.data);

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

	UInt32 GetIconFiltered(TESObjectREFR* obj)
	{
		Actor* object = DYNAMIC_CAST(obj, TESObjectREFR, Actor);
		return GetIconFiltered(object);
	}

	UInt32 GetIcon(TESObjectREFR* obj)
	{
		Actor* object = DYNAMIC_CAST(obj, TESObjectREFR, Actor);
		return GetIcon(object);
	}

	UInt32 GetIcon(Actor* actor)
	{
		static TESForm* playerForm = DYNAMIC_CAST(*g_thePlayer, PlayerCharacter, TESForm);
		static Actor*   playerActor = DYNAMIC_CAST(*g_thePlayer, PlayerCharacter, Actor);

		if (!actor)
		{
			_DMESSAGE("Actor for GetIcon() is null!");
			return ICON_HIDDEN;
		}

		UInt32 result = 0;
		TESForm* actorForm = DYNAMIC_CAST(actor, Actor, TESForm);
		TESNPC*  actorNPC = ToNPC(actor);

		if (!actorNPC)
		{
			return ICON_HIDDEN;
		}

		std::string name(actorNPC->fullName.name.data);
		SInt8  rank = RelationshipRanks::GetRelationshipRank(actorForm, playerForm);
		bool   hostileToPlayer = CALL_MEMBER_FN(actor, IsHostileToActor)(playerActor);

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
		else if (rank >= 1)
		{
			result = 4;
		}
		else if (actor->race->data.raceFlags & TESRace::kRace_Child)
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
			IconVisitor visitor;

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

	void SetFactions(StaticFunctionTag *base, VMArray<TESFaction*> facs)
	{
		UInt32 i;
		TESFaction *fac;
		int len = facs.Length();
		factions.clear();
		factions.reserve(len);

		for (i = 0; i < len; ++i)
		{
			facs.Get(&fac, i);

			if (fac == NULL)
			{
				continue;
			}

			//_DMESSAGE("Faction: %s", fac->fullName.name.data);
			factions.push_back(fac);
		}
	}

	void UpdateSettings(StaticFunctionTag *base, VMArray<float> _floatSettings, VMArray<bool> _settings, float r, float g, float b)
	{
		arrayLock.Enter();
		queuedUpdate = true;
		Util::ProperArray<float>(&_floatSettings, floats);
		Util::ProperArray<bool>(&_settings, toggles);
		// barColor = _barsColor; //_barsColor->abgr;
		barColor = (((UInt32)r) << 16) | (((UInt32)g) << 8) | ((UInt32)b);
		arrayLock.Leave();
	}

	void SetVisible(StaticFunctionTag *base, bool visible)
	{
		arrayLock.Enter();
		queuedVisibility = visible ? 1 : 0;
		arrayLock.Leave();
	}

	void StartZoomOut(StaticFunctionTag *base)
	{
		arrayLock.Enter();

		if (zoomState == ZOOMED)
		{
			zoomProgress = 0.0f;
		}
		else
		{
			zoomState = ZOOM_IN;
		}
		arrayLock.Leave();
	}

	float GetRectRatio(StaticFunctionTag *base)
	{
		arrayLock.Enter();
		float result = isCircle ? 1.0f : height_to_width_scale;
		arrayLock.Leave();
		return result;
	}

	void ToggleVisible(StaticFunctionTag* base)
	{
		arrayLock.Enter();

		if (widget == NULL)
		{
			arrayLock.Leave();
			return;
		}

		GFxValue::DisplayInfo info;

		if (widget->GetDisplayInfo(&info))
		{
			queuedVisibility = !info._visible;
		}

		arrayLock.Leave();
	}

	void SetInside(StaticFunctionTag* base, bool inside)
	{
		arrayLock.Enter();
		playerIsInside = inside;
		queuedAppear = true;
		arrayLock.Leave();
	}

	float lerp(float start, float end, float time)
	{
		return (start * (1.0f - time)) + (end * time);
	}

	float rlerp(float f1, float f2, float f3)
	{
		return (f3 - f1) / (f2 - f1);
	}

	bool RegisterFuncs(VMClassRegistry* registry)
	{
		int i;
		all_active_tweens = std::vector<MapTween>();
		tracked = std::vector<TrackedType*>();
		trackedIDs = std::vector<UInt32>();
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

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, bool, Actor*>
			("MinimapAdd", "MinimapSKSE", Minimap::Add, registry));
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, SInt32, Actor*>
			("MinimapRemove", "MinimapSKSE", Minimap::Remove, registry));
		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, void>
			("MinimapClear", "MinimapSKSE", Minimap::Clear, registry));
		registry->RegisterFunction(
			new NativeFunction2 <StaticFunctionTag, void, Actor*, UInt32>
			("MinimapSet", "MinimapSKSE", Minimap::Set, registry));
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void, bool>
			("MinimapSetVisible", "MinimapSKSE", Minimap::SetVisible, registry));
		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, void>
			("MinimapToggleVisible", "MinimapSKSE", Minimap::ToggleVisible, registry));
		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void, bool>
			("MinimapSetInside", "MinimapSKSE", Minimap::SetInside, registry));
		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, float>
			("MinimapGetRectRatio", "MinimapSKSE", Minimap::GetRectRatio, registry));

		registry->RegisterFunction(
			new NativeFunction1 <StaticFunctionTag, void, VMArray<TESFaction*>>
			("MinimapSetFactions", "MinimapSKSE", Minimap::SetFactions, registry));
		registry->RegisterFunction(
			new NativeFunction5 <StaticFunctionTag, void, VMArray<float>, VMArray<bool>, float, float, float>
			("MinimapUpdateSettings", "MinimapSKSE", Minimap::UpdateSettings, registry));
		registry->RegisterFunction(
			new NativeFunction0 <StaticFunctionTag, void>
			("MinimapStartZoom", "MinimapSKSE", Minimap::StartZoomOut, registry));

		registry->SetFunctionFlags("MinimapSKSE", "MinimapAdd", VMClassRegistry::kFunctionFlag_NoWait);
		registry->SetFunctionFlags("MinimapSKSE", "MinimapRemove", VMClassRegistry::kFunctionFlag_NoWait);
		registry->SetFunctionFlags("MinimapSKSE", "MinimapClear", VMClassRegistry::kFunctionFlag_NoWait);
		registry->SetFunctionFlags("MinimapSKSE", "MinimapSet", VMClassRegistry::kFunctionFlag_NoWait);
		registry->SetFunctionFlags("MinimapSKSE", "MinimapSetVisible", VMClassRegistry::kFunctionFlag_NoWait);
		registry->SetFunctionFlags("MinimapSKSE", "MinimapToggleVisible", VMClassRegistry::kFunctionFlag_NoWait);
		registry->SetFunctionFlags("MinimapSKSE", "MinimapSetInside", VMClassRegistry::kFunctionFlag_NoWait);
		registry->SetFunctionFlags("MinimapSKSE", "MinimapSetFactions", VMClassRegistry::kFunctionFlag_NoWait);
		registry->SetFunctionFlags("MinimapSKSE", "MinimapUpdateSettings", VMClassRegistry::kFunctionFlag_NoWait);
		registry->SetFunctionFlags("MinimapSKSE", "MinimapStartZoom", VMClassRegistry::kFunctionFlag_NoWait);
		registry->SetFunctionFlags("MinimapSKSE", "MinimapGetRectRatio", VMClassRegistry::kFunctionFlag_NoWait);
		return true;
	}

}