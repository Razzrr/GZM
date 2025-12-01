|- GronnzMaster State Machine Include
|- Central state management for all automation
|- Version 1.0

|- Valid States: IDLE, COMBAT, FOLLOWING, CAMPING, ZONING, DEAD, REZZING, LOOTING, BUFFING, PULLING

Sub InitStateVars
   |- Core state variables
   /if (!${Defined[GMState]}) /declare GMState string outer IDLE
   /if (!${Defined[GMPrevState]}) /declare GMPrevState string outer IDLE
   /if (!${Defined[GMPrevState2]}) /declare GMPrevState2 string outer IDLE
   /if (!${Defined[GMStateTimer]}) /declare GMStateTimer timer outer
   /if (!${Defined[GMStateDebug]}) /declare GMStateDebug bool outer FALSE

   |- Combat tracking
   /if (!${Defined[GMCombatTarget]}) /declare GMCombatTarget int outer 0
   /if (!${Defined[GMCombatStartTime]}) /declare GMCombatStartTime int outer 0

   |- Position capture for return-to-location after combat
   /if (!${Defined[ACStartY]}) /declare ACStartY float outer 0
   /if (!${Defined[ACStartX]}) /declare ACStartX float outer 0
   /if (!${Defined[ACStartZ]}) /declare ACStartZ float outer 0

   |- Follow tracking
   /if (!${Defined[GMFollowTarget]}) /declare GMFollowTarget string outer
   /if (!${Defined[GMFollowDistance]}) /declare GMFollowDistance int outer 20

   |- Camp tracking
   /if (!${Defined[GMCampLoc]}) /declare GMCampLoc string outer
   /if (!${Defined[GMCampY]}) /declare GMCampY float outer 0
   /if (!${Defined[GMCampX]}) /declare GMCampX float outer 0
   /if (!${Defined[GMCampZ]}) /declare GMCampZ float outer 0
   /if (!${Defined[GMCampRadius]}) /declare GMCampRadius int outer 50
   /if (!${Defined[GMCampActive]}) /declare GMCampActive bool outer FALSE

   |- Zone tracking
   /if (!${Defined[GMLastZoneID]}) /declare GMLastZoneID int outer ${Zone.ID}
   /if (!${Defined[GMZoneStartTime]}) /declare GMZoneStartTime int outer 0

   |- Automation flags (master controls)
   /if (!${Defined[GMPause]}) /declare GMPause bool outer FALSE
   /if (!${Defined[GMAutomationEnabled]}) /declare GMAutomationEnabled bool outer TRUE

   |- Immune tracking array (50 mobs max per zone)
   /if (!${Defined[ImmuneList]}) /declare ImmuneList[50] string outer
/return

|- Set state with validation and history tracking
Sub SetState(newState)
   /declare validStates string local IDLE|COMBAT|FOLLOWING|CAMPING|ZONING|DEAD|REZZING|LOOTING|BUFFING|PULLING

   |- Validate new state
   /if (!${validStates.Find[${newState}]}) {
      /echo [GZM_State] ERROR: Invalid state "${newState}"
      /return
   }

   |- Skip if already in this state
   /if (${GMState.Equal[${newState}]}) /return

   |- Update state history (shift previous states)
   /varset GMPrevState2 ${GMPrevState}
   /varset GMPrevState ${GMState}

   |- Set new state
   /varset GMState ${newState}
   /varset GMStateTimer 0

   |- Debug output
   /if (${GMStateDebug}) {
      /echo [GZM_State] ${GMPrevState} -> ${GMState}
   }

   |- State transition actions
   /call OnStateEnter ${newState}
/return

|- Actions to perform when entering a new state
Sub OnStateEnter(state)
   /if (${state.Equal[COMBAT]}) {
      /varset GMCombatStartTime ${Time.SecondsSinceMidnight}
      |- Capture position before combat for return-to-camp
      /varset ACStartY ${Me.Y}
      /varset ACStartX ${Me.X}
      /varset ACStartZ ${Me.Z}
      |- Sync with legacy InCombat variable
      /if (${Defined[InCombat]}) /varset InCombat 1
      /squelch /melee plugin=1
   } else /if (${state.Equal[IDLE]}) {
      /varset GMCombatTarget 0
      |- Sync with legacy InCombat variable
      /if (${Defined[InCombat]}) /varset InCombat 0
      /squelch /melee plugin=0
   } else /if (${state.Equal[DEAD]}) {
      /squelch /melee plugin=0
      /squelch /nav stop
      /squelch /stick off
      |- Stop bard twist if applicable
      /if (${Defined[IAmABard]}) {
         /if (${IAmABard}) /squelch /twist off
      }
      |- Sync with legacy InCombat variable
      /if (${Defined[InCombat]}) /varset InCombat 0
   } else /if (${state.Equal[ZONING]}) {
      /squelch /melee plugin=0
      /squelch /nav stop
      /squelch /stick off
      |- Stop bard twist if applicable
      /if (${Defined[IAmABard]}) {
         /if (${IAmABard}) /squelch /twist off
      }
      |- Sync with legacy InCombat variable
      /if (${Defined[InCombat]}) /varset InCombat 0
   } else /if (${state.Equal[FOLLOWING]}) {
      /squelch /melee plugin=0
   } else /if (${state.Equal[CAMPING]}) {
      /varset GMCampActive TRUE
      /varset GMCampY ${Me.Y}
      /varset GMCampX ${Me.X}
      /varset GMCampZ ${Me.Z}
      /varset GMCampLoc ${Me.Y},${Me.X}
      |- Also set ACStart for return-to-camp
      /varset ACStartY ${Me.Y}
      /varset ACStartX ${Me.X}
      /varset ACStartZ ${Me.Z}
   } else /if (${state.Equal[REZZING]}) {
      /squelch /melee plugin=0
      /squelch /nav stop
      /squelch /stick off
   }
/return

|- Get current state
Sub GetState
/return ${GMState}

|- Check if in specific state
Sub IsState(checkState)
/return ${GMState.Equal[${checkState}]}

|- Check if in combat state
Sub InCombatState
/return ${GMState.Equal[COMBAT]}

|- Check if automation should run
Sub CanAutomate
   /if (${GMPause}) /return FALSE
   /if (!${GMAutomationEnabled}) /return FALSE
   /if (${GMState.Equal[DEAD]}) /return FALSE
   /if (${GMState.Equal[ZONING]}) /return FALSE
   /if (${GMState.Equal[REZZING]}) /return FALSE
/return TRUE

|- Pause all automation
Sub PauseAutomation
   /varset GMPause TRUE
   /echo [GZM_State] Automation PAUSED
   /squelch /melee plugin=0
   /squelch /nav stop
   /squelch /stick off
/return

|- Resume automation
Sub ResumeAutomation
   /varset GMPause FALSE
   /echo [GZM_State] Automation RESUMED
/return

|- Toggle pause
Sub TogglePause
   /if (${GMPause}) {
      /call ResumeAutomation
   } else {
      /call PauseAutomation
   }
/return

|- Make camp at current location
Sub MakeCamp(radius)
   /if (!${radius}) /varset radius 50
   /varset GMCampY ${Me.Y}
   /varset GMCampX ${Me.X}
   /varset GMCampZ ${Me.Z}
   /varset GMCampLoc ${Me.Y},${Me.X}
   /varset GMCampRadius ${radius}
   /varset GMCampActive TRUE
   /call SetState CAMPING
   /echo [GZM_State] Camp set at ${Me.Y}, ${Me.X} with radius ${radius}
/return

|- Clear camp
Sub ClearCamp
   /varset GMCampY 0
   /varset GMCampX 0
   /varset GMCampZ 0
   /varset GMCampLoc
   /varset GMCampActive FALSE
   /call SetState IDLE
   /echo [GZM_State] Camp cleared
/return

|- Return to camp location
Sub ReturnToCamp
   /if (!${GMCampActive}) /return
   /declare distToCamp float local ${Math.Distance[${Me.Y},${Me.X}:${GMCampY},${GMCampX}]}
   /if (${distToCamp}>${GMCampRadius}) {
      /if (${GMStateDebug}) /echo [GZM_State] Returning to camp (${Int[${distToCamp}]} away)
      /nav loc ${GMCampY} ${GMCampX} ${GMCampZ}
      /delay 3s !${Navigation.Active}
   }
/return

|- Check if within camp radius
Sub IsInCamp
   /if (!${GMCampActive}) /return TRUE
   /declare distToCamp float local ${Math.Distance[${Me.Y},${Me.X}:${GMCampY},${GMCampX}]}
/return ${Bool[${distToCamp}<=${GMCampRadius}]}

|- Set follow target
Sub SetFollowTarget(targetName, distance)
   /if (!${distance}) /varset distance 20
   /varset GMFollowTarget ${targetName}
   /varset GMFollowDistance ${distance}
   /call SetState FOLLOWING
   /echo [GZM_State] Following ${targetName} at distance ${distance}
/return

|- Clear follow target
Sub ClearFollowTarget
   /varset GMFollowTarget
   /squelch /nav stop
   /squelch /stick off
   /call SetState IDLE
   /echo [GZM_State] Follow cleared
/return

|- Check if zone changed
Sub CheckZoneChange
   /if (${Zone.ID}!=${GMLastZoneID}) {
      /varset GMLastZoneID ${Zone.ID}
      /return TRUE
   }
/return FALSE

|- Get state history as string
Sub GetStateHistory
/return ${GMPrevState2}->${GMPrevState}->${GMState}

|- Clear immune list
Sub ClearImmuneList
   /declare i int local
   /for i 1 to 50
      /varset ImmuneList[${i}]
   /next i
/return

|- Add mob to immune list
Sub AddImmune(mobName, immuneType)
   /declare i int local
   /for i 1 to 50
      /if (!${ImmuneList[${i}].Length}) {
         /varset ImmuneList[${i}] ${mobName}|${immuneType}
         /if (${GMStateDebug}) /echo [GZM_State] Added immune: ${mobName} - ${immuneType}
         /return
      }
      |- Update existing entry
      /if (${ImmuneList[${i}].Arg[1,|].Equal[${mobName}]}) {
         /if (!${ImmuneList[${i}].Find[${immuneType}]}) {
            /varset ImmuneList[${i}] ${ImmuneList[${i}]}|${immuneType}
         }
         /return
      }
   /next i
/return

|- Check if mob is immune to spell type
Sub IsImmune(mobName, immuneType)
   /declare i int local
   /for i 1 to 50
      /if (${ImmuneList[${i}].Arg[1,|].Equal[${mobName}]}) {
         /if (${ImmuneList[${i}].Find[${immuneType}]}) /return TRUE
      }
   /next i
/return FALSE

|- Save immunities to INI
Sub SaveImmunes
   /declare IniFile string local ${MacroQuest.Path}\config\GZM_Immunes_${Zone.ShortName}.ini
   /declare i int local
   /declare mobName string local
   /declare immuneTypes string local

   /for i 1 to 50
      /if (${ImmuneList[${i}].Length}) {
         /varset mobName ${ImmuneList[${i}].Arg[1,|]}
         /varset immuneTypes ${ImmuneList[${i}].Right[-${Math.Calc[${mobName.Length}+1]}]}
         /ini "${IniFile}" "Immunes" "Mob${i}" "${mobName}"
         /ini "${IniFile}" "${mobName}" "Types" "${immuneTypes}"
      }
   /next i
   /echo [GZM_State] Immunities saved for ${Zone.ShortName}
/return

|- Load immunities from INI
Sub LoadImmunes
   /declare IniFile string local ${MacroQuest.Path}\config\GZM_Immunes_${Zone.ShortName}.ini
   /declare i int local
   /declare mobName string local

   |- Clear existing list
   /call ClearImmuneList

   |- Load from INI if exists
   /for i 1 to 50
      /varset mobName ${Ini[${IniFile},Immunes,Mob${i}]}
      /if (${mobName.Length} && ${mobName.NotEqual[NULL]}) {
         /varset ImmuneList[${i}] ${mobName}|${Ini[${IniFile},${mobName},Types]}
      }
   /next i

   /if (${GMStateDebug}) /echo [GZM_State] Immunities loaded for ${Zone.ShortName}
/return
