|- GronnzMaster Timer Throttling Include
|- Prevents spam by gating function calls with timers
|- Version 1.0

|- Timer values are in tenths of seconds (5 = 0.5s, 10 = 1s, etc.)
|- MQ2 timer format: /varset Timer 10 means 1 second

Sub InitTimerVars
   |- High Priority Timers (fast checks)
   /if (!${Defined[HealCheckTimer]}) /declare HealCheckTimer timer outer
   /if (!${Defined[CureCheckTimer]}) /declare CureCheckTimer timer outer

   |- Medium Priority Timers
   /if (!${Defined[CombatCheckTimer]}) /declare CombatCheckTimer timer outer
   /if (!${Defined[MezCheckTimer]}) /declare MezCheckTimer timer outer
   /if (!${Defined[DebuffCheckTimer]}) /declare DebuffCheckTimer timer outer
   /if (!${Defined[PetCheckTimer]}) /declare PetCheckTimer timer outer

   |- Low Priority Timers
   /if (!${Defined[BuffCheckTimer]}) /declare BuffCheckTimer timer outer
   /if (!${Defined[PullCheckTimer]}) /declare PullCheckTimer timer outer
   /if (!${Defined[LootCheckTimer]}) /declare LootCheckTimer timer outer

   |- Very Low Priority Timers
   /if (!${Defined[SitCheckTimer]}) /declare SitCheckTimer timer outer
   /if (!${Defined[FoodCheckTimer]}) /declare FoodCheckTimer timer outer
   /if (!${Defined[FollowCheckTimer]}) /declare FollowCheckTimer timer outer
   /if (!${Defined[CampCheckTimer]}) /declare CampCheckTimer timer outer

   |- Cooldown/Spam Prevention Timers
   /if (!${Defined[BroadcastTimer]}) /declare BroadcastTimer timer outer
   /if (!${Defined[TargetSwitchTimer]}) /declare TargetSwitchTimer timer outer
   /if (!${Defined[CommandSpamTimer]}) /declare CommandSpamTimer timer outer

   |- Default timer values (tenths of seconds)
   /if (!${Defined[HealCheckDelay]}) /declare HealCheckDelay int outer 3
   /if (!${Defined[CureCheckDelay]}) /declare CureCheckDelay int outer 5
   /if (!${Defined[CombatCheckDelay]}) /declare CombatCheckDelay int outer 5
   /if (!${Defined[MezCheckDelay]}) /declare MezCheckDelay int outer 5
   /if (!${Defined[DebuffCheckDelay]}) /declare DebuffCheckDelay int outer 10
   /if (!${Defined[PetCheckDelay]}) /declare PetCheckDelay int outer 10
   /if (!${Defined[BuffCheckDelay]}) /declare BuffCheckDelay int outer 30
   /if (!${Defined[PullCheckDelay]}) /declare PullCheckDelay int outer 20
   /if (!${Defined[LootCheckDelay]}) /declare LootCheckDelay int outer 10
   /if (!${Defined[SitCheckDelay]}) /declare SitCheckDelay int outer 50
   /if (!${Defined[FoodCheckDelay]}) /declare FoodCheckDelay int outer 600
   /if (!${Defined[FollowCheckDelay]}) /declare FollowCheckDelay int outer 5
   /if (!${Defined[CampCheckDelay]}) /declare CampCheckDelay int outer 20
   /if (!${Defined[BroadcastDelay]}) /declare BroadcastDelay int outer 20
   /if (!${Defined[TargetSwitchDelay]}) /declare TargetSwitchDelay int outer 3
   /if (!${Defined[CommandSpamDelay]}) /declare CommandSpamDelay int outer 5

   |- Debug flag
   /if (!${Defined[TimerDebug]}) /declare TimerDebug bool outer FALSE
/return

|- Reset all timers to 0 (allow immediate checks)
Sub ResetAllTimers
   /varset HealCheckTimer 0
   /varset CureCheckTimer 0
   /varset CombatCheckTimer 0
   /varset MezCheckTimer 0
   /varset DebuffCheckTimer 0
   /varset PetCheckTimer 0
   /varset BuffCheckTimer 0
   /varset PullCheckTimer 0
   /varset LootCheckTimer 0
   /varset SitCheckTimer 0
   /varset FoodCheckTimer 0
   /varset FollowCheckTimer 0
   /varset CampCheckTimer 0
   /varset BroadcastTimer 0
   /varset TargetSwitchTimer 0
   /varset CommandSpamTimer 0
   /if (${TimerDebug}) /echo [GZM_Timers] All timers reset
/return

|- Reset combat-related timers (after combat ends)
Sub ResetCombatTimers
   /varset CombatCheckTimer 0
   /varset MezCheckTimer 0
   /varset DebuffCheckTimer 0
   /varset PetCheckTimer 0
   /varset TargetSwitchTimer 0
   /if (${TimerDebug}) /echo [GZM_Timers] Combat timers reset
/return

|- Reset non-combat timers (after zone/death)
Sub ResetIdleTimers
   /varset BuffCheckTimer 0
   /varset SitCheckTimer 0
   /varset FoodCheckTimer 0
   /varset FollowCheckTimer 0
   /varset CampCheckTimer 0
   /if (${TimerDebug}) /echo [GZM_Timers] Idle timers reset
/return

|- Check if timer allows action, return TRUE if ready
Sub TimerReady(timerName)
   /if (!${${timerName}}) /return TRUE
/return FALSE

|- Set timer after action
Sub SetTimer(timerName, delayValue)
   /varset ${timerName} ${delayValue}
   /if (${TimerDebug}) /echo [GZM_Timers] ${timerName} set to ${delayValue}
/return

|- Generic throttled call - check timer, call sub if ready, reset timer
|- Usage: /call ThrottledCall HealCheckTimer CheckHeals ${HealCheckDelay}
Sub ThrottledCall(timerName, subToCall, delayValue)
   /if (!${${timerName}}) {
      /call ${subToCall}
      /varset ${timerName} ${delayValue}
      /if (${TimerDebug}) /echo [GZM_Timers] Called ${subToCall}, timer set to ${delayValue}
   }
/return

|- Heal check gate
Sub CanCheckHeals
   /if (!${HealCheckTimer}) {
      /varset HealCheckTimer ${HealCheckDelay}
      /return TRUE
   }
/return FALSE

|- Cure check gate
Sub CanCheckCures
   /if (!${CureCheckTimer}) {
      /varset CureCheckTimer ${CureCheckDelay}
      /return TRUE
   }
/return FALSE

|- Combat check gate
Sub CanCheckCombat
   /if (!${CombatCheckTimer}) {
      /varset CombatCheckTimer ${CombatCheckDelay}
      /return TRUE
   }
/return FALSE

|- Mez check gate
Sub CanCheckMez
   /if (!${MezCheckTimer}) {
      /varset MezCheckTimer ${MezCheckDelay}
      /return TRUE
   }
/return FALSE

|- Debuff check gate
Sub CanCheckDebuffs
   /if (!${DebuffCheckTimer}) {
      /varset DebuffCheckTimer ${DebuffCheckDelay}
      /return TRUE
   }
/return FALSE

|- Pet check gate
Sub CanCheckPet
   /if (!${PetCheckTimer}) {
      /varset PetCheckTimer ${PetCheckDelay}
      /return TRUE
   }
/return FALSE

|- Buff check gate
Sub CanCheckBuffs
   /if (!${BuffCheckTimer}) {
      /varset BuffCheckTimer ${BuffCheckDelay}
      /return TRUE
   }
/return FALSE

|- Pull check gate
Sub CanCheckPull
   /if (!${PullCheckTimer}) {
      /varset PullCheckTimer ${PullCheckDelay}
      /return TRUE
   }
/return FALSE

|- Loot check gate
Sub CanCheckLoot
   /if (!${LootCheckTimer}) {
      /varset LootCheckTimer ${LootCheckDelay}
      /return TRUE
   }
/return FALSE

|- Sit/Med check gate
Sub CanCheckSitMed
   /if (!${SitCheckTimer}) {
      /varset SitCheckTimer ${SitCheckDelay}
      /return TRUE
   }
/return FALSE

|- Food/Drink check gate
Sub CanCheckFood
   /if (!${FoodCheckTimer}) {
      /varset FoodCheckTimer ${FoodCheckDelay}
      /return TRUE
   }
/return FALSE

|- Follow check gate
Sub CanCheckFollow
   /if (!${FollowCheckTimer}) {
      /varset FollowCheckTimer ${FollowCheckDelay}
      /return TRUE
   }
/return FALSE

|- Camp check gate
Sub CanCheckCamp
   /if (!${CampCheckTimer}) {
      /varset CampCheckTimer ${CampCheckDelay}
      /return TRUE
   }
/return FALSE

|- Broadcast spam prevention
Sub CanBroadcast
   /if (!${BroadcastTimer}) {
      /varset BroadcastTimer ${BroadcastDelay}
      /return TRUE
   }
/return FALSE

|- Target switch spam prevention
Sub CanSwitchTarget
   /if (!${TargetSwitchTimer}) {
      /varset TargetSwitchTimer ${TargetSwitchDelay}
      /return TRUE
   }
/return FALSE

|- Command spam prevention
Sub CanSendCommand
   /if (!${CommandSpamTimer}) {
      /varset CommandSpamTimer ${CommandSpamDelay}
      /return TRUE
   }
/return FALSE

|- Delay a specific timer (add time without reset)
Sub DelayTimer(timerName, addTime)
   /varcalc ${timerName} ${${timerName}}+${addTime}
   /if (${TimerDebug}) /echo [GZM_Timers] ${timerName} delayed by ${addTime}
/return

|- Force trigger cooldown (set timer regardless of current value)
Sub ForceCooldown(timerName, cooldownTime)
   /varset ${timerName} ${cooldownTime}
   /if (${TimerDebug}) /echo [GZM_Timers] ${timerName} forced to ${cooldownTime}
/return

|- Check all timer statuses (debug)
Sub ShowTimerStatus
   /echo [GZM_Timers] === Timer Status ===
   /echo Heal: ${HealCheckTimer} | Cure: ${CureCheckTimer}
   /echo Combat: ${CombatCheckTimer} | Mez: ${MezCheckTimer} | Debuff: ${DebuffCheckTimer}
   /echo Pet: ${PetCheckTimer} | Buff: ${BuffCheckTimer}
   /echo Pull: ${PullCheckTimer} | Loot: ${LootCheckTimer}
   /echo Sit: ${SitCheckTimer} | Food: ${FoodCheckTimer}
   /echo Follow: ${FollowCheckTimer} | Camp: ${CampCheckTimer}
   /echo ========================
/return

|- Load timer settings from INI
Sub LoadTimerSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /varset HealCheckDelay ${Ini[${IniFile},Timers,HealCheckDelay,${HealCheckDelay}]}
   /varset CureCheckDelay ${Ini[${IniFile},Timers,CureCheckDelay,${CureCheckDelay}]}
   /varset CombatCheckDelay ${Ini[${IniFile},Timers,CombatCheckDelay,${CombatCheckDelay}]}
   /varset MezCheckDelay ${Ini[${IniFile},Timers,MezCheckDelay,${MezCheckDelay}]}
   /varset DebuffCheckDelay ${Ini[${IniFile},Timers,DebuffCheckDelay,${DebuffCheckDelay}]}
   /varset PetCheckDelay ${Ini[${IniFile},Timers,PetCheckDelay,${PetCheckDelay}]}
   /varset BuffCheckDelay ${Ini[${IniFile},Timers,BuffCheckDelay,${BuffCheckDelay}]}
   /varset PullCheckDelay ${Ini[${IniFile},Timers,PullCheckDelay,${PullCheckDelay}]}
   /varset LootCheckDelay ${Ini[${IniFile},Timers,LootCheckDelay,${LootCheckDelay}]}
   /varset SitCheckDelay ${Ini[${IniFile},Timers,SitCheckDelay,${SitCheckDelay}]}
   /varset FoodCheckDelay ${Ini[${IniFile},Timers,FoodCheckDelay,${FoodCheckDelay}]}
   /varset FollowCheckDelay ${Ini[${IniFile},Timers,FollowCheckDelay,${FollowCheckDelay}]}
   /varset CampCheckDelay ${Ini[${IniFile},Timers,CampCheckDelay,${CampCheckDelay}]}

   /if (${TimerDebug}) /echo [GZM_Timers] Timer settings loaded from ${IniFile}
/return

|- Save timer settings to INI
Sub SaveTimerSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /ini "${IniFile}" "Timers" "HealCheckDelay" "${HealCheckDelay}"
   /ini "${IniFile}" "Timers" "CureCheckDelay" "${CureCheckDelay}"
   /ini "${IniFile}" "Timers" "CombatCheckDelay" "${CombatCheckDelay}"
   /ini "${IniFile}" "Timers" "MezCheckDelay" "${MezCheckDelay}"
   /ini "${IniFile}" "Timers" "DebuffCheckDelay" "${DebuffCheckDelay}"
   /ini "${IniFile}" "Timers" "PetCheckDelay" "${PetCheckDelay}"
   /ini "${IniFile}" "Timers" "BuffCheckDelay" "${BuffCheckDelay}"
   /ini "${IniFile}" "Timers" "PullCheckDelay" "${PullCheckDelay}"
   /ini "${IniFile}" "Timers" "LootCheckDelay" "${LootCheckDelay}"
   /ini "${IniFile}" "Timers" "SitCheckDelay" "${SitCheckDelay}"
   /ini "${IniFile}" "Timers" "FoodCheckDelay" "${FoodCheckDelay}"
   /ini "${IniFile}" "Timers" "FollowCheckDelay" "${FollowCheckDelay}"
   /ini "${IniFile}" "Timers" "CampCheckDelay" "${CampCheckDelay}"

   /echo [GZM_Timers] Timer settings saved to ${IniFile}
/return
