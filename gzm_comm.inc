|- GronnzMaster Communication System Include
|- Unified interface for DanNet and EQBC/NetBots
|- Supports automatic fallback between systems
|- Version 1.0

|- Communication Priority:
|- 1. DanNet (preferred - built-in, works local + LAN)
|- 2. EQBC/NetBots (fallback - requires server, best for multi-PC/internet)

Sub InitCommVars
   |- System availability flags
   /if (!${Defined[UseDanNet]}) /declare UseDanNet bool outer TRUE
   /if (!${Defined[UseEQBC]}) /declare UseEQBC bool outer FALSE
   /if (!${Defined[UseNetBots]}) /declare UseNetBots bool outer FALSE

   |- Primary system selection (dannet, eqbc, or auto)
   /if (!${Defined[CommPrimary]}) /declare CommPrimary string outer auto

   |- Connection status
   /if (!${Defined[DanNetConnected]}) /declare DanNetConnected bool outer FALSE
   /if (!${Defined[EQBCConnected]}) /declare EQBCConnected bool outer FALSE
   /if (!${Defined[NetBotsConnected]}) /declare NetBotsConnected bool outer FALSE

   |- Observer setup tracking
   /if (!${Defined[DanNetObserversSetup]}) /declare DanNetObserversSetup bool outer FALSE
   /if (!${Defined[NetBotsObserversSetup]}) /declare NetBotsObserversSetup bool outer FALSE

   |- Broadcast channel/group
   /if (!${Defined[CommChannel]}) /declare CommChannel string outer all

   |- Debug
   /if (!${Defined[CommDebug]}) /declare CommDebug bool outer FALSE
/return

|- Initialize communication systems
Sub InitComm
   /echo \ay[GZM_Comm] Initializing communication systems...

   |- Check DanNet
   /call CheckDanNet

   |- Check EQBC
   /call CheckEQBC

   |- Check NetBots
   /call CheckNetBots

   |- Determine primary system
   /call SelectPrimaryComm

   |- Setup observers for active system
   /call SetupObservers

   |- Report status
   /call ReportCommStatus
/return

|- Check DanNet availability and connection
Sub CheckDanNet
   /if (${Plugin[MQ2DanNet].Name.Length}) {
      /varset DanNetConnected TRUE
      /squelch /dnet commandecho off
      /if (${CommDebug}) /echo [GZM_Comm] DanNet plugin loaded
   } else {
      /varset DanNetConnected FALSE
      /varset UseDanNet FALSE
   }
/return

|- Check EQBC availability and connection
Sub CheckEQBC
   /if (${Plugin[MQ2EQBC].Name.Length}) {
      /if (${EQBC.Connected}) {
         /varset EQBCConnected TRUE
         /if (${CommDebug}) /echo [GZM_Comm] EQBC connected to server
      } else {
         /varset EQBCConnected FALSE
         /if (${CommDebug}) /echo [GZM_Comm] EQBC plugin loaded but not connected
      }
   } else {
      /varset EQBCConnected FALSE
      /varset UseEQBC FALSE
   }
/return

|- Check NetBots availability
Sub CheckNetBots
   /if (${Plugin[MQ2NetBots].Name.Length}) {
      /if (${NetBots.Client.Length}) {
         /varset NetBotsConnected TRUE
         /if (${CommDebug}) /echo [GZM_Comm] NetBots active with ${NetBots.Counts} clients
      } else {
         /varset NetBotsConnected FALSE
      }
   } else {
      /varset NetBotsConnected FALSE
      /varset UseNetBots FALSE
   }
/return

|- Select primary communication system
Sub SelectPrimaryComm
   /if (${CommPrimary.Equal[dannet]}) {
      |- User forced DanNet
      /if (${DanNetConnected}) {
         /varset UseDanNet TRUE
         /varset UseEQBC FALSE
      } else {
         /echo \ar[GZM_Comm] DanNet requested but not available, falling back
         /goto :AutoSelect
      }
   } else /if (${CommPrimary.Equal[eqbc]}) {
      |- User forced EQBC
      /if (${EQBCConnected}) {
         /varset UseEQBC TRUE
         /varset UseDanNet FALSE
      } else {
         /echo \ar[GZM_Comm] EQBC requested but not connected, falling back
         /goto :AutoSelect
      }
   } else {
      |- Auto-select best available
:AutoSelect
      /if (${DanNetConnected}) {
         /varset UseDanNet TRUE
         /varset UseEQBC ${EQBCConnected}
         /if (${CommDebug}) /echo [GZM_Comm] Auto-selected DanNet as primary
      } else /if (${EQBCConnected}) {
         /varset UseEQBC TRUE
         /varset UseDanNet FALSE
         /if (${CommDebug}) /echo [GZM_Comm] Auto-selected EQBC as primary (DanNet unavailable)
      } else {
         /varset UseDanNet FALSE
         /varset UseEQBC FALSE
         /echo \ar[GZM_Comm] WARNING: No communication system available!
      }
   }

   |- Enable NetBots if EQBC is active
   /if (${UseEQBC} && ${NetBotsConnected}) {
      /varset UseNetBots TRUE
   }
/return

|- Setup observers for active communication system
Sub SetupObservers
   /if (${UseDanNet} && !${DanNetObserversSetup}) {
      /call SetupDanNetObservers
   }

   /if (${UseNetBots} && !${NetBotsObserversSetup}) {
      /call SetupNetBotsObservers
   }
/return

|- Setup DanNet observers
Sub SetupDanNetObservers
   /if (!${DanNetConnected}) /return
   /if (${DanNetObserversSetup}) /return

   /squelch /dobserve ${Me.CleanName} -q Target.ID
   /squelch /dobserve ${Me.CleanName} -q Target.CleanName
   /squelch /dobserve ${Me.CleanName} -q Me.PctHPs
   /squelch /dobserve ${Me.CleanName} -q Me.PctMana
   /squelch /dobserve ${Me.CleanName} -q Me.PctEndurance
   /squelch /dobserve ${Me.CleanName} -q Me.Combat
   /squelch /dobserve ${Me.CleanName} -q Me.Casting.ID
   /squelch /dobserve ${Me.CleanName} -q Me.State
   /squelch /dobserve ${Me.CleanName} -q Me.XTarget[1].ID

   /varset DanNetObserversSetup TRUE
   /if (${CommDebug}) /echo [GZM_Comm] DanNet observers configured
/return

|- Setup NetBots observers (via EQBC)
Sub SetupNetBotsObservers
   /if (!${NetBotsConnected}) /return
   /if (${NetBotsObserversSetup}) /return

   |- NetBots auto-shares most data, but we can request specific updates
   /squelch /netbots send

   /varset NetBotsObserversSetup TRUE
   /if (${CommDebug}) /echo [GZM_Comm] NetBots observers configured
/return

|- Report communication status
Sub ReportCommStatus
   /echo \ay[GZM_Comm] Communication Status:
   /if (${UseDanNet}) {
      /echo \ag  DanNet: ACTIVE (Primary) - Peers: ${DanNet.PeerCount}
   } else /if (${DanNetConnected}) {
      /echo \aw  DanNet: Available (Standby)
   } else {
      /echo \ar  DanNet: Not Available
   }

   /if (${UseEQBC}) {
      /echo \ag  EQBC: ACTIVE ${If[${UseDanNet},(Secondary),(Primary)]} - Server: ${EQBC.Server}
   } else /if (${EQBCConnected}) {
      /echo \aw  EQBC: Connected (Standby)
   } else /if (${Plugin[MQ2EQBC].Name.Length}) {
      /echo \ay  EQBC: Plugin loaded, not connected
   } else {
      /echo \aw  EQBC: Not loaded
   }

   /if (${UseNetBots}) {
      /echo \ag  NetBots: ACTIVE - Clients: ${NetBots.Counts}
   } else /if (${NetBotsConnected}) {
      /echo \aw  NetBots: Available (Standby)
   } else {
      /echo \aw  NetBots: Not Available
   }
/return

|- ============================================================================
|- UNIFIED BROADCAST FUNCTIONS
|- These automatically use the best available system
|- ============================================================================

|- Broadcast to all characters (both systems if available)
Sub BCast(message)
   /if (${UseDanNet}) {
      /dgae /echo ${message}
   }
   /if (${UseEQBC}) {
      /bca ${message}
   }
   /if (!${UseDanNet} && !${UseEQBC}) {
      /echo ${message}
   }
/return

|- Broadcast to group only
Sub BCastGroup(message)
   /if (${UseDanNet}) {
      /dgge /echo ${message}
   }
   /if (${UseEQBC}) {
      /bcg ${message}
   }
   /if (!${UseDanNet} && !${UseEQBC}) {
      /gsay ${message}
   }
/return

|- Broadcast to specific character
Sub BCastTo(charName, message)
   /if (${UseDanNet}) {
      /dex ${charName} /echo ${message}
   }
   /if (${UseEQBC}) {
      /bct ${charName} ${message}
   }
/return

|- Execute command on all characters
Sub ExecAll(command)
   /if (${UseDanNet}) {
      /dgae ${command}
   }
   /if (${UseEQBC}) {
      /bcaa /${command}
   }
/return

|- Execute command on group only
Sub ExecGroup(command)
   /if (${UseDanNet}) {
      /dgge ${command}
   }
   /if (${UseEQBC}) {
      /bcga /${command}
   }
/return

|- Execute command on specific character
Sub ExecOn(charName, command)
   /if (${UseDanNet}) {
      /dex ${charName} ${command}
   }
   /if (${UseEQBC}) {
      /bct ${charName} /${command}
   }
/return

|- ============================================================================
|- DATA QUERY FUNCTIONS
|- Query data from other characters using best available system
|- ============================================================================

|- Get HP% of a character
Sub GetCharHP(charName)
   /if (${UseDanNet}) {
      /if (${DanNet[${charName}].O[Me.PctHPs].Received}) {
         /return ${DanNet[${charName}].O[Me.PctHPs]}
      }
   }
   /if (${UseNetBots}) {
      /if (${NetBots[${charName}].ID}) {
         /return ${NetBots[${charName}].PctHPs}
      }
   }
   |- Fallback to spawn data
   /if (${Spawn[pc ${charName}].ID}) {
      /return ${Spawn[pc ${charName}].PctHPs}
   }
/return 0

|- Get Mana% of a character
Sub GetCharMana(charName)
   /if (${UseDanNet}) {
      /if (${DanNet[${charName}].O[Me.PctMana].Received}) {
         /return ${DanNet[${charName}].O[Me.PctMana]}
      }
   }
   /if (${UseNetBots}) {
      /if (${NetBots[${charName}].ID}) {
         /return ${NetBots[${charName}].PctMana}
      }
   }
/return 0

|- Get Target ID of a character
Sub GetCharTarget(charName)
   /if (${UseDanNet}) {
      /if (${DanNet[${charName}].O[Target.ID].Received}) {
         /return ${DanNet[${charName}].O[Target.ID]}
      }
   }
   /if (${UseNetBots}) {
      /if (${NetBots[${charName}].ID}) {
         /return ${NetBots[${charName}].TargetID}
      }
   }
/return 0

|- Check if character is in combat
Sub IsCharInCombat(charName)
   /if (${UseDanNet}) {
      /if (${DanNet[${charName}].O[Me.Combat].Received}) {
         /return ${DanNet[${charName}].O[Me.Combat]}
      }
   }
   /if (${UseNetBots}) {
      /if (${NetBots[${charName}].ID}) {
         /return ${NetBots[${charName}].Attacking}
      }
   }
/return FALSE

|- Get list of connected peers/clients
Sub GetPeerList
   /declare peerList string local

   /if (${UseDanNet}) {
      /declare i int local
      /for i 1 to ${DanNet.PeerCount}
         /if (${peerList.Length}) {
            /varset peerList ${peerList},${DanNet.Peers.Arg[${i}, ]}
         } else {
            /varset peerList ${DanNet.Peers.Arg[${i}, ]}
         }
      /next i
   } else /if (${UseNetBots}) {
      /varset peerList ${NetBots.Client}
   }

/return ${peerList}

|- Get count of connected peers
Sub GetPeerCount
   /if (${UseDanNet}) {
      /return ${DanNet.PeerCount}
   }
   /if (${UseNetBots}) {
      /return ${NetBots.Counts}
   }
/return 0

|- ============================================================================
|- SPECIAL BROADCAST FUNCTIONS (for specific events)
|- ============================================================================

|- Broadcast death notification
Sub BCastDeath
   /declare msg string local Death: ${Me.CleanName} has died in ${Zone.ShortName}!
   /if (${UseDanNet}) {
      /dgt ${msg}
   }
   /if (${UseEQBC}) {
      /bc ${msg}
   }
/return

|- Broadcast zone notification
Sub BCastZone
   /declare msg string local Zone: ${Me.CleanName} arrived in ${Zone.ShortName}
   /if (${UseDanNet}) {
      /dgt ${msg}
   }
   /if (${UseEQBC}) {
      /bc ${msg}
   }
/return

|- Broadcast assist call
Sub BCastAssist(targetID)
   /if (${UseDanNet}) {
      /dgae /assist ${Spawn[${targetID}].CleanName}
   }
   /if (${UseEQBC}) {
      /bcaa //assist ${Spawn[${targetID}].CleanName}
   }
/return

|- Broadcast camp command
Sub BCastCamp
   /if (${UseDanNet}) {
      /dgge /makecamp on
   }
   /if (${UseEQBC}) {
      /bcga //makecamp on
   }
/return

|- ============================================================================
|- CONNECTION MANAGEMENT
|- ============================================================================

|- Attempt to connect EQBC to server
Sub ConnectEQBC(server, port)
   /if (!${Plugin[MQ2EQBC].Name.Length}) {
      /echo [GZM_Comm] EQBC plugin not loaded
      /return FALSE
   }

   /if (!${server.Length}) /varset server 127.0.0.1
   /if (!${port}) /varset port 2112

   /echo [GZM_Comm] Connecting to EQBC server ${server}:${port}...
   /bccmd connect ${server} ${port}
   /delay 5s ${EQBC.Connected}

   /if (${EQBC.Connected}) {
      /varset EQBCConnected TRUE
      /call SelectPrimaryComm
      /echo \ag[GZM_Comm] EQBC connected successfully
      /return TRUE
   } else {
      /echo \ar[GZM_Comm] EQBC connection failed
      /return FALSE
   }
/return

|- Disconnect from EQBC
Sub DisconnectEQBC
   /if (${EQBC.Connected}) {
      /bccmd quit
      /varset EQBCConnected FALSE
      /varset UseEQBC FALSE
      /call SelectPrimaryComm
      /echo [GZM_Comm] EQBC disconnected
   }
/return

|- Refresh communication status
Sub RefreshComm
   /call CheckDanNet
   /call CheckEQBC
   /call CheckNetBots
   /call SelectPrimaryComm
   /call ReportCommStatus
/return

|- Switch primary communication system
Sub SwitchPrimary(system)
   /varset CommPrimary ${system}
   /call SelectPrimaryComm
   /call ReportCommStatus
/return
