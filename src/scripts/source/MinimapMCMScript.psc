Scriptname MinimapMCMScript extends SKI_ConfigBase
import MinimapSKSE

MinimapMasterScript Property Minimap Auto

Int OID_TurnedOn
Int OID_Uninstall
Int OID_FactoryReset

Int OID_MaxActors
Int OID_Radius
Int OID_Scale
Int OID_PlayerScale
Int OID_IconScale
Int OID_Shape

Int OID_BGAlpha
Int OID_BarsAlpha
Int OID_IconAlpha

Int OID_AllowInside
Int OID_CheckAbove
Int OID_CheckBelow

Int OID_PosX
Int OID_PosY
Int OID_PulseRate

Int OID_ShowBars
Int OID_BarsColorR
Int OID_BarsColorG
Int OID_BarsColorB
Int OID_SimpleMode
Int OID_RemoveOnLoot

Int OID_Guards
Int OID_Followers
Int OID_Allies
Int OID_Neutrals
Int OID_Children
Int OID_Enemies
Int OID_Corpses
Int OID_Killed
Int OID_Horses
Int OID_Predators
Int OID_Prey
Int OID_Domestic
Int OID_Dogs
Int OID_Vampires
Int OID_Dragons
Int OID_Skeevers

Int OID_Blacksmith
Int OID_Innkeeper
Int OID_Merchant
Int OID_Spider
Int OID_Giant
Int OID_Mammmoth
Int OID_Fox
Int OID_Troll
Int OID_Horker
Int OID_Familiar

Int OID_ToggleKey
Int OID_ForceReset
Int OID_ZoomOut
Int OID_ZoomAmount

Int _toggleVisibleKey = 34
Int _forceResetKey = -1
Int _zoomOutKey = 48
Float[] _floatSettings
Bool[] _settings

Float verticalCheckAbove = 300.0
Float verticalCheckBelow = 140.0
Bool _installed = true
Int flag

Int _numFloatSettings = 13
Int _numSettings = 23
Float _barsColorR = 10.0
Float _barsColorG = 10.0
Float _barsColorB = 10.0

Event OnPageReset(string page)
    SetCursorFillMode(TOP_TO_BOTTOM)
    SetCursorPosition(0)

	; Is the radar not turned on (20) or Uninstalled (22)?
	if !_settings[20] || _settings[22]
		flag = OPTION_FLAG_DISABLED
	else
		flag = OPTION_FLAG_NONE
	endif

    If page == "$MM_PageMain"

		String installText = "$MM_TUninstall1"
		Int turnedOnFlag = OPTION_FLAG_NONE
		if !_installed
			turnedOnFlag = OPTION_FLAG_DISABLED
			installText = "$MM_TUninstall2"
		endif

		if _installed
			AddHeaderOption("$MM_HGeneral")

			OID_TurnedOn		= AddToggleOption("$MM_TTurnedOn", _settings[20], turnedOnFlag)
			OID_MaxActors		= AddSliderOption("$MM_TLimit", _floatSettings[9], "{0}", flag)
			OID_Radius			= AddSliderOption("$MM_TRadius", _floatSettings[0], "{0}", flag)
			OID_Scale			= AddSliderOption("$MM_TMasterScale", _floatSettings[6], "{0}%", flag)
			OID_PlayerScale		= AddSliderOption("$MM_TPlayerScale", _floatSettings[7], "{0}%", flag)
			OID_IconScale		= AddSliderOption("$MM_TIconScale", _floatSettings[8], "{0}%", flag)
			OID_Shape			= AddToggleOption("$MM_TCircle", _settings[19], flag)

			AddHeaderOption("$MM_HOpacity")
			OID_BGAlpha   = AddSliderOption("$MM_TBackgroundAlpha", _floatSettings[3], "{0}%", flag)
			OID_BarsAlpha = AddSliderOption("$MM_TBarsAlpha", _floatSettings[4], "{0}%", flag)
			OID_IconAlpha = AddSliderOption("$MM_TIconAlpha", _floatSettings[5], "{0}%", flag)

			AddHeaderOption("")
			OID_AllowInside = AddToggleOption("$MM_TAllowInside", _settings[21], flag)
			OID_CheckAbove  = AddSliderOption("$MM_TCheckAbove", _floatSettings[11], "{0}", flag)
			OID_CheckBelow  = AddSliderOption("$MM_TCheckBelow", _floatSettings[12], "{0}", flag)

		    SetCursorPosition(1)
	    	AddHeaderOption("$MM_HPosition")
	    	OID_PosX = AddSliderOption("$MM_TXPos", _floatSettings[1], "{0}", flag)
	    	OID_PosY = AddSliderOption("$MM_TYPos", _floatSettings[2], "{0}", flag)

	    	AddHeaderOption("")
	    	OID_PulseRate    = AddSliderOption("$MM_TPulseRate", _floatSettings[13], "{0} seconds", flag)
			OID_FactoryReset = AddTextOption("$MM_TFactoryReset", "")
	    EndIf
		
	    SetCursorPosition(21)
		OID_Uninstall = AddTextOption(installText, "")
    
    elseif page == "$MM_PageOptions"
    	If !_installed
    		Return
    	EndIf

    	OID_ShowBars   = AddToggleOption("$MM_TShowBars", _settings[0], flag)
    	OID_BarsColorR = AddSliderOption("$MM_TBarsColorR", _barsColorR, "{0}", flag)
    	OID_BarsColorG = AddSliderOption("$MM_TBarsColorG", _barsColorG, "{0}", flag)
    	OID_BarsColorB = AddSliderOption("$MM_TBarsColorB", _barsColorB, "{0}", flag)

    	OID_SimpleMode   = AddToggleOption("$MM_TSimpleMode", _settings[1], flag)
		OID_RemoveOnLoot = AddToggleOption("$MM_TRemoveLoot", _settings[18], flag)
		
		AddHeaderOption("")
		SetCursorPosition(1)
    	OID_Guards = AddToggleOption("$MM_OPGuards", _settings[2], flag)
    	OID_Followers = AddToggleOption("$MM_OPFollowers", _settings[3], flag)
    	OID_Allies = AddToggleOption("$MM_OPAllies", _settings[4], flag)
    	OID_Neutrals = AddToggleOption("$MM_OPNeutrals", _settings[5], flag)
    	OID_Children = AddToggleOption("$MM_OPChildren", _settings[6], flag)
    	OID_Enemies = AddToggleOption("$MM_OPEnemies", _settings[7], flag)
    	OID_Corpses = AddToggleOption("$MM_OPCorpses", _settings[8], flag)
    	OID_Killed = AddToggleOption("$MM_OPKilled", _settings[9], flag)
    	OID_Horses = AddToggleOption("$MM_OPHorses", _settings[10], flag)
    	OID_Predators = AddToggleOption("$MM_OPPredators", _settings[11], flag)
    	OID_Prey = AddToggleOption("$MM_OPPrey", _settings[12], flag)
    	OID_Domestic = AddToggleOption("$MM_OPDomestic", _settings[13], flag)
    	OID_Dogs = AddToggleOption("$MM_OPDogs", _settings[14], flag)
    	OID_Vampires = AddToggleOption("$MM_OPVampires", _settings[15], flag)
    	OID_Dragons = AddToggleOption("$MM_OPDragons", _settings[16], flag)
    	OID_Skeevers = AddToggleOption("$MM_OPSkeevers", _settings[17], flag)

    	OID_Blacksmith = AddToggleOption("$MM_OPBlacksmith", _settings[23], flag)
    	OID_Innkeeper = AddToggleOption("$MM_OPInnkeeper", _settings[24], flag)
    	OID_Merchant = AddToggleOption("$MM_OPMerchant", _settings[25], flag)
    	OID_Spider = AddToggleOption("$MM_OPSpider", _settings[26], flag)
    	OID_Giant = AddToggleOption("$MM_OPGiant", _settings[27], flag)
    	OID_Mammmoth = AddToggleOption("$MM_OPMammoth", _settings[28], flag)
    	OID_Fox = AddToggleOption("$MM_OPFox", _settings[29], flag)
    	OID_Troll = AddToggleOption("$MM_OPTroll", _settings[30], flag)
    	OID_Horker = AddToggleOption("$MM_OPHorker", _settings[31], flag)
    	; Settings (32) and (33) were previously reserved for falmer and forsworn.
    	OID_Familiar = AddToggleOption("$MM_OPFamiliar", _settings[34], flag)

    elseif page == "$MM_PageKeybinds"
    	if !_installed
    		Return
    	EndIf

		OID_ToggleKey = AddKeyMapOption("$MM_TToggleVisible", _toggleVisibleKey, OPTION_FLAG_WITH_UNMAP)
		OID_ForceReset = AddKeyMapOption("$MM_TForceReset", _forceResetKey, OPTION_FLAG_WITH_UNMAP)
		OID_ZoomOut = AddKeyMapOption("$MM_TZoomOut", _zoomOutKey, OPTION_FLAG_WITH_UNMAP)
		OID_ZoomAmount = AddSliderOption("$MM_TZoomAmount", _floatSettings[10], "{0}")
	Endif
EndEvent

Event OnOptionHighlight(Int Option)
	If Option == OID_TurnedOn
        SetInfoText("$MM_DTurnedOn")
    ElseIf Option == OID_Uninstall
    	If _installed
        	SetInfoText("$MM_DUninstall1")
		Else
			SetInfoText("$MM_DUninstall2")
		EndIf
	ElseIf Option == OID_FactoryReset
		SetInfoText("$MM_DFactoryReset")
	ElseIf Option == OID_MaxActors
        SetInfoText("$MM_DMaxObjects")
    ElseIf Option == OID_Radius
        SetInfoText("$MM_DRadius")
    ElseIf Option == OID_Scale
        SetInfoText("$MM_DScale")
    ElseIf Option == OID_PlayerScale
        SetInfoText("$MM_DPlayerScale")
    ElseIf Option == OID_IconScale
        SetInfoText("$MM_DIconScale")
    Elseif option == OID_Shape
    	SetInfoText("$MM_DShape")

    ElseIf Option == OID_BGAlpha
		SetInfoText("$MM_DBGAlpha")
    ElseIf Option == OID_BarsAlpha
    	SetInfoText("$MM_DBarsAlpha")
    ElseIf Option == OID_IconAlpha
		SetInfoText("$MM_DIconAlpha")

    elseif option == OID_AllowInside
    	SetInfoText("$MM_DAllowInside")
    elseif option == OID_CheckAbove
    	SetInfoText("$MM_DCheckAbove")
	elseif option == OID_CheckBelow
    	SetInfoText("$MM_DCheckBelow")

    ElseIf Option == OID_PosX
        SetInfoText("$MM_DXPos")
    ElseIf Option == OID_PosY
        SetInfoText("$MM_DYPos")
    ElseIf option == OID_PulseRate
    	SetInfoText("$MM_DPulseRate")

    ElseIf Option == OID_ToggleKey
		SetInfoText("$MM_DToggleVisible")
	ElseIf Option == OID_ForceReset
		SetInfoText("$MM_DForceReset")
	ElseIf Option == OID_ZoomOut
		SetInfoText("$MM_DZoomOut")
	ElseIf Option == OID_ZoomAmount
		SetInfoText("$MM_DZoomAmount")

	ElseIf Option == OID_ShowBars
		SetInfoText("$MM_DShowBars")
	ElseIf Option == OID_SimpleMode
		SetInfoText("$MM_DSimpleMode")
	ElseIf Option == OID_RemoveOnLoot
		SetInfoText("$MM_DRemoveOnLoot")

	ElseIf Option == OID_Guards
		SetInfoText("$MM_DGuards")
	ElseIf Option == OID_Followers
		SetInfoText("$MM_DFollowers")
	ElseIf Option == OID_Allies
		SetInfoText("$MM_DAllies")
	ElseIf Option == OID_Neutrals
		SetInfoText("$MM_DNeutrals")
	ElseIf Option == OID_Children
		SetInfoText("$MM_DChildren")
	ElseIf Option == OID_Enemies
		SetInfoText("$MM_DEnemies")
	ElseIf Option == OID_Corpses
		SetInfoText("$MM_DCorpses")
	ElseIf Option == OID_Killed
		SetInfoText("$MM_DKilled")
	ElseIf Option == OID_Horses
		SetInfoText("$MM_DHorses")
	ElseIf Option == OID_Predators
		SetInfoText("$MM_DPredators")
	ElseIf Option == OID_Prey
		SetInfoText("$MM_DPrey")
	ElseIf Option == OID_Domestic
		SetInfoText("$MM_DDomestic")
	ElseIf Option == OID_Dogs
		SetInfoText("$MM_DDogs")
	ElseIf Option == OID_Vampires
		SetInfoText("$MM_DVampires")
	ElseIf Option == OID_Dragons
		SetInfoText("$MM_DDragons")
	ElseIf Option == OID_Skeevers
		SetInfoText("$MM_DSkeevers")
	ElseIf Option == OID_Blacksmith
		SetInfoText("$MM_DBlacksmith")
	ElseIf Option == OID_Innkeeper
		SetInfoText("$MM_DInnkeeper")
	ElseIf Option == OID_Merchant
		SetInfoText("$MM_DMerchant")
	ElseIf Option == OID_Spider
		SetInfoText("$MM_DSpider")
	ElseIf Option == OID_Giant
		SetInfoText("$MM_DGiant")
	ElseIf Option == OID_Mammmoth
		SetInfoText("$MM_DMammoth")
	ElseIf Option == OID_Fox
		SetInfoText("$MM_DFox")
	ElseIf Option == OID_Troll
		SetInfoText("$MM_DTroll")
	ElseIf Option == OID_Horker
		SetInfoText("$MM_DHorker")
	ElseIf Option == OID_Familiar
		SetInfoText("$MM_DFamiliar")
    Endif
EndEvent
Event OnOptionSliderOpen(Int Option)
	If option == OID_MaxActors
		SetSliderDialogStartValue(_floatSettings[9])
		SetSliderDialogDefaultValue(32.0)
		SetSliderDialogRange(1.0, 128.0)
		SetSliderDialogInterval(1.0)
	elseIf option == OID_Radius
        SetSliderDialogStartValue(_floatSettings[0])
        SetSliderDialogDefaultValue(3000.0)
		SetSliderDialogRange(1000, 16000)
        SetSliderDialogInterval(100.0)
    elseIf option == OID_Scale
        SetSliderDialogStartValue(_floatSettings[6])
        SetSliderDialogDefaultValue(160.0)
        SetSliderDialogRange(40.0, 200.0)
        SetSliderDialogInterval(10.0)
	elseIf option == OID_PlayerScale
        SetSliderDialogStartValue(_floatSettings[7])
        SetSliderDialogDefaultValue(100.0)
        SetSliderDialogRange(20.0, 200.0)
        SetSliderDialogInterval(10.0)
    elseIf option == OID_IconScale
        SetSliderDialogStartValue(_floatSettings[8])
        SetSliderDialogDefaultValue(100.0)
        SetSliderDialogRange(40.0, 800.0)
        SetSliderDialogInterval(10.0)
	ElseIf option == OID_BGAlpha
        SetSliderDialogStartValue(_floatSettings[3])
        SetSliderDialogDefaultValue(20.0)
        SetSliderDialogRange(0.0, 100.0)
        SetSliderDialogInterval(1.0)
    ElseIf option == OID_BarsAlpha
        SetSliderDialogStartValue(_floatSettings[4])
        SetSliderDialogDefaultValue(75.0)
        SetSliderDialogRange(0.0, 100.0)
        SetSliderDialogInterval(1.0)
    ElseIf option == OID_IconAlpha
        SetSliderDialogStartValue(_floatSettings[5])
        SetSliderDialogDefaultValue(100.0)
        SetSliderDialogRange(0.0, 100.0)
        SetSliderDialogInterval(1.0)
   ElseIf option == OID_PosX
        SetSliderDialogStartValue(_floatSettings[1])
        SetSliderDialogDefaultValue(0.0)
        SetSliderDialogRange(-60.0, 1280.0 - 180.0 * _floatSettings[6]/100.0)
        SetSliderDialogInterval(5.0)
    ElseIf option == OID_PosY
        SetSliderDialogStartValue(_floatSettings[2])
        SetSliderDialogDefaultValue(0.0)
        SetSliderDialogRange(-60.0, 720.0 - 180.0 * _floatSettings[6]/100.0 * MinimapGetRectRatio())
        SetSliderDialogInterval(5.0)
	elseIf option == OID_ZoomAmount
        SetSliderDialogStartValue(_floatSettings[10])
		SetSliderDialogDefaultValue(3000.0)
		SetSliderDialogRange(1000.0, 12000.0)
		SetSliderDialogInterval(100.0)
	elseIf option == OID_CheckAbove
        SetSliderDialogStartValue(_floatSettings[11])
		SetSliderDialogDefaultValue(300.0)
		SetSliderDialogRange(0.0, 2000.0)
		SetSliderDialogInterval(20.0)
	elseIf option == OID_CheckBelow
        SetSliderDialogStartValue(_floatSettings[12])
		SetSliderDialogDefaultValue(180.0)
		SetSliderDialogRange(0.0, 2000.0)
		SetSliderDialogInterval(20.0)
	elseIf option == OID_PulseRate
        SetSliderDialogStartValue(_floatSettings[13])
		SetSliderDialogDefaultValue(6.0)
		SetSliderDialogRange(2.0, 30.0)
		SetSliderDialogInterval(1.0)
	elseIf option == OID_BarsColorR
        SetSliderDialogStartValue(_barsColorR)
		SetSliderDialogDefaultValue(40.0)
		SetSliderDialogRange(1.0, 255.0)
		SetSliderDialogInterval(1.0)
	elseIf option == OID_BarsColorG
        SetSliderDialogStartValue(_barsColorG)
		SetSliderDialogDefaultValue(40.0)
		SetSliderDialogRange(1.0, 255.0)
		SetSliderDialogInterval(1.0)
	elseIf option == OID_BarsColorB
        SetSliderDialogStartValue(_barsColorB)
		SetSliderDialogDefaultValue(40.0)
		SetSliderDialogRange(1.0, 255.0)
		SetSliderDialogInterval(1.0)
	EndIf
EndEvent

Int function GetVersion()
	return 4
EndFunction
event OnVersionUpdate(int v)
	if v > 1
		; Debug.trace("Updating Radar to Version " + v)
		DoInitialize()
	endIf
endEvent
Event OnConfigClose()
	ApplySettings()
endevent
function ApplySettings()
	Minimap.ApplySettings(_floatSettings, _settings, _barsColorR, _barsColorG, _barsColorB)
endfunction
Event OnOptionSliderAccept(Int option, Float value)
	If option == OID_MaxActors
		_floatSettings[9] = value
		SetSliderOptionValue(option, value, "{0}")
	ElseIf option == OID_Radius
		_floatSettings[0] = value
        SetSliderOptionValue(option, value, "{0}")
    ElseIf option == OID_Scale
		_floatSettings[6] = value
        SetSliderOptionValue(option, value, "{0}%")

        ; Try to limit radar to not go off the screen to the right.
		Float maxRight = 1280.0 - 180.0 * value/100.0 - 20.0
		if _floatSettings[1] > maxRight
			_floatSettings[1] = maxRight
			SetSliderOptionValue(OID_PosX, _floatSettings[1], "{0}")
		EndIf

        ; Try to limit radar to not go off the screen at the bottom.
		Float maxDown = 720.0 - 180.0 * value/100.0 * MinimapGetRectRatio() - 20.0
		if _floatSettings[2] > maxDown
			_floatSettings[2] = maxDown
			SetSliderOptionValue(OID_PosY, _floatSettings[2], "{0}")
		endif

	ElseIf option == OID_PlayerScale
		_floatSettings[7] = value
        SetSliderOptionValue(option, value, "{0}%")
    ElseIf option == OID_IconScale
		_floatSettings[8] = value
        SetSliderOptionValue(option, value, "{0}%")
	ElseIf option == OID_PosX
		_floatSettings[1] = value
		SetSliderOptionValue(option, value, "{0}")
	ElseIf option == OID_PosY
		_floatSettings[2] = value
		SetSliderOptionValue(option, value, "{0}")
	ElseIf option == OID_BGAlpha
		_floatSettings[3] = value
		SetSliderOptionValue(option, value, "{0}%")
	ElseIf option == OID_BarsAlpha
		_floatSettings[4] = value
		SetSliderOptionValue(option, value, "{0}%")
	ElseIf option == OID_IconAlpha
		_floatSettings[5] = value
		SetSliderOptionValue(option, value, "{0}%")
	ElseIf option == OID_ZoomAmount
		_floatSettings[10] = value
		SetSliderOptionValue(option, value, "{0}")
	ElseIf option == OID_CheckAbove
		_floatSettings[11] = value
		SetSliderOptionValue(option, value, "{0}")
	ElseIf option == OID_CheckBelow
		_floatSettings[12] = value
		SetSliderOptionValue(option, value, "{0}")
	ElseIf option == OID_PulseRate
		_floatSettings[13] = value
		SetSliderOptionValue(option, value, "{0} seconds")
	ElseIf option == OID_BarsColorR
		_barsColorR = value
		SetSliderOptionValue(option, value, "{0}")
	ElseIf option == OID_BarsColorG
		_barsColorG = value
		SetSliderOptionValue(option, value, "{0}")
	ElseIf option == OID_BarsColorB
		_barsColorB = value
		SetSliderOptionValue(option, value, "{0}")
	Endif
EndEvent
Event OnOptionSelect(int option)
	Int idx = -1
	if option == OID_TurnedOn
		idx = 20
		ForcePageReset()
	elseif option == OID_ShowBars
		idx = 0
	elseif option == OID_SimpleMode
		idx = 1
	elseif option == OID_Guards
		idx = 2
	elseif option == OID_Followers
		idx = 3
	elseif option == OID_Allies
		idx = 4
	elseif option == OID_Neutrals
		idx = 5
	elseif option == OID_Children
		idx = 6
	elseif option == OID_Enemies
		idx = 7
	elseif option == OID_Corpses
		idx = 8
	elseif option == OID_Killed
		idx = 9
	elseif option == OID_Horses
		idx = 10
	elseif option == OID_Predators
		idx = 11
	elseif option == OID_Prey
		idx = 12
	elseif option == OID_Domestic
		idx = 13
	elseif option == OID_Dogs
		idx = 14
	elseif option == OID_Vampires
		idx = 15
	elseif option == OID_Dragons
		idx = 16
	elseif option == OID_Skeevers
		idx = 17
	elseif option == OID_RemoveOnLoot
		idx = 18
	elseif option == OID_Shape
		idx = 19
	; (20) is at the top of this if/else.
	elseif option == OID_AllowInside
		idx = 21
	elseif option == OID_Uninstall
		idx = 22
	ElseIf option == OID_Blacksmith
		idx = 23
	ElseIf option == OID_Innkeeper
		idx = 24
	ElseIf option == OID_Merchant
		idx = 25
	ElseIf option == OID_Spider
		idx = 26
	ElseIf option == OID_Giant
		idx = 27
	ElseIf option == OID_Mammmoth
		idx = 28
	ElseIf option == OID_Fox
		idx = 29
	ElseIf option == OID_Troll
		idx = 30
	ElseIf option == OID_Horker
		idx = 31
	ElseIf option == OID_Familiar
		idx = 34
	ElseIf option == OID_FactoryReset
		DoFactorySettings()
		DoFactorySettings2()
		; ApplySettings()
		ForcePageReset()
	endif

	if idx >= 0
		_settings[idx] = !_settings[idx]
		SetToggleOptionValue(option, _settings[idx])

		; (22) = Uninstall
		if idx == 22
			SetInstalled(!_installed)
			ForcePageReset()
		endif
	endIf
EndEvent

Function DoFactorySettings()

	_floatSettings = new Float[14]
	_settings = new Bool[35]
	_numFloatSettings = 14
	_numSettings = 35

	Int i = 2
	while i < _numSettings
		_settings[i] = true
		i += 1
	EndWhile

	_settings[0] = False ; Show bars
	_settings[1] = False ; Simple mode
	_settings[2] = False ; Killed
	_settings[8] = False ; Corpses
	_settings[11] = False ; Predators
	_settings[12] = False ; Prey
	_settings[13] = False ; Domestic
	_settings[22] = False ; Uninstall
	_settings[29] = False ; Fox

	_floatSettings[0] = 2600.0
	_floatSettings[1] = 25.0
	_floatSettings[2] = 25.0
	_floatSettings[3] = 45.0
	_floatSettings[4] = 75.0
	_floatSettings[5] = 100.0
	_floatSettings[6] = 140.0
	_floatSettings[7] = 100.0
	_floatSettings[8] = 100.0
	_floatSettings[9] = 128.0
	_floatSettings[10] = 3400.0
	_floatSettings[11] = verticalCheckAbove
	_floatSettings[12] = verticalCheckBelow
	_floatSettings[13] = 6.0

EndFunction

Function DoFactorySettings2()

	_barsColorR = 4.0
	_barsColorG = 4.0
	_barsColorB = 4.0

EndFunction

; To be called once on first run and once every new version.
Function DoInitialize()
	Pages = new String[3]
	Pages[0] = "$MM_PageMain"
	Pages[1] = "$MM_PageOptions"
	Pages[2] = "$MM_PageKeybinds"
	; ModName = "$MM_ModName"

	; This is true if it is the first run of this function.
	if !_floatSettings
		DoFactorySettings()
	; Update radar from V0.1 to V0.2b if necessary.
	ElseIf _numSettings < 35 || _numFloatSettings < 14
		Float[] _oldFloats = _floatSettings
		Bool[] _oldSettings = _settings
		_floatSettings = new Float[14]
		_settings = new Bool[35]

		Int i = 0
		; _numFloatSettings represents the old (V0.1) array size.
		while i < _numFloatSettings
			_floatSettings[i] = _oldFloats[i]
			i += 1
		EndWhile

		_floatSettings[13] = 6.0
		_numFloatSettings = 14

		i = 0
		; _numSettings represents the old (V0.1) array size.
		while i < _numSettings
			_settings[i] = _oldSettings[i]
			i += 1
		EndWhile

		_numSettings = 35
		While i < _numSettings
			_settings[i] = True
			i += 1
		EndWhile
	endif
endfunction

Function SetInstalled(Bool installed)
	_installed = installed
	Minimap.SetInstalled(installed)

	if !installed
		UnregisterForAllKeys()
	Else
		EveryLoad()
	endif
endfunction

Function EveryLoad()
	RegisterForKey(_toggleVisibleKey)
	RegisterForKey(_forceResetKey)
	RegisterForKey(_zoomOutKey)
EndFunction

Event OnOptionKeymapChange(Int option, Int code, String conflictControl, String conflictName)
	bool continue = true
	if conflictControl != ""
		string msg = ""
		if conflictName != ""
			msg = "Key is already used by:\n" + conflictControl + "\n(" + conflictName + ")\nDo you still want to continue?"
		else
			msg = "Key is already used by:\n" + conflictControl + "\nDo you still want to continue?"
		endIf

		continue = ShowMessage(msg, True, "$MM_KeybindYes", "$MM_KeybindNo")
	endIf

	if continue
		if option == OID_ForceReset
			_forceResetKey = SetBind(_forceResetKey, code)
			SetKeyMapOptionValue(option, _forceResetKey)
		elseif option == OID_ZoomOut
			_zoomOutKey = SetBind(_zoomOutKey, code)
			SetKeyMapOptionValue(option, _zoomOutKey)
		elseif option == OID_ToggleKey
			_toggleVisibleKey = SetBind(_toggleVisibleKey, code)
			SetKeyMapOptionValue(option, _toggleVisibleKey)
		endif
	endif
EndEvent

Int Function SetBind(Int oldCode, Int newCode)
	UnregisterForKey(oldCode)
	RegisterForKey(newCode)
	return newCode
endfunction

Event OnKeyDown(Int code)
	if code == _toggleVisibleKey
		MinimapToggleVisible()
	elseif code == _forceResetKey
		Debug.Notification("Forcing radar reset...")
		Minimap.DoReset()
	elseif code == _zoomOutKey
		Minimap.ZoomIn()
	Endif
EndEvent

string function GetCustomControl(int code)
	if code == _toggleVisibleKey
		return "Toggle Radar Visibility"
	elseif code == _forceResetKey
		return "Force Radar Reset"
	elseif code == _zoomOutKey
		return "Zoom Out Radar"
	else
		return ""
	endIf
endFunction

event OnOptionDefault(Int option)
	If option == OID_ToggleKey
		_toggleVisibleKey = SetBind(_toggleVisibleKey, 34)
	ElseIf option == OID_ForceReset
		_forceResetKey = SetBind(_forceResetKey, 35)
	elseif option == OID_ZoomOut
		_zoomOutKey = SetBind(_zoomOutKey, 48)
	endif
EndEvent

Event OnInit()
	Parent.OnInit()
	DoInitialize()
EndEvent
