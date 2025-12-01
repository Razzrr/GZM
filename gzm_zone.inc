|- GronnzMaster Zone Transition Handling Include
|- Complete zone handling with state reset and follow resumption
|- Version 1.0

|- Events for zone detection
#Event GMZoning "LOADING, PLEASE WAIT..."
#Event GMZoning "You have entered #1#."
#Event GMZoneIn "You have entered #1#."

Sub InitZoneVars
   |- Zone tracking
   /if (!${Defined[PreviousZoneID]}) /declare PreviousZoneID int outer 0
   /if (!${Defined[PreviousZoneShort]}) /declare PreviousZoneShort string outer
   /if (!${Defined[CurrentZoneID]}) /declare CurrentZoneID int outer ${Zone.ID}
   /if (!${Defined[CurrentZoneShort]}) /declare CurrentZoneShort string outer ${Zone.ShortName}
   /if (!${Defined[ZoneCount]}) /declare ZoneCount int outer 0
   /if (!${Defined[LastZoneTime]}) /declare LastZoneTime int outer 0

   |- Zone settle time (seconds to wait after zoning)
   /if (!${Defined[ZoneSettleTime]}) /declare ZoneSettleTime int outer 3

   |- Follow resumption settings
   /if (!${Defined[ResumeFollowAfterZone]}) /declare ResumeFollowAfterZone bool outer TRUE
   /if (!${Defined[FollowWaitTime]}) /declare FollowWaitTime int outer 5

   |- Auto-buff after zone
   /if (!${Defined[AutoBuffAfterZone]}) /declare AutoBuffAfterZone bool outer TRUE

   |- Zone-specific settings reload
   /if (!${Defined[ReloadSettingsOnZone]}) /declare ReloadSettingsOnZone bool outer TRUE

   |- Debug
   /if (!${Defined[ZoneDebug]}) /declare ZoneDebug bool outer FALSE
/return

|- Event handler for zoning start
Sub Event_GMZoning
   /if (${ZoneDebug}) /echo [GZM_Zone] Zoning event triggered
   /call HandleZoning
/return

|- Event handler for zone-in complete
Sub Event_GMZoneIn(Line, ZoneName)
   /if (${ZoneDebug}) /echo [GZM_Zone] Entered ${ZoneName}
   |- Zone-in is handled by HandleZoning completion
/return

|- Main zone handling routine
Sub HandleZoning
   /echo [GZM_Zone] Zone transition detected...

   |- Set state to ZONING
   /call SetState ZONING

   |- Stop all movement and combat
   /squelch /melee plugin=0
   /squelch /nav stop
   /squelch /stick off
   /squelch /moveto off

   |- Store previous zone info
   /varset PreviousZoneID ${CurrentZoneID}
   /varset PreviousZoneShort ${CurrentZoneShort}

   |- Wait for zone to complete
   /delay 1s
   /delay 10s ${Zone.ID}!=${PreviousZoneID} || ${Zone.ID}

   |- Additional settle time
   /delay ${ZoneSettleTime}s

   |- Update current zone tracking
   /varset CurrentZoneID ${Zone.ID}
   /varset CurrentZoneShort ${Zone.ShortName}
   /varcalc ZoneCount ${ZoneCount}+1
   /varset LastZoneTime ${Time.SecondsSinceMidnight}

   /echo [GZM_Zone] Arrived in ${Zone.Name} (${Zone.ShortName})

   |- Full state reset
   /call ZoneStateReset

   |- Reload zone-specific data
   /if (${ReloadSettingsOnZone}) {
      /call ReloadZoneData
   }

   |- Handle follow resumption
   /if (${ResumeFollowAfterZone}) {
      /call ResumeFollowAfterZone
   }

   |- Set state to IDLE
   /call SetState IDLE

   |- Re-enable core automation
   /call EnableCoreAutomation

   |- Broadcast zone complete
   /if (${Plugin[MQ2DanNet].IsLoaded}) {
      /dgt Zone: ${Me.CleanName} arrived in ${Zone.ShortName}
   }
/return

|- Full state reset for zone transitions
Sub ZoneStateReset
   /if (${ZoneDebug}) /echo [GZM_Zone] Performing full state reset

   |- Combat state reset
   /if (${Defined[GMCombatTarget]}) /varset GMCombatTarget 0
   /if (${Defined[AssistTarget]}) /varset AssistTarget 0
   /if (${Defined[CurrentTarget]}) /varset CurrentTarget 0
   /if (${Defined[InCombat]}) /varset InCombat FALSE
   /if (${Defined[CombatTimer]}) /varset CombatTimer 0

   |- Target clearing
   /squelch /target clear

   |- MQ2Melee reset
   /squelch /melee plugin=0

   |- Movement reset
   /squelch /nav stop
   /squelch /stick off
   /squelch /moveto off
   /if (${Defined[StuckCount]}) /varset StuckCount 0

   |- Mez tracking reset
   /if (${Defined[ClearMezList]}) /call ClearMezList

   |- Debuff tracking reset
   /if (${Defined[ClearDebuffTracking]}) /call ClearDebuffTracking

   |- Flush all pending events
   /doevents flush

   |- Reset all timers
   /if (${Defined[ResetAllTimers]}) /call ResetAllTimers

   |- Clear camp (camps don't persist across zones)
   /if (${Defined[GMCampActive]}) {
      /varset GMCampActive FALSE
      /varset GMCampLoc
      /varset GMCampY 0
      /varset GMCampX 0
      /varset GMCampZ 0
   }

   |- Pet state reset
   /if (${Me.Pet.ID}) {
      /pet back off
      /pet follow
   }

   /if (${ZoneDebug}) /echo [GZM_Zone] State reset complete
/return

|- Reload zone-specific data
Sub ReloadZoneData
   /if (${ZoneDebug}) /echo [GZM_Zone] Reloading zone data

   |- Load spell immunities for this zone
   /if (${Defined[LoadImmunes]}) /call LoadImmunes

   |- Load zone-specific settings if they exist
   /declare ZoneIniFile string local ${MacroQuest.Path}\config\GZM_${Zone.ShortName}.ini
   /if (${Ini[${ZoneIniFile}].Length}) {
      /if (${ZoneDebug}) /echo [GZM_Zone] Found zone-specific settings: ${ZoneIniFile}
      |- Load any zone-specific overrides here
   }

   /if (${ZoneDebug}) /echo [GZM_Zone] Zone data reload complete
/return

|- Resume follow after zoning
Sub ResumeFollowAfterZone
   |- Check if we had a follow target
   /if (!${Defined[GMFollowTarget]}) /return
   /if (!${GMFollowTarget.Length}) /return

   /echo [GZM_Zone] Attempting to resume follow on ${GMFollowTarget}

   |- Wait for follow target to appear in zone
   /delay ${FollowWaitTime}s ${Spawn[pc ${GMFollowTarget}].ID}

   /if (${Spawn[pc ${GMFollowTarget}].ID}) {
      /if (${Defined[DoFollow]}) /varset DoFollow TRUE
      /echo [GZM_Zone] Resuming follow on ${GMFollowTarget}
   } else {
      /echo [GZM_Zone] Follow target ${GMFollowTarget} not found in zone
      /varset GMFollowTarget
      /if (${Defined[DoFollow]}) /varset DoFollow FALSE
   }
/return

|- Enable core automation after zone
Sub EnableCoreAutomation
   |- Always re-enable heals and cures
   /if (${Defined[DoHeals]}) /varset DoHeals TRUE
   /if (${Defined[DoCures]}) /varset DoCures TRUE

   |- Re-enable buffs if setting allows
   /if (${AutoBuffAfterZone}) {
      /if (${Defined[DoBuffs]}) /varset DoBuffs TRUE
   }

   |- Note: Combat functions (DoPull, DoCombat, etc.) require camp/assist setup
   /if (${ZoneDebug}) /echo [GZM_Zone] Core automation enabled
/return

|- Manual zone check (can be called from main loop)
Sub CheckZoneChange
   /if (${Zone.ID}!=${CurrentZoneID}) {
      /call HandleZoning
      /return TRUE
   }
/return FALSE

|- Clear mez list (stub - override in GZM_mez.inc if it exists)
Sub ClearMezList
   /if (${Defined[MezList]}) {
      /declare i int local
      /for i 1 to 20
         /if (${Defined[MezList[${i}]]}) /varset MezList[${i}] 0
      /next i
   }
/return

|- Clear debuff tracking (stub - override in GZM_combat.inc if it exists)
Sub ClearDebuffTracking
   /if (${Defined[DebuffedMobs]}) {
      /declare i int local
      /for i 1 to 20
         /if (${Defined[DebuffedMobs[${i}]]}) /varset DebuffedMobs[${i}] 0
      /next i
   }
/return

|- Get zone statistics
Sub GetZoneStats
   /echo [GZM_Zone] === Zone Statistics ===
   /echo Current Zone: ${Zone.Name} (${Zone.ShortName})
   /echo Zone ID: ${Zone.ID}
   /echo Zones this session: ${ZoneCount}
   /if (${PreviousZoneShort.Length}) {
      /echo Previous Zone: ${PreviousZoneShort}
   }
   /echo ========================
/return

|- Zone to specific zone via door/portal
Sub ZoneVia(zoneName, doorName)
   /echo [GZM_Zone] Attempting to zone to ${zoneName} via ${doorName}

   |- Find and target the door/portal
   /if (${Spawn[${doorName}].ID}) {
      /target id ${Spawn[${doorName}].ID}
      /delay 1s ${Target.ID}
      /if (${Target.Distance}<15) {
         /click right
         /delay 5s ${Zone.ShortName.NotEqual[${CurrentZoneShort}]}
      } else {
         /echo [GZM_Zone] Too far from ${doorName} - move closer
      }
   } else {
      /echo [GZM_Zone] Cannot find ${doorName}
   }
/return

|- Safe zone (use gate/evac/etc)
Sub EmergencyZone
   /echo [GZM_Zone] Emergency zone requested

   |- Try gate first
   /if (${Me.Book[Gate].ID}) {
      /casting "Gate"
      /delay 10s ${Zone.ID}!=${CurrentZoneID}
      /return
   }

   |- Try evac spells
   /if (${Me.Book[Exodus].ID}) {
      /casting "Exodus"
      /delay 10s ${Zone.ID}!=${CurrentZoneID}
      /return
   }

   /echo [GZM_Zone] No escape spell available
/return

|- Load zone settings from INI
Sub LoadZoneSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /varset ZoneSettleTime ${Ini[${IniFile},Zone,ZoneSettleTime,${ZoneSettleTime}]}
   /varset ResumeFollowAfterZone ${Ini[${IniFile},Zone,ResumeFollowAfterZone,${ResumeFollowAfterZone}]}
   /varset FollowWaitTime ${Ini[${IniFile},Zone,FollowWaitTime,${FollowWaitTime}]}
   /varset AutoBuffAfterZone ${Ini[${IniFile},Zone,AutoBuffAfterZone,${AutoBuffAfterZone}]}
   /varset ReloadSettingsOnZone ${Ini[${IniFile},Zone,ReloadSettingsOnZone,${ReloadSettingsOnZone}]}

   /if (${ZoneDebug}) /echo [GZM_Zone] Settings loaded from ${IniFile}
/return

|- Save zone settings to INI
Sub SaveZoneSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /ini "${IniFile}" "Zone" "ZoneSettleTime" "${ZoneSettleTime}"
   /ini "${IniFile}" "Zone" "ResumeFollowAfterZone" "${ResumeFollowAfterZone}"
   /ini "${IniFile}" "Zone" "FollowWaitTime" "${FollowWaitTime}"
   /ini "${IniFile}" "Zone" "AutoBuffAfterZone" "${AutoBuffAfterZone}"
   /ini "${IniFile}" "Zone" "ReloadSettingsOnZone" "${ReloadSettingsOnZone}"

   /echo [GZM_Zone] Settings saved to ${IniFile}
/return
