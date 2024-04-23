scriptName MinimapSKSE Hidden

Bool Function MinimapAdd(Actor a) global native
Int Function MinimapRemove(Actor a) global native

Function MinimapClear() global native
Function MinimapSet(Actor a, Int newID) global native
Function MinimapSetVisible(Bool visible) global native

Function MinimapStartZoom() global native
Function MinimapSetFactions(Faction[] factions) global native
Function MinimapUpdateSettings(Float[] floatSettings, Bool[] settings, Float barsColorR, Float barsColorG, Float barsColorB) global native
Function MinimapSetInside(bool inside) global native
Function MinimapToggleVisible() global native

Float Function MinimapGetRectRatio() global native
