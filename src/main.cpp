#include <shlobj.h>
#include <math.h>
#include <algorithm>
#include <sstream>

#include <mmsystem.h>
#include <math.h>

#include "skse64/PluginAPI.h"
#include "skse64/ScaleformMovie.h"
#include "skse64/GameAPI.h"
#include "skse64/GameData.h"
#include "skse64/GameCamera.h"
#include "skse64/GameInput.h"
#include "skse64/PluginManager.h"
#include "skse64/ScaleformCallbacks.h"
#include "skse64/NiNodes.h"

#include "Minimap.h"

using namespace Minimap;

PluginHandle				g_pluginHandle = kPluginHandle_Invalid;
SKSEScaleformInterface		* g_scaleform = NULL;
SKSESerializationInterface	* g_serialization = NULL;
SKSEPapyrusInterface		* g_papyrus = NULL;
TESCameraState				* lastCameraState = NULL;

inline bool IsState(UInt32 state)
{
	static PlayerCamera* cam = PlayerCamera::GetSingleton();
	return (cam->cameraState == cam->cameraStates[state]);
}

inline bool EnteredState(UInt32 state)
{
	static PlayerCamera* cam = PlayerCamera::GetSingleton();
	return (cam->cameraState == cam->cameraStates[state] && cam->cameraState != lastCameraState);
}

inline bool ExitedState(UInt32 state)
{
	static PlayerCamera* cam = PlayerCamera::GetSingleton();
	return (cam->cameraState != cam->cameraStates[state] && lastCameraState == cam->cameraStates[state]);
}

// SCALEFORM BEGIN

class SKSEMinimapUpdate : public GFxFunctionHandler
{
public:
	inline void checkForSettingsChange()
	{
		if (queuedUpdate)
		{
			queuedUpdate = false;
			maxActors = (UInt32)floats[kParam_MaxActors];
			isCircle = toggles[kType_Circle];
			showBars = toggles[kType_ShowBars];
			turnedOn = (toggles[kType_TurnedOn] && toggles[kType_Uninstall] == false);
			allowInside = toggles[kType_AllowInside];
			rwidth = floats[kParam_Radius];
			checkAbove = floats[kParam_CheckAbove];
			checkBelow = -floats[kParam_CheckBelow];

			minimapState = VISIBLE;
			zoomState = NO_ZOOM;
			zoomProgress = 0.0f;
			fadeProgress = 0.0f;
			refreshMinimapScale(0.0f);

			GFxValue arr[11];
			arr[0].SetNumber((double)floats[kParam_X]);
			arr[1].SetNumber((double)floats[kParam_Y]);
			arr[2].SetNumber((double)floats[kParam_MasterScale]);
			arr[3].SetNumber((double)floats[kParam_PlayerScale]);
			arr[4].SetNumber((double)floats[kParam_AlphaBG]);
			arr[5].SetNumber((double)floats[kParam_AlphaBars]);
			arr[6].SetNumber((double)floats[kParam_AlphaIcons]);
			arr[7].SetBool(isCircle);
			arr[8].SetBool(showBars);
			arr[9].SetBool(turnedOn);
			arr[10].SetNumber((double)barColor);
			widget->Invoke("updateSettings", NULL, arr, 11);
		}
	}

	inline void checkVisibilityChange()
	{
		if (queuedVisibility >= 0)
		{
			GFxValue arr[1];

			arr[0].SetBool(queuedVisibility == 1);

			widget->Invoke("setRadarShown", NULL, arr, 1);
			queuedVisibility = -1;
		}
	}

	inline void checkCameraChanges()
	{
		// Hide the horse icon on mount.
		if (EnteredState(PlayerCamera::kCameraState_Horse))
		{
			NiPointer<TESObjectREFR> horseObject = NULL;

			// LookupREFRByHandle(&(*g_thePlayer)->lastRiddenHorseHandle, &horseObject); For old SKSE version.
            LookupREFRByHandle((*g_thePlayer)->lastRiddenHorseHandle, horseObject);

			if (horseObject)
			{
				Actor* horse = DYNAMIC_CAST(horseObject, TESObjectREFR, Actor);

				if (horse)
				{
					Remove(NULL, horse);
					playerHorse = horse;
				}
			}
		}
		// Show the horse's icon again on dismount.
		else if (ExitedState(PlayerCamera::kCameraState_Horse))
		{
			if (playerHorse)
			{
				Add(NULL, playerHorse);
			}
		}

		static PlayerCamera* camera = PlayerCamera::GetSingleton();
	}

	inline void on_tween_finish(MapTween& tween, int index)
	{
		GFxValue arr[1];
		int flags = tween.flags;

		if (flags & FLAG_HIDE_ON_FINISH)
		{
			arr[0].SetNumber((double)index);
			// Make sure hide_on_finish tween is LAST otherwise it will make actor null and then crash when accessed by subsequent tweens.
			widget->Invoke("hideActor", NULL, arr, 1);
		}
	}

	inline void checkQueuedIconRemovals()
	{
		int i;
		UInt32 len = queuedRemovals.size(), j;
		GFxValue arr[1];

		for (j = 0; j < len; ++j)
		{
			int index = queuedRemovals.at(j);
			arr[0].SetNumber((double)index);

			for (i = all_active_tweens.size() - 1; i >= 0; --i)
			{
				MapTween& tween = all_active_tweens[i];

				if (index < tracked.size() && tween.is_targeting(tracked.at(index)))
				{
					on_tween_finish(tween, index);
					all_active_tweens.erase(all_active_tweens.begin() + i);
				}
			}

			widget->Invoke("removeActor", NULL, arr, 1);
		}

		queuedRemovals.clear();
	}

	inline void setLastActor(int index)
	{
		if (index >= tracked.size() || index < 0)
		{
			// _MESSAGE("Invalid lastActor!");
			return;
		}

		lastActor = tracked.at(index);
	}

	inline void checkQueuedIconAdditions()
	{
		UInt32 len = numQueuedAdds, i;
		GFxValue arr[1];

		for (i = 0; i < len; ++i)
		{
			int index = (tracked.size() - (int)len + i);
			int ID = GetIconFiltered(tracked.at(index));

			arr[0].SetNumber((double)ID);
			trackedIDs.at(index) = ID;
			setLastActor(index);
			widget->Invoke("addActor", NULL, arr, 1);
		}

		numQueuedAdds = 0;
	}

	void setIconForActor(int index, UInt32 ID)
	{
		if (trackedIDs.at(index) == ID)
		{
			return;
		}

		GFxValue arr[2];

		arr[0].SetNumber((double)index);
		arr[1].SetNumber((double)ID);

		trackedIDs.at(index) = ID;
		setLastActor(index);
		widget->Invoke("setActor", NULL, arr, 2);
	}

	inline void checkQueuedIconSets()
	{
		UInt32 i;
		setLock.Enter();

		for (i = 0; i < queuedSets.size(); ++i)
		{
			auto iter = find(tracked.begin(), tracked.end(), queuedSets.at(i).actor);

			if (iter == tracked.end())
			{
				continue;
			}

			int actorIndex = distance(tracked.begin(), iter);
			UInt32 filtered_id = FilterIcon(queuedSets.at(i).ID, DYNAMIC_CAST(*iter, TESObjectREFR, Actor));
			setIconForActor(actorIndex, filtered_id);
		}

		queuedSets.clear();
		setLock.Leave();
	}

	inline void checkZoom(float delta)
	{
		if (zoomState != NO_ZOOM)
		{
			if (zoomState == ZOOM_IN)
			{
				zoomProgress += delta * 2.55f;

				if (zoomProgress >= 1.2f)
				{
					zoomState = ZOOMED;
					zoomProgress = 0.0f;
				}
			}
			else if (zoomState == ZOOMED)
			{
				zoomProgress += delta * 0.20f;

				if (zoomProgress >= 1.2f)
				{
					zoomState = ZOOM_OUT;
					zoomProgress = 1.2f;
				}
			}
			else if (zoomState == ZOOM_OUT)
			{
				zoomProgress -= delta * 1.8f;

				if (zoomProgress <= 0.0f)
				{
					zoomState = NO_ZOOM;
					zoomProgress = 0.0f;
				}
			}

			float zoomExtraRadius;

			if (zoomState == ZOOMED)
			{
				zoomExtraRadius = floats[kParam_ZoomAmount];
			}
			else
			{
				zoomExtraRadius = floats[kParam_ZoomAmount] * zoomProgress / 1.2f;
			}

			refreshMinimapScale(zoomExtraRadius);
		}
	}

	inline void checkFadeOnZeroActors(float delta, int actorsRendered, int lowAttribute)
	{
		if (!setting_context_fade_enabled)
		{
			return;
		}

		int should_fade_out = (actorsRendered == 0 && !lowAttribute);

		if (minimapState != INVISIBLE)
		{
			if (minimapState == FADING_IN)
			{
				if (should_fade_out)
				{
					minimapState = FADING_OUT;
				}
				else
				{
					fadeProgress += delta * setting_fade_in_speed;

					if (fadeProgress >= 1.0f)
					{
						minimapState = VISIBLE;
						fadeProgress = 0.0f;
					}
				}
			}
			else if (minimapState == VISIBLE)
			{
				if (should_fade_out)
				{
					fadeProgress += delta * setting_visible_speed;

					if (fadeProgress >= 1.0f)
					{
						minimapState = FADING_OUT;
						fadeProgress = 1.0f;
					}
				}
				else
				{
					fadeProgress = 0.0f;
				}
			}
			else if (minimapState == FADING_OUT)
			{
				if (should_fade_out)
				{
					fadeProgress -= delta * setting_fade_out_speed;

					if (fadeProgress <= 0.0f)
					{
						minimapState = INVISIBLE;
						fadeProgress = 0.0f;
					}
				}
				else
				{
					minimapState = FADING_IN;
				}
			}
		}
		else if (!should_fade_out)
		{
			minimapState = FADING_IN;
		}

		static float new_alpha;

		if (minimapState == VISIBLE)
		{
			new_alpha = 100.0f;
		}
		else if (minimapState == INVISIBLE)
		{
			new_alpha = 0.0f;
		}
		else if (minimapState == FADING_IN || minimapState == FADING_OUT)
		{
			new_alpha = Minimap::lerp(0.0f, 100.0f, fadeProgress);
		}

		setMinimapAlpha(new_alpha);
	}

	inline void setMinimapAlpha(float new_alpha)
	{
		GFxValue::DisplayInfo info;

		if (widget->GetDisplayInfo(&info))
		{
			info.SetAlpha(new_alpha);
			widget->SetDisplayInfo(&info);
		}
	}

	inline int checkPeriodics(Actor* playerActor, float delta)
	{
		static UInt32
			healthID = LookupActorValueByName("Health"),
			staminaID = LookupActorValueByName("Stamina"),
			magickaID = LookupActorValueByName("Magicka"),
			extraID = LookupActorValueByName("carryweight"),
			invID = LookupActorValueByName("InventoryWeight"), len, i, lastID, newID;
		static float frameTime = 0.0f, health, stamina, magicka;

		if (isCircle && showBars && (minimapState != INVISIBLE || setting_context_fade_enabled))
		{
			GFxValue arr[4];

			health = clamp(0.0, 1.0, Util::GetPercentage(playerActor, healthID));
			stamina = clamp(0.0, 1.0, Util::GetPercentage(playerActor, staminaID));
			magicka = clamp(0.0, 1.0, Util::GetPercentage(playerActor, magickaID));

			arr[0].SetNumber(health);
			arr[1].SetNumber(stamina);
			arr[2].SetNumber(magicka);
			arr[3].SetNumber(clamp(0.0, 1.0, (double)(playerActor->actorValueOwner.GetCurrent(invID) / playerActor->actorValueOwner.GetCurrent(extraID))));
			widget->Invoke("updateBars", NULL, arr, 4);
		}

		frameTime += delta;

		if (frameTime >= 0.6f)
		{
			frameTime -= 0.6f;
			len = tracked.size();

			for (i = 0; i < len; ++i)
			{
				lastID = trackedIDs.at(i);

				if (lastID == VICTIM || lastID == CORPSE /*|| lastID == ICON_HIDDEN*/)
				{
					// Corpse or dead icon, to prevent overriding.
					continue;
				}

				newID = GetIconFiltered(tracked.at(i));

				// Second clause prevents: (death of victim -> hidden -> poll to corpse).
				if (newID != lastID && !(lastID == ICON_HIDDEN && newID == CORPSE))
				{
					setIconForActor(i, newID);
				}
			}
		}

		// Fade in minimap if one of the player's attributes is low.
		return (toggles[kType_ShowBars] && isCircle &&
			(health <= setting_attribute_threshold ||
				stamina <= setting_attribute_threshold ||
				magicka <= setting_attribute_threshold));
	}

	virtual void Invoke(Args * args)
	{
		static TESObjectREFR* player = DYNAMIC_CAST(*g_thePlayer, PlayerCharacter, TESObjectREFR);
		static Actor* playerActor = DYNAMIC_CAST(*g_thePlayer, PlayerCharacter, Actor);
		static PlayerCamera* cam = PlayerCamera::GetSingleton();
		static float delta, fade_poll_time = 0.0f;
		static int actorsRendered, lastActorsRendered;

		arrayLock.Enter();

		if (queuedAppear)
		{
			GFxValue arr[1];

			arr[0].SetBool(ALLOW_INSIDE && turnedOn);

			widget->Invoke("setRadarShown", NULL, arr, 1);
			queuedAppear = false;
		}

		if (!turnedOn || ALLOW_INSIDE == false)
		{
			checkForSettingsChange();
			arrayLock.Leave();
			return;
		}

		delta = max(1.0f / (144.0f), (float)args->args[0].GetNumber() * (1 / 1000.0f));

		checkCameraChanges();
		checkForSettingsChange();
		checkVisibilityChange();
		checkQueuedIconAdditions();
		checkQueuedIconRemovals();

		int low_attribute = checkPeriodics(playerActor, delta);
		checkQueuedIconSets();
		checkZoom(delta);

		actorsRendered = renderMinimap(player, delta);

		if (fade_poll_time > 0.0f)
		{
			fade_poll_time -= delta;
		}
		else
		{
			fade_poll_time = 2.20f + pow(floats[kParam_Radius] / 2600.0f, 2.0f);
			lastActorsRendered = actorsRendered;
		}

		checkFadeOnZeroActors(delta, lastActorsRendered, low_attribute);
		lastCameraState = cam->cameraState;
		arrayLock.Leave();
	}

	void refreshMinimapScale(float extra)
	{
		radius_extra = extra;

		if (playerIsInside && inside_radius > 0.0f)
		{
			actualWidth = (inside_radius + radius_extra);
			actualHeight = actualWidth * get_rect_ratio();
		}
		else
		{
			actualWidth = rwidth + extra;
			actualHeight = actualWidth * get_rect_ratio();
		}

		actualScale = sqrt(abs(floats[kParam_IconScale] / 100.0f)) * 100.0f; // *(CELL / actualWidth);
	}

	bool render_actor(float x, float y, GFxValue::DisplayInfo* info, int actorIndex, TrackedType* actorObject)
	{
		int i;
		bool tweened = false;

		info->SetPosition(x, y);
		info->SetScale(actualScale, actualScale);
		info->SetVisible(true);

		for (i = all_active_tweens.size() - 1; i >= 0; --i)
		{
			MapTween& tween = all_active_tweens.at(i);

			if (tween.target_object == actorObject)
			{
				tween.apply_value(info);
				tweened = true;
				bool bail = (tween.tween_type == TWEEN_DELAY);

				if (tween.is_complete())
				{
					// _MESSAGE("-- Tween expired: %.2fs", tween.duration);
					on_tween_finish(tween, actorIndex);
					all_active_tweens.erase(all_active_tweens.begin() + i);
				}

				if (bail)
				{
					// No more tweening for this object.
					return false;
				}
			}
		}

		return tweened;
	}

	inline float get_percentage_near_edge(float heightAbovePlayer)
	{
		if (heightAbovePlayer >= 0.0f)
		{
			return rlerp(0.0f, checkAbove, heightAbovePlayer);
		}
		else // < 0.0f
		{
			return rlerp(0.0f, checkBelow, heightAbovePlayer);
		}
	}

	inline float get_rect_ratio()
	{
		return (isCircle ? 1.0f : height_to_width_scale);
	}

	int renderMinimap(TESObjectREFR* player, float delta)
	{
		int i, actors_rendered;
		UInt32 j;
		GFxValue v;
		GFxValue::DisplayInfo info;

		float   angle = player->rot.z, // "angle" is in radians.
			px = player->pos.x,
			py = player->pos.y,
			tmp = pxRadius / actualWidth,
			xDiff, yDiff, xStretch, yStretch, heightAbovePlayer;
		
		double player_angle = 0.0;
		static double last_player_angle = 0.0;
		TrackedType* delayedTweenObject = NULL;
		actors_rendered = 0;

		if (IsState(PlayerCamera::kCameraState_ThirdPerson2))
		{
			float rot1, rot2, rot3;
			PlayerCamera::GetSingleton()->cameraState->camera->cameraNode->m_worldTransform.rot.GetEulerAngles(&rot1, &rot2, &rot3);

			auto cam = PlayerCamera::GetSingleton()->cameraState->camera->cameraNode;
			auto& cam_pos = cam->/*m_children.m_data[0]->*/m_worldTransform.pos;
			auto& player_pos = player->pos;

			float theta, abs_rot1 = abs(rot1);
			static float almost_zero = 0.0001f;

			if (rot2 >= -PI / 2 && rot2 <= PI / 2 && abs(PI - abs_rot1) <= 0.8f) // rot1 drops to about +/- 3.1 when sprinting.
			{
				theta = rot2 + PI;
			}
			else if (rot2 <= PI / 2 && rot2 >= 0.0f && abs_rot1 <= almost_zero)
			{
				theta = 2 * PI - rot2;
			}
			else
			{
				theta = -rot2;
			}

			angle = theta;
			player_angle = (double)((player->rot.z - angle) * 180.0f / PI);
		}

		if (last_player_angle != player_angle)
		{
			GFxValue::DisplayInfo player_arrow_info;

			if (player_arrow.GetDisplayInfo(&player_arrow_info))
			{
				player_arrow_info.SetRotation(player_angle);
				player_arrow.SetDisplayInfo(&player_arrow_info);
			}

			last_player_angle = player_angle;
		}

		float
			c = cosf(angle),
			s = sinf(angle);

		for (i = 0; i < all_active_tweens.size(); ++i)
		{
			MapTween& tween = all_active_tweens.at(i);

			if (tween.target_object != delayedTweenObject)
			{
				tween.tick(delta);

				if (tween.tween_type == TWEEN_DELAY)
				{
					delayedTweenObject = tween.target_object;
				}
			}
		}

		if (north_indicator && north_indicator->GetDisplayInfo(&info))
		{
			float north_angle = -angle + PI / 2;

			if (isCircle)
			{
				info.SetPosition(cosf(north_angle) * pxRadius + pxRadius,
					sinf(north_angle) * pxRadius + pxRadius);

				north_indicator->SetDisplayInfo(&info);
			}
		}

		bool bail = (minimapState == INVISIBLE);

		for (j = 0; j < tracked.size(); ++j)
		{
			TrackedType* a = tracked.at(j);

			if (!a)
			{
				continue;
			}

			xDiff = a->pos.x - px;
			yDiff = a->pos.y - py;
			xStretch = (xDiff*c - yDiff*s);
			yStretch = -(xDiff*s + yDiff*c);

			if (!actors.GetElement(j, &v) || v.type != (GFxValue::kType_DisplayObject | GFxValue::kTypeFlag_Managed))
			{
				continue;
			}

			if (!v.GetDisplayInfo(&info))
			{
				continue;
			}

			if (playerIsInside)
			{
				// Actor's height above the player.
				heightAbovePlayer = (a->pos.z - player->pos.z);

				if ((checkAbove != 0.0f && heightAbovePlayer >= checkAbove)
					|| (checkBelow != 0.0f && heightAbovePlayer <= checkBelow))
				{
					info.SetVisible(false);
				}
				else
				{
					if (!bail)
					{
						float percentageNearEdge = get_percentage_near_edge(heightAbovePlayer);

						bool tweened = render_actor(xStretch * tmp + pxRadius, yStretch * tmp + pxRadius * get_rect_ratio(),
							&info, j, a);

						if (percentageNearEdge >= inside_fade_threshold)
						{
							info.SetAlpha((double)lerp(0.0f, 100.0f,
								(1.0f - percentageNearEdge) / (1.0f - inside_fade_threshold)));
						}
						else if (!tweened)
						{
							info.SetAlpha(100.0);
						}
					}

					++actors_rendered;
				}
			}
			else // The minimap is visible.
			{
#define TOLERANCE 1900

				// Icon is off minimap.
				if (// (isCircle && (abs(xDiff) > actualWidth + TOLERANCE || abs(yDiff)  > actualWidth + TOLERANCE)) ||
					/*(!isCircle &&*/ (abs(xDiff) > actualWidth + TOLERANCE || abs(yDiff) > actualHeight + TOLERANCE))
				{
					info.SetVisible(false);
				}
				else
				{
					if (!bail)
					{
						render_actor(xStretch * tmp + pxRadius, yStretch * tmp + pxRadius * get_rect_ratio(),
							&info, j, a);
					}

					++actors_rendered;
				}
			}

			v.SetDisplayInfo(&info);
		}

		return actors_rendered;
	}
};


class SKSEInit : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args)
	{
		ASSERT(args->numArgs == 4);

		widgetValue = args->args[0];
		height_to_width_scale = (float)args->args[1].GetNumber();
		actors = args->args[2];
		player_arrow = args->args[3];

		widget = &widgetValue;
	}
};

class SKSETween : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args)
	{
		int flags;

		if (args->numArgs < 4)
		{
			return;
		}

		if (!lastActor)
		{
			return;
		}

		flags = (args->numArgs == 4 ? 0 : (int)args->args[4].GetNumber());

		MapTween tween = MapTween(lastActor, (int)args->args[0].GetNumber(), (float)args->args[1].GetNumber(), (float)args->args[2].GetNumber(),
			(float)args->args[3].GetNumber(), flags);

		all_active_tweens.push_back(tween);
	}
};
class SKSESetFadeSettings : public GFxFunctionHandler
{
public:
	virtual void Invoke(Args * args)
	{
		if (args->numArgs != 8)
		{
			return;
		}

		setting_fade_in_speed = 1.0f / (float)args->args[0].GetNumber();
		setting_fade_out_speed = 1.0f / (float)args->args[1].GetNumber();
		setting_visible_speed = 1.0f / (float)args->args[2].GetNumber();
		setting_attribute_threshold = (float)args->args[3].GetNumber();
		setting_context_fade_enabled = args->args[4].GetBool();

		inside_radius = (float)args->args[5].GetNumber();
		inside_fade_threshold = (float)args->args[6].GetNumber();

		north = args->args[7];
		north_indicator = &north;
	}
};

bool RegisterScaleform(GFxMovieView* view, GFxValue* root)
{
	RegisterFunction <SKSEInit>(root, view, "init");
	RegisterFunction <SKSEMinimapUpdate>(root, view, "update");
	RegisterFunction <SKSETween>(root, view, "tween");
	RegisterFunction <SKSESetFadeSettings>(root, view, "set_extra_settings");
	return true;
}

extern "C"
{
	bool SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim Special Edition\\SKSE\\skse_minimap.log");
		gLog.SetPrintLevel(IDebugLog::kLevel_FatalError);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "Skyrim Minimap";
		info->version = 2;
		g_pluginHandle = skse->GetPluginHandle();
		
		if (skse->isEditor)
		{
			return false;
		}
		
		g_scaleform = (SKSEScaleformInterface *)skse->QueryInterface(kInterface_Scaleform);
		
		if (!g_scaleform)
		{
			_MESSAGE("Couldn't get scaleform interface.");
			return false;
		}

		if (g_scaleform->interfaceVersion < SKSEScaleformInterface::kInterfaceVersion)
		{
			_MESSAGE("Scaleform interface too old (%d, expected %d).", g_scaleform->interfaceVersion, SKSEScaleformInterface::kInterfaceVersion);
			return false;
		}
		
		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface * skse)
	{
		g_scaleform->Register("minimap", RegisterScaleform);
		g_papyrus = (SKSEPapyrusInterface *)skse->QueryInterface(kInterface_Papyrus);
		bool res = g_papyrus->Register(RegisterFuncs);

		if(!res)
		{
			_MESSAGE("Minimap register failed!");
			return false;
		}
		
		return true;
	}
};
