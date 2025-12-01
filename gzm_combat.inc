|**
    GZM_combat.inc - GronnzMaster Combat/DPS System

    Comprehensive combat routines for all classes including:
    - Nukes, DoTs, Debuffs
    - Combat abilities and disciplines
    - AA abilities
    - Burns
    - Class-specific combat logic
    - Combat entry/exit hooks
**|

| ============================================================================
| COMBAT CONSTANTS
| ============================================================================
#DEFINE MAXNUKES 15
#DEFINE MAXDOTS 15
#DEFINE MAXDEBUFFS 15
#DEFINE MAXBURNS 25
#DEFINE MAXABILITIES 10
#DEFINE MAXCOMBATABILITIES 15

| ============================================================================
| COMBAT HOOK VARIABLES
| ============================================================================
Sub InitCombatHooks
    | Combat entry/exit hooks (commands to execute)
    /if (!${Defined[OnCombatEnterHook]}) /declare OnCombatEnterHook string outer
    /if (!${Defined[OnCombatExitHook]}) /declare OnCombatExitHook string outer
    /if (!${Defined[BeforeCombatHook]}) /declare BeforeCombatHook string outer
    /if (!${Defined[AfterCombatHook]}) /declare AfterCombatHook string outer

    | Per-loop combat hooks (called every combat iteration)
    /if (!${Defined[CombatLoopHook]}) /declare CombatLoopHook string outer

    | Class-specific hooks
    /if (!${Defined[ClassCombatHook]}) /declare ClassCombatHook string outer

    | Combat state tracking for hooks
    /if (!${Defined[CombatEnteredOnce]}) /declare CombatEnteredOnce bool outer FALSE
    /if (!${Defined[LastCombatTarget]}) /declare LastCombatTarget int outer 0

    | Combat metrics
    /if (!${Defined[CombatDuration]}) /declare CombatDuration int outer 0
    /if (!${Defined[TotalDamageDealt]}) /declare TotalDamageDealt int outer 0
    /if (!${Defined[TotalKills]}) /declare TotalKills int outer 0

    | Hook debug
    /if (!${Defined[CombatHookDebug]}) /declare CombatHookDebug bool outer FALSE
/return

| ============================================================================
| COMBAT ENTRY HOOK - Called when entering combat
| ============================================================================
Sub OnCombatEnter
    /if (${CombatEnteredOnce}) /return
    /varset CombatEnteredOnce TRUE
    /varset CombatDuration 0

    /if (${CombatHookDebug}) /echo [GZM_Combat] Entering combat

    | Execute before-combat hook (things to do before engaging)
    /if (${BeforeCombatHook.Length}) {
        /if (${CombatHookDebug}) /echo [GZM_Combat] Executing BeforeCombatHook: ${BeforeCombatHook}
        /docommand ${BeforeCombatHook}
    }

    | Execute on-enter hook
    /if (${OnCombatEnterHook.Length}) {
        /if (${CombatHookDebug}) /echo [GZM_Combat] Executing OnCombatEnterHook: ${OnCombatEnterHook}
        /docommand ${OnCombatEnterHook}
    }

    | Class-specific combat entry
    /call ClassCombatEnter
/return

| ============================================================================
| COMBAT EXIT HOOK - Called when exiting combat
| ============================================================================
Sub OnCombatExit
    /if (!${CombatEnteredOnce}) /return
    /varset CombatEnteredOnce FALSE
    /varset LastCombatTarget 0

    /if (${CombatHookDebug}) /echo [GZM_Combat] Exiting combat (duration: ${CombatDuration}s)

    | Execute after-combat hook
    /if (${AfterCombatHook.Length}) {
        /if (${CombatHookDebug}) /echo [GZM_Combat] Executing AfterCombatHook: ${AfterCombatHook}
        /docommand ${AfterCombatHook}
    }

    | Execute on-exit hook
    /if (${OnCombatExitHook.Length}) {
        /if (${CombatHookDebug}) /echo [GZM_Combat] Executing OnCombatExitHook: ${OnCombatExitHook}
        /docommand ${OnCombatExitHook}
    }

    | Class-specific combat exit
    /call ClassCombatExit
/return

| ============================================================================
| COMBAT LOOP HOOK - Called every combat iteration
| ============================================================================
Sub OnCombatLoop
    | Track combat duration
    /varcalc CombatDuration ${CombatDuration}+1

    | Execute per-loop hook
    /if (${CombatLoopHook.Length}) {
        /docommand ${CombatLoopHook}
    }

    | Execute custom combat hook (from GronnzMaster_custom.inc)
    /if (${Defined[CustomCombatHook]}) /call CustomCombatHook

    | Class combat hook
    /if (${ClassCombatHook.Length}) {
        /docommand ${ClassCombatHook}
    }
/return

| ============================================================================
| CLASS-SPECIFIC COMBAT ENTRY - Called on combat start
| ============================================================================
Sub ClassCombatEnter
    /if (${Me.Class.ShortName.Equal[BRD]}) {
        | Bard: Start combat melody
        /if (${Defined[CombatMelody]} && ${CombatMelody.Length}) {
            /if (${Plugin[MQ2Twist].Name.Length}) {
                /twist ${CombatMelody}
            }
        }
    } else /if (${Me.Class.ShortName.Equal[NEC]}) {
        | Necro: Ensure lich is up
        /if (${Defined[LichSpell]} && ${LichSpell.Length}) {
            /if (!${Me.Buff[${LichSpell}].ID} && ${Me.SpellReady[${LichSpell}]}) {
                /call GZMCast "${LichSpell}" ${Me.ID}
            }
        }
    } else /if (${Me.Class.ShortName.Equal[ENC]}) {
        | Enchanter: Check aura
        /if (${Defined[EnchanterAura]} && ${EnchanterAura.Length}) {
            /if (!${Me.Aura[1].ID}) {
                /call GZMCast "${EnchanterAura}" ${Me.ID}
            }
        }
    } else /if (${Select[${Me.Class.ShortName},MAG,BST,SHM]}) {
        | Pet classes: Ensure pet is ready
        /if (${Me.Pet.ID} && !${Me.Pet.Combat} && ${MyTargetID}) {
            /pet attack
        }
    }
/return

| ============================================================================
| CLASS-SPECIFIC COMBAT EXIT - Called on combat end
| ============================================================================
Sub ClassCombatExit
    /if (${Me.Class.ShortName.Equal[BRD]}) {
        | Bard: Switch to idle melody
        /if (${Defined[IdleMelody]} && ${IdleMelody.Length}) {
            /if (${Plugin[MQ2Twist].Name.Length}) {
                /twist ${IdleMelody}
            }
        }
    } else /if (${Me.Class.ShortName.Equal[ROG]}) {
        | Rogue: Re-hide if possible
        /if (${Me.AbilityReady[Hide]} && !${Me.Invis}) {
            /doability Hide
        }
    } else /if (${Select[${Me.Class.ShortName},MAG,BST,SHM,NEC]}) {
        | Pet classes: Pet back off
        /if (${Me.Pet.ID}) {
            /pet back off
        }
    }
/return

| ============================================================================
| LOAD COMBAT HOOK SETTINGS
| ============================================================================
Sub LoadCombatHookSettings(IniFile)
    /if (!${IniFile.Length}) /return

    /varset OnCombatEnterHook ${Ini[${IniFile},CombatHooks,OnCombatEnterHook,${OnCombatEnterHook}]}
    /varset OnCombatExitHook ${Ini[${IniFile},CombatHooks,OnCombatExitHook,${OnCombatExitHook}]}
    /varset BeforeCombatHook ${Ini[${IniFile},CombatHooks,BeforeCombatHook,${BeforeCombatHook}]}
    /varset AfterCombatHook ${Ini[${IniFile},CombatHooks,AfterCombatHook,${AfterCombatHook}]}
    /varset CombatLoopHook ${Ini[${IniFile},CombatHooks,CombatLoopHook,${CombatLoopHook}]}
    /varset ClassCombatHook ${Ini[${IniFile},CombatHooks,ClassCombatHook,${ClassCombatHook}]}

    /if (${CombatHookDebug}) /echo [GZM_Combat] Combat hook settings loaded from ${IniFile}
/return

| ============================================================================
| SAVE COMBAT HOOK SETTINGS
| ============================================================================
Sub SaveCombatHookSettings(IniFile)
    /if (!${IniFile.Length}) /return

    /ini "${IniFile}" "CombatHooks" "OnCombatEnterHook" "${OnCombatEnterHook}"
    /ini "${IniFile}" "CombatHooks" "OnCombatExitHook" "${OnCombatExitHook}"
    /ini "${IniFile}" "CombatHooks" "BeforeCombatHook" "${BeforeCombatHook}"
    /ini "${IniFile}" "CombatHooks" "AfterCombatHook" "${AfterCombatHook}"
    /ini "${IniFile}" "CombatHooks" "CombatLoopHook" "${CombatLoopHook}"
    /ini "${IniFile}" "CombatHooks" "ClassCombatHook" "${ClassCombatHook}"

    /echo [GZM_Combat] Combat hook settings saved to ${IniFile}
/return

| ============================================================================
| GET COMBAT STATUS
| ============================================================================
Sub GetCombatHookStatus
    /echo [GZM_Combat] === Combat Hook Status ===
    /echo In Combat: ${If[${CombatEnteredOnce},Yes,No]}
    /echo Combat Duration: ${CombatDuration}s
    /echo Total Kills: ${TotalKills}
    /echo OnCombatEnterHook: ${If[${OnCombatEnterHook.Length},${OnCombatEnterHook},Not Set]}
    /echo OnCombatExitHook: ${If[${OnCombatExitHook.Length},${OnCombatExitHook},Not Set]}
    /echo BeforeCombatHook: ${If[${BeforeCombatHook.Length},${BeforeCombatHook},Not Set]}
    /echo AfterCombatHook: ${If[${AfterCombatHook.Length},${AfterCombatHook},Not Set]}
    /echo ========================
/return

| ============================================================================
| SUB: Initialize Combat System
| ============================================================================
Sub InitCombatSystem
    /declare i int local

    | Initialize combat hooks first
    /call InitCombatHooks

    | Nuke arrays
    /declare NukeSpell[${MAXNUKES}] string outer
    /declare NukeSpellGem[${MAXNUKES}] int outer
    /declare NukeSpellPct[${MAXNUKES}] int outer
    /declare NukeSpellMana[${MAXNUKES}] int outer
    /declare NukeSpellCount int outer 0

    | DoT arrays
    /declare DoTSpell[${MAXDOTS}] string outer
    /declare DoTSpellGem[${MAXDOTS}] int outer
    /declare DoTSpellPct[${MAXDOTS}] int outer
    /declare DoTSpellCount int outer 0

    | Debuff arrays
    /declare DebuffSpell[${MAXDEBUFFS}] string outer
    /declare DebuffSpellType[${MAXDEBUFFS}] string outer
    /declare DebuffSpellCount int outer 0

    | ========================================================================
    | ENHANCED DEBUFF SYSTEM
    | Cooldown timers, resist tracking, per-target debuff tracking
    | ========================================================================
    /declare DebuffTimer[${MAXDEBUFFS}] timer outer
    /declare DebuffResistCount[${MAXDEBUFFS}] int outer
    /declare DebuffMaxResists int outer 3
    /declare DebuffRecastTime int outer 30
    /declare DebuffResistAnnounce string outer
    /declare DebuffLandedAnnounce string outer
    /declare DebuffTargetID[${MAXDEBUFFS}] int outer
    /declare DebuffDebug bool outer FALSE

    | Burn arrays
    /declare BurnAbility[${MAXBURNS}] string outer
    /declare BurnAbilityType[${MAXBURNS}] string outer
    /declare BurnAbilityCount int outer 0

    | Combat ability arrays
    /declare CombatAbility[${MAXCOMBATABILITIES}] string outer
    /declare CombatAbilityTimer[${MAXCOMBATABILITIES}] timer outer
    /declare CombatAbilityEndurance[${MAXCOMBATABILITIES}] int outer
    /declare CombatAbilityCount int outer 0

    | Combat state
    /declare NukeOn int outer 1
    /declare DoTOn int outer 1
    /declare DebuffOn int outer 1
    /declare CombatAbilitiesOn int outer 1
    /declare UseGoM int outer 1
    /declare GoMSpell string outer
    /declare NukeManaPct int outer 30
    /declare DoTManaPct int outer 40
    /declare StopDPSPct int outer 5
    /declare DebuffNamed int outer 0
    /declare DoTStopPct int outer 10

    | Load combat spells
    /call LoadCombatSpells

    /echo \ag[GZM-Combat] Combat system initialized - ${NukeSpellCount} nukes, ${DoTSpellCount} DoTs, ${DebuffSpellCount} debuffs loaded
/return

| ============================================================================
| SUB: Load Combat Spells from INI
| ============================================================================
Sub LoadCombatSpells
    /declare i int local
    /declare spell string local

    | Load nukes
    /for i 1 to ${MAXNUKES}
        /varset spell ${Ini[${IniFileName},DPS,Nuke${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset NukeSpell[${i}] ${spell}
            /varset NukeSpellGem[${i}] ${Ini[${IniFileName},DPS,Nuke${i}Gem,0]}
            /varset NukeSpellPct[${i}] ${Ini[${IniFileName},DPS,Nuke${i}Pct,98]}
            /varset NukeSpellMana[${i}] ${Ini[${IniFileName},DPS,Nuke${i}Mana,20]}
            /varcalc NukeSpellCount ${NukeSpellCount}+1
            GZMDEBUG_COMBAT Loaded nuke ${i}: ${spell}
        }
    /next i

    | Load DoTs
    /for i 1 to ${MAXDOTS}
        /varset spell ${Ini[${IniFileName},DPS,DoT${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset DoTSpell[${i}] ${spell}
            /varset DoTSpellGem[${i}] ${Ini[${IniFileName},DPS,DoT${i}Gem,0]}
            /varset DoTSpellPct[${i}] ${Ini[${IniFileName},DPS,DoT${i}Pct,98]}
            /varcalc DoTSpellCount ${DoTSpellCount}+1
            GZMDEBUG_COMBAT Loaded DoT ${i}: ${spell}
        }
    /next i

    | Load debuffs
    /for i 1 to ${MAXDEBUFFS}
        /varset spell ${Ini[${IniFileName},Debuffs,Debuff${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset DebuffSpell[${i}] ${spell}
            /varset DebuffSpellType[${i}] ${Ini[${IniFileName},Debuffs,Debuff${i}Type,Slow]}
            /varcalc DebuffSpellCount ${DebuffSpellCount}+1
            GZMDEBUG_COMBAT Loaded debuff ${i}: ${spell}
        }
    /next i

    | Load burns
    /for i 1 to ${MAXBURNS}
        /varset spell ${Ini[${IniFileName},Burns,Burn${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset BurnAbility[${i}] ${spell}
            /varset BurnAbilityType[${i}] ${Ini[${IniFileName},Burns,Burn${i}Type,AA]}
            /varcalc BurnAbilityCount ${BurnAbilityCount}+1
            GZMDEBUG_COMBAT Loaded burn ${i}: ${spell}
        }
    /next i

    | Load combat abilities
    /for i 1 to ${MAXCOMBATABILITIES}
        /varset spell ${Ini[${IniFileName},Combat,Ability${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset CombatAbility[${i}] ${spell}
            /varset CombatAbilityEndurance[${i}] ${Ini[${IniFileName},Combat,Ability${i}End,10]}
            /varcalc CombatAbilityCount ${CombatAbilityCount}+1
        }
    /next i

    | Load settings
    /varset NukeOn ${Ini[${IniFileName},DPS,NukeOn,1]}
    /varset DoTOn ${Ini[${IniFileName},DPS,DoTOn,1]}
    /varset DebuffOn ${Ini[${IniFileName},Debuffs,DebuffOn,1]}
    /varset NukeManaPct ${Ini[${IniFileName},DPS,NukeManaPct,30]}
    /varset DoTManaPct ${Ini[${IniFileName},DPS,DoTManaPct,40]}
    /varset StopDPSPct ${Ini[${IniFileName},DPS,StopDPSPct,5]}
    /varset GoMSpell ${Ini[${IniFileName},DPS,GoMSpell]}
/return

| ============================================================================
| SUB: Main DPS Routine (called from main combat)
| ============================================================================
Sub DoDPS
    /if (!${DPSOn}) /return
    /if (!${MyTargetID}) /return
    /if (${Target.ID} != ${MyTargetID}) {
        /target id ${MyTargetID}
        /delay 3 ${Target.ID} == ${MyTargetID}
    }
    /if (!${Target.ID}) /return
    /if (${Target.PctHPs} <= ${StopDPSPct}) /return

    | Check Gift of Mana
    /if (${UseGoM} && ${GoMTimer} && ${GoMSpell.Length}) {
        /call CastGoMSpell
        /return
    }

    | Priority 1: Debuffs (if enabled and target is named or debuff always on)
    /if (${DebuffOn} && (${Target.Named} || !${DebuffNamed})) {
        /call CheckDebuffs
    }

    | Priority 2: DoTs
    /if (${DoTOn} && ${Me.PctMana} >= ${DoTManaPct}) {
        /call CheckDoTs
    }

    | Priority 3: Nukes
    /if (${NukeOn} && ${Me.PctMana} >= ${NukeManaPct}) {
        /call CheckNukes
    }

    | Priority 4: Combat Abilities
    /if (${CombatAbilitiesOn}) {
        /call CheckCombatAbilities
    }

    | Class-specific combat
    /call ClassSpecificCombat
/return

| ============================================================================
| SUB: Cast GoM Spell (Gift of Mana)
| ============================================================================
Sub CastGoMSpell
    /if (${Me.SpellReady[${GoMSpell}]}) {
        GZMDEBUG_COMBAT Using Gift of Mana on ${GoMSpell}
        /call CastDPS "${GoMSpell}" ${MyTargetID}
    }
/return

| ============================================================================
| SUB: Check Debuffs (Enhanced with cooldowns and resist tracking)
| ============================================================================
Sub CheckDebuffs
    /declare i int local
    /declare targetChanged bool local FALSE

    /for i 1 to ${DebuffSpellCount}
        /if (!${DebuffSpell[${i}].Length}) /continue

        | Check if target changed - reset resist counts
        /if (${DebuffTargetID[${i}]} != ${MyTargetID}) {
            /varset DebuffTargetID[${i}] ${MyTargetID}
            /varset DebuffResistCount[${i}] 0
            /varset targetChanged TRUE
        }

        | Check cooldown timer
        /if (${DebuffTimer[${i}]}) {
            /if (${DebuffDebug}) /echo [GZM_Debuff] ${DebuffSpell[${i}]} on cooldown: ${DebuffTimer[${i}]}
            /continue
        }

        | Check resist count - skip if exceeded max resists
        /if (${DebuffResistCount[${i}]} >= ${DebuffMaxResists}) {
            /if (${DebuffDebug}) /echo [GZM_Debuff] ${DebuffSpell[${i}]} resisted ${DebuffResistCount[${i}]} times - skipping
            /continue
        }

        | Check if debuff already on target
        /if (${Target.Buff[${DebuffSpell[${i}]}].ID}) /continue
        /if (${Target.Buff[${Spell[${DebuffSpell[${i}]}].Trigger}].ID}) /continue

        | Cast debuff
        /if (${Me.SpellReady[${DebuffSpell[${i}]}]}) {
            /if (${DebuffDebug}) /echo [GZM_Debuff] Casting debuff: ${DebuffSpell[${i}]} on ${Target.CleanName}
            /call CastDebuff "${DebuffSpell[${i}]}" ${MyTargetID} ${i}
            /return
        }
    /next i
/return

| ============================================================================
| SUB: Cast Debuff (with resist tracking)
| ============================================================================
Sub CastDebuff(string SpellName, int TargetID, int DebuffIndex)
    /if (!${SpellName.Length}) /return
    /if (!${TargetID}) /return

    /declare castResult string local

    | Target if needed
    /if (${Target.ID} != ${TargetID}) {
        /target id ${TargetID}
        /delay 5 ${Target.ID} == ${TargetID}
    }

    | Stand if sitting
    /if (${Me.Sitting}) /stand

    | Cast
    /if (${Me.SpellReady[${SpellName}]}) {
        /cast "${SpellName}"
        /delay 3s ${Me.Casting.ID}
        /delay 10s !${Me.Casting.ID}

        | Check cast result
        /call CheckDebuffResult ${SpellName} ${TargetID} ${DebuffIndex}
    }
/return

| ============================================================================
| SUB: Check Debuff Result (resist tracking)
| ============================================================================
Sub CheckDebuffResult(string SpellName, int TargetID, int DebuffIndex)
    | Check if debuff landed
    /if (${Target.ID} == ${TargetID}) {
        /if (${Target.Buff[${SpellName}].ID} || ${Target.Buff[${Spell[${SpellName}].Trigger}].ID}) {
            | Debuff landed
            /varset DebuffResistCount[${DebuffIndex}] 0
            /varset DebuffTimer[${DebuffIndex}] ${DebuffRecastTime}s

            /if (${DebuffLandedAnnounce.Length}) {
                /docommand ${DebuffLandedAnnounce} ${SpellName} landed on ${Target.CleanName}
            }
            /if (${DebuffDebug}) /echo [GZM_Debuff] ${SpellName} LANDED on ${Target.CleanName}
        } else {
            | Debuff resisted
            /varcalc DebuffResistCount[${DebuffIndex}] ${DebuffResistCount[${DebuffIndex}]}+1
            /varset DebuffTimer[${DebuffIndex}] 10s

            /if (${DebuffResistAnnounce.Length}) {
                /docommand ${DebuffResistAnnounce} ${SpellName} RESISTED by ${Target.CleanName} (${DebuffResistCount[${DebuffIndex}]}/${DebuffMaxResists})
            }
            /if (${DebuffDebug}) /echo [GZM_Debuff] ${SpellName} RESISTED by ${Target.CleanName} (${DebuffResistCount[${DebuffIndex}]}/${DebuffMaxResists})
        }
    }
/return

| ============================================================================
| SUB: Reset Debuff Tracking (call when target dies/changes)
| ============================================================================
Sub ResetDebuffTracking
    /declare i int local
    /for i 1 to ${MAXDEBUFFS}
        /varset DebuffResistCount[${i}] 0
        /varset DebuffTargetID[${i}] 0
    /next i
    /if (${DebuffDebug}) /echo [GZM_Debuff] Debuff tracking reset
/return

| ============================================================================
| SUB: Load Debuff Settings
| ============================================================================
Sub LoadDebuffSettings(IniFile)
    /if (!${IniFile.Length}) /return

    /varset DebuffMaxResists ${Ini[${IniFile},Debuffs,DebuffMaxResists,${DebuffMaxResists}]}
    /varset DebuffRecastTime ${Ini[${IniFile},Debuffs,DebuffRecastTime,${DebuffRecastTime}]}
    /varset DebuffResistAnnounce ${Ini[${IniFile},Debuffs,DebuffResistAnnounce,${DebuffResistAnnounce}]}
    /varset DebuffLandedAnnounce ${Ini[${IniFile},Debuffs,DebuffLandedAnnounce,${DebuffLandedAnnounce}]}

    /if (${DebuffDebug}) /echo [GZM_Debuff] Settings loaded from ${IniFile}
/return

| ============================================================================
| SUB: Save Debuff Settings
| ============================================================================
Sub SaveDebuffSettings(IniFile)
    /if (!${IniFile.Length}) /return

    /ini "${IniFile}" "Debuffs" "DebuffMaxResists" "${DebuffMaxResists}"
    /ini "${IniFile}" "Debuffs" "DebuffRecastTime" "${DebuffRecastTime}"
    /ini "${IniFile}" "Debuffs" "DebuffResistAnnounce" "${DebuffResistAnnounce}"
    /ini "${IniFile}" "Debuffs" "DebuffLandedAnnounce" "${DebuffLandedAnnounce}"

    /echo [GZM_Debuff] Settings saved to ${IniFile}
/return

| ============================================================================
| SUB: Get Debuff Status
| ============================================================================
Sub GetDebuffStatus
    /echo [GZM_Debuff] === Debuff System Status ===
    /echo Debuffs On: ${If[${DebuffOn},Yes,No]}
    /echo Debuff Named Only: ${If[${DebuffNamed},Yes,No]}
    /echo Debuffs Loaded: ${DebuffSpellCount}
    /echo Max Resists Before Skip: ${DebuffMaxResists}
    /echo Recast Delay: ${DebuffRecastTime}s
    /echo ========================
/return

| ============================================================================
| SUB: Check DoTs
| ============================================================================
Sub CheckDoTs
    /declare i int local

    | Don't DoT if mob is low
    /if (${Target.PctHPs} <= ${DoTStopPct}) /return

    /for i 1 to ${DoTSpellCount}
        /if (!${DoTSpell[${i}].Length}) /continue

        | Check HP threshold
        /if (${Target.PctHPs} > ${DoTSpellPct[${i}]}) /continue

        | Check if DoT already on target
        /if (${Target.MyBuff[${DoTSpell[${i}]}].ID}) /continue
        /if (${Target.MyBuff[${Spell[${DoTSpell[${i}]}].Trigger}].ID}) /continue

        | Cast DoT
        /if (${Me.SpellReady[${DoTSpell[${i}]}]}) {
            GZMDEBUG_COMBAT Casting DoT: ${DoTSpell[${i}]}
            /call CastDPS "${DoTSpell[${i}]}" ${MyTargetID}
            /return
        }
    /next i
/return

| ============================================================================
| SUB: Check Nukes
| ============================================================================
Sub CheckNukes
    /declare i int local

    /for i 1 to ${NukeSpellCount}
        /if (!${NukeSpell[${i}].Length}) /continue

        | Check HP threshold
        /if (${Target.PctHPs} > ${NukeSpellPct[${i}]}) /continue

        | Check mana
        /if (${Me.PctMana} < ${NukeSpellMana[${i}]}) /continue

        | Cast nuke
        /if (${Me.SpellReady[${NukeSpell[${i}]}]}) {
            GZMDEBUG_COMBAT Casting nuke: ${NukeSpell[${i}]}
            /call CastDPS "${NukeSpell[${i}]}" ${MyTargetID}
            /return
        }
    /next i
/return

| ============================================================================
| SUB: Check Combat Abilities
| ============================================================================
Sub CheckCombatAbilities
    /declare i int local

    /for i 1 to ${CombatAbilityCount}
        /if (!${CombatAbility[${i}].Length}) /continue
        /if (${CombatAbilityTimer[${i}]}) /continue
        /if (${Me.PctEndurance} < ${CombatAbilityEndurance[${i}]}) /continue

        | Check if it's a disc
        /if (${Me.CombatAbility[${CombatAbility[${i}]}].ID}) {
            /if (${Me.CombatAbilityReady[${CombatAbility[${i}]}]}) {
                GZMDEBUG_COMBAT Using combat ability: ${CombatAbility[${i}]}
                /disc "${CombatAbility[${i}]}"
                /varset CombatAbilityTimer[${i}] 60
                /return
            }
        }

        | Check if it's an ability
        /if (${Me.AbilityReady[${CombatAbility[${i}]}]}) {
            GZMDEBUG_COMBAT Using ability: ${CombatAbility[${i}]}
            /doability "${CombatAbility[${i}]}"
            /varset CombatAbilityTimer[${i}] 60
            /return
        }
    /next i
/return

| ============================================================================
| SUB: Do Burn
| ============================================================================
Sub DoBurn
    /declare i int local

    GZMDEBUG_COMBAT === BURN ACTIVATED ===

    /for i 1 to ${BurnAbilityCount}
        /if (!${BurnAbility[${i}].Length}) /continue

        | Check type and use appropriately
        /if (${BurnAbilityType[${i}].Equal[AA]}) {
            /if (${Me.AltAbilityReady[${BurnAbility[${i}]}]}) {
                GZMDEBUG_COMBAT Burn AA: ${BurnAbility[${i}]}
                /alt act ${Me.AltAbility[${BurnAbility[${i}]}].ID}
                /delay 5
            }
        } else /if (${BurnAbilityType[${i}].Equal[Disc]}) {
            /if (${Me.CombatAbilityReady[${BurnAbility[${i}]}]}) {
                GZMDEBUG_COMBAT Burn Disc: ${BurnAbility[${i}]}
                /disc "${BurnAbility[${i}]}"
                /delay 5
            }
        } else /if (${BurnAbilityType[${i}].Equal[Item]}) {
            /if (${FindItem[${BurnAbility[${i}]}].TimerReady} == 0) {
                GZMDEBUG_COMBAT Burn Item: ${BurnAbility[${i}]}
                /useitem "${BurnAbility[${i}]}"
                /delay 5
            }
        } else /if (${BurnAbilityType[${i}].Equal[Spell]}) {
            /if (${Me.SpellReady[${BurnAbility[${i}]}]}) {
                GZMDEBUG_COMBAT Burn Spell: ${BurnAbility[${i}]}
                /cast "${BurnAbility[${i}]}"
                /delay 5s ${Me.Casting.ID}
                /delay 5s !${Me.Casting.ID}
            }
        } else /if (${BurnAbilityType[${i}].Equal[Ability]}) {
            /if (${Me.AbilityReady[${BurnAbility[${i}]}]}) {
                GZMDEBUG_COMBAT Burn Ability: ${BurnAbility[${i}]}
                /doability "${BurnAbility[${i}]}"
                /delay 5
            }
        }
    /next i

    | Broadcast burn
    /if (${UseDanNet}) {
        /dgtell all BURN ACTIVE - ${Me.CleanName}
    }
/return

| ============================================================================
| SUB: Cast DPS Spell
| ============================================================================
Sub CastDPS(string SpellName, int TargetID)
    /if (!${SpellName.Length}) /return
    /if (!${TargetID}) /return

    | Target if needed
    /if (${Target.ID} != ${TargetID}) {
        /target id ${TargetID}
        /delay 5 ${Target.ID} == ${TargetID}
    }

    | Stand if sitting
    /if (${Me.Sitting}) /stand

    | Cast
    /if (${Me.SpellReady[${SpellName}]}) {
        /cast "${SpellName}"
        /delay 3s ${Me.Casting.ID}
        /delay 10s !${Me.Casting.ID}
    }
/return

| ============================================================================
| SUB: Class Specific Combat
| ============================================================================
Sub ClassSpecificCombat
    /if (${Me.Class.ShortName.Equal[WAR]}) {
        /call WarriorCombat
    } else /if (${Me.Class.ShortName.Equal[PAL]}) {
        /call PaladinCombat
    } else /if (${Me.Class.ShortName.Equal[SHD]}) {
        /call ShadowknightCombat
    } else /if (${Me.Class.ShortName.Equal[ROG]}) {
        /call RogueCombat
    } else /if (${Me.Class.ShortName.Equal[MNK]}) {
        /call MonkCombat
    } else /if (${Me.Class.ShortName.Equal[BER]}) {
        /call BerserkerCombat
    } else /if (${Me.Class.ShortName.Equal[RNG]}) {
        /call RangerCombat
    } else /if (${Me.Class.ShortName.Equal[BST]}) {
        /call BeastlordCombat
    } else /if (${Me.Class.ShortName.Equal[WIZ]}) {
        /call WizardCombat
    } else /if (${Me.Class.ShortName.Equal[MAG]}) {
        /call MagicianCombat
    } else /if (${Me.Class.ShortName.Equal[NEC]}) {
        /call NecromancerCombat
    } else /if (${Me.Class.ShortName.Equal[ENC]}) {
        /call EnchanterCombat
    } else /if (${Me.Class.ShortName.Equal[CLR]}) {
        /call ClericCombat
    } else /if (${Me.Class.ShortName.Equal[DRU]}) {
        /call DruidCombat
    } else /if (${Me.Class.ShortName.Equal[SHM]}) {
        /call ShamanCombat
    } else /if (${Me.Class.ShortName.Equal[BRD]}) {
        /call BardCombat
    }
/return

| ============================================================================
| TANK COMBAT ROUTINES
| ============================================================================

Sub WarriorCombat
    | Brace for Impact
    /if (${Me.AltAbilityReady[Brace for Impact]} && ${Me.PctHPs} < 50) {
        /alt act ${Me.AltAbility[Brace for Impact].ID}
    }

    | Fortitude Discipline
    /if (${Me.CombatAbilityReady[Fortitude Discipline]} && ${Me.PctHPs} < 30) {
        /disc "Fortitude Discipline"
    }

    | Taunt
    /if (${Me.AbilityReady[Taunt]} && ${Target.ID} && ${Me.TargetOfTarget.ID} != ${Me.ID}) {
        /doability Taunt
    }

    | Bash/Slam
    /if (${Me.AbilityReady[Bash]}) {
        /doability Bash
    }

    | Kick
    /if (${Me.AbilityReady[Kick]}) {
        /doability Kick
    }
/return

Sub PaladinCombat
    | Preservation of the Faithful (heal)
    /if (${Me.AltAbilityReady[Preservation of the Faithful]} && ${Me.PctHPs} < 40) {
        /alt act ${Me.AltAbility[Preservation of the Faithful].ID}
    }

    | Lay on Hands
    /if (${Me.AltAbilityReady[Lay on Hands]} && ${Me.PctHPs} < 20) {
        /alt act ${Me.AltAbility[Lay on Hands].ID}
    }

    | Taunt
    /if (${Me.AbilityReady[Taunt]} && ${Target.ID} && ${Me.TargetOfTarget.ID} != ${Me.ID}) {
        /doability Taunt
    }

    | Stuns
    /if (${Me.AbilityReady[Bash]}) {
        /doability Bash
    }
/return

Sub ShadowknightCombat
    | Thought Leech
    /if (${Me.AltAbilityReady[Thought Leech]} && ${Me.PctMana} < 30) {
        /alt act ${Me.AltAbility[Thought Leech].ID}
    }

    | Leech Touch
    /if (${Me.AltAbilityReady[Leech Touch]} && ${Me.PctHPs} < 50) {
        /alt act ${Me.AltAbility[Leech Touch].ID}
    }

    | Taunt
    /if (${Me.AbilityReady[Taunt]} && ${Target.ID} && ${Me.TargetOfTarget.ID} != ${Me.ID}) {
        /doability Taunt
    }

    | Bash
    /if (${Me.AbilityReady[Bash]}) {
        /doability Bash
    }
/return

| ============================================================================
| MELEE DPS COMBAT ROUTINES
| ============================================================================

Sub RogueCombat
    | Backstab (if behind target)
    /if (${Me.AbilityReady[Backstab]}) {
        | Check if behind target
        /if (${Me.AbilityReady[Backstab]}) {
            /doability Backstab
        }
    }

    | Assassinate
    /if (${Me.AltAbilityReady[Assassinate]} && ${Target.Level} <= 100) {
        /alt act ${Me.AltAbility[Assassinate].ID}
    }

    | Hide/Evade
    /if (${Me.AltAbilityReady[Escape]} && ${Me.PctHPs} < 30) {
        /alt act ${Me.AltAbility[Escape].ID}
    }
/return

Sub MonkCombat
    | Flying Kick
    /if (${Me.AbilityReady[Flying Kick]}) {
        /doability "Flying Kick"
    }

    | Tiger Claw
    /if (${Me.AbilityReady[Tiger Claw]}) {
        /doability "Tiger Claw"
    }

    | Eagle Strike
    /if (${Me.AbilityReady[Eagle Strike]}) {
        /doability "Eagle Strike"
    }

    | Round Kick
    /if (${Me.AbilityReady[Round Kick]}) {
        /doability "Round Kick"
    }

    | Mend
    /if (${Me.AbilityReady[Mend]} && ${Me.PctHPs} < 40) {
        /doability Mend
    }

    | Feign Death
    /if (${Me.AbilityReady[Feign Death]} && ${Me.PctHPs} < 20) {
        /doability "Feign Death"
    }
/return

Sub BerserkerCombat
    | Frenzy
    /if (${Me.AbilityReady[Frenzy]}) {
        /doability Frenzy
    }

    | Volley
    /if (${Me.AltAbilityReady[Volley]} && ${Target.Distance} > 30) {
        /alt act ${Me.AltAbility[Volley].ID}
    }

    | War Cry
    /if (${Me.CombatAbilityReady[War Cry]}) {
        /disc "War Cry"
    }
/return

Sub RangerCombat
    | Bow if at range
    /if (${Target.Distance} > ${MeleeDistance}) {
        /if (!${Me.AutoFire}) /autofire on
    } else {
        /if (${Me.AutoFire}) /autofire off
    }

    | Kick
    /if (${Me.AbilityReady[Kick]} && ${Target.Distance} <= ${MeleeDistance}) {
        /doability Kick
    }

    | Headshot
    /if (${Me.AltAbilityReady[Headshot]} && ${Target.Body.Name.Equal[Humanoid]}) {
        | Headshot is passive
    }
/return

Sub BeastlordCombat
    | Pet attack
    /if (${Me.Pet.ID} && !${Me.Pet.Combat}) {
        /pet attack
    }

    | Kick
    /if (${Me.AbilityReady[Kick]}) {
        /doability Kick
    }

    | Round Kick
    /if (${Me.AbilityReady[Round Kick]}) {
        /doability "Round Kick"
    }

    | Feral Swipe
    /if (${Me.AltAbilityReady[Feral Swipe]}) {
        /alt act ${Me.AltAbility[Feral Swipe].ID}
    }
/return

| ============================================================================
| CASTER DPS COMBAT ROUTINES
| ============================================================================

Sub WizardCombat
    | Harvest of Druzzil (mana recourse)
    /if (${Me.AltAbilityReady[Harvest of Druzzil]} && ${Me.PctMana} < 50) {
        /alt act ${Me.AltAbility[Harvest of Druzzil].ID}
    }

    | Arcane Fusion
    /if (${Me.AltAbilityReady[Arcane Fusion]} && ${Target.PctHPs} > 50) {
        /alt act ${Me.AltAbility[Arcane Fusion].ID}
    }

    | Force of Will
    /if (${Me.AltAbilityReady[Force of Will]}) {
        /alt act ${Me.AltAbility[Force of Will].ID}
    }
/return

Sub MagicianCombat
    | Pet attack
    /if (${Me.Pet.ID} && !${Me.Pet.Combat}) {
        /pet attack
    }

    | Servant of Ro
    /if (${Me.AltAbilityReady[Servant of Ro]}) {
        /alt act ${Me.AltAbility[Servant of Ro].ID}
    }

    | Heart of Flames
    /if (${Me.AltAbilityReady[Heart of Flames]} && ${Me.PctMana} < 30) {
        /alt act ${Me.AltAbility[Heart of Flames].ID}
    }
/return

Sub NecromancerCombat
    | Pet attack
    /if (${Me.Pet.ID} && !${Me.Pet.Combat}) {
        /pet attack
    }

    | Death Bloom (mana)
    /if (${Me.AltAbilityReady[Death Bloom]} && ${Me.PctMana} < 50 && ${Me.PctHPs} > 70) {
        /alt act ${Me.AltAbility[Death Bloom].ID}
    }

    | Life Burn
    /if (${Me.AltAbilityReady[Life Burn]} && ${Target.Named} && ${Me.PctHPs} > 80) {
        /alt act ${Me.AltAbility[Life Burn].ID}
    }

    | Feign Death
    /if (${Me.AltAbilityReady[Death's Effigy]} && ${Me.PctHPs} < 20) {
        /alt act ${Me.AltAbility[Death's Effigy].ID}
    }
/return

Sub EnchanterCombat
    | Mana check
    /if (${Me.PctMana} < 20) {
        /if (${Me.AltAbilityReady[Gather Mana]}) {
            /alt act ${Me.AltAbility[Gather Mana].ID}
        }
    }

    | Stasis (root/stun)
    /if (${Me.AltAbilityReady[Stasis]} && ${SpawnCount[npc radius 30]} > 2) {
        /alt act ${Me.AltAbility[Stasis].ID}
    }
/return

| ============================================================================
| PRIEST COMBAT ROUTINES
| ============================================================================

Sub ClericCombat
    | Mostly handled by healing routines
    | Add any cleric DPS here

    | Yaulp
    /if (!${Me.Buff[Yaulp].ID} && ${Me.SpellReady[Yaulp]}) {
        /cast "Yaulp"
    }

    | Turn Undead
    /if (${Target.Body.Name.Equal[Undead]} && ${Me.AltAbilityReady[Turn Undead]}) {
        /alt act ${Me.AltAbility[Turn Undead].ID}
    }
/return

Sub DruidCombat
    | Nature's Boon if low
    /if (${Me.AltAbilityReady[Nature's Boon]} && ${Me.PctHPs} < 40) {
        /alt act ${Me.AltAbility[Nature's Boon].ID}
    }

    | Season's Wrath
    /if (${Me.AltAbilityReady[Season's Wrath]} && ${Target.Named}) {
        /alt act ${Me.AltAbility[Season's Wrath].ID}
    }
/return

Sub ShamanCombat
    | Cannibalize
    /if (${Me.AltAbilityReady[Cannibalization]} && ${Me.PctMana} < 50 && ${Me.PctHPs} > 80) {
        /alt act ${Me.AltAbility[Cannibalization].ID}
    }

    | Pet attack
    /if (${Me.Pet.ID} && !${Me.Pet.Combat}) {
        /pet attack
    }

    | Rabid Bear
    /if (${Me.AltAbilityReady[Rabid Bear]}) {
        /alt act ${Me.AltAbility[Rabid Bear].ID}
    }
/return

Sub BardCombat
    | Bards handled by twist - see GZM_bard.inc
/return

| ============================================================================
| END OF COMBAT INCLUDE
| ============================================================================
