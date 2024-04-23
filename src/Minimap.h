#pragma once
#pragma comment(lib, "winmm.lib")

#include <iostream>
#include <vector>

#include "skse64/PapyrusNativeFunctions.h"
#include "skse64/GameReferences.h"
#include "skse64/PapyrusForm.h"
#include "skse64/GameObjects.h"
#include "skse64/Hooks_Scaleform.h"

#include "skse64/GameRTTI.h"
#include "skse64/GameMenus.h"
#include "skse64/PapyrusUI.h"

#include "common/ITypes.h"
#include "MinimapUtil.h"

#define CELL 4096.0f
#define PI 3.1415927f
#define pxRadius 90.0f

#define NUM_TOGGLES kNumSettings
#define NUM_FLOATS  kNumFloatParams

#define MINIMAP_DEBUG 1
#define ICON_HIDDEN 200
#define CORPSE 5
#define VICTIM 12
#define arrsize(x) sizeof(x) / sizeof(x[0])

#define clamp(a,b,c) max((a), min((b), (c))) // min, max, value.
#define ALLOW_INSIDE (!playerIsInside || allowInside)

namespace Minimap
{
	typedef TESObjectREFR TrackedType;

	enum { TWEEN_ALPHA = 0, TWEEN_X, TWEEN_Y, TWEEN_ROTATION, TWEEN_X_SCALE, TWEEN_Y_SCALE, TWEEN_DELAY };
	enum { FLAGS_NONE = 0, FLAG_XPOS, FLAG_YPOS, FLAG_HIDE_ON_FINISH = 4 };

	float lerp(float start, float end, float time);
	float rlerp(float f1, float f2, float f3);

	struct SetStruct
	{
		TrackedType* actor;
		UInt32		 ID;
	};

	struct MapTween
	{
		float start_value, end_value, value, duration, max_duration;
		TrackedType* target_object;
		int tween_type, flags;

		MapTween(TrackedType* targetObject, int type, float start, float end, float _duration, int _flags)
		{
			target_object = targetObject;
			tween_type = type;
			start_value = start;
			end_value = end;
			max_duration = _duration;
			flags = _flags;

			duration = 0.0f;
		}

		~MapTween()
		{
			// delete target; no.
		}

		void tick(float delta)
		{
			duration += delta;

			if (duration > max_duration)
			{
				duration = max_duration;
			}
		}

		bool is_complete()
		{
			return (duration >= max_duration);
		}

		bool is_targeting(TrackedType* target)
		{
			return (target_object == target);
		}

		void apply_value(GFxValue::DisplayInfo* info)
		{
			if (tween_type == TWEEN_DELAY)
			{
				return;
			}

			if (flags == FLAG_XPOS)
			{
				value = lerp(start_value, info->_x, duration / max_duration);
			}
			else if (flags == FLAG_YPOS)
			{
				value = lerp(start_value, info->_y, duration / max_duration);
			}
			else
			{
				value = lerp(start_value, end_value, duration / max_duration);
			}

			switch (tween_type)
			{
			case TWEEN_ALPHA:
			{
				info->SetAlpha(value);
			}
			break;

			case TWEEN_X:
			{
				info->SetX(value);
			}
			break;

			case TWEEN_Y:
			{
				info->SetY(value);
			}
			break;

			case TWEEN_X_SCALE:
			{
				info->SetXScale(value);
			}
			break;

			case TWEEN_Y_SCALE:
			{
				info->SetYScale(value);
			}
			break;

			case TWEEN_ROTATION:
			{
				info->SetRotation(value);
			}
			break;

			default:
				break;
			}
		}
	};

	enum MinimapToggles
	{
		kType_ShowBars = 0, kType_SimpleMode, kType_Guards, kType_Followers, kType_Allies,
		kType_Neutrals, kType_Children, kType_Enemies, kType_Corpse, kType_Killed, kType_Horses, kType_Predators,
		kType_Prey, kType_Domestic, kType_Dogs, kType_Vampires, kType_Dragons, kType_Skeevers, kType_RemoveOnLoot,
		kType_Circle, kType_TurnedOn, kType_AllowInside, kType_Uninstall, kType_Blacksmith, kType_Innkeeper, kType_Merchant,
		kType_Spider, kType_Giant, kType_Mammoth, kType_Fox, kType_Troll, kType_Horker, kType_Falmer, kType_Forsworn, kType_Familiar, kNumSettings
	};

	enum FloatParams
	{
		kParam_Radius = 0, kParam_X, kParam_Y, kParam_AlphaBG,
		kParam_AlphaBars, kParam_AlphaIcons, kParam_MasterScale, kParam_PlayerScale,
		kParam_IconScale, kParam_MaxActors, kParam_ZoomAmount, kParam_CheckAbove, kParam_CheckBelow, kParam_PulseRate, kNumFloatParams
	};

	enum ZoomState { ZOOM_IN, ZOOM_OUT, ZOOMED, NO_ZOOM };
	enum FadeState { INVISIBLE, VISIBLE, FADING_IN, FADING_OUT };

	extern UInt32 factionIDs[];
	extern int numQueuedAdds;
	extern std::vector<TESFaction*> factions;
	extern std::vector<int> queuedRemovals;
	extern std::vector<SetStruct> queuedSets;
	extern std::vector<TrackedType*> tracked;
	extern std::vector<UInt32>		 trackedIDs;
	extern std::vector<MapTween> all_active_tweens;

	extern ICriticalSection			arrayLock, setLock, tweenLock;
	extern GFxValue*				widget;
	extern GFxValue					widgetValue;
	extern TrackedType* lastActor;
	extern signed char				queuedVisibility;
	extern bool						queuedClear;
	extern bool						queuedUpdate;
	extern bool						queuedAppear;
	extern Actor*					playerHorse;
	extern UInt32					lastFactionID;

	extern float					rwidth;
	extern float					height_to_width_scale;
	extern bool						showBars;
	extern bool						isCircle;
	extern bool						turnedOn;
	extern bool						allowInside;
	extern bool						playerIsInside;
	extern float					checkAbove;
	extern float					checkBelow;
	extern UInt32					maxActors;
	extern bool*					toggles;
	extern float*					floats;

	extern float	 fadeProgress;
	extern float	 zoomProgress;
	extern ZoomState zoomState;
	extern FadeState minimapState;

	extern float setting_fade_in_speed;
	extern float setting_fade_out_speed;
	extern float setting_visible_speed;
	extern float setting_attribute_threshold;
	extern bool  setting_context_fade_enabled;
	extern float inside_radius;
	extern float radius_extra;
	extern float inside_fade_threshold;

	extern float					actualWidth;
	extern float					actualHeight;
	extern double					actualScale;
	extern UInt32					barColor;
	
	extern GFxValue north;
	extern GFxValue* north_indicator;

	extern GFxValue	actors;
	extern GFxValue	player_arrow;

	class IconVisitor : public Actor::FactionVisitor
	{
	public:
		IconVisitor::IconVisitor() {}

		virtual bool Accept(TESFaction* faction, SInt8 rank)
		{
			int len = factions.size(), i;

			// Reversed to allow the Skeever faction to come before the Prey faction.
			for (i = len - 1; i >= 0; --i)
			{
				if (faction->formID == factions.at(i)->formID)
				{
					// at i == 0, faction == "CurrentFollowerFaction"
					if (i == 0)
					{
						// _DMESSAGE("Follower rank: %d", rank);
						// rank < 0 means not yet a real follower.
						if (rank < 0)
						{
							continue;
						}
					}

					lastFactionID = factionIDs[i];
					return true;
				}
			}

			return false;
		}
	};

	TESNPC* ToNPC(Actor* actor);
	bool RegisterFuncs(VMClassRegistry* registry);

	UInt32 GetIcon(Actor* actor);
	UInt32 GetIcon(TESObjectREFR* obj);
	UInt32 GetIconFiltered(TESObjectREFR* obj);
	UInt32 GetIconFiltered(Actor* actor);
	UInt32 FilterIcon(UInt32 icon, Actor* actor);

	bool Add(StaticFunctionTag *base, Actor* actor);
	SInt32 Remove(StaticFunctionTag *base, Actor* actor);
	void Set(StaticFunctionTag *base, Actor* actor, UInt32 ID);
	void Clear(StaticFunctionTag *base);
	void StartZoomOut(StaticFunctionTag *base);

	float GetRectRatio(StaticFunctionTag* base);
	bool WaitForClear(StaticFunctionTag* base);
	void SetInside(StaticFunctionTag* base, bool inside);
	void SetFactions(StaticFunctionTag *base, VMArray<TESFaction*> facs);
	void UpdateSettings(StaticFunctionTag *base, VMArray<float> _floatSettings, VMArray<bool> _settings, float r, float g, float b);
	void SetVisible(StaticFunctionTag *base, bool visible);
	void ToggleVisible(StaticFunctionTag* base);
}