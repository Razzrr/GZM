|- GZM Hotkeys - Keyboard Control System
|- Version 1.0
|-
|- Provides hotkey bindings for quick macro control without GUI
|- Uses MQ2 /bind command for reliable key handling
|-
|- Default Key Layout (Tilde/Backtick area - won't conflict with EQ):
|-   `     = Pause/Resume (backtick)
|-   ~     = Burn! (Shift+backtick)
|-   Ctrl+` = Back Off
|-   Alt+`  = Group Pause (DanNet)
|-
|-   Ctrl+Q = Toggle Heals
|-   Ctrl+W = Toggle DPS
|-   Ctrl+E = Toggle Buffs
|-   Ctrl+R = Toggle Mez
|-   Ctrl+T = Toggle Slow
|-   Ctrl+Y = Toggle Pull
|-   Ctrl+U = Toggle Pet
|-   Ctrl+I = Toggle Loot
|-
|-   Alt+Q  = Make Camp
|-   Alt+W  = Chase MA
|-   Alt+E  = Stay Here
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

    |- Backtick/Tilde Core Controls
    /squelch /bind GZM_Pause       ` /gzmpause
    /squelch /bind GZM_Burn        ~ /burn
    /squelch /bind GZM_BackOff     Ctrl+` /backoff

    |- Ctrl+Letter Toggles (left hand friendly)
    /squelch /bind GZM_Heals       Ctrl+Q /gzmtoggle HealsOn
    /squelch /bind GZM_DPS         Ctrl+W /gzmtoggle DPSOn
    /squelch /bind GZM_Buffs       Ctrl+E /gzmtoggle BuffsOn
    /squelch /bind GZM_Mez         Ctrl+R /gzmtoggle MezOn
    /squelch /bind GZM_Slow        Ctrl+T /gzmtoggle SlowOn
    /squelch /bind GZM_Pull        Ctrl+Y /gzmtoggle PullOn
    /squelch /bind GZM_Pet         Ctrl+U /gzmtoggle PetOn
    /squelch /bind GZM_Loot        Ctrl+I /gzmtoggle LootOn

    |- Alt+Letter Movement/Actions
    /squelch /bind GZM_Camp        Alt+Q /gzmcamp
    /squelch /bind GZM_Chase       Alt+W /chaseme
    /squelch /bind GZM_Stay        Alt+E /stayhere

    |- Shift+Ctrl for more toggles
    /squelch /bind GZM_Melee       Ctrl+Shift+Q /gzmtoggle MeleeOn
    /squelch /bind GZM_Assist      Ctrl+Shift+W /gzmtoggle AssistOn
    /squelch /bind GZM_Charm       Ctrl+Shift+R /gzmtoggle CharmOn
    /squelch /bind GZM_Items       Ctrl+Shift+I /gzmtoggle ItemClicksOn

    |- Shift+Alt for value adjustments
    /squelch /bind GZM_HealUp      Alt+Shift+Q /gzmadjust HealPct 5
    /squelch /bind GZM_HealDown    Ctrl+Alt+Q /gzmadjust HealPct -5
    /squelch /bind GZM_AssistUp    Alt+Shift+W /gzmadjust AssistAt 5
    /squelch /bind GZM_AssistDown  Ctrl+Alt+W /gzmadjust AssistAt -5

    |- Alt+Backtick for group commands (DanNet)
    /squelch /bind GZM_GrpPause    Alt+` /dgge /varset Paused 1
    /squelch /bind GZM_GrpResume   Alt+Shift+` /dgge /varset Paused 0
    /squelch /bind GZM_GrpBurn     Alt+~ /dgge /burn
    /squelch /bind GZM_GrpBackOff  Ctrl+Alt+` /dgge /backoff

    /echo \ag[GZM-Keys] Hotkeys ready - `=Pause ~=Burn Ctrl+`=BackOff
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
    /echo \aw `      = Pause/Resume
    /echo \aw ~      = Burn!
    /echo \aw Ctrl+` = Back Off
    /echo \ag--------------------------------
    /echo \aw Ctrl+Q = Heals    Ctrl+U = Pet
    /echo \aw Ctrl+W = DPS      Ctrl+I = Loot
    /echo \aw Ctrl+E = Buffs    Ctrl+Y = Pull
    /echo \aw Ctrl+R = Mez      Ctrl+T = Slow
    /echo \ag--------------------------------
    /echo \aw Alt+Q  = Make Camp
    /echo \aw Alt+W  = Chase MA
    /echo \aw Alt+E  = Stay Here
    /echo \ag--------------------------------
    /echo \aw Alt+`  = Group Pause (DanNet)
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
