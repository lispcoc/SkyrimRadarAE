Scriptname MinimapMonitorScript extends ActiveMagicEffect

Bool Property ForcedDispel = False Auto
Actor Property player Auto
Quest Property Minimap Auto

; String name
MinimapMasterScript QuestScript
MinimapMonitorScript SelfScript
Actor SelfActor
 
Event OnEffectStart(Actor target, Actor caster)
	SelfActor = target
	SelfScript = Self
	QuestScript = Minimap as MinimapMasterScript
	; name = SelfActor.GetActorBase().GetName()
	QuestScript.AddToRadar(SelfActor, Self)
EndEvent

Function ForceDispel()
	ForcedDispel = True
	If SelfScript
		SelfScript.Dispel()
	EndIf
endfunction

Event OnEffectFinish(Actor target, Actor caster)
	if ForcedDispel
		Return
	endif
	if QuestScript && SelfActor && target
		QuestScript.RemoveFromRadar(SelfActor)
	endif
	GoToState("Finished")
EndEvent

; This MagicEffect doesn't dispel on death.
Event OnDying(Actor killer)
	if QuestScript && SelfActor
		if killer == player || killer.IsInFaction(QuestScript.IconFactions2[0]) || SelfActor.GetRelationshipRank(player) >= 1
			QuestScript.SetActor(SelfActor, 12)
		else
			QuestScript.SetActor(SelfActor, 5)
		endif
	endif
EndEvent

State Finished
endstate
