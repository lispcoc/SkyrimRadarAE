Scriptname MinimapMasterScript extends SKI_WidgetBase
import MinimapSKSE

Actor Property player Auto
MinimapPlayerScript Property PlayerScript Auto
Faction[] Property IconFactions2 Auto ; Renamed to "reset" the variable in existing savegames.

Int _numActors
MinimapMonitorScript[] Scripts
Bool _removeOnLoot = True
Bool _installed = True
Float _zoomedRadius = 0.0
Float _radius = 0.0
Bool _resetting = False
Bool _arrayLock = False

Function AddToRadar(Actor target, MinimapMonitorScript script)
	if _numActors < 0
		_numActors = 0
	EndIf

	if target == None
		Debug.Trace("-- Attempt to add null to radar!")
		return
	EndIf
	
	while _arrayLock
		Utility.Wait(0.1)
	endwhile

	_arrayLock = True
	If MinimapAdd(target)
		Scripts[_numActors] = script
		_numActors += 1
	EndIf
	_arrayLock = False
EndFunction

Function ZoomIn()
	MinimapStartZoom()
	PlayerScript.SetRadius(_zoomedRadius)
	Utility.Wait(0.0001)
	PlayerScript.ForcePulse()
	PlayerScript.SetRadius(_radius)
EndFunction

Function DoReset()
	_resetting = True
	MinimapClear()
	MinimapMonitorScript _back
	while _numActors > 0
		while _arrayLock
			Utility.Wait(0.1)
		endwhile

		_arrayLock = True
		_back = Scripts[_numActors - 1]
		if _back
			_back.ForceDispel()
		EndIf
		_numActors -= 1
		_arrayLock = False
	EndWhile
	_resetting = False
	Utility.Wait(0.1)
	PlayerScript.ForcePulse()
EndFunction

Function NewLocation(Bool nowInside)
	MinimapSetInside(nowInside)
	DoReset()
EndFunction

Function ApplySettings(Float[] _floatSettings, Bool[] _settings, Float _barsColorR, Float _barsColorG, Float _barsColorB)
	MinimapUpdateSettings(_floatSettings, _settings, _barsColorR, _barsColorG, _barsColorB)
	MinimapSetInside(player.IsInInterior())
	_radius = _floatSettings[0]
	PlayerScript.SetRadius(_radius)
	_removeOnLoot = _settings[18]
	_zoomedRadius = _floatSettings[12] + _radius
	PlayerScript.SetPulseRate(_floatSettings[13])
	Utility.Wait(0.05)
	DoReset()
endfunction

Function RemoveLootedCorpse(Actor target)
	if !_removeOnLoot
		Return
	EndIf
	SetActor(target, 200)
EndFunction

Function RemoveFromRadar(Actor target)
	If _resetting
		Return
	EndIf
	if target == None
		Debug.Trace("-- Rejected removal from radar.")
		Return
	EndIf	
	Int result = MinimapRemove(target)

	if result >= 0
		Int i = result
		_numActors -= 1

		while i < _numActors
			Scripts[i] = Scripts[i + 1]
			i += 1
		endwhile
	EndIf
EndFunction

Function SetActor(Actor target, Int ID)
	MinimapSet(target, ID)
EndFunction

Event OnWidgetReset()
	MinimapSetFactions(IconFactions2)
	Parent.OnWidgetReset()
	MinimapSetVisible(True)
EndEvent

Event OnInit()
	Parent.OnInit()
	_numActors = 0
	Scripts = new MinimapMonitorScript[64]
EndEvent

Function SetInstalled(Bool installed)
	If installed
		RegisterForSingleUpdate(1.0)
		_installed = True
	Else		
		UnregisterForUpdate()
		_installed = False
	EndIf
	PlayerScript.SetInstalled(installed)
EndFunction

String Function GetWidgetSource()
	Return "skyrim_minimap/minimap.swf"
EndFunction

String Function GetWidgetType()
	Return "MinimapMasterScript"
EndFunction
