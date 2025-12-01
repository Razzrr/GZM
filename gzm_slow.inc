|**
    GZM_slow.inc - GronnzMaster Slow/Debuff Priority System

    Priority slow system for:
    - Shaman (primary slower)
    - Enchanter (secondary slower)
    - Beastlord (emergency slow)

    Features:
    - Auto-slow on engage
    - Slow verification
    - Cripple/malo on named
    - Priority debuff ordering
**|

| ============================================================================
| SLOW CONSTANTS
| ============================================================================
#DEFINE MAXSLOWSPELLS 4

| ============================================================================
| SUB: Initialize Slow System
| ============================================================================
Sub InitSlowSystem
    /declare i int local

    | Slow spells (priority order)
    /declare SlowSpell[${MAXSLOWSPELLS}] string outer
    /declare SlowSpellType[${MAXSLOWSPELLS}] string outer
    /declare SlowSpellCount int outer 0

    | Slow state
    /declare SlowOn int outer 1
    /declare SlowPriority int outer 1
    /declare SlowVerify int outer 1
    /declare SlowRadius int outer 100
    /declare SlowNamed int outer 1
    /declare SlowAll int outer 1

    | Tracking
    /declare SlowedMobs[30] int outer
    /declare SlowedMobCount int outer 0
    /declare LastSlowTarget int outer 0
    /declare SlowAttempts int outer 0
    /declare SlowTimer timer outer 0

    | Additional debuffs
    /declare CrippleOn int outer 1
    /declare CrippleSpell string outer
    /declare MaloOn int outer 1
    /declare MaloSpell string outer
    /declare TashOn int outer 0
    /declare TashSpell string outer

    | Load settings
    /call LoadSlowSettings

    /echo \ag[GZM-Slow] Slow system initialized - ${SlowSpellCount} slow spells loaded
/return

| ============================================================================
| SUB: Load Slow Settings
| ============================================================================
Sub LoadSlowSettings
    /declare i int local
    /declare spell string local

    | Load slow spells
    /for i 1 to ${MAXSLOWSPELLS}
        /varset spell ${Ini[${IniFileName},Slow,SlowSpell${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset SlowSpell[${i}] ${spell}
            /varset SlowSpellType[${i}] ${Ini[${IniFileName},Slow,SlowSpell${i}Type,Single]}
            /varcalc SlowSpellCount ${SlowSpellCount}+1
        }
    /next i

    | If no slow spells defined, check debuffs section for slow type
    /if (!${SlowSpellCount}) {
        /for i 1 to 10
            /varset spell ${Ini[${IniFileName},Debuffs,Debuff${i}]}
            /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
                /if (${Ini[${IniFileName},Debuffs,Debuff${i}Type].Equal[Slow]}) {
                    /varcalc SlowSpellCount ${SlowSpellCount}+1
                    /varset SlowSpell[${SlowSpellCount}] ${spell}
                    /varset SlowSpellType[${SlowSpellCount}] Single
                }
            }
        /next i
    }

    | Load settings
    /varset SlowOn ${Ini[${IniFileName},Slow,SlowOn,1]}
    /varset SlowPriority ${Ini[${IniFileName},Slow,SlowPriority,1]}
    /varset SlowNamed ${Ini[${IniFileName},Slow,SlowNamed,1]}
    /varset SlowAll ${Ini[${IniFileName},Slow,SlowAll,1]}

    | Load additional debuffs
    /varset CrippleOn ${Ini[${IniFileName},Slow,CrippleOn,1]}
    /varset CrippleSpell ${Ini[${IniFileName},Slow,CrippleSpell,Cripple]}
    /varset MaloOn ${Ini[${IniFileName},Slow,MaloOn,1]}
    /varset MaloSpell ${Ini[${IniFileName},Slow,MaloSpell,Malo]}
    /varset TashOn ${Ini[${IniFileName},Slow,TashOn,0]}
    /varset TashSpell ${Ini[${IniFileName},Slow,TashSpell,Tashania]}
/return

| ============================================================================
| SUB: Check Slow (PRIORITY - called before DPS)
| ============================================================================
Sub CheckSlow
    /if (!${SlowOn}) /return
    /if (!${SlowSpellCount}) /return
    /if (!${InCombat}) /return
    /if (${Me.Casting.ID}) /return
    /if (${SlowTimer}) /return

    | Get current target
    /declare targetID int local ${MyTargetID}
    /if (!${targetID}) /varset targetID ${Target.ID}
    /if (!${targetID}) /return

    | Check if target needs slow
    /if (!${Spawn[${targetID}].Type.Equal[NPC]}) /return

    | Priority 1: Slow MA's target first
    /if (${SlowPriority} && ${targetID}) {
        /if (!${IsSlowed[${targetID}]}) {
            /call DoSlow ${targetID}
            /return
        }
    }

    | Priority 2: Slow adds if SlowAll enabled
    /if (${SlowAll}) {
        /call SlowAdds
    }

    /varset SlowTimer 10
/return

| ============================================================================
| SUB: Check if Mob is Slowed
| ============================================================================
Sub IsSlowed(int MobID)
    /if (!${MobID}) /return 0
    /if (!${Spawn[${MobID}].ID}) /return 0

    | Check for slow buff on target
    /target id ${MobID}
    /delay 3 ${Target.ID} == ${MobID}

    | Check various slow spell names
    /if (${Target.Slowed.ID}) /return 1
    /if (${Target.Buff[Turgur].ID}) /return 1
    /if (${Target.Buff[Slow].ID}) /return 1

    | Check our slowed mob tracking
    /declare i int local
    /for i 1 to ${SlowedMobCount}
        /if (${SlowedMobs[${i}]} == ${MobID}) /return 1
    /next i

/return 0

| ============================================================================
| SUB: Do Slow
| ============================================================================
Sub DoSlow(int TargetID)
    /if (!${TargetID}) /return
    /if (${SlowAttempts} > 3) {
        GZMDEBUG_COMBAT Slow: Too many attempts on ${Spawn[${TargetID}].CleanName} - skipping
        /call AddSlowedMob ${TargetID}
        /varset SlowAttempts 0
        /return
    }

    /declare slowSpell string local

    GZMDEBUG_COMBAT SLOWING ${Spawn[${TargetID}].CleanName}

    | Target
    /if (${Target.ID} != ${TargetID}) {
        /target id ${TargetID}
        /delay 5 ${Target.ID} == ${TargetID}
    }

    | Stand
    /if (${Me.Sitting}) /stand

    | Priority: Tash first if enchanter (lowers resist)
    /if (${TashOn} && ${TashSpell.Length} && ${Me.Class.ShortName.Equal[ENC]}) {
        /if (!${Target.Buff[${TashSpell}].ID} && ${Me.SpellReady[${TashSpell}]}) {
            GZMDEBUG_COMBAT Tashing before slow
            /cast "${TashSpell}"
            /delay 3s ${Me.Casting.ID}
            /delay 5s !${Me.Casting.ID}
        }
    }

    | Priority: Malo first if shaman (lowers resist)
    /if (${MaloOn} && ${MaloSpell.Length} && ${Me.Class.ShortName.Equal[SHM]}) {
        /if (!${Target.Buff[${MaloSpell}].ID} && !${Target.Buff[Malo].ID} && ${Me.SpellReady[${MaloSpell}]}) {
            GZMDEBUG_COMBAT Maloing before slow
            /cast "${MaloSpell}"
            /delay 3s ${Me.Casting.ID}
            /delay 5s !${Me.Casting.ID}
        }
    }

    | Find and cast slow
    /call FindSlowSpell
    /varset slowSpell ${Macro.Return}

    /if (${slowSpell.Length}) {
        /echo \ay[GZM-Slow] Slowing ${Target.CleanName}
        /cast "${slowSpell}"
        /delay 3s ${Me.Casting.ID}
        /delay 10s !${Me.Casting.ID}

        | Verify slow landed
        /if (${SlowVerify}) {
            /delay 5
            /if (${Target.Slowed.ID} || ${Target.Buff[Turgur].ID}) {
                /echo \ag[GZM-Slow] Slow landed on ${Target.CleanName}
                /call AddSlowedMob ${TargetID}
                /varset SlowAttempts 0

                | Follow up with cripple on named
                /if (${CrippleOn} && ${Target.Named} && ${CrippleSpell.Length}) {
                    /if (${Me.SpellReady[${CrippleSpell}]}) {
                        GZMDEBUG_COMBAT Crippling named target
                        /cast "${CrippleSpell}"
                        /delay 3s ${Me.Casting.ID}
                        /delay 8s !${Me.Casting.ID}
                    }
                }
            } else {
                /echo \ar[GZM-Slow] Slow RESISTED on ${Target.CleanName}
                /varcalc SlowAttempts ${SlowAttempts}+1
            }
        }
    } else {
        GZMDEBUG_COMBAT No slow spell ready!
    }

    | Return to MA target
    /if (${MyTargetID} && ${Target.ID} != ${MyTargetID}) {
        /target id ${MyTargetID}
    }
/return

| ============================================================================
| SUB: Find Ready Slow Spell
| ============================================================================
Sub FindSlowSpell
    /declare i int local

    /for i 1 to ${SlowSpellCount}
        /if (${SlowSpell[${i}].Length} && ${Me.SpellReady[${SlowSpell[${i}]}]}) {
            /return ${SlowSpell[${i}]}
        }
    /next i

    | Try AA slow
    /if (${Me.AltAbilityReady[Slowing Helix]}) {
        /return "Slowing Helix"
    }
/return

| ============================================================================
| SUB: Slow Adds
| ============================================================================
Sub SlowAdds
    /declare i int local
    /declare targetID int local

    | Check XTarget for unslowed adds
    /for i 1 to ${Me.XTargetSlots}
        /if (${Me.XTarget[${i}].ID}) {
            /varset targetID ${Me.XTarget[${i}].ID}

            | Skip main target
            /if (${targetID} == ${MyTargetID}) /continue

            | Skip if already slowed
            /if (${Bool[${IsSlowed[${targetID}]}]}) /continue

            | Skip if already tracked
            /declare j int local
            /declare found int local 0
            /for j 1 to ${SlowedMobCount}
                /if (${SlowedMobs[${j}]} == ${targetID}) /varset found 1
            /next j
            /if (${found}) /continue

            | Slow this add
            GZMDEBUG_COMBAT Slowing add: ${Spawn[${targetID}].CleanName}
            /call DoSlow ${targetID}
            /return
        }
    /next i
/return

| ============================================================================
| SUB: Add to Slowed Mob Tracking
| ============================================================================
Sub AddSlowedMob(int MobID)
    /declare i int local

    | Check if already tracked
    /for i 1 to ${SlowedMobCount}
        /if (${SlowedMobs[${i}]} == ${MobID}) /return
    /next i

    | Add to list
    /varcalc SlowedMobCount ${SlowedMobCount}+1
    /if (${SlowedMobCount} > 30) /varset SlowedMobCount 1
    /varset SlowedMobs[${SlowedMobCount}] ${MobID}
/return

| ============================================================================
| SUB: Clear Slowed Mobs (call on combat end)
| ============================================================================
Sub ClearSlowedMobs
    /declare i int local

    /for i 1 to 30
        /varset SlowedMobs[${i}] 0
    /next i
    /varset SlowedMobCount 0
    /varset SlowAttempts 0
/return

| ============================================================================
| EVENT: Slow Resisted
| ============================================================================
Sub Event_SlowResisted
    GZMDEBUG_COMBAT Slow was RESISTED!
    /varcalc SlowAttempts ${SlowAttempts}+1
/return

| ============================================================================
| END OF SLOW INCLUDE
| ============================================================================
