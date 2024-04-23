;BEGIN FRAGMENT CODE - Do not edit anything between this and the end comment
;NEXT FRAGMENT INDEX 10
Scriptname PRKF_MinimapLootPerk_01007E85 Extends Perk Hidden

;BEGIN FRAGMENT Fragment_7
Function Fragment_7(ObjectReference akTargetRef, Actor akActor)
;BEGIN CODE
Minimap.RemoveLootedCorpse(akTargetRef as Actor)
;END CODE
EndFunction
;END FRAGMENT

;END FRAGMENT CODE - Do not edit anything between this and the begin comment


MinimapMasterScript Property Minimap Auto