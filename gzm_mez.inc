|**
    GZM_mez.inc - GronnzMaster Mez/CC System

    Comprehensive crowd control for:
    - Enchanter (primary)
    - Bard (secondary)

    Features:
    - XTarget scanning for adds
    - Mez immunity tracking
    - AE vs Single mez selection
    - Remez before break
    - Priority targeting
**|

| ============================================================================
| MEZ CONSTANTS
| ============================================================================
#DEFINE MAXMEZSPELLS 6
#DEFINE MAXMEZIMMUNE 50

| ============================================================================
| SUB: Initialize Mez System
| ============================================================================
Sub InitMezSystem
    /declare i int local

    | Mez spell arrays
    /declare MezSpell[${MAXMEZSPELLS}] string outer
    /declare MezSpellType[${MAXMEZSPELLS}] string outer
    /declare MezSpellGem[${MAXMEZSPELLS}] int outer
    /declare MezSpellCount int outer 0

    | Mez immune tracking
    /declare MezImmune[${MAXMEZIMMUNE}] string outer
    /declare MezImmuneCount int outer 0

    | Mez state
    /declare MezOn int outer 0
    /declare MezRadius int outer 60
    /declare MezMinLevel int outer 1
    /declare MezMaxLevel int outer 999
    /declare MezStartCount int outer 2
    /declare MezStopPct int outer 10
    /declare MezAECount int outer 3
    /declare MezCheckTimer timer outer 0

    | Currently mezzed mobs tracking
    /declare MezzedMob[20] int outer
    /declare MezzedMobTimer[20] timer outer
    /declare MezzedMobCount int outer 0

    | Load settings
    /call LoadMezSettings

    /echo \ag[GZM-Mez] Mez system initialized - ${MezSpellCount} mez spells loaded
/return

| ============================================================================
| SUB: Load Mez Settings from INI
| ============================================================================
Sub LoadMezSettings
    /declare i int local
    /declare spell string local

    | Load mez spells
    /for i 1 to ${MAXMEZSPELLS}
        /varset spell ${Ini[${IniFileName},Mez,MezSpell${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset MezSpell[${i}] ${spell}
            /varset MezSpellType[${i}] ${Ini[${IniFileName},Mez,MezSpell${i}Type,Single]}
            /varset MezSpellGem[${i}] ${Ini[${IniFileName},Mez,MezSpell${i}Gem,0]}
            /varcalc MezSpellCount ${MezSpellCount}+1
            GZMDEBUG_MEZ Loaded mez ${i}: ${spell} (${MezSpellType[${i}]})
        }
    /next i

    | Load immune mobs from INI
    /for i 1 to 20
        /varset spell ${Ini[${IniFileName},Mez,MezImmune${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset MezImmune[${i}] ${spell}
            /varcalc MezImmuneCount ${MezImmuneCount}+1
        }
    /next i

    | Load settings
    /varset MezOn ${Ini[${IniFileName},Mez,MezOn,0]}
    /varset MezRadius ${Ini[${IniFileName},Mez,MezRadius,60]}
    /varset MezMinLevel ${Ini[${IniFileName},Mez,MezMinLevel,1]}
    /varset MezMaxLevel ${Ini[${IniFileName},Mez,MezMaxLevel,999]}
    /varset MezStartCount ${Ini[${IniFileName},Mez,MezStartCount,2]}
    /varset MezStopPct ${Ini[${IniFileName},Mez,MezStopPct,10]}
    /varset MezAECount ${Ini[${IniFileName},Mez,MezAECount,3]}
/return

| ============================================================================
| SUB: Main Mez Check (called from main loop)
| ============================================================================
Sub CheckMez
    /if (!${MezOn}) /return
    /if (!${InCombat}) /return
    /if (${Me.Casting.ID} && !${Me.Class.ShortName.Equal[BRD]}) /return
    /if (${MezCheckTimer}) /return

    /declare mobCount int local 0
    /declare i int local
    /declare mezTarget int local 0

    | Count hostile mobs in radius
    /varset mobCount ${SpawnCount[npc radius ${MezRadius} targetable zradius ${MaxZRange}]}

    | Only mez if enough mobs
    /if (${mobCount} < ${MezStartCount}) /return

    GZMDEBUG_MEZ ${mobCount} mobs in camp - checking for mez targets

    | Priority 1: Check for mobs about to break mez
    /call CheckMezBreaking
    /if (${Macro.Return}) /return

    | Priority 2: Scan XTarget for unmezzed adds
    /call ScanXTargetForMez
    /if (${Macro.Return}) /return

    | Priority 3: Scan nearby NPCs for unmezzed mobs
    /call ScanNearbyForMez

    /varset MezCheckTimer 5
/return

| ============================================================================
| SUB: Check for Mez About to Break
| ============================================================================
Sub CheckMezBreaking
    /declare i int local
    /declare targetID int local

    /for i 1 to ${MezzedMobCount}
        /if (${MezzedMob[${i}]} && ${MezzedMobTimer[${i}]} < 60) {
            /varset targetID ${MezzedMob[${i}]}
            | Verify mob is still alive and mezzed
            /if (${Spawn[${targetID}].ID} && ${Spawn[${targetID}].PctHPs} > 0) {
                | Check if mez is wearing off
                /if (${Spawn[${targetID}].Buff[Mesmerize].Duration} < 6 || ${Spawn[${targetID}].Standing}) {
                    GZMDEBUG_MEZ Remezzing ${Spawn[${targetID}].CleanName} - mez wearing off
                    /call DoMez ${targetID}
                    /return 1
                }
            } else {
                | Mob dead, clear slot
                /varset MezzedMob[${i}] 0
                /varset MezzedMobTimer[${i}] 0
            }
        }
    /next i
/return 0

| ============================================================================
| SUB: Scan XTarget for Mez Targets
| ============================================================================
Sub ScanXTargetForMez
    /declare i int local
    /declare targetID int local
    /declare targetType string local

    | Scan extended target window
    /for i 1 to ${Me.XTargetSlots}
        /if (${Me.XTarget[${i}].ID}) {
            /varset targetID ${Me.XTarget[${i}].ID}
            /varset targetType ${Me.XTarget[${i}].TargetType}

            | Check if it's an auto-hater (add on someone)
            /if (${targetType.Equal[Auto Hater]}) {
                | Skip if it's the main assist's target
                /if (${targetID} == ${MyTargetID}) /continue

                | Skip if already mezzed
                /if (${Spawn[${targetID}].Mezzed.ID}) /continue

                | Skip if immune
                /if (${Bool[${CheckMezImmune[${Spawn[${targetID}].CleanName}]}]}) /continue

                | Skip if level out of range
                /if (${Spawn[${targetID}].Level} < ${MezMinLevel} || ${Spawn[${targetID}].Level} > ${MezMaxLevel}) /continue

                | Skip if out of range
                /if (${Spawn[${targetID}].Distance} > ${MezRadius}) /continue

                GZMDEBUG_MEZ Found XTarget add to mez: ${Spawn[${targetID}].CleanName}
                /call DoMez ${targetID}
                /return 1
            }
        }
    /next i
/return 0

| ============================================================================
| SUB: Scan Nearby NPCs for Mez
| ============================================================================
Sub ScanNearbyForMez
    /declare i int local
    /declare targetID int local
    /declare mobCount int local ${SpawnCount[npc radius ${MezRadius} targetable zradius ${MaxZRange}]}

    /for i 1 to ${mobCount}
        /varset targetID ${NearestSpawn[${i},npc radius ${MezRadius} targetable zradius ${MaxZRange}].ID}

        | Skip main target
        /if (${targetID} == ${MyTargetID}) /continue

        | Skip if not aggro on group
        /if (!${Spawn[${targetID}].PctAggro}) /continue

        | Skip if already mezzed
        /if (${Spawn[${targetID}].Mezzed.ID}) /continue

        | Skip if immune
        /if (${Bool[${CheckMezImmune[${Spawn[${targetID}].CleanName}]}]}) /continue

        | Skip if level out of range
        /if (${Spawn[${targetID}].Level} < ${MezMinLevel} || ${Spawn[${targetID}].Level} > ${MezMaxLevel}) /continue

        GZMDEBUG_MEZ Found nearby add to mez: ${Spawn[${targetID}].CleanName}
        /call DoMez ${targetID}
        /return 1
    /next i
/return 0

| ============================================================================
| SUB: Check Mez Immunity
| ============================================================================
Sub CheckMezImmune(string MobName)
    /declare i int local

    /for i 1 to ${MezImmuneCount}
        /if (${MobName.Find[${MezImmune[${i}]}]}) {
            /return 1
        }
    /next i
/return 0

| ============================================================================
| SUB: Add to Mez Immune List
| ============================================================================
Sub AddMezImmune(string MobName)
    /varcalc MezImmuneCount ${MezImmuneCount}+1
    /varset MezImmune[${MezImmuneCount}] ${MobName}
    /ini "${IniFileName}" "Mez" "MezImmune${MezImmuneCount}" "${MobName}"
    /echo \ay[GZM-Mez] Added ${MobName} to mez immune list
/return

| ============================================================================
| SUB: Do Mez
| ============================================================================
Sub DoMez(int TargetID)
    /if (!${TargetID}) /return

    /declare mezSpell string local
    /declare mobCount int local ${SpawnCount[npc radius 30 from ${Spawn[${TargetID}].ID} targetable]}

    GZMDEBUG_MEZ Attempting to mez ${Spawn[${TargetID}].CleanName}

    | Select mez spell based on situation
    | Use AE mez if multiple mobs clustered
    /if (${mobCount} >= ${MezAECount}) {
        /call FindMezSpell AE
        /varset mezSpell ${Macro.Return}
    }

    | Fall back to single target mez
    /if (!${mezSpell.Length}) {
        /call FindMezSpell Single
        /varset mezSpell ${Macro.Return}
    }

    | Try PBAE as last resort
    /if (!${mezSpell.Length}) {
        /call FindMezSpell PBAE
        /varset mezSpell ${Macro.Return}
    }

    /if (!${mezSpell.Length}) {
        GZMDEBUG_MEZ No mez spell ready!
        /return
    }

    | Target the mob
    /target id ${TargetID}
    /delay 5 ${Target.ID} == ${TargetID}

    | Stand if sitting
    /if (${Me.Sitting}) /stand

    | Cast mez
    GZMDEBUG_MEZ Casting ${mezSpell} on ${Target.CleanName}
    /cast "${mezSpell}"
    /delay 5s ${Me.Casting.ID}
    /delay 10s !${Me.Casting.ID}

    | Track mezzed mob
    /call TrackMezzedMob ${TargetID}

    | Return to MA target
    /if (${MyTargetID}) {
        /target id ${MyTargetID}
    }
/return

| ============================================================================
| SUB: Find Ready Mez Spell
| ============================================================================
Sub FindMezSpell(string MezType)
    /declare i int local

    /for i 1 to ${MezSpellCount}
        /if (${MezSpellType[${i}].Equal[${MezType}]} || ${MezType.Equal[Any]}) {
            /if (${Me.SpellReady[${MezSpell[${i}]}]}) {
                /return ${MezSpell[${i}]}
            }
        }
    /next i
/return

| ============================================================================
| SUB: Track Mezzed Mob
| ============================================================================
Sub TrackMezzedMob(int MobID)
    /declare i int local
    /declare slot int local 0

    | Find empty slot or existing entry
    /for i 1 to 20
        /if (${MezzedMob[${i}]} == ${MobID} || !${MezzedMob[${i}]}) {
            /varset slot ${i}
            /break
        }
    /next i

    /if (${slot}) {
        /varset MezzedMob[${slot}] ${MobID}
        /varset MezzedMobTimer[${slot}] 900 | 90 seconds default mez duration
        /if (${slot} > ${MezzedMobCount}) {
            /varset MezzedMobCount ${slot}
        }
        GZMDEBUG_MEZ Tracking mezzed mob in slot ${slot}: ${Spawn[${MobID}].CleanName}
    }
/return

| ============================================================================
| EVENT: Mez Immune
| ============================================================================
Sub Event_MezImmune
    /if (${Target.ID}) {
        /call AddMezImmune "${Target.CleanName}"
    }
/return

| ============================================================================
| EVENT: Mez Broke
| ============================================================================
Sub Event_MezBroke(string Line, string MobName, string Breaker)
    GZMDEBUG_MEZ MEZ BROKE: ${MobName} woken by ${Breaker}

    | Find and remez immediately
    /declare mobID int local ${Spawn[npc ${MobName}].ID}
    /if (${mobID} && ${mobID} != ${MyTargetID}) {
        /call DoMez ${mobID}
    }
/return

| ============================================================================
| SUB: Bind - Add Mez Immune
| ============================================================================
Sub Bind_AddMezImmune(string MobName)
    /if (${MobName.Length}) {
        /call AddMezImmune "${MobName}"
    } else /if (${Target.ID}) {
        /call AddMezImmune "${Target.CleanName}"
    } else {
        /echo \arUsage: /addmezimmune [mobname] or target a mob
    }
/return

| ============================================================================
| END OF MEZ INCLUDE
| ============================================================================
