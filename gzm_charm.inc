|**
    GZM_charm.inc - GronnzMaster Charm Pet System

    Comprehensive charm handling for:
    - Enchanter (any mob type)
    - Necromancer (undead only)
    - Druid (animal only)

    Features:
    - Regular charm with duration tracking
    - Dire Charm AA (permanent)
    - Root/Stun before recharm
    - Charm break instant recovery
    - Buff charmed pet (haste, etc)
    - Undead body type check for necros
    - Animal body type check for druids
    - Send charm pet to attack
**|

| ============================================================================
| CHARM CONSTANTS
| ============================================================================
#DEFINE MAXCHARMBUFFS 4

| ============================================================================
| SUB: Initialize Charm System
| ============================================================================
Sub InitCharmSystem
    /declare i int local

    | Charm state
    /declare CharmOn int outer 0
    /declare CharmSpell string outer
    /declare CharmTarget int outer 0
    /declare CharmTargetName string outer
    /declare CharmTimer timer outer 0
    /declare CharmDuration int outer 1200
    /declare CharmBreakTimer timer outer 0
    /declare CharmAttempts int outer 0

    | Charm settings
    /declare CharmMinLevel int outer 1
    /declare CharmMaxLevel int outer 999
    /declare CharmRadius int outer 100
    /declare CharmInCombat int outer 1
    /declare CharmMobCount int outer 3

    | Dire Charm
    /declare DireCharmOn int outer 1
    /declare DireCharmSpell string outer "Dire Charm"
    /declare DireCharmMaxLevel int outer 46

    | Charm recovery spells
    /declare CharmRootSpell string outer
    /declare CharmStunSpell string outer
    /declare CharmMezSpell string outer

    | Charm pet buffs
    /declare CharmBuff[${MAXCHARMBUFFS}] string outer
    /declare CharmBuffCount int outer 0

    | Charm pet attack
    /declare CharmPetAttack int outer 1

    | Class-specific
    /declare CharmType string outer Any
    | ENC = Any, NEC = Undead, DRU = Animal

    | Load settings
    /call LoadCharmSettings

    /echo \ag[GZM-Charm] Charm system initialized
/return

| ============================================================================
| SUB: Load Charm Settings
| ============================================================================
Sub LoadCharmSettings
    /declare i int local
    /declare spell string local

    | Basic settings
    /varset CharmOn ${Ini[${IniFileName},Charm,CharmOn,0]}
    /varset CharmSpell ${Ini[${IniFileName},Charm,CharmSpell]}
    /varset CharmMinLevel ${Ini[${IniFileName},Charm,CharmMinLevel,1]}
    /varset CharmMaxLevel ${Ini[${IniFileName},Charm,CharmMaxLevel,999]}
    /varset CharmRadius ${Ini[${IniFileName},Charm,CharmRadius,100]}
    /varset CharmDuration ${Ini[${IniFileName},Charm,CharmDuration,1200]}
    /varset CharmMobCount ${Ini[${IniFileName},Charm,CharmMobCount,3]}

    | Dire Charm
    /varset DireCharmOn ${Ini[${IniFileName},Charm,DireCharmOn,1]}
    /varset DireCharmSpell ${Ini[${IniFileName},Charm,DireCharmSpell,Dire Charm]}
    /varset DireCharmMaxLevel ${Ini[${IniFileName},Charm,DireCharmMaxLevel,46]}

    | Recovery spells
    /varset CharmRootSpell ${Ini[${IniFileName},Charm,CharmRootSpell]}
    /varset CharmStunSpell ${Ini[${IniFileName},Charm,CharmStunSpell]}
    /varset CharmMezSpell ${Ini[${IniFileName},Charm,CharmMezSpell]}

    | Set class-specific charm type
    /if (${Me.Class.ShortName.Equal[ENC]}) {
        /varset CharmType Any
        /if (!${CharmRootSpell.Length}) /varset CharmRootSpell "Root"
        /if (!${CharmStunSpell.Length}) /varset CharmStunSpell "Stun"
        /if (!${CharmMezSpell.Length}) /varset CharmMezSpell "Mesmerize"
    } else /if (${Me.Class.ShortName.Equal[NEC]}) {
        /varset CharmType Undead
        /if (!${CharmSpell.Length}) /varset CharmSpell "Charm Undead"
    } else /if (${Me.Class.ShortName.Equal[DRU]}) {
        /varset CharmType Animal
        /if (!${CharmSpell.Length}) /varset CharmSpell "Charm Animals"
    }

    | Charm buffs (haste, etc)
    /for i 1 to ${MAXCHARMBUFFS}
        /varset spell ${Ini[${IniFileName},Charm,CharmBuff${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset CharmBuff[${i}] ${spell}
            /varcalc CharmBuffCount ${CharmBuffCount}+1
        }
    /next i

    | Default haste buff for enchanters
    /if (${Me.Class.ShortName.Equal[ENC]} && !${CharmBuffCount}) {
        /varset CharmBuff[1] "Hastening of Aednia"
        /varset CharmBuffCount 1
    }
/return

| ============================================================================
| SUB: Check Charm (called from main loop)
| ============================================================================
Sub CheckCharm
    /if (!${CharmOn}) /return
    /if (!${CharmSpell.Length} && !${DireCharmOn}) /return

    | Priority 1: EMERGENCY - Charm broke, handle immediately!
    /if (${CharmTarget} && !${Me.Pet.ID}) {
        GZMDEBUG_PET !!! CHARM BROKE ON ${CharmTargetName} !!!
        /call HandleCharmBreak
        /return
    }

    | Priority 2: Check charm duration - refresh before it breaks
    /if (${Me.Pet.ID} && ${CharmTarget}) {
        /if (${CharmTimer} < 150) {
            GZMDEBUG_PET Charm on ${CharmTargetName} wearing off (${CharmTimer} ticks left)
            /call RefreshCharm
            /return
        }
    }

    | Priority 3: Send charm pet to attack
    /if (${Me.Pet.ID} && ${CharmPetAttack} && ${InCombat} && ${MyTargetID}) {
        /if (${Me.Pet.Target.ID} != ${MyTargetID}) {
            /pet attack ${MyTargetID}
        }
    }

    | Priority 4: Buff charm pet
    /if (${Me.Pet.ID} && !${InCombat}) {
        /call BuffCharmPet
    }

    | Priority 5: Find new charm target if we don't have one
    /if (!${Me.Pet.ID} && ${InCombat} && ${CharmInCombat} && !${CharmBreakTimer}) {
        /call FindCharmTarget
    }
/return

| ============================================================================
| SUB: Handle Charm Break - EMERGENCY RESPONSE
| ============================================================================
Sub HandleCharmBreak
    /declare targetID int local ${CharmTarget}

    /echo \ar[GZM-Charm] CHARM BROKE! Emergency response!

    | Clear charm tracking
    /varset CharmTarget 0

    | Check if mob still exists
    /if (!${Spawn[${targetID}].ID}) {
        /echo \ay[GZM-Charm] Charm target no longer exists
        /varset CharmTargetName
        /return
    }

    | Mob is free and angry - need to control it!
    /target id ${targetID}
    /delay 5 ${Target.ID} == ${targetID}

    | Stand if sitting
    /if (${Me.Sitting}) /stand

    | Priority 1: Try to stun it first (instant)
    /if (${CharmStunSpell.Length} && ${Me.SpellReady[${CharmStunSpell}]}) {
        /echo \ay[GZM-Charm] Stunning ${Target.CleanName}
        /cast "${CharmStunSpell}"
        /delay 3s ${Me.Casting.ID}
        /delay 5s !${Me.Casting.ID}
    }

    | Priority 2: Root it if stun didn't work or isn't ready
    /if (${CharmRootSpell.Length} && ${Me.SpellReady[${CharmRootSpell}]}) {
        /if (!${Target.Rooted.ID}) {
            /echo \ay[GZM-Charm] Rooting ${Target.CleanName}
            /cast "${CharmRootSpell}"
            /delay 3s ${Me.Casting.ID}
            /delay 5s !${Me.Casting.ID}
        }
    }

    | Priority 3: Mez it as backup while we recharm
    /if (${CharmMezSpell.Length} && ${Me.SpellReady[${CharmMezSpell}]}) {
        /if (!${Target.Mezzed.ID}) {
            /echo \ay[GZM-Charm] Mezzing ${Target.CleanName}
            /cast "${CharmMezSpell}"
            /delay 3s ${Me.Casting.ID}
            /delay 8s !${Me.Casting.ID}
        }
    }

    | Now recharm
    /varcalc CharmAttempts ${CharmAttempts}+1

    /if (${CharmAttempts} > 3) {
        /echo \ar[GZM-Charm] Too many charm attempts on ${Target.CleanName} - giving up!
        /varset CharmAttempts 0
        /varset CharmBreakTimer 300
        /varset CharmTargetName
        /return
    }

    /call DoCharm ${targetID}
/return

| ============================================================================
| SUB: Refresh Charm (before it breaks)
| ============================================================================
Sub RefreshCharm
    /if (!${CharmTarget}) /return
    /if (!${Spawn[${CharmTarget}].ID}) /return

    GZMDEBUG_PET Refreshing charm on ${CharmTargetName}

    | Target charm pet
    /target id ${CharmTarget}
    /delay 5 ${Target.ID} == ${CharmTarget}

    | Recast charm
    /call DoCharm ${CharmTarget}
/return

| ============================================================================
| SUB: Find Charm Target
| ============================================================================
Sub FindCharmTarget
    /declare i int local
    /declare targetID int local
    /declare mobCount int local ${SpawnCount[npc radius ${CharmRadius} targetable zradius ${MaxZRange}]}

    | Don't charm if not enough mobs
    /if (${mobCount} < ${CharmMobCount}) /return

    GZMDEBUG_PET Looking for charm target (${mobCount} mobs in range)

    /for i 1 to ${mobCount}
        /varset targetID ${NearestSpawn[${i},npc radius ${CharmRadius} targetable zradius ${MaxZRange}].ID}

        | Skip main target
        /if (${targetID} == ${MyTargetID}) /continue

        | Check level - try Dire Charm first on lower levels
        /if (${DireCharmOn} && ${Spawn[${targetID}].Level} <= ${DireCharmMaxLevel}) {
            /if (${Bool[${IsCharmable[${targetID}]}]}) {
                GZMDEBUG_PET Dire Charm candidate: ${Spawn[${targetID}].CleanName}
                /call DoDireCharm ${targetID}
                /if (${Me.Pet.ID}) /return
            }
        }

        | Regular charm level check
        /if (${Spawn[${targetID}].Level} < ${CharmMinLevel} || ${Spawn[${targetID}].Level} > ${CharmMaxLevel}) /continue

        | Check if charmable for this class
        /if (!${Bool[${IsCharmable[${targetID}]}]}) /continue

        | Skip named mobs
        /if (${Spawn[${targetID}].Named}) /continue

        GZMDEBUG_PET Found charm target: ${Spawn[${targetID}].CleanName}
        /call DoCharm ${targetID}
        /return
    /next i
/return

| ============================================================================
| SUB: Check if Mob is Charmable
| ============================================================================
Sub IsCharmable(int MobID)
    /if (!${MobID}) /return 0
    /if (!${Spawn[${MobID}].ID}) /return 0

    /declare bodyType string local ${Spawn[${MobID}].Body.Name}

    | Check class-specific charm type
    /if (${CharmType.Equal[Undead]}) {
        | Necromancer - undead only
        /if (!${bodyType.Equal[Undead]}) {
            GZMDEBUG_PET ${Spawn[${MobID}].CleanName} is not undead (${bodyType})
            /return 0
        }
    } else /if (${CharmType.Equal[Animal]}) {
        | Druid - animal only
        /if (!${bodyType.Equal[Animal]}) {
            GZMDEBUG_PET ${Spawn[${MobID}].CleanName} is not an animal (${bodyType})
            /return 0
        }
    }
    | Enchanter (Any) - can charm most mobs

    | Check for uncharmable body types
    /if (${bodyType.Equal[Undead Summoned]}) /return 0
    /if (${bodyType.Equal[Construct]}) /return 0

/return 1

| ============================================================================
| SUB: Do Dire Charm (Permanent)
| ============================================================================
Sub DoDireCharm(int TargetID)
    /if (!${TargetID}) /return
    /if (!${DireCharmOn}) /return

    | Check if Dire Charm AA is ready
    /if (!${Me.AltAbilityReady[${DireCharmSpell}]}) /return

    GZMDEBUG_PET Attempting DIRE CHARM on ${Spawn[${TargetID}].CleanName}

    | Target
    /target id ${TargetID}
    /delay 5 ${Target.ID} == ${TargetID}

    | Stand
    /if (${Me.Sitting}) /stand

    | Use Dire Charm AA
    /echo \ag[GZM-Charm] Using DIRE CHARM on ${Target.CleanName}
    /alt act ${Me.AltAbility[${DireCharmSpell}].ID}
    /delay 5s ${Me.Casting.ID}
    /delay 10s !${Me.Casting.ID}

    | Check success
    /delay 10
    /if (${Me.Pet.ID}) {
        /varset CharmTarget ${TargetID}
        /varset CharmTargetName ${Spawn[${TargetID}].CleanName}
        /varset CharmTimer 999999
        /varset CharmAttempts 0
        /echo \ag[GZM-Charm] DIRE CHARM successful - ${CharmTargetName} is permanently charmed!

        | Configure dire charmed pet
        /pet hold
        /delay 2
        /pet notaunt
    } else {
        /echo \ar[GZM-Charm] Dire Charm FAILED on ${Target.CleanName}
    }
/return

| ============================================================================
| SUB: Do Regular Charm
| ============================================================================
Sub DoCharm(int TargetID)
    /if (!${TargetID}) /return
    /if (!${CharmSpell.Length}) /return

    GZMDEBUG_PET Attempting charm on ${Spawn[${TargetID}].CleanName}

    | Target
    /target id ${TargetID}
    /delay 5 ${Target.ID} == ${TargetID}

    | Stand
    /if (${Me.Sitting}) /stand

    | Cast charm
    /if (${Me.SpellReady[${CharmSpell}]}) {
        /echo \ay[GZM-Charm] Charming ${Target.CleanName}
        /cast "${CharmSpell}"
        /delay 5s ${Me.Casting.ID}
        /delay 15s !${Me.Casting.ID}

        | Check success
        /delay 10
        /if (${Me.Pet.ID}) {
            /varset CharmTarget ${TargetID}
            /varset CharmTargetName ${Spawn[${TargetID}].CleanName}
            /varset CharmTimer ${CharmDuration}
            /varset CharmAttempts 0
            /echo \ag[GZM-Charm] Charm successful - ${CharmTargetName}

            | Configure charmed pet
            /pet hold
            /delay 2
            /pet notaunt
            /delay 2

            | Buff the charm pet
            /call BuffCharmPet
        } else {
            /echo \ar[GZM-Charm] Charm FAILED on ${Target.CleanName}
            /varcalc CharmAttempts ${CharmAttempts}+1
            /if (${CharmAttempts} > 3) {
                /varset CharmBreakTimer 300
            }
        }
    } else {
        GZMDEBUG_PET Charm spell not ready!
    }
/return

| ============================================================================
| SUB: Buff Charm Pet
| ============================================================================
Sub BuffCharmPet
    /if (!${Me.Pet.ID}) /return
    /if (!${CharmBuffCount}) /return
    /if (${Me.Casting.ID}) /return

    /declare i int local
    /declare buff string local

    /for i 1 to ${CharmBuffCount}
        /varset buff ${CharmBuff[${i}]}
        /if (!${buff.Length}) /continue

        | Check if buff on pet
        /if (${Me.Pet.Buff[${buff}].ID}) /continue
        /if (${Me.Pet.Buff[${Spell[${buff}].Trigger}].ID}) /continue

        | Cast buff on pet
        /if (${Me.SpellReady[${buff}]}) {
            GZMDEBUG_PET Buffing charm pet with ${buff}
            /target id ${Me.Pet.ID}
            /delay 3 ${Target.ID} == ${Me.Pet.ID}
            /cast "${buff}"
            /delay 3s ${Me.Casting.ID}
            /delay 10s !${Me.Casting.ID}
            /return
        }
    /next i
/return

| ============================================================================
| SUB: Release Charm Pet
| ============================================================================
Sub ReleaseCharmPet
    /if (!${Me.Pet.ID}) /return
    /if (!${CharmTarget}) /return

    /echo \ay[GZM-Charm] Releasing charm pet ${CharmTargetName}

    | Mez it first if possible
    /if (${CharmMezSpell.Length} && ${Me.SpellReady[${CharmMezSpell}]}) {
        /target id ${CharmTarget}
        /delay 3
    }

    | Kill pet command releases charm
    /pet kill

    | Immediately mez the released mob
    /if (${CharmMezSpell.Length} && ${Me.SpellReady[${CharmMezSpell}]}) {
        /if (${Target.ID} == ${CharmTarget}) {
            /cast "${CharmMezSpell}"
            /delay 3s ${Me.Casting.ID}
            /delay 8s !${Me.Casting.ID}
        }
    }

    /varset CharmTarget 0
    /varset CharmTargetName
    /varset CharmTimer 0
/return

| ============================================================================
| EVENT: Charm Broke
| ============================================================================
Sub Event_CharmBroke
    GZMDEBUG_PET EVENT: Charm broke!
    | HandleCharmBreak will be called by CheckCharm when it sees no pet
/return

| ============================================================================
| EVENT: Charm Wore Off
| ============================================================================
Sub Event_CharmWoreOff(string Line, string SpellName)
    GZMDEBUG_PET EVENT: Charm wore off - ${SpellName}
/return

| ============================================================================
| BINDS
| ============================================================================

Sub Bind_CharmOn
    /varset CharmOn ${If[${CharmOn},0,1]}
    /echo \ay[GZM-Charm] Charm: ${If[${CharmOn},ENABLED,DISABLED]}
/return

Sub Bind_CharmTarget(string TargetName)
    /if (${TargetName.Length}) {
        /declare targetID int local ${Spawn[npc ${TargetName}].ID}
        /if (${targetID}) {
            /call DoCharm ${targetID}
        } else {
            /echo \arCould not find NPC: ${TargetName}
        }
    } else /if (${Target.ID} && ${Target.Type.Equal[NPC]}) {
        /call DoCharm ${Target.ID}
    } else {
        /echo \arUsage: /charmtarget [mobname] or target an NPC
    }
/return

Sub Bind_ReleaseCharm
    /call ReleaseCharmPet
/return

Sub Bind_DireCharm
    /if (${Target.ID} && ${Target.Type.Equal[NPC]}) {
        /call DoDireCharm ${Target.ID}
    } else {
        /echo \arTarget an NPC first
    }
/return

| ============================================================================
| END OF CHARM INCLUDE
| ============================================================================
