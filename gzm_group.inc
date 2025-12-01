|- GronnzMaster Group Management Include
|- Save/restore group compositions, invite handling
|- Auto-accept invites and role management
|- Version 1.0

|- Group invite events
#Event GMInvited "#1# invites you to join a group."
#Event GMRaidInvited "#1# invites you to join a raid."

Sub InitGroupVars
   |- Group management
   /if (!${Defined[AutoAcceptInvite]}) /declare AutoAcceptInvite bool outer TRUE
   /if (!${Defined[AutoAcceptRaidInvite]}) /declare AutoAcceptRaidInvite bool outer TRUE

   |- Tank/Puller name settings (manual overrides group roles)
   /if (!${Defined[TankName]}) /declare TankName string outer
   /if (!${Defined[PullerName]}) /declare PullerName string outer
   /if (!${Defined[TankManual]}) /declare TankManual bool outer FALSE
   /if (!${Defined[PullerManual]}) /declare PullerManual bool outer FALSE

   |- Group formation settings
   /if (!${Defined[DefaultGroupName]}) /declare DefaultGroupName string outer Normal

   |- Debug
   /if (!${Defined[GroupDebug]}) /declare GroupDebug bool outer FALSE
/return

|- ============================================================================
|- EVENT HANDLERS
|- ============================================================================

|- Auto-accept group invites
Sub Event_GMInvited(line, inviter)
   /if (!${AutoAcceptInvite}) /return

   /echo [GZM_Group] Received group invite from ${inviter}
   /delay 5

   |- Accept the invite
   /if (${Window[ConfirmationDialogBox].Open}) {
      /notify ConfirmationDialogBox Yes_Button leftmouseup
      /delay 1s
   }

   /invite
   /delay 3s ${Group.Members}
/return

|- Auto-accept raid invites
Sub Event_GMRaidInvited(line, inviter)
   /if (!${AutoAcceptRaidInvite}) /return

   /echo [GZM_Group] Received raid invite from ${inviter}
   /delay 5

   |- Accept the invite
   /if (${Window[ConfirmationDialogBox].Open}) {
      /notify ConfirmationDialogBox Yes_Button leftmouseup
      /delay 1s
   }

   /raidaccept
   /delay 3s ${Raid.Members}
/return

|- ============================================================================
|- GROUP FORMATION COMMANDS
|- ============================================================================

|- Save current group composition to INI
|- Usage: /call SaveGroup "MyGroup"
Sub SaveGroup(groupName)
   |- Only save if we are group leader
   /if (!${Group.Leader.ID}) {
      /echo [GZM_Group] Cannot save - not in a group
      /return
   }
   /if (!${Group.Member[0].Leader}) {
      /echo [GZM_Group] Cannot save - not group leader
      /return
   }

   /if (!${groupName.Length}) {
      /varset groupName ${DefaultGroupName}
   }

   /declare gMembers string local ${Me.Name}
   /declare i int local

   |- Build member list
   /for i 1 to ${Group.Members}
      /varset gMembers ${gMembers} ${Group.Member[${i}].Name}
   /next i

   |- Save to INI
   /if (${Defined[IniFileName]}) {
      /ini "${IniFileName}" "Groups" "${groupName}" "${gMembers}"
      /echo [GZM_Group] Saved group "${groupName}": ${gMembers}
   } else {
      /echo [GZM_Group] No INI file defined - cannot save group
   }
/return

|- Reform saved group composition
|- Usage: /call GroupUp "MyGroup"
Sub GroupUp(groupName)
   /if (!${groupName.Length}) {
      /varset groupName ${DefaultGroupName}
   }

   /declare IniString string local
   /declare i int local

   |- Read group from INI
   /if (${Defined[IniFileName]}) {
      /varset IniString ${Ini[${IniFileName},Groups,${groupName},NOTFOUND]}
   }

   /if (!${IniString.Length} || ${IniString.Equal[NOTFOUND]}) {
      /echo [GZM_Group] No saved group "${groupName}" found
      /return
   }

   |- Check if we should be leader (first name in list)
   /if (${IniString.Arg[1].NotEqual[${Me.Name}]}) {
      /if (${GroupDebug}) /echo [GZM_Group] Not designated leader for "${groupName}"
      /return
   }

   /echo [GZM_Group] Forming group "${groupName}"

   |- Invite each member not already in group
   /for i 2 to ${Math.Calc[${IniString.Count[ ]}+1]}
      /declare memberName string local ${IniString.Arg[${i}]}
      /if (!${memberName.Length}) /continue

      |- Skip if already in group
      /if (${Group.Member[${memberName}].Index}) {
         /if (${GroupDebug}) /echo [GZM_Group] ${memberName} already in group
         /continue
      }

      |- Check if member is online
      /if (!${Spawn[pc ${memberName}].ID}) {
         /echo [GZM_Group] ${memberName} not found - skipping
         /continue
      }

      /echo [GZM_Group] Inviting ${memberName}
      /invite ${memberName}
      /delay 6
   /next i

   /echo [GZM_Group] Group formation complete
/return

|- ============================================================================
|- TANK/PULLER MANAGEMENT
|- ============================================================================

|- Set tank from group roles or manual override
Sub SetTank
   |- Skip if manual override
   /if (${TankManual}) /return

   |- Check group main tank role
   /if (${Group.MainTank.ID}) {
      /varset TankName ${Group.MainTank.CleanName}
      /return
   }

   |- Check group main assist role
   /if (${Group.MainAssist.ID}) {
      /varset TankName ${Group.MainAssist.CleanName}
      /return
   }
/return

|- Set puller from group roles or manual override
Sub SetPuller
   |- Skip if manual override
   /if (${PullerManual}) /return

   |- Check group puller role
   /if (${Group.Puller.ID}) {
      /varset PullerName ${Group.Puller.CleanName}
      /return
   }
/return

|- Manually set tank (won't be overridden by group roles)
Sub SetTankManual(tankName)
   /varset TankName ${tankName}
   /varset TankManual TRUE
   /echo [GZM_Group] Tank manually set to: ${tankName}
/return

|- Manually set puller (won't be overridden by group roles)
Sub SetPullerManual(pullerName)
   /varset PullerName ${pullerName}
   /varset PullerManual TRUE
   /echo [GZM_Group] Puller manually set to: ${pullerName}
/return

|- Clear manual tank override
Sub ClearTankManual
   /varset TankManual FALSE
   /call SetTank
   /echo [GZM_Group] Tank manual override cleared
/return

|- Clear manual puller override
Sub ClearPullerManual
   /varset PullerManual FALSE
   /call SetPuller
   /echo [GZM_Group] Puller manual override cleared
/return

|- ============================================================================
|- SINGLE FILE FOLLOW
|- Make bots follow in a single file line
|- ============================================================================

Sub SingleFileFollow
   /if (!${Plugin[MQ2DanNet].IsLoaded}) {
      /echo [GZM_Group] SingleFile requires DanNet
      /return
   }

   /echo [GZM_Group] Setting up single file follow...

   /declare i int local
   /declare prevMember string local ${Me.CleanName}
   /declare peerList string local
   /declare currentPeer string local

   |- Get peer list
   /for i 1 to ${DanNet.PeerCount}
      /varset currentPeer ${DanNet.Peers.Arg[${i}, ]}
      /if (${currentPeer.Equal[${Me.CleanName}]}) /continue

      |- Tell this peer to follow the previous member
      /dex ${currentPeer} /varset FollowTarget ${prevMember}
      /dex ${currentPeer} /call SetFollowTarget ${prevMember} 15

      /varset prevMember ${currentPeer}
      /delay 3
   /next i

   /echo [GZM_Group] Single file follow configured
/return

|- ============================================================================
|- GROUP UTILITIES
|- ============================================================================

|- Get group member by index (0-based, 0 is self)
Sub GetGroupMember(index)
   /if (!${Group.Members}) /return
   /if (${index}>=${Group.Members}) /return

/return ${Group.Member[${index}].CleanName}

|- Check if character is in my group
Sub IsInMyGroup(charName)
   /if (!${Group.Members}) /return FALSE
   /if (${Group.Member[${charName}].Index}) /return TRUE
/return FALSE

|- Get group size
Sub GetGroupSize
/return ${Group.Members}

|- Leave current group
Sub LeaveGroup
   /if (!${Group.Members}) {
      /echo [GZM_Group] Not in a group
      /return
   }
   /disband
   /echo [GZM_Group] Left group
/return

|- Make me group leader (if possible)
Sub MakeLeader
   /if (!${Group.Members}) {
      /echo [GZM_Group] Not in a group
      /return
   }
   /if (${Group.Member[0].Leader}) {
      /echo [GZM_Group] Already leader
      /return
   }
   /makeleader ${Me.CleanName}
   /delay 2s ${Group.Member[0].Leader}
/return

|- ============================================================================
|- XTARGET MANAGEMENT
|- Fill XTargets with out-of-group characters
|- ============================================================================

Sub FillXTargets
   /if (!${Plugin[MQ2DanNet].IsLoaded}) {
      /echo [GZM_Group] FillXTargets requires DanNet
      /return
   }

   /declare i int local
   /declare xSlot int local 1
   /declare currentPeer string local

   /echo [GZM_Group] Filling XTargets with DanNet peers...

   /for i 1 to ${DanNet.PeerCount}
      /varset currentPeer ${DanNet.Peers.Arg[${i}, ]}
      /if (${currentPeer.Equal[${Me.CleanName}]}) /continue

      |- Skip if in my group
      /if (${Group.Member[${currentPeer}].Index}) /continue

      |- Add to XTarget
      /xtarget set ${xSlot} ${Spawn[pc ${currentPeer}].ID}
      /varcalc xSlot ${xSlot}+1

      /if (${xSlot}>10) /break
   /next i

   /echo [GZM_Group] XTargets filled
/return

|- ============================================================================
|- INI FUNCTIONS
|- ============================================================================

Sub LoadGroupSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /varset AutoAcceptInvite ${Ini[${IniFile},Group,AutoAcceptInvite,${AutoAcceptInvite}]}
   /varset AutoAcceptRaidInvite ${Ini[${IniFile},Group,AutoAcceptRaidInvite,${AutoAcceptRaidInvite}]}
   /varset TankName ${Ini[${IniFile},Group,TankName,${TankName}]}
   /varset PullerName ${Ini[${IniFile},Group,PullerName,${PullerName}]}
   /varset TankManual ${Ini[${IniFile},Group,TankManual,${TankManual}]}
   /varset PullerManual ${Ini[${IniFile},Group,PullerManual,${PullerManual}]}

   |- Update from group roles if not manual
   /call SetTank
   /call SetPuller

   /if (${GroupDebug}) /echo [GZM_Group] Settings loaded from ${IniFile}
/return

Sub SaveGroupSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /ini "${IniFile}" "Group" "AutoAcceptInvite" "${AutoAcceptInvite}"
   /ini "${IniFile}" "Group" "AutoAcceptRaidInvite" "${AutoAcceptRaidInvite}"
   /ini "${IniFile}" "Group" "TankName" "${TankName}"
   /ini "${IniFile}" "Group" "PullerName" "${PullerName}"
   /ini "${IniFile}" "Group" "TankManual" "${TankManual}"
   /ini "${IniFile}" "Group" "PullerManual" "${PullerManual}"

   /echo [GZM_Group] Settings saved to ${IniFile}
/return

|- Get group status
Sub GetGroupStatus
   /echo [GZM_Group] === Group Status ===
   /echo In Group: ${If[${Group.Members},Yes (${Group.Members} members),No]}
   /if (${Group.Members}) {
      /echo Leader: ${Group.Leader.CleanName}
      /echo Main Tank: ${If[${Group.MainTank.ID},${Group.MainTank.CleanName},Not Set]}
      /echo Main Assist: ${If[${Group.MainAssist.ID},${Group.MainAssist.CleanName},Not Set]}
      /echo Puller: ${If[${Group.Puller.ID},${Group.Puller.CleanName},Not Set]}
   }
   /echo Tank Name: ${If[${TankName.Length},${TankName} ${If[${TankManual},(Manual),]},Not Set]}
   /echo Puller Name: ${If[${PullerName.Length},${PullerName} ${If[${PullerManual},(Manual),]},Not Set]}
   /echo Auto Accept Invites: ${If[${AutoAcceptInvite},ON,OFF]}
   /echo ========================
/return

