|- GronnzMaster Waypoint System Include
|- Record, save, and playback waypoint paths
|- Includes stuck detection and recovery
|- Version 1.0

Sub InitWaypointVars
   |- Waypoint settings
   /if (!${Defined[MaxWaypoints]}) /declare MaxWaypoints int outer 100
   /if (!${Defined[WaypointTolerance]}) /declare WaypointTolerance int outer 10
   /if (!${Defined[StuckCheckTime]}) /declare StuckCheckTime int outer 5
   /if (!${Defined[StuckMoveThreshold]}) /declare StuckMoveThreshold int outer 5

   |- Waypoint storage
   /if (!${Defined[WaypointY]}) /declare WaypointY[100] float outer
   /if (!${Defined[WaypointX]}) /declare WaypointX[100] float outer
   /if (!${Defined[WaypointZ]}) /declare WaypointZ[100] float outer
   /if (!${Defined[WaypointCount]}) /declare WaypointCount int outer 0

   |- Current path info
   /if (!${Defined[CurrentPathName]}) /declare CurrentPathName string outer
   /if (!${Defined[CurrentWaypoint]}) /declare CurrentWaypoint int outer 0
   /if (!${Defined[PathDirection]}) /declare PathDirection int outer 1
   /if (!${Defined[PathActive]}) /declare PathActive bool outer FALSE
   /if (!${Defined[PathLoop]}) /declare PathLoop bool outer FALSE

   |- Recording state
   /if (!${Defined[RecordingPath]}) /declare RecordingPath bool outer FALSE
   /if (!${Defined[RecordPathName]}) /declare RecordPathName string outer

   |- Stuck detection
   /if (!${Defined[LastY]}) /declare LastY float outer 0
   /if (!${Defined[LastX]}) /declare LastX float outer 0
   /if (!${Defined[StuckCount]}) /declare StuckCount int outer 0

   |- INI file
   /if (!${Defined[WaypointIniFile]}) /declare WaypointIniFile string outer ${MacroQuest.Path}\config\GZM_Waypoints_${Zone.ShortName}.ini

   |- Debug
   /if (!${Defined[WaypointDebug]}) /declare WaypointDebug bool outer FALSE
/return

|- Start recording a new path
Sub StartRecording(pathName)
   /if (${RecordingPath}) {
      /echo [GZM_Waypoint] Already recording path "${RecordPathName}"
      /return
   }

   |- Clear existing waypoints
   /call ClearWaypoints

   /varset RecordingPath TRUE
   /varset RecordPathName ${pathName}
   /varset WaypointCount 0

   /echo [GZM_Waypoint] Recording path "${pathName}" - duck to add waypoints, /gm stoprecord to finish
/return

|- Stop recording path
Sub StopRecording
   /if (!${RecordingPath}) {
      /echo [GZM_Waypoint] Not recording
      /return
   }

   /varset RecordingPath FALSE

   /if (${WaypointCount}>0) {
      /call SavePath "${RecordPathName}"
      /echo [GZM_Waypoint] Path "${RecordPathName}" saved with ${WaypointCount} waypoints
   } else {
      /echo [GZM_Waypoint] No waypoints recorded
   }

   /varset RecordPathName
/return

|- Add current location as waypoint (called when ducking during recording)
Sub RecordWaypoint
   /if (!${RecordingPath}) /return

   /if (${WaypointCount}>=${MaxWaypoints}) {
      /echo [GZM_Waypoint] Maximum waypoints reached (${MaxWaypoints})
      /return
   }

   /varcalc WaypointCount ${WaypointCount}+1
   /varset WaypointY[${WaypointCount}] ${Me.Y}
   /varset WaypointX[${WaypointCount}] ${Me.X}
   /varset WaypointZ[${WaypointCount}] ${Me.Z}

   /echo [GZM_Waypoint] Waypoint ${WaypointCount} recorded: ${Int[${Me.Y}]}, ${Int[${Me.X}]}, ${Int[${Me.Z}]}
/return

|- Clear all waypoints
Sub ClearWaypoints
   /declare i int local
   /for i 1 to ${MaxWaypoints}
      /varset WaypointY[${i}] 0
      /varset WaypointX[${i}] 0
      /varset WaypointZ[${i}] 0
   /next i
   /varset WaypointCount 0
   /varset CurrentWaypoint 0
   /varset PathActive FALSE
/return

|- Save path to INI
Sub SavePath(pathName)
   /if (!${WaypointCount}) {
      /echo [GZM_Waypoint] No waypoints to save
      /return
   }

   /if (${WaypointDebug}) /echo [GZM_Waypoint] Saving path "${pathName}" to ${WaypointIniFile}

   |- Save waypoint count
   /ini "${WaypointIniFile}" "${pathName}" "WaypointCount" "${WaypointCount}"

   |- Save each waypoint
   /declare i int local
   /for i 1 to ${WaypointCount}
      /ini "${WaypointIniFile}" "${pathName}" "WP${i}_Y" "${WaypointY[${i}]}"
      /ini "${WaypointIniFile}" "${pathName}" "WP${i}_X" "${WaypointX[${i}]}"
      /ini "${WaypointIniFile}" "${pathName}" "WP${i}_Z" "${WaypointZ[${i}]}"
   /next i

   /echo [GZM_Waypoint] Path "${pathName}" saved (${WaypointCount} waypoints)
/return

|- Load path from INI
Sub LoadPath(pathName)
   /if (${WaypointDebug}) /echo [GZM_Waypoint] Loading path "${pathName}"

   |- Clear existing
   /call ClearWaypoints

   |- Get waypoint count
   /varset WaypointCount ${Ini[${WaypointIniFile},${pathName},WaypointCount,0]}
   /if (!${WaypointCount}) {
      /echo [GZM_Waypoint] Path "${pathName}" not found or empty
      /return FALSE
   }

   |- Load each waypoint
   /declare i int local
   /for i 1 to ${WaypointCount}
      /varset WaypointY[${i}] ${Ini[${WaypointIniFile},${pathName},WP${i}_Y,0]}
      /varset WaypointX[${i}] ${Ini[${WaypointIniFile},${pathName},WP${i}_X,0]}
      /varset WaypointZ[${i}] ${Ini[${WaypointIniFile},${pathName},WP${i}_Z,0]}
   /next i

   /varset CurrentPathName ${pathName}
   /echo [GZM_Waypoint] Path "${pathName}" loaded (${WaypointCount} waypoints)
/return TRUE

|- Play a path (forward or reverse)
|- direction: 1 = forward, -1 = reverse
Sub PlayPath(pathName, direction, loop)
   |- Load path if not already loaded
   /if (!${CurrentPathName.Equal[${pathName}]} || !${WaypointCount}) {
      /call LoadPath "${pathName}"
      /if (${Macro.Return.NotEqual[TRUE]}) /return FALSE
   }

   /if (!${direction}) /varset direction 1
   /varset PathDirection ${direction}
   /varset PathLoop ${Bool[${loop}]}

   |- Set starting waypoint
   /if (${direction}==1) {
      /varset CurrentWaypoint 1
   } else {
      /varset CurrentWaypoint ${WaypointCount}
   }

   /varset PathActive TRUE
   /echo [GZM_Waypoint] Playing path "${pathName}" (${If[${direction}==1,forward,reverse]}${If[${PathLoop}, looping,]})

   |- Start moving to first waypoint
   /call MoveToWaypoint ${CurrentWaypoint}
/return TRUE

|- Stop path playback
Sub StopPath
   /varset PathActive FALSE
   /squelch /nav stop
   /echo [GZM_Waypoint] Path stopped
/return

|- Move to a specific waypoint
Sub MoveToWaypoint(wpNum)
   /if (${wpNum}<1 || ${wpNum}>${WaypointCount}) {
      /if (${WaypointDebug}) /echo [GZM_Waypoint] Invalid waypoint: ${wpNum}
      /return FALSE
   }

   /declare destY float local ${WaypointY[${wpNum}]}
   /declare destX float local ${WaypointX[${wpNum}]}
   /declare destZ float local ${WaypointZ[${wpNum}]}

   /if (${WaypointDebug}) /echo [GZM_Waypoint] Moving to WP${wpNum}: ${Int[${destY}]}, ${Int[${destX}]}

   |- Use MQ2Nav for navigation
   /nav loc ${destY} ${destX} ${destZ}
/return TRUE

|- Process waypoint movement (call in main loop)
Sub ProcessWaypoints
   /if (!${PathActive}) /return
   /if (${RecordingPath}) /return

   |- Check if we've reached current waypoint
   /declare distToWP float local ${Math.Distance[${Me.Y},${Me.X}:${WaypointY[${CurrentWaypoint}]},${WaypointX[${CurrentWaypoint}]}]}

   /if (${distToWP}<=${WaypointTolerance}) {
      /if (${WaypointDebug}) /echo [GZM_Waypoint] Reached waypoint ${CurrentWaypoint}

      |- Move to next waypoint
      /varcalc CurrentWaypoint ${CurrentWaypoint}+${PathDirection}

      |- Check if path complete
      /if (${PathDirection}==1 && ${CurrentWaypoint}>${WaypointCount}) {
         /if (${PathLoop}) {
            /varset CurrentWaypoint 1
            /echo [GZM_Waypoint] Path complete, looping...
         } else {
            /varset PathActive FALSE
            /echo [GZM_Waypoint] Path complete
            /return
         }
      } else /if (${PathDirection}==-1 && ${CurrentWaypoint}<1) {
         /if (${PathLoop}) {
            /varset CurrentWaypoint ${WaypointCount}
            /echo [GZM_Waypoint] Path complete, looping...
         } else {
            /varset PathActive FALSE
            /echo [GZM_Waypoint] Path complete
            /return
         }
      }

      /call MoveToWaypoint ${CurrentWaypoint}
   }

   |- Check for stuck
   /call CheckStuck
/return

|- Check if we're stuck
Sub CheckStuck
   /declare currentDist float local ${Math.Distance[${Me.Y},${Me.X}:${LastY},${LastX}]}

   /if (${currentDist}<${StuckMoveThreshold} && ${Navigation.Active}) {
      /varcalc StuckCount ${StuckCount}+1
      /if (${StuckCount}>${StuckCheckTime}) {
         /echo [GZM_Waypoint] Stuck detected, attempting to recover
         /call HandleStuck
      }
   } else {
      /varset StuckCount 0
   }

   /varset LastY ${Me.Y}
   /varset LastX ${Me.X}
/return

|- Handle being stuck
Sub HandleStuck
   /varset StuckCount 0

   |- Stop current nav
   /squelch /nav stop
   /delay 5

   |- Try strafing
   /nomodkey /keypress strafe_right hold
   /delay 5
   /nomodkey /keypress strafe_right

   /delay 5

   |- Resume navigation
   /call MoveToWaypoint ${CurrentWaypoint}
/return

|- Duck event handler for recording
Sub Event_Ducked
   /if (${RecordingPath}) {
      /call RecordWaypoint
   }
/return

|- List saved paths for current zone
Sub ListPaths
   /echo [GZM_Waypoint] Paths for ${Zone.ShortName}:
   /declare sections string local ${Ini[${WaypointIniFile}]}

   |- Parse sections (path names)
   /if (${sections.Length} && ${sections.NotEqual[NULL]}) {
      /echo ${sections}
   } else {
      /echo No paths saved for this zone
   }
/return

|- Delete a saved path
Sub DeletePath(pathName)
   |- Can't easily delete INI sections, so we just clear the waypoint count
   /ini "${WaypointIniFile}" "${pathName}" "WaypointCount" "0"
   /echo [GZM_Waypoint] Path "${pathName}" deleted
/return

|- Get distance to next waypoint
Sub GetDistanceToWaypoint
   /if (!${PathActive} || !${CurrentWaypoint}) /return 0
/return ${Int[${Math.Distance[${Me.Y},${Me.X}:${WaypointY[${CurrentWaypoint}]},${WaypointX[${CurrentWaypoint}]}]}]}

|- Pause path playback
Sub PausePath
   /if (!${PathActive}) /return
   /squelch /nav stop
   /echo [GZM_Waypoint] Path paused at waypoint ${CurrentWaypoint}
/return

|- Resume path playback
Sub ResumePath
   /if (!${PathActive}) /return
   /call MoveToWaypoint ${CurrentWaypoint}
   /echo [GZM_Waypoint] Path resumed
/return

|- Reverse direction mid-path
Sub ReverseDirection
   /if (!${PathActive}) /return
   /varcalc PathDirection ${PathDirection}*-1
   /echo [GZM_Waypoint] Direction reversed (now ${If[${PathDirection}==1,forward,reverse]})
/return

|- Show current path status
Sub ShowPathStatus
   /echo [GZM_Waypoint] === Path Status ===
   /echo Recording: ${If[${RecordingPath},YES (${RecordPathName}),No]}
   /echo Active Path: ${If[${PathActive},${CurrentPathName},None]}
   /if (${PathActive}) {
      /echo Current Waypoint: ${CurrentWaypoint}/${WaypointCount}
      /echo Direction: ${If[${PathDirection}==1,Forward,Reverse]}
      /echo Loop: ${If[${PathLoop},Yes,No]}
      /call GetDistanceToWaypoint
      /echo Distance to WP: ${Macro.Return}
   }
   /echo Total Waypoints Loaded: ${WaypointCount}
   /echo ========================
/return
