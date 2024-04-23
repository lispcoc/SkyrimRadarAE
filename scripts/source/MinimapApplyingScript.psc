Scriptname MinimapApplyingScript extends ActiveMagicEffect

Spell Property MonitorAbility Auto
 
Event OnEffectStart(Actor target, Actor caster)
	target.AddSpell(MonitorAbility)
EndEvent