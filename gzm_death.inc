|- GronnzMaster Death & Resurrection Handling Include
|- Complete death handling with rez acceptance and recovery
|- Version 1.0

|- Events for death detection
#Event GMSlain "You have been slain by#*#"
#Event GMSlain "You died."
#Event GMSlain "You have been killed#*#"

Sub InitDeathVars
   |- Rez settings
   /if (!${Defined[MinRezPct]}) /declare MinRezPct int outer 90
   /if (!${Defined[RezTimeout]}) /declare RezTimeout int outer 300
   /if (!${Defined[AutoAcceptRez]}) /declare AutoAcceptRez bool outer TRUE

   |- Consent settings
   /if (!${Defined[DoConsent]}) /declare DoConsent bool outer TRUE
   /if (!${Defined[ConsentGroup]}) /declare ConsentGroup bool outer TRUE
   /if (!${Defined[ConsentRaid]}) /declare ConsentRaid bool outer TRUE
   /if (!${Defined[ConsentDanNet]}) /declare ConsentDanNet bool outer TRUE

   |- Death tracking
   /if (!${Defined[DeathCount]}) /declare DeathCount int outer 0
   /if (!${Defined[LastDeathTime]}) /declare LastDeathTime int outer 0
   /if (!${Defined[LastDeathZone]}) /declare LastDeathZone string outer

   |- Recovery settings
   /if (!${Defined[PostRezDelay]}) /declare PostRezDelay int outer 5
   /if (!${Defined[AutoResumeAfterRez]}) /declare AutoResumeAfterRez bool outer FALSE

   |- Debug
   /if (!${Defined[DeathDebug]}) /declare DeathDebug bool outer FALSE
/return

|- Event handler for death
Sub Event_GMSlain
   /if (${DeathDebug}) /echo [GZM_Death] Death event triggered
   /call HandleDeath
/return

|- Main death handling routine
Sub HandleDeath
   |- Audio/visual alert
   /beep
   /echo [GZM_Death] *** ${Me.CleanName} HAS DIED ***

   |- Broadcast death to group via DanNet
   /if (${Plugin[MQ2DanNet].IsLoaded}) {
      /dgt Death: ${Me.CleanName} has died in ${Zone.ShortName}!
   }

   |- Track death statistics
   /varcalc DeathCount ${DeathCount}+1
   /varset LastDeathTime ${Time.SecondsSinceMidnight}
   /varset LastDeathZone ${Zone.ShortName}

   |- Set state to DEAD
   /call SetState DEAD

   |- Disable ALL automation flags
   /call DisableAllAutomation

   |- Reset all combat state
   /call FullCombatReset

   |- Clear targeting
   /squelch /target clear

   |- Disable MQ2Melee
   /squelch /melee plugin=0

   |- Stop all movement
   /squelch /nav stop
   /squelch /stick off
   /squelch /moveto off

   |- Flush all pending events to prevent stale processing
   /doevents flush

   |- Wait for resurrection if enabled
   /if (${AutoAcceptRez}) {
      /call WaitForRez
   } else {
      /echo [GZM_Death] AutoAcceptRez disabled - waiting for manual resurrection
   }
/return

|- Disable all automation flags
Sub DisableAllAutomation
   /if (${Defined[DoHeals]}) /varset DoHeals FALSE
   /if (${Defined[DoBuffs]}) /varset DoBuffs FALSE
   /if (${Defined[DoCombat]}) /varset DoCombat FALSE
   /if (${Defined[DoDebuffs]}) /varset DoDebuffs FALSE
   /if (${Defined[DoCures]}) /varset DoCures FALSE
   /if (${Defined[DoMez]}) /varset DoMez FALSE
   /if (${Defined[DoPull]}) /varset DoPull FALSE
   /if (${Defined[DoFollow]}) /varset DoFollow FALSE
   /if (${Defined[DoLoot]}) /varset DoLoot FALSE
   /if (${Defined[DoPet]}) /varset DoPet FALSE
   /if (${Defined[DoCharm]}) /varset DoCharm FALSE

   /if (${DeathDebug}) /echo [GZM_Death] All automation disabled
/return

|- Full combat state reset
Sub FullCombatReset
   |- Clear combat targeting
   /if (${Defined[GMCombatTarget]}) /varset GMCombatTarget 0
   /if (${Defined[AssistTarget]}) /varset AssistTarget 0
   /if (${Defined[CurrentTarget]}) /varset CurrentTarget 0
   /if (${Defined[InCombat]}) /varset InCombat FALSE

   |- Clear mez tracking if exists
   /if (${Defined[MezList]}) {
      /declare i int local
      /for i 1 to 20
         /if (${Defined[MezList[${i}]]}) /varset MezList[${i}] 0
      /next i
   }

   |- Clear debuff tracking if exists
   /if (${Defined[DebuffedMobs]}) {
      /declare j int local
      /for j 1 to 20
         /if (${Defined[DebuffedMobs[${j}]]}) /varset DebuffedMobs[${j}] 0
      /next j
   }

   /if (${DeathDebug}) /echo [GZM_Death] Combat state reset complete
/return

|- Wait for resurrection with full respawn handling
Sub WaitForRez
   /declare RezTimer timer local ${RezTimeout}s
   /declare RespawnTimer timer local 600s
   /declare CheckInterval timer local

   /echo [GZM_Death] Waiting for resurrection (${RezTimeout}s timeout, min ${MinRezPct}%)
   /call SetState REZZING

   :WaitLoop
   |- Check if still dead/hovering
   /if (${Me.State.Equal[HOVER]}) {
      |- Check for rez confirmation dialog
      /if (${Window[ConfirmationDialogBox].Open}) {
         /call CheckRezDialog
         /if (${Macro.Return.Equal[ACCEPTED]}) {
            |- Wait for character to fully load after rez
            /echo [GZM_Death] Rez accepted - waiting for character to stabilize...
            /delay 600s ${Me.ID}
            /delay 5s
            /doevents flush
            /call PostRezRecovery
            /return
         }
      }

      |- Brief delay between checks
      /delay 5

      |- Continue waiting if timer not expired
      /if (${RezTimer}) /goto :WaitLoop
   } else /if (!${Me.State.Equal[HOVER]} && ${Me.State.NotEqual[BIND]}) {
      |- We're alive somehow (maybe accepted rez already)
      /echo [GZM_Death] No longer hovering - checking if rezzed
      |- Wait for character to stabilize
      /delay 600s ${Me.ID}
      /delay 5s
      /if (${Me.CurrentHPs}>0) {
         /doevents flush
         /call PostRezRecovery
         /return
      }
   }

   |- Timeout reached - respawn at bind
   /if (${Me.State.Equal[HOVER]}) {
      /echo [GZM_Death] Rez timeout - respawning at bind point
      /notify RespawnWnd RW_ZoneButton leftmouseup
      |- Wait for full respawn
      /echo [GZM_Death] Waiting for respawn to complete...
      /delay 600s ${Me.ID}
      /delay 5s
      /doevents flush
      |- Check for invalid zone
      /if (!${Zone.ID}) {
         /echo [GZM_Death] Invalid zone detected - waiting...
         /delay 15s ${Zone.ID}
      }
      /call PostRezRecovery
   }
/return

|- Check resurrection dialog and accept if meets criteria
Sub CheckRezDialog
   |- Parse rez percentage from dialog text
   /declare DialogText string local ${Window[ConfirmationDialogBox].Child[cd_textoutput].Text}
   /declare RezPct int local 0

   |- Extract percentage - look for number followed by %
   /if (${DialogText.Find[%]}) {
      |- Find the number before the % sign
      /declare PercentPos int local ${DialogText.Find[%]}
      /declare NumStart int local ${Math.Calc[${PercentPos}-3]}
      /if (${NumStart}<1) /varset NumStart 1
      /declare NumStr string local ${DialogText.Mid[${NumStart},${Math.Calc[${PercentPos}-${NumStart}]}]}
      |- Extract just digits
      /if (${NumStr.Find[0]} || ${NumStr.Find[1]} || ${NumStr.Find[2]} || ${NumStr.Find[3]} || ${NumStr.Find[4]} || ${NumStr.Find[5]} || ${NumStr.Find[6]} || ${NumStr.Find[7]} || ${NumStr.Find[8]} || ${NumStr.Find[9]}) {
         /varset RezPct ${Int[${NumStr}]}
      }
   }

   /if (${DeathDebug}) /echo [GZM_Death] Rez dialog detected - ${RezPct}%

   |- Check if meets minimum threshold
   /if (${RezPct}>=${MinRezPct}) {
      /echo [GZM_Death] Accepting ${RezPct}% resurrection
      /notify ConfirmationDialogBox Yes_Button leftmouseup
      /delay 3s !${Me.State.Equal[HOVER]}

      /if (!${Me.State.Equal[HOVER]}) {
         /call PostRezRecovery
         /return ACCEPTED
      }
   } else /if (${RezPct}>0 && ${RezPct}<${MinRezPct}) {
      /echo [GZM_Death] Declining ${RezPct}% rez (minimum: ${MinRezPct}%)
      /notify ConfirmationDialogBox No_Button leftmouseup
      /delay 5
   }

/return WAITING

|- Post-resurrection recovery
Sub PostRezRecovery
   /echo [GZM_Death] Resurrection accepted - beginning recovery

   |- Wait for zone to stabilize
   /delay ${PostRezDelay}s

   |- Set state to IDLE
   /call SetState IDLE

   |- Handle corpse consent
   /if (${DoConsent}) {
      /call DoCorpseConsent
   }

   |- Reload zone-specific data
   /if (${Defined[LoadImmunes]}) /call LoadImmunes

   |- Reset all timers for fresh start
   /if (${Defined[ResetAllTimers]}) /call ResetAllTimers

   |- Re-enable core automation (heals, buffs, cures)
   /if (${Defined[DoHeals]}) /varset DoHeals TRUE
   /if (${Defined[DoBuffs]}) /varset DoBuffs TRUE
   /if (${Defined[DoCures]}) /varset DoCures TRUE

   |- Optional: Re-enable all automation
   /if (${AutoResumeAfterRez}) {
      /delay 3s
      /call EnableAllAutomation
      /echo [GZM_Death] Full automation resumed
   } else {
      /echo [GZM_Death] Heals/Buffs/Cures enabled. Use /gm camp to resume full automation.
   }

   /dgt Rez complete: ${Me.CleanName} recovered in ${Zone.ShortName}
/return

|- Enable all automation flags
Sub EnableAllAutomation
   /if (${Defined[DoHeals]}) /varset DoHeals TRUE
   /if (${Defined[DoBuffs]}) /varset DoBuffs TRUE
   /if (${Defined[DoCombat]}) /varset DoCombat TRUE
   /if (${Defined[DoDebuffs]}) /varset DoDebuffs TRUE
   /if (${Defined[DoCures]}) /varset DoCures TRUE
   /if (${Defined[DoMez]}) /varset DoMez TRUE
   /if (${Defined[DoLoot]}) /varset DoLoot TRUE
   /if (${Defined[DoPet]}) /varset DoPet TRUE

   |- Note: DoFollow and DoPull are not auto-enabled for safety
   /if (${DeathDebug}) /echo [GZM_Death] Core automation enabled
/return

|- Corpse consent for group/raid/dannet
Sub DoCorpseConsent
   /echo [GZM_Death] Issuing corpse consent...

   |- Group consent
   /if (${ConsentGroup} && ${Group.Members}) {
      /consent group
      /delay 5
      /if (${DeathDebug}) /echo [GZM_Death] Group consent issued
   }

   |- Raid consent
   /if (${ConsentRaid} && ${Raid.Members}) {
      /consent raid
      /delay 5
      /if (${DeathDebug}) /echo [GZM_Death] Raid consent issued
   }

   |- DanNet peer consent (GronnzMaster equivalent of NetBots)
   /if (${ConsentDanNet} && ${Plugin[MQ2DanNet].IsLoaded}) {
      /declare i int local
      /declare peerName string local

      /for i 1 to ${DanNet.PeerCount}
         /varset peerName ${DanNet.Peers.Arg[${i}, ]}
         /if (${peerName.Length} && ${peerName.NotEqual[${Me.CleanName}]}) {
            /consent ${peerName}
            /delay 3
         }
      /next i
      /if (${DeathDebug}) /echo [GZM_Death] DanNet consent issued to ${DanNet.PeerCount} peers
   }

   /echo [GZM_Death] Consent complete
/return

|- Manual death recovery (if auto-rez was disabled or failed)
Sub ManualRecovery
   /if (${Me.State.Equal[HOVER]}) {
      /echo [GZM_Death] Still dead - cannot manually recover
      /return
   }

   /call PostRezRecovery
/return

|- Get death statistics
Sub GetDeathStats
   /echo [GZM_Death] === Death Statistics ===
   /echo Total deaths this session: ${DeathCount}
   /if (${LastDeathTime}>0) {
      /echo Last death: ${LastDeathZone}
   }
   /echo ========================
/return

|- Load death settings from INI
Sub LoadDeathSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /varset MinRezPct ${Ini[${IniFile},Death,MinRezPct,${MinRezPct}]}
   /varset RezTimeout ${Ini[${IniFile},Death,RezTimeout,${RezTimeout}]}
   /varset AutoAcceptRez ${Ini[${IniFile},Death,AutoAcceptRez,${AutoAcceptRez}]}
   /varset DoConsent ${Ini[${IniFile},Death,DoConsent,${DoConsent}]}
   /varset ConsentGroup ${Ini[${IniFile},Death,ConsentGroup,${ConsentGroup}]}
   /varset ConsentRaid ${Ini[${IniFile},Death,ConsentRaid,${ConsentRaid}]}
   /varset ConsentDanNet ${Ini[${IniFile},Death,ConsentDanNet,${ConsentDanNet}]}
   /varset PostRezDelay ${Ini[${IniFile},Death,PostRezDelay,${PostRezDelay}]}
   /varset AutoResumeAfterRez ${Ini[${IniFile},Death,AutoResumeAfterRez,${AutoResumeAfterRez}]}

   /if (${DeathDebug}) /echo [GZM_Death] Settings loaded from ${IniFile}
/return

|- Save death settings to INI
Sub SaveDeathSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /ini "${IniFile}" "Death" "MinRezPct" "${MinRezPct}"
   /ini "${IniFile}" "Death" "RezTimeout" "${RezTimeout}"
   /ini "${IniFile}" "Death" "AutoAcceptRez" "${AutoAcceptRez}"
   /ini "${IniFile}" "Death" "DoConsent" "${DoConsent}"
   /ini "${IniFile}" "Death" "ConsentGroup" "${ConsentGroup}"
   /ini "${IniFile}" "Death" "ConsentRaid" "${ConsentRaid}"
   /ini "${IniFile}" "Death" "ConsentDanNet" "${ConsentDanNet}"
   /ini "${IniFile}" "Death" "PostRezDelay" "${PostRezDelay}"
   /ini "${IniFile}" "Death" "AutoResumeAfterRez" "${AutoResumeAfterRez}"

   /echo [GZM_Death] Settings saved to ${IniFile}
/return
