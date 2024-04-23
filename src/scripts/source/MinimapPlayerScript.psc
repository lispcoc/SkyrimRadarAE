Scriptname MinimapPlayerScript extends ReferenceAlias  

Spell Property CloakAbility Auto
Actor Property player Auto
Perk Property OnLoot Auto
MinimapMasterScript Property Minimap Auto
MinimapMCMScript Property MCM Auto

Bool pulseLock = false
Bool playerWasInside = true
Bool enabled = true
Float _pulseRate = 6.0

Function EveryLoad()
	player.RemovePerk(OnLoot)
	player.AddPerk(OnLoot)
	MCM.EveryLoad()
endfunction

Event OnInit()
	EveryLoad()
	MCM.ApplySettings()
	RegisterForSingleUpdate(1)
endevent

Event OnPlayerLoadGame()
	if enabled
		EveryLoad()
		Minimap.NewLocation(player.IsInInterior())
		Utility.Wait(0.25)
		MCM.ApplySettings()
	endif
endEvent

Event OnLocationChange(Location oldLoc, Location newLoc)
	bool inside = player.IsInInterior()
	; Did player go from an exterior to interior (or vice versa)?
	if(inside || (playerWasInside && !inside))
		Minimap.NewLocation(inside)
	endIf
	playerWasInside = inside
endEvent

Function SetInstalled(Bool installed)
	if installed
		enabled = True
		RegisterForSingleUpdate(1.0)
	else
		enabled = False
		UnregisterForUpdate()
	endif
endfunction

Function ForcePulse()
	while pulseLock
		Utility.Wait(0.05)
	EndWhile
	pulseLock = True
	player.AddSpell(CloakAbility, false)
	Utility.Wait(1.0)
	player.RemoveSpell(CloakAbility)
	pulseLock = False
endfunction

Function SetRadius(Float radius)
	; " * 0.047" converts game units to feet, then "+ 96.0" is safety padding to make the cloak radius slightly bigger than what the radar is visually.
	; This sets CloakEffect's magnitude.
	CloakAbility.SetNthEffectMagnitude(0, radius * 0.047 + 96.0)
endFunction

Function SetPulseRate(Float rate)
	_pulseRate = rate
endfunction
 
Event OnUpdate() 
	ForcePulse()
	RegisterForSingleUpdate(_pulseRate)
EndEvent
