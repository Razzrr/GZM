|**
    GZM_chchain.inc - GronnzMaster Complete Heal Chain System

    Features:
    - Multi-cleric CH rotation (up to 6 clerics)
    - DanNet synchronized timing
    - Auto slot detection
    - Tank health monitoring
    - Chain restart on death
    - Fallback handling for missed turns
    - Duck heals when target is high HP
**|

| ============================================================================
| CH CHAIN CONSTANTS
| ============================================================================
#DEFINE MAXCHAINMEMBERS 6

| ============================================================================
| SUB: Initialize CH Chain System
| ============================================================================
Sub InitCHChain
    /declare i int local

    | CH Chain members
    /declare CHMember[${MAXCHAINMEMBERS}] string outer
    /declare CHMemberCount int outer 0

    | CH Chain state
    /declare CHRotOn int outer 0
    /declare CHMySlot int outer 0
    /declare CHCurrentSlot int outer 0
    /declare CHTarget int outer 0
    /declare CHTargetName string outer
    /declare CHSpell string outer "Complete Heal"
    /declare CHDelay int outer 90
    /declare CHStartPct int outer 80
    /declare CHCastTime int outer 100
    /declare CHMinMana int outer 400

    | CH Timing
    /declare CHOneBefore string outer
    /declare CHTwoBefore string outer
    /declare CHNextCleric string outer
    /declare CHTimer timer outer 0
    /declare CHCasting int outer 0
    /declare CHWaitingForGo int outer 0
    /declare CHFallbackTimer timer outer 0
    /declare CHDuckHeals int outer 0
    /declare CHDuckPct int outer 98

    | Load settings from INI
    /varset CHSpell ${Ini[${IniFileName},CHRotation,CHSpell,Complete Heal]}
    /varset CHDelay ${Ini[${IniFileName},CHRotation,CHDelay,90]}
    /varset CHStartPct ${Ini[${IniFileName},CHRotation,CHStartPct,80]}
    /varset CHMinMana ${Ini[${IniFileName},CHRotation,CHMinMana,400]}
    /varset CHDuckHeals ${Ini[${IniFileName},CHRotation,CHDuckHeals,0]}
    /varset CHDuckPct ${Ini[${IniFileName},CHRotation,CHDuckPct,98]}

    /echo \ag[GZM-CH] CH Chain system initialized
/return

| ============================================================================
| SUB: Start CH Rotation
| ============================================================================
Sub StartCHRotation(string Member1, string Member2, string Member3, string Member4, string Member5, string Member6)
    /declare i int local

    | Clear existing chain
    /for i 1 to ${MAXCHAINMEMBERS}
        /varset CHMember[${i}]
    /next i
    /varset CHMemberCount 0

    | Add members to chain
    /if (${Member1.Length} && ${Member1.NotEqual[NULL]}) {
        /varcalc CHMemberCount ${CHMemberCount}+1
        /varset CHMember[${CHMemberCount}] ${Member1}
    }
    /if (${Member2.Length} && ${Member2.NotEqual[NULL]}) {
        /varcalc CHMemberCount ${CHMemberCount}+1
        /varset CHMember[${CHMemberCount}] ${Member2}
    }
    /if (${Member3.Length} && ${Member3.NotEqual[NULL]}) {
        /varcalc CHMemberCount ${CHMemberCount}+1
        /varset CHMember[${CHMemberCount}] ${Member3}
    }
    /if (${Member4.Length} && ${Member4.NotEqual[NULL]}) {
        /varcalc CHMemberCount ${CHMemberCount}+1
        /varset CHMember[${CHMemberCount}] ${Member4}
    }
    /if (${Member5.Length} && ${Member5.NotEqual[NULL]}) {
        /varcalc CHMemberCount ${CHMemberCount}+1
        /varset CHMember[${CHMemberCount}] ${Member5}
    }
    /if (${Member6.Length} && ${Member6.NotEqual[NULL]}) {
        /varcalc CHMemberCount ${CHMemberCount}+1
        /varset CHMember[${CHMemberCount}] ${Member6}
    }

    | Find my slot
    /for i 1 to ${CHMemberCount}
        /if (${CHMember[${i}].Equal[${Me.CleanName}]}) {
            /varset CHMySlot ${i}
        }
    /next i

    | Calculate who is before me
    /if (${CHMySlot} > 1) {
        /varset CHOneBefore ${CHMember[${Math.Calc[${CHMySlot}-1]}]}
    } else {
        /varset CHOneBefore ${CHMember[${CHMemberCount}]}
    }

    /if (${CHMySlot} > 2) {
        /varset CHTwoBefore ${CHMember[${Math.Calc[${CHMySlot}-2]}]}
    } else /if (${CHMySlot} == 2) {
        /varset CHTwoBefore ${CHMember[${CHMemberCount}]}
    } else {
        /varset CHTwoBefore ${CHMember[${Math.Calc[${CHMemberCount}-1]}]}
    }

    | Calculate who is after me (next cleric)
    /if (${CHMySlot} < ${CHMemberCount}) {
        /varset CHNextCleric ${CHMember[${Math.Calc[${CHMySlot}+1]}]}
    } else {
        /varset CHNextCleric ${CHMember[1]}
    }

    | Set target
    /varset CHTarget ${Spawn[=${MainAssist}].ID}
    /varset CHTargetName ${MainAssist}

    | Activate
    /varset CHRotOn 1
    /varset CHCurrentSlot 1
    /varset CHWaitingForGo 0
    /varset CHCasting 0

    /echo \ag[GZM-CH] CH Chain STARTED
    /echo \ag  Members: ${CHMemberCount}
    /echo \ag  My Slot: ${CHMySlot}
    /echo \ag  One Before: ${CHOneBefore}
    /echo \ag  Two Before: ${CHTwoBefore}
    /echo \ag  Next Cleric: ${CHNextCleric}
    /echo \ag  Target: ${CHTargetName}
    /echo \ag  Spell: ${CHSpell}
    /echo \ag  Delay: ${CHDelay}

    | Announce
    /if (${UseDanNet}) {
        /dgtell all CH Chain Active - Target: ${CHTargetName}
    }

    | If I'm slot 1, start the chain automatically
    /if (${CHMySlot} == 1) {
        /echo \ag[GZM-CH] I am slot 1 - initiating chain in 3 seconds
        /delay 30
        /varset CHWaitingForGo 1
    }
/return

| ============================================================================
| SUB: Stop CH Rotation
| ============================================================================
Sub StopCHRotation
    /varset CHRotOn 0
    /varset CHMySlot 0
    /varset CHCurrentSlot 0
    /varset CHCasting 0

    /echo \ay[GZM-CH] CH Chain STOPPED

    /if (${UseDanNet}) {
        /dgtell all CH Chain Ended
    }
/return

| ============================================================================
| SUB: Check CH Chain (called from main loop)
| ============================================================================
Sub CheckCHChain
    /if (!${CHRotOn}) /return
    /if (!${CHMySlot}) /return
    /if (!${CHTarget}) /return
    /if (${CHCasting}) /return

    | Verify target is alive
    /if (!${Spawn[${CHTarget}].ID} || ${Spawn[${CHTarget}].Dead}) {
        /echo \ar[GZM-CH] CH Target dead or missing!
        /return
    }

    | Check if we're waiting for our turn (GO signal received)
    /if (${CHWaitingForGo}) {
        /call DoCHCast
        /return
    }

    | Check fallback timer - if previous cleric missed their turn
    /if (${CHFallbackTimer} && !${CHFallbackTimer}) {
        | Timer expired - previous cleric didn't cast, take over
        /echo \ay[GZM-CH] Fallback triggered - previous cleric missed turn
        /varset CHWaitingForGo 1
        /call DoCHCast
    }
/return

| ============================================================================
| SUB: Check if I can cast CH
| Returns: 1 if ready, 0 if not
| ============================================================================
Sub CHCanCast
    /declare result int local 0

    | Check if I'm alive
    /if (${Me.Hovering} || ${Me.Dead}) {
        GZMDEBUG_HEAL CH: Cannot cast - I am dead
        /return 0
    }

    | Check mana
    /if (${Me.CurrentMana} < ${CHMinMana}) {
        GZMDEBUG_HEAL CH: Cannot cast - not enough mana (${Me.CurrentMana}/${CHMinMana})
        /return 0
    }

    | Check range to target
    /if (${Spawn[${CHTarget}].Distance} > ${Spell[${CHSpell}].MyRange}) {
        GZMDEBUG_HEAL CH: Cannot cast - target too far (${Spawn[${CHTarget}].Distance} > ${Spell[${CHSpell}].MyRange})
        /return 0
    }

    | Check if spell is ready
    /if (!${Me.SpellReady[${CHSpell}]}) {
        GZMDEBUG_HEAL CH: Cannot cast - spell not ready
        /return 0
    }

    | Check if already casting
    /if (${Me.Casting.ID}) {
        GZMDEBUG_HEAL CH: Cannot cast - already casting
        /return 0
    }

    /return 1
/return

| ============================================================================
| EVENT: CH One Before Me Started Casting
| ============================================================================
Sub Event_CHOneBefore
    /if (!${CHRotOn}) /return
    /if (!${CHMySlot}) /return

    GZMDEBUG_HEAL CH: ${CHOneBefore} started casting - GET READY

    | Set fallback timer in case they don't finish and tell us to go
    | Fallback after 15 seconds (150 tenths)
    /varset CHFallbackTimer 150
/return

| ============================================================================
| EVENT: CH Two Before Me Started Casting
| ============================================================================
Sub Event_CHTwoBefore
    /if (!${CHRotOn}) /return
    /if (!${CHMySlot}) /return
    /if (${CHCasting}) /return

    GZMDEBUG_HEAL CH: ${CHTwoBefore} started casting - preparing for my turn

    | Two before me started, one before me should start soon
    | This is just awareness - we wait for GO signal or fallback
/return

| ============================================================================
| EVENT: My GO signal received
| ============================================================================
Sub Event_CHGo
    /if (!${CHRotOn}) /return
    /if (!${CHMySlot}) /return
    /if (${CHCasting}) /return

    GZMDEBUG_HEAL CH: GO signal received - starting cast

    | Clear fallback timer
    /varset CHFallbackTimer 0

    | Set flag and cast
    /varset CHWaitingForGo 1
/return

| ============================================================================
| SUB: Do CH Cast
| ============================================================================
Sub DoCHCast
    /if (${CHCasting}) /return
    /if (!${Spawn[${CHTarget}].ID}) /return

    | Clear the waiting flag
    /varset CHWaitingForGo 0
    /varset CHCasting 1

    | Check if we can cast
    /call CHCanCast
    /if (!${Macro.Return}) {
        | Cannot cast - pass to next cleric
        /echo \ay[GZM-CH] Cannot cast - passing to ${CHNextCleric}
        /if (${UseDanNet}) {
            /dgtell all ${CHNextCleric} GO -- ${Me.CleanName} skipped!
        } else {
            /rs ${CHNextCleric} GO -- ${Me.CleanName} skipped!
        }
        /varset CHCasting 0
        /return
    }

    | Target CH target
    /target id ${CHTarget}
    /delay 5 ${Target.ID} == ${CHTarget}

    | Stand if sitting
    /if (${Me.Sitting}) /stand

    | Announce casting with triple-name pattern for chain tracking
    /if (${UseDanNet}) {
        /dgtell all |${Me.CleanName}${Me.CleanName}${Me.CleanName}| CH ON ${CHTargetName}
    } else {
        /rs ${CHMySlot}${CHMySlot}${CHMySlot} CH ON ${CHTargetName}
    }

    | Cast CH
    GZMDEBUG_HEAL CH: Casting ${CHSpell} on ${Spawn[${CHTarget}].CleanName}
    /casting "${CHSpell}" -targetid|${CHTarget}

    | Wait for cast to start
    /delay 5s ${Me.Casting.ID}

    | Wait the configured delay before telling next cleric to go
    /delay ${CHDelay}

    | Tell next cleric to GO
    /if (${UseDanNet}) {
        /dgtell all ${CHNextCleric} GO
    } else {
        /rs ${CHNextCleric} GO
    }
    GZMDEBUG_HEAL CH: Told ${CHNextCleric} to GO

    | Calculate remaining cast time and wait
    /declare remainDelay int local
    /varcalc remainDelay ${CHCastTime} - ${CHDelay}
    /if (${remainDelay} > 0) {
        /delay ${remainDelay}
    }

    | Wait for cast to finish
    /delay 5s !${Me.Casting.ID}

    | Check if we should duck the heal (target at high HP)
    /if (${CHDuckHeals} && ${Spawn[${CHTarget}].PctHPs} >= ${CHDuckPct}) {
        /stopcast
        /if (${UseDanNet}) {
            /dgtell all DUCKING CH - ${CHTargetName} at ${Spawn[${CHTarget}].PctHPs}%
        }
        /echo \ay[GZM-CH] Ducked CH - target at ${Spawn[${CHTarget}].PctHPs}%
    }

    /varset CHCasting 0

    GZMDEBUG_HEAL CH: Cast complete
/return

| ============================================================================
| SUB: Manual CH (for when someone dies)
| ============================================================================
Sub ManualCH
    /if (!${CHRotOn}) /return
    /if (!${Spawn[${CHTarget}].ID}) /return

    GZMDEBUG_HEAL CH: Manual cast triggered

    | Target
    /target id ${CHTarget}
    /delay 5 ${Target.ID} == ${CHTarget}

    | Cast
    /if (${Me.Sitting}) /stand
    /cast "${CHSpell}"
    /delay 5s ${Me.Casting.ID}
    /delay 15s !${Me.Casting.ID}
/return

| ============================================================================
| BIND: Start CH Chain
| ============================================================================
Sub Bind_StartCH(string Params)
    | Format: /startch member1 member2 member3 ...
    /declare p1 string local ${Params.Arg[1]}
    /declare p2 string local ${Params.Arg[2]}
    /declare p3 string local ${Params.Arg[3]}
    /declare p4 string local ${Params.Arg[4]}
    /declare p5 string local ${Params.Arg[5]}
    /declare p6 string local ${Params.Arg[6]}

    /if (!${p1.Length}) {
        /echo \arUsage: /startch cleric1 cleric2 cleric3 [cleric4] [cleric5] [cleric6]
        /return
    }

    /call StartCHRotation "${p1}" "${p2}" "${p3}" "${p4}" "${p5}" "${p6}"
/return

| ============================================================================
| BIND: Stop CH Chain
| ============================================================================
Sub Bind_StopCH
    /call StopCHRotation
/return

| ============================================================================
| BIND: Set CH Target
| ============================================================================
Sub Bind_CHTarget(string TargetName)
    /if (${TargetName.Length}) {
        /varset CHTarget ${Spawn[=${TargetName}].ID}
        /varset CHTargetName ${TargetName}
    } else /if (${Target.ID}) {
        /varset CHTarget ${Target.ID}
        /varset CHTargetName ${Target.CleanName}
    }
    /echo \ag[GZM-CH] CH Target set to: ${CHTargetName}
/return

| ============================================================================
| EVENT: Start CH Rotation (broadcast)
| ============================================================================
Sub Event_StartCHRot(string Line, string P1, string P2, string P3, string P4, string P5, string P6)
    /echo \ag[GZM-CH] Received CH Chain start command
    /call StartCHRotation "${P1}" "${P2}" "${P3}" "${P4}" "${P5}" "${P6}"
/return

| ============================================================================
| EVENT: End CH Rotation (broadcast)
| ============================================================================
Sub Event_EndCHRot
    /call StopCHRotation
/return

| ============================================================================
| EVENT: CH Target Changed
| ============================================================================
Sub Event_CHTargetChange(string Line, string NewTarget)
    /if (!${CHRotOn}) /return

    | Update target if different
    /if (!${NewTarget.Equal[${CHTargetName}]}) {
        /varset CHTargetName ${NewTarget}
        /varset CHTarget ${Spawn[=${NewTarget}].ID}
        /echo \ag[GZM-CH] CH Target changed to: ${CHTargetName}
    }
/return

| ============================================================================
| EVENT: CH Target Died
| ============================================================================
Sub Event_CHTargetDead
    /if (!${CHRotOn}) /return

    /echo \ar[GZM-CH] CH Target ${CHTargetName} has died!

    | Stop current cast if casting
    /if (${Me.Casting.ID} && ${Me.Casting.Name.Equal[${CHSpell}]}) {
        /stopcast
        /echo \ay[GZM-CH] Stopped CH cast - target dead
    }

    | Announce
    /if (${UseDanNet}) {
        /dgtell all CH Target ${CHTargetName} is DEAD - chain paused
    }

    | Clear target - chain will wait for new target
    /varset CHTarget 0
    /varset CHCasting 0
    /varset CHWaitingForGo 0
/return

| ============================================================================
| BIND: Set CH Delay
| ============================================================================
Sub Bind_CHDelay(string Delay)
    /if (${Delay.Length}) {
        /varset CHDelay ${Delay}
        /echo \ag[GZM-CH] CH Delay set to: ${CHDelay}
    } else {
        /echo \ag[GZM-CH] Current CH Delay: ${CHDelay}
    }
/return

| ============================================================================
| BIND: Toggle Duck Heals
| ============================================================================
Sub Bind_CHDuck(string Toggle)
    /if (${Toggle.Equal[on]}) {
        /varset CHDuckHeals 1
        /echo \ag[GZM-CH] Duck Heals ENABLED at ${CHDuckPct}%
    } else /if (${Toggle.Equal[off]}) {
        /varset CHDuckHeals 0
        /echo \ag[GZM-CH] Duck Heals DISABLED
    } else {
        /varcalc CHDuckHeals 1 - ${CHDuckHeals}
        /if (${CHDuckHeals}) {
            /echo \ag[GZM-CH] Duck Heals ENABLED at ${CHDuckPct}%
        } else {
            /echo \ag[GZM-CH] Duck Heals DISABLED
        }
    }
/return

| ============================================================================
| BIND: Show CH Status
| ============================================================================
Sub Bind_CHStatus
    /echo \ag===== CH Chain Status =====
    /if (${CHRotOn}) {
        /echo \ag  Chain: ACTIVE
        /echo \ag  My Slot: ${CHMySlot} of ${CHMemberCount}
        /echo \ag  Target: ${CHTargetName} (ID: ${CHTarget})
        /echo \ag  Spell: ${CHSpell}
        /echo \ag  Delay: ${CHDelay}
        /echo \ag  Next Cleric: ${CHNextCleric}
        /echo \ag  Duck Heals: ${If[${CHDuckHeals},ON at ${CHDuckPct}%,OFF]}
        /echo \ag  Currently Casting: ${If[${CHCasting},YES,NO]}
        /echo \ag  Waiting for GO: ${If[${CHWaitingForGo},YES,NO]}
    } else {
        /echo \ay  Chain: INACTIVE
    }
    /echo \ag===========================
/return

| ============================================================================
| END OF CH CHAIN INCLUDE
| ============================================================================
