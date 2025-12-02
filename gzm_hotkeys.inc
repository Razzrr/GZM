|- GZM Hotkeys - Keyboard Control System
|- Version 1.0
|-
|- Provides hotkey bindings for quick macro control without GUI
|- Uses MQ2 /bind command for reliable key handling
|-
|- Default Key Layout:
|-   F1  = Pause/Resume
|-   F2  = Burn
|-   F3  = Toggle Heals
|-   F4  = Toggle DPS
|-   F5  = Toggle Mez
|-   F6  = Toggle Pull
|-   F7  = Toggle Buffs
|-   F8  = Toggle Pet
|-   F9  = Make Camp Here
|-   F10 = Back Off (stop combat)
|-   F11 = Chase MA
|-   F12 = Stop/Stay Here
|-
|- Usage: Hotkeys are auto-registered when macro starts
|- Customize: Edit the key assignments in InitHotkeys

|- ============================================================================
|- INITIALIZATION
|- ============================================================================

Sub InitHotkeys
    /if (!${Defined[HotkeysEnabled]}) /declare HotkeysEnabled bool outer TRUE

    |- Only setup if enabled
    /if (!${HotkeysEnabled}) /return

    /echo \ag[GZM-Keys] Setting up hotkeys...

    |- Core Controls
    /squelch /bind GZM_Pause       F1 /gzmpause
    /squelch /bind GZM_Burn        F2 /burn
    /squelch /bind GZM_Heals       F3 /gzmtoggle HealsOn
    /squelch /bind GZM_DPS         F4 /gzmtoggle DPSOn

    |- Feature Controls
    /squelch /bind GZM_Mez         F5 /gzmtoggle MezOn
    /squelch /bind GZM_Pull        F6 /gzmtoggle PullOn
    /squelch /bind GZM_Buffs       F7 /gzmtoggle BuffsOn
    /squelch /bind GZM_Pet         F8 /gzmtoggle PetOn

    |- Movement Controls
    /squelch /bind GZM_Camp        F9 /gzmcamp
    /squelch /bind GZM_BackOff     F10 /backoff
    /squelch /bind GZM_Chase       F11 /chaseme
    /squelch /bind GZM_Stay        F12 /stayhere

    |- Shift+Key for less common toggles
    /squelch /bind GZM_Melee       Shift+F3 /gzmtoggle MeleeOn
    /squelch /bind GZM_Assist      Shift+F4 /gzmtoggle AssistOn
    /squelch /bind GZM_Slow        Shift+F5 /gzmtoggle SlowOn
    /squelch /bind GZM_Charm       Shift+F6 /gzmtoggle CharmOn
    /squelch /bind GZM_Loot        Shift+F7 /gzmtoggle LootOn
    /squelch /bind GZM_Items       Shift+F8 /gzmtoggle ItemClicksOn

    |- Ctrl+Key for value adjustments
    /squelch /bind GZM_HealUp      Ctrl+F3 /gzmadjust HealPct 5
    /squelch /bind GZM_HealDown    Ctrl+Shift+F3 /gzmadjust HealPct -5
    /squelch /bind GZM_AssistUp    Ctrl+F4 /gzmadjust AssistAt 5
    /squelch /bind GZM_AssistDown  Ctrl+Shift+F4 /gzmadjust AssistAt -5

    |- Alt+Key for group commands (DanNet)
    /squelch /bind GZM_GrpPause    Alt+F1 /dgge /varset Paused 1
    /squelch /bind GZM_GrpResume   Alt+Shift+F1 /dgge /varset Paused 0
    /squelch /bind GZM_GrpBurn     Alt+F2 /dgge /burn
    /squelch /bind GZM_GrpBackOff  Alt+F10 /dgge /backoff

    /echo \ag[GZM-Keys] Hotkeys ready - F1=Pause F2=Burn F10=BackOff
/return

|- Remove all GZM hotkey bindings
Sub ClearHotkeys
    /squelch /bind GZM_Pause      nope
    /squelch /bind GZM_Burn       nope
    /squelch /bind GZM_Heals      nope
    /squelch /bind GZM_DPS        nope
    /squelch /bind GZM_Mez        nope
    /squelch /bind GZM_Pull       nope
    /squelch /bind GZM_Buffs      nope
    /squelch /bind GZM_Pet        nope
    /squelch /bind GZM_Camp       nope
    /squelch /bind GZM_BackOff    nope
    /squelch /bind GZM_Chase      nope
    /squelch /bind GZM_Stay       nope
    /squelch /bind GZM_Melee      nope
    /squelch /bind GZM_Assist     nope
    /squelch /bind GZM_Slow       nope
    /squelch /bind GZM_Charm      nope
    /squelch /bind GZM_Loot       nope
    /squelch /bind GZM_Items      nope
    /squelch /bind GZM_HealUp     nope
    /squelch /bind GZM_HealDown   nope
    /squelch /bind GZM_AssistUp   nope
    /squelch /bind GZM_AssistDown nope
    /squelch /bind GZM_GrpPause   nope
    /squelch /bind GZM_GrpResume  nope
    /squelch /bind GZM_GrpBurn    nope
    /squelch /bind GZM_GrpBackOff nope
    /echo \ay[GZM-Keys] Hotkeys cleared
/return

|- ============================================================================
|- BIND HANDLERS - Called by hotkey presses
|- ============================================================================

|- /gzmpause - Toggle pause state
Sub Bind_GZMPause
    /if (${Paused}) {
        /varset Paused 0
        /echo \ag[GZM] RESUMED
    } else {
        /varset Paused 1
        /echo \ay[GZM] PAUSED
    }
/return

|- /gzmtoggle - Toggle any boolean variable
Sub Bind_GZMToggle(varName)
    /if (!${Defined[${varName}]}) {
        /echo \ar[GZM] Unknown variable: ${varName}
        /return
    }

    /declare currentVal int local ${${varName}}
    /if (${currentVal}) {
        /varset ${varName} 0
        /echo \ay[GZM] ${varName} = OFF
    } else {
        /varset ${varName} 1
        /echo \ag[GZM] ${varName} = ON
    }
/return

|- /gzmadjust - Adjust numeric variable by amount
Sub Bind_GZMAdjust(varName, amount)
    /if (!${Defined[${varName}]}) {
        /echo \ar[GZM] Unknown variable: ${varName}
        /return
    }

    /declare currentVal int local ${${varName}}
    /declare newVal int local ${Math.Calc[${currentVal}+${amount}]}

    |- Clamp to reasonable bounds
    /if (${newVal} < 1) /varset newVal 1
    /if (${newVal} > 100) /varset newVal 100

    /varset ${varName} ${newVal}
    /echo \ag[GZM] ${varName} = ${newVal}
/return

|- /gzmcamp - Make camp at current location
Sub Bind_GZMCamp
    /varset CampXLoc ${Me.X}
    /varset CampYLoc ${Me.Y}
    /varset CampZLoc ${Me.Z}
    /varset ReturnToCamp 1
    /echo \ag[GZM] Camp set at ${Int[${Me.Y}]}, ${Int[${Me.X}]}
/return

|- ============================================================================
|- HUD CONTROL
|- ============================================================================

|- Load GZM HUD
Sub LoadGZMHud
    /squelch /hud load GZM_HUD
    /squelch /hud gzm 1
    /echo \ag[GZM-HUD] HUD loaded
/return

|- Unload GZM HUD
Sub UnloadGZMHud
    /squelch /hud gzm 0
    /echo \ay[GZM-HUD] HUD hidden
/return

|- Toggle GZM HUD visibility
Sub ToggleGZMHud
    /squelch /hud gzm toggle
/return

|- ============================================================================
|- QUICK REFERENCE - Show hotkey help
|- ============================================================================

Sub ShowHotkeyHelp
    /echo \ag================================
    /echo \ag  GZM Hotkey Reference
    /echo \ag================================
    /echo \aw F1  = Pause/Resume
    /echo \aw F2  = Burn
    /echo \aw F3  = Toggle Heals
    /echo \aw F4  = Toggle DPS
    /echo \aw F5  = Toggle Mez
    /echo \aw F6  = Toggle Pull
    /echo \aw F7  = Toggle Buffs
    /echo \aw F8  = Toggle Pet
    /echo \aw F9  = Make Camp
    /echo \aw F10 = Back Off
    /echo \aw F11 = Chase MA
    /echo \aw F12 = Stay Here
    /echo \ag--------------------------------
    /echo \aw Shift+F3-F8 = More Toggles
    /echo \aw Ctrl+F3/F4  = Adjust Values
    /echo \aw Alt+F1/F2   = Group Commands
    /echo \ag================================
/return

|- ============================================================================
|- BIND COMMAND HANDLERS FOR /gzmkeys and /gzmhud
|- ============================================================================

|- /gzmkeys - Show hotkey help
Sub Bind_GZMKeys
    /call ShowHotkeyHelp
/return

|- /gzmhud - Toggle HUD visibility
Sub Bind_GZMHUD
    /call ToggleGZMHud
/return
