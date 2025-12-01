|**
    GZM_pull.inc - GronnzMaster Pulling System

    Comprehensive pulling routines including:
    - Standard pulling
    - Chain pulling
    - Hunter mode (auto-hunting)
    - Nav-based pathing
    - Pull ranking coordination
    - Multiple pull methods (spell, arrow, ability)
    - Retry with strafe on failed pulls
    - Before/After pull hooks
**|

| Pull Events
#Event CantHitPull "You can't hit them from here#*#"
#Event CantHitPull "You cannot see your target#*#"
#Event ImHitPull "#*# YOU, but #*#"
#Event ImHitPull "#*#YOU for#*#points of damage."

| ============================================================================
| PULL CONSTANTS
| ============================================================================
#DEFINE MAXPULLSPELLS 5
#DEFINE MAXPULLIGNORE 50
#DEFINE MAXPULLONLY 50

| ============================================================================
| SUB: Initialize Pull System
| ============================================================================
Sub InitPullSystem
    /declare i int local

    | Pull spell/ability arrays
    /declare PullSpell[${MAXPULLSPELLS}] string outer
    /declare PullSpellType[${MAXPULLSPELLS}] string outer
    /declare PullSpellRange[${MAXPULLSPELLS}] int outer
    /declare PullSpellCount int outer 0

    | Pull ignore/only lists
    /declare PullIgnore[${MAXPULLIGNORE}] string outer
    /declare PullIgnoreCount int outer 0
    /declare PullOnly[${MAXPULLONLY}] string outer
    /declare PullOnlyCount int outer 0

    | Pull state variables
    /declare Pulling int outer 0
    /declare PullTargetID int outer 0
    /declare PullAttempts int outer 0
    /declare MaxPullAttempts int outer 3
    /declare PullSuccessTimer timer outer 0
    /declare LastPullID int outer 0

    | Pull settings
    /declare PullRange int outer 200
    /declare PullZRange int outer 50
    /declare PullMinLevel int outer 1
    /declare PullMaxLevel int outer 999
    /declare PullArc int outer 360
    /declare PullArcCenter int outer 0
    /declare PullHold int outer 0
    /declare PullDelay int outer 10
    /declare PullWait timer outer 0

    | Chain pull settings
    /declare ChainPull int outer 0
    /declare ChainPullHP int outer 20
    /declare ChainPullMobCount int outer 1
    /declare ChainPullDelay int outer 5

    | Hunter settings
    /declare HunterMode int outer 0
    /declare HunterRadius int outer 500
    /declare HunterZRadius int outer 100
    /declare HunterNavMesh int outer 1

    | Nav pull settings
    /declare PullNavOn int outer 1
    /declare PullNavTimeout int outer 30
    /declare PullNavStuckDist float outer 5

    | Pull ranking (for coordinated pulls)
    /declare PullRanking int outer 0
    /declare PullRankingTimer timer outer 0
    /declare MyPullRank int outer 1

    | Pull method preferences
    /declare PullWithSpell int outer 1
    /declare PullWithBow int outer 1
    /declare PullWithAbility int outer 1
    /declare PreferredPullMethod string outer spell

    | Pull Retry System
    /declare PullRetries int outer 3
    /declare PullStrafeOnFail int outer 1
    /declare FailCheck int outer 0

    | Before/After Pull Hooks
    /declare PullBeforeHook string outer
    /declare PullAfterHook string outer
    /declare PullAnnounce string outer

    | Load settings
    /call LoadPullSettings

    /echo \ag[GZM-Pull] Pull system initialized - ${PullSpellCount} pull abilities loaded
/return

| ============================================================================
| SUB: Load Pull Settings from INI
| ============================================================================
Sub LoadPullSettings
    /declare i int local
    /declare spell string local

    | Load pull spells/abilities
    /for i 1 to ${MAXPULLSPELLS}
        /varset spell ${Ini[${IniFileName},Pull,PullSpell${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset PullSpell[${i}] ${spell}
            /varset PullSpellType[${i}] ${Ini[${IniFileName},Pull,PullSpell${i}Type,Spell]}
            /varset PullSpellRange[${i}] ${Ini[${IniFileName},Pull,PullSpell${i}Range,200]}
            /varcalc PullSpellCount ${PullSpellCount}+1
            GZMDEBUG_PULL Loaded pull spell ${i}: ${spell}
        }
    /next i

    | Load ignore list
    /for i 1 to ${MAXPULLIGNORE}
        /varset spell ${Ini[${IniFileName},PullIgnore,Ignore${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset PullIgnore[${i}] ${spell}
            /varcalc PullIgnoreCount ${PullIgnoreCount}+1
        }
    /next i

    | Load pull only list
    /for i 1 to ${MAXPULLONLY}
        /varset spell ${Ini[${IniFileName},PullOnly,Only${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset PullOnly[${i}] ${spell}
            /varcalc PullOnlyCount ${PullOnlyCount}+1
        }
    /next i

    | Load settings
    /varset PullRange ${Ini[${IniFileName},Pull,PullRange,200]}
    /varset PullZRange ${Ini[${IniFileName},Pull,PullZRange,50]}
    /varset PullMinLevel ${Ini[${IniFileName},Pull,PullMinLevel,1]}
    /varset PullMaxLevel ${Ini[${IniFileName},Pull,PullMaxLevel,999]}
    /varset PullArc ${Ini[${IniFileName},Pull,PullArc,360]}
    /varset PullDelay ${Ini[${IniFileName},Pull,PullDelay,10]}

    /varset ChainPull ${Ini[${IniFileName},Pull,ChainPull,0]}
    /varset ChainPullHP ${Ini[${IniFileName},Pull,ChainPullHP,20]}
    /varset ChainPullMobCount ${Ini[${IniFileName},Pull,ChainPullMobCount,1]}

    /varset HunterMode ${Ini[${IniFileName},Pull,HunterMode,0]}
    /varset HunterRadius ${Ini[${IniFileName},Pull,HunterRadius,500]}
    /varset HunterNavMesh ${Ini[${IniFileName},Pull,HunterNavMesh,1]}

    /varset PullNavOn ${Ini[${IniFileName},Pull,PullNavOn,1]}
    /varset PullRanking ${Ini[${IniFileName},Pull,PullRanking,0]}

    /varset PreferredPullMethod ${Ini[${IniFileName},Pull,PreferredPullMethod,spell]}
/return

| ============================================================================
| SUB: Main Pull Check (called from main loop)
| ============================================================================
Sub CheckPulling
    /if (!${PullOn}) /return
    /if (${PullHold}) /return
    /if (${PullWait}) /return
    /if (${BackOffFlag}) /return

    | Check if we're the right role
    /if (!${Select[${Role},Puller,PullerTank,PullerPetTank,Hunter,HunterTank,HunterPetTank]}) /return

    | Chain pull check
    /if (${ChainPull} && ${InCombat}) {
        /call CheckChainPull
        /return
    }

    | Don't pull if mobs in camp
    /declare MobCount int local ${SpawnCount[npc targetable radius ${CampRadius} zradius ${MaxZRange}]}
    /if (${MobCount} > 0) /return

    | Hunter mode
    /if (${HunterMode}) {
        /call HunterPull
        /return
    }

    | Standard pull
    /call StandardPull
/return

| ============================================================================
| SUB: Standard Pull
| ============================================================================
Sub StandardPull
    /declare PullID int local 0

    | Find a mob to pull
    /call FindMobToPull
    /varset PullID ${Macro.Return}

    /if (!${PullID}) {
        GZMDEBUG_PULL No valid pull target found
        /return
    }

    | Execute pull
    /call ExecutePull ${PullID}
/return

| ============================================================================
| SUB: Chain Pull
| ============================================================================
Sub CheckChainPull
    /declare MobCount int local ${SpawnCount[xtarhater radius ${CampRadius}]}

    | Don't chain pull if too many mobs
    /if (${MobCount} >= ${ChainPullMobCount}) /return

    | Don't chain pull if current target is high HP
    /if (${Target.PctHPs} > ${ChainPullHP}) /return

    | Find next pull target
    /declare PullID int local 0
    /call FindMobToPull
    /varset PullID ${Macro.Return}

    /if (${PullID}) {
        GZMDEBUG_PULL Chain pull - pulling ${Spawn[${PullID}].CleanName}
        /call ExecutePull ${PullID}
    }
/return

| ============================================================================
| SUB: Hunter Pull (auto-hunt mode)
| ============================================================================
Sub HunterPull
    /declare PullID int local 0

    | Find mob within hunter radius
    /call FindHunterTarget
    /varset PullID ${Macro.Return}

    /if (!${PullID}) {
        GZMDEBUG_PULL Hunter: No targets in range
        /return
    }

    GZMDEBUG_PULL Hunter: Found ${Spawn[${PullID}].CleanName} at ${Spawn[${PullID}].Distance}

    | Navigate to mob if needed
    /if (${HunterNavMesh} && ${Plugin[MQ2Nav].Name.Length}) {
        /if (${Spawn[${PullID}].Distance} > ${PullRange}) {
            GZMDEBUG_NAV Hunter: Navigating to ${Spawn[${PullID}].CleanName}
            /nav id ${PullID}
            /delay ${PullNavTimeout}s !${Nav.Active} || ${Spawn[${PullID}].Distance} < ${PullRange}
            /squelch /nav stop
        }
    }

    | Execute pull
    /call ExecutePull ${PullID}
/return

| ============================================================================
| SUB: Find Mob To Pull
| ============================================================================
Sub FindMobToPull
    /declare i int local
    /declare checkID int local 0
    /declare searchString string local

    | Build search string
    /varset searchString "npc targetable radius ${PullRange} zradius ${PullZRange}"

    | Add level range
    /if (${PullMinLevel} > 1 || ${PullMaxLevel} < 999) {
        /varset searchString "${searchString} level ${PullMinLevel} ${PullMaxLevel}"
    }

    | Add noalert (ignore list)
    /varset searchString "${searchString} noalert 5"

    | Check pull ranking - coordinate with other pullers
    /if (${PullRanking} && ${UseDanNet}) {
        /call CheckPullRanking
        /if (!${Macro.Return}) {
            /return 0
        }
    }

    | Search for mobs
    /for i 1 to ${SpawnCount[${searchString}]}
        /varset checkID ${NearestSpawn[${i},${searchString}].ID}

        /if (!${checkID}) /continue

        | Check ignore list
        /call IsMobIgnored ${checkID}
        /if (${Macro.Return}) /continue

        | Check pull only list
        /if (${PullOnlyCount} > 0) {
            /call IsMobPullOnly ${checkID}
            /if (!${Macro.Return}) /continue
        }

        | Check arc
        /if (${PullArc} < 360) {
            /call IsInPullArc ${checkID}
            /if (!${Macro.Return}) /continue
        }

        | Check LOS
        /if (!${Spawn[${checkID}].LineOfSight}) {
            | If nav available, check path
            /if (${PullNavOn} && ${Plugin[MQ2Nav].Name.Length}) {
                /if (!${Nav.PathExists[id ${checkID}]}) /continue
            } else {
                /continue
            }
        }

        | Valid target found
        /return ${checkID}
    /next i

/return 0

| ============================================================================
| SUB: Find Hunter Target
| ============================================================================
Sub FindHunterTarget
    /declare searchString string local
    /declare i int local
    /declare checkID int local 0

    /varset searchString "npc targetable radius ${HunterRadius} zradius ${HunterZRadius} noalert 5"

    /for i 1 to ${SpawnCount[${searchString}]}
        /varset checkID ${NearestSpawn[${i},${searchString}].ID}

        /if (!${checkID}) /continue

        | Check ignore list
        /call IsMobIgnored ${checkID}
        /if (${Macro.Return}) /continue

        | Check nav path if using navmesh
        /if (${HunterNavMesh} && ${Plugin[MQ2Nav].Name.Length}) {
            /if (!${Nav.PathExists[id ${checkID}]}) /continue
        }

        /return ${checkID}
    /next i

/return 0

| ============================================================================
| SUB: Execute Pull
| ============================================================================
Sub ExecutePull(int MobID)
    /if (!${MobID}) /return
    /if (!${Spawn[${MobID}].ID}) /return

    /varset Pulling 1
    /varset PullTargetID ${MobID}
    /varset PullAttempts 0
    /varset FailCheck 0

    /declare strafeLeft int local 1
    /declare tDistance int local 0

    GZMDEBUG_PULL Pulling ${Spawn[${MobID}].CleanName} (${MobID})

    | Execute before-pull hook
    /if (${PullBeforeHook.Length}) {
        GZMDEBUG_PULL Executing before-pull hook
        /call ExecPullHook "${PullBeforeHook}"
    }

    | Target the mob
    /target id ${MobID}
    /delay 10 ${Target.ID} == ${MobID}

    /if (${Target.ID} != ${MobID}) {
        GZMDEBUG_PULL Failed to target pull mob
        /varset Pulling 0
        /return
    }

    | Navigate closer if needed
    /if (${Target.Distance} > ${PullRange} && ${PullNavOn} && ${Plugin[MQ2Nav].Name.Length}) {
        GZMDEBUG_NAV Navigating to pull range
        /nav id ${MobID} distance=${PullRange}
        /delay ${PullNavTimeout}s !${Nav.Active}
        /squelch /nav stop
    }

    | Execute pull action with retry and strafe
    :PullAttempt
    /varcalc PullAttempts ${PullAttempts}+1
    /varset tDistance ${Target.Distance}
    /varset FailCheck 0

    /if (${PullAttempts} > ${MaxPullAttempts}) {
        GZMDEBUG_PULL Max pull attempts reached
        /call AddToIgnore ${MobID}
        /varset Pulling 0
        /varset PullWait ${PullDelay}s
        /return
    }

    /call DoPullAction ${MobID}

    | Check for cant-hit event
    /doevents CantHitPull

    | Strafe on fail
    /if (${FailCheck} && !${Me.CombatState.Equal[combat]}) {
        /if (${PullAttempts} <= ${PullRetries} && ${PullStrafeOnFail}) {
            GZMDEBUG_PULL Pull failed, strafing and retrying (${PullAttempts}/${PullRetries})
            /if (${strafeLeft}) {
                /nomodkey /keypress strafe_left hold
                /delay 3
                /nomodkey /keypress strafe_left
                /varset strafeLeft 0
            } else {
                /nomodkey /keypress strafe_right hold
                /delay 6
                /nomodkey /keypress strafe_right
                /varset strafeLeft 1
            }
            /goto :PullAttempt
        }
    }

    | Wait for aggro
    /delay 30 ${Me.TargetOfTarget.ID} == ${Me.ID} || ${Spawn[${MobID}].Distance} < 30

    | Check if we got aggro
    /if (${Me.TargetOfTarget.ID} != ${Me.ID} && ${Spawn[${MobID}].Distance} > 30) {
        GZMDEBUG_PULL Pull attempt ${PullAttempts} failed, retrying
        /goto :PullAttempt
    }

    | Execute after-pull hook
    /if (${PullAfterHook.Length}) {
        GZMDEBUG_PULL Executing after-pull hook
        /call ExecPullHook "${PullAfterHook}"
    }

    | Announce pull
    /if (${PullAnnounce.Length}) {
        /docommand ${PullAnnounce}
    }

    | Return to camp
    /if (${ReturnToCamp}) {
        /call ReturnToCampLoc
    }

    /varset Pulling 0
    /varset LastPullID ${MobID}
    /varset PullWait ${PullDelay}s
    GZMDEBUG_PULL Pull complete
/return

| ============================================================================
| SUB: Execute Pull Hook Command
| ============================================================================
Sub ExecPullHook(string hookCmd)
    /if (!${hookCmd.Length}) /return

    | Parse and execute the command
    /if (${hookCmd.Left[1].Equal[/]}) {
        | Direct command
        /docommand ${hookCmd}
    } else {
        | Sub call
        /call ${hookCmd}
    }
/return

| ============================================================================
| SUB: Do Pull Action
| ============================================================================
Sub DoPullAction(int MobID)
    /declare i int local

    | Try preferred method first
    /if (${PreferredPullMethod.Equal[spell]} && ${PullWithSpell}) {
        /call PullWithSpellAction ${MobID}
        /if (${Macro.Return}) /return
    } else /if (${PreferredPullMethod.Equal[bow]} && ${PullWithBow}) {
        /call PullWithBowAction ${MobID}
        /if (${Macro.Return}) /return
    } else /if (${PreferredPullMethod.Equal[ability]} && ${PullWithAbility}) {
        /call PullWithAbilityAction ${MobID}
        /if (${Macro.Return}) /return
    }

    | Try configured pull spells
    /for i 1 to ${PullSpellCount}
        /if (!${PullSpell[${i}].Length}) /continue
        /if (${Target.Distance} > ${PullSpellRange[${i}]}) /continue

        /if (${PullSpellType[${i}].Equal[Spell]}) {
            /if (${Me.SpellReady[${PullSpell[${i}]}]}) {
                GZMDEBUG_PULL Pull spell: ${PullSpell[${i}]}
                /cast "${PullSpell[${i}]}"
                /delay 3s ${Me.Casting.ID}
                /delay 5s !${Me.Casting.ID}
                /return
            }
        } else /if (${PullSpellType[${i}].Equal[AA]}) {
            /if (${Me.AltAbilityReady[${PullSpell[${i}]}]}) {
                GZMDEBUG_PULL Pull AA: ${PullSpell[${i}]}
                /alt act ${Me.AltAbility[${PullSpell[${i}]}].ID}
                /delay 10
                /return
            }
        } else /if (${PullSpellType[${i}].Equal[Item]}) {
            /if (${FindItem[${PullSpell[${i}]}].TimerReady} == 0) {
                GZMDEBUG_PULL Pull item: ${PullSpell[${i}]}
                /useitem "${PullSpell[${i}]}"
                /delay 10
                /return
            }
        } else /if (${PullSpellType[${i}].Equal[Ability]}) {
            /if (${Me.AbilityReady[${PullSpell[${i}]}]}) {
                GZMDEBUG_PULL Pull ability: ${PullSpell[${i}]}
                /doability "${PullSpell[${i}]}"
                /delay 10
                /return
            }
        }
    /next i

    | Fallback: bow
    /call PullWithBowAction ${MobID}
    /if (${Macro.Return}) /return

    | Fallback: melee
    GZMDEBUG_PULL Fallback: melee pull
    /if (${Target.Distance} <= ${MeleeDistance}) {
        /attack on
        /delay 10
        /attack off
    }
/return

| ============================================================================
| SUB: Pull With Spell
| ============================================================================
Sub PullWithSpellAction(int MobID)
    | Find first ready pull spell
    /declare i int local

    /for i 1 to ${PullSpellCount}
        /if (${PullSpellType[${i}].Equal[Spell]} && ${Me.SpellReady[${PullSpell[${i}]}]}) {
            /if (${Target.Distance} <= ${PullSpellRange[${i}]}) {
                /cast "${PullSpell[${i}]}"
                /delay 3s ${Me.Casting.ID}
                /delay 5s !${Me.Casting.ID}
                /return 1
            }
        }
    /next i
/return 0

| ============================================================================
| SUB: Pull With Bow
| ============================================================================
Sub PullWithBowAction(int MobID)
    /if (!${PullWithBow}) /return 0
    /if (${Target.Distance} > 200) /return 0

    | Check for ranged weapon
    /if (${Me.Inventory[ranged].ID}) {
        /autofire on
        /delay 20
        /autofire off
        /return 1
    }
/return 0

| ============================================================================
| SUB: Pull With Ability
| ============================================================================
Sub PullWithAbilityAction(int MobID)
    | Class-specific pull abilities
    /if (${Me.Class.ShortName.Equal[MNK]}) {
        /if (${Me.AbilityReady[Distant Strike]}) {
            /doability "Distant Strike"
            /return 1
        }
    } else /if (${Me.Class.ShortName.Equal[BER]}) {
        /if (${Me.AbilityReady[Throw Stone]}) {
            /doability "Throw Stone"
            /return 1
        }
    } else /if (${Me.Class.ShortName.Equal[WAR]}) {
        /if (${Me.AltAbilityReady[Projection of Fury]}) {
            /alt act ${Me.AltAbility[Projection of Fury].ID}
            /return 1
        }
    }
/return 0

| ============================================================================
| SUB: Is Mob Ignored
| ============================================================================
Sub IsMobIgnored(int MobID)
    /declare i int local
    /declare mobName string local ${Spawn[${MobID}].CleanName}

    /for i 1 to ${PullIgnoreCount}
        /if (${PullIgnore[${i}].Length} && ${mobName.Find[${PullIgnore[${i}]}]}) {
            /return 1
        }
    /next i
/return 0

| ============================================================================
| SUB: Is Mob Pull Only
| ============================================================================
Sub IsMobPullOnly(int MobID)
    /declare i int local
    /declare mobName string local ${Spawn[${MobID}].CleanName}

    /for i 1 to ${PullOnlyCount}
        /if (${PullOnly[${i}].Length} && ${mobName.Find[${PullOnly[${i}]}]}) {
            /return 1
        }
    /next i
/return 0

| ============================================================================
| SUB: Is In Pull Arc
| ============================================================================
Sub IsInPullArc(int MobID)
    /declare mobHeading float local ${Spawn[${MobID}].HeadingTo.DegreesCCW}
    /declare arcStart float local ${Math.Calc[${PullArcCenter} - (${PullArc}/2)]}
    /declare arcEnd float local ${Math.Calc[${PullArcCenter} + (${PullArc}/2)]}

    /if (${mobHeading} >= ${arcStart} && ${mobHeading} <= ${arcEnd}) {
        /return 1
    }
/return 0

| ============================================================================
| SUB: Check Pull Ranking (coordinated pulls)
| ============================================================================
Sub CheckPullRanking
    /if (!${UseDanNet}) /return 1

    | Query other pullers
    /declare i int local
    /declare peerRank int local

    | For now, simple implementation - could be expanded
    /if (${PullRankingTimer}) {
        /return 0
    }

    /varset PullRankingTimer 50
/return 1

| ============================================================================
| SUB: Add To Ignore (temporary)
| ============================================================================
Sub AddToIgnore(int MobID)
    /squelch /alert add 5 id ${MobID}
    GZMDEBUG_PULL Added ${Spawn[${MobID}].CleanName} to ignore list
/return

| ============================================================================
| BIND HANDLERS
| ============================================================================

Sub Bind_PullHold
    /varset PullHold ${If[${PullHold},0,1]}
    /echo \ayPull Hold: ${If[${PullHold},ON,OFF]}
/return

Sub Bind_SetPullArc(string ArcDegrees)
    /if (${ArcDegrees.Length}) {
        /varset PullArc ${ArcDegrees}
        /ini "${IniFileName}" "Pull" "PullArc" "${ArcDegrees}"
        /echo \ayPull Arc set to ${ArcDegrees} degrees
    } else {
        /echo \ayCurrent Pull Arc: ${PullArc} degrees
    }
/return

Sub Bind_SetPullRanking(string Rank)
    /if (${Rank.Length}) {
        /varset MyPullRank ${Rank}
        /echo \ayPull Ranking set to ${MyPullRank}
    } else {
        /echo \ayCurrent Pull Rank: ${MyPullRank}
    }
/return

Sub Bind_AddPull(string MobName)
    /if (${MobName.Length}) {
        /varcalc PullOnlyCount ${PullOnlyCount}+1
        /varset PullOnly[${PullOnlyCount}] ${MobName}
        /ini "${IniFileName}" "PullOnly" "Only${PullOnlyCount}" "${MobName}"
        /echo \agAdded ${MobName} to pull only list
    } else /if (${Target.ID}) {
        /varcalc PullOnlyCount ${PullOnlyCount}+1
        /varset PullOnly[${PullOnlyCount}] ${Target.CleanName}
        /ini "${IniFileName}" "PullOnly" "Only${PullOnlyCount}" "${Target.CleanName}"
        /echo \agAdded ${Target.CleanName} to pull only list
    }
/return

Sub Bind_AddIgnore(string MobName)
    /if (${MobName.Length}) {
        /varcalc PullIgnoreCount ${PullIgnoreCount}+1
        /varset PullIgnore[${PullIgnoreCount}] ${MobName}
        /ini "${IniFileName}" "PullIgnore" "Ignore${PullIgnoreCount}" "${MobName}"
        /echo \agAdded ${MobName} to ignore list
    } else /if (${Target.ID}) {
        /varcalc PullIgnoreCount ${PullIgnoreCount}+1
        /varset PullIgnore[${PullIgnoreCount}] ${Target.CleanName}
        /ini "${IniFileName}" "PullIgnore" "Ignore${PullIgnoreCount}" "${Target.CleanName}"
        /echo \agAdded ${Target.CleanName} to ignore list
    }
/return

| ============================================================================
| PULL EVENT HANDLERS
| ============================================================================

| Detected we can't hit the target - set fail flag for retry/strafe
Sub Event_CantHitPull
    /if (${Pulling}) {
        /varset FailCheck 1
        GZMDEBUG_PULL Can't hit target - flagging for retry
    }
    /doevents flush CantHitPull
/return

| Detected we got hit - we have aggro from pull
Sub Event_ImHitPull
    /if (${Pulling} && !${PullTargetID}) {
        | Got hit while pulling - acquire target
        /if (${Target.ID} != ${Me.ID} && ${Target.Type.Equal[NPC]}) {
            /varset PullTargetID ${Target.ID}
            GZMDEBUG_PULL Got hit - acquired pull target ${PullTargetID}
        }
    }
    /doevents flush ImHitPull
/return

| ============================================================================
| END OF PULL INCLUDE
| ============================================================================
