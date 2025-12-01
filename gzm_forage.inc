|- GronnzMaster Foraging System Include
|- Auto-forage during idle time (Rangers, Druids, etc)
|- Supports RNG/DRU/BRD/ROG classes
|- Version 1.0

|- Forage event detection
#Event GMForage "You have#*#scrounged up#*#"

Sub InitForageVars
   |- Forage settings
   /if (!${Defined[DoForage]}) /declare DoForage bool outer FALSE
   /if (!${Defined[ForageTimer]}) /declare ForageTimer timer outer
   /if (!${Defined[ItemForaged]}) /declare ItemForaged bool outer FALSE

   |- Forage announce channel
   /if (!${Defined[ForageAnnounce]}) /declare ForageAnnounce string outer

   |- Debug
   /if (!${Defined[ForageDebug]}) /declare ForageDebug bool outer FALSE
/return

|- Event handler for forage
Sub Event_GMForage(line)
   /varset ItemForaged TRUE
/return

|- ============================================================================
|- MAIN FORAGE ROUTINE
|- Call from main loop when idle
|- ============================================================================
Sub CheckForage
   /if (!${DoForage}) /return
   /if (${ForageTimer}) /return

   |- Safety checks
   /if (${Me.Invis}) /return
   /if (${Defined[InCombat]}) {
      /if (${InCombat}) /return
   }
   /if (${Me.Casting.ID}) /return
   /if (${Me.Moving}) /return

   |- Skip if windows open that could cause problems
   /if (${Window[InventoryWindow].Open}) /return
   /if (${Window[BigBankWnd].Open}) /return

   |- Check if we can forage
   /if (!${Me.Ability[forage]}) /return
   /if (!${Me.AbilityReady[forage]}) {
      /varset ForageTimer 10s
      /return
   }

   |- Clear cursor first
   /if (${Cursor.ID}) {
      /if (${Defined[ClearCursor]}) {
         /call ClearCursor
         /if (${Cursor.ID}) /return
      } else {
         /autoinventory
         /delay 1s !${Cursor.ID}
         /if (${Cursor.ID}) /return
      }
   }

   /call DoForage
   /varset ForageTimer 10s
/return

|- Perform forage action and handle results
Sub DoForage
   /declare CursorItemID int local

   |- Stand up if needed
   /if (!${Me.Mount.ID} && !${Me.Standing}) {
      /stand
      /delay 5
   }

   |- Clear old forage events
   /doevents flush GMForage

   |- Perform forage ability
   /doability forage
   /delay 1s ${Cursor.ID}
   /delay 2

   |- Check for foraged item
   /varset ItemForaged FALSE
   /doevents GMForage

   /while (${ItemForaged}) {
      /delay 5 ${Cursor.ID}
      /if (!${Cursor.ID}) /break

      /varset CursorItemID ${Cursor.ID}

      |- Determine what to do with foraged item
      /call ProcessForagedItem

      /delay 5
      /varset ItemForaged FALSE
      /doevents GMForage
   }

   |- Ensure cursor is clear
   /delay 2s !${Cursor.ID}
   /doevents flush GMForage
/return

|- Process a foraged item based on loot rules
Sub ProcessForagedItem
   /if (!${Cursor.ID}) /return

   /declare itemName string local ${Cursor.Name}
   /declare action string local KEEP

   /if (${ForageDebug}) /echo [GZM_Forage] Foraged: ${itemName}

   |- Check loot INI for action (if we have loot system)
   /if (${Defined[LootINI]}) {
      /declare firstLetter string local ${itemName.Left[1]}
      /declare iniAction string local ${Ini[${LootINI},${firstLetter},${itemName}]}

      /if (${iniAction.Length} && ${iniAction.NotEqual[NULL]}) {
         /varset action ${iniAction}
      } else {
         |- Item not in INI - add it with default KEEP
         /ini "${LootINI}" "${firstLetter}" "${itemName}" "Keep"
         /echo [GZM_Forage] New forage item added to loot INI: ${itemName}

         |- Announce if configured
         /if (${ForageAnnounce.Length}) {
            /docommand ${ForageAnnounce} New forage item: ${itemName}
         }
      }
   }

   |- Process based on action
   /if (${action.Equal[Destroy]}) {
      /echo [GZM_Forage] Destroying: ${itemName}
      /destroy
   } else /if (${action.Equal[Ignore]} || ${action.Equal[Drop]}) {
      /if (${Cursor.NoDrop}) {
         /echo [GZM_Forage] Cannot drop No-Drop item: ${itemName} - destroying
         /destroy
      } else {
         /echo [GZM_Forage] Dropping: ${itemName}
         /drop
      }
   } else /if (${action.Find[Keep]} || ${action.Find[KEEP]}) {
      |- Keep the item
      /echo [GZM_Forage] Keeping: ${itemName}
      /autoinventory
      /delay 2s !${Cursor.ID}

      /if (${Cursor.ID}) {
         /echo [GZM_Forage] No room for ${itemName} - dropping
         /drop
      }
   } else /if (${action.Find[Sell]}) {
      |- Keep for selling
      /echo [GZM_Forage] Keeping to sell: ${itemName}
      /autoinventory
      /delay 2s !${Cursor.ID}
   } else {
      |- Default: keep it
      /echo [GZM_Forage] Foraged: ${itemName} (keeping)
      /autoinventory
      /delay 2s !${Cursor.ID}
   }
/return

|- ============================================================================
|- UTILITY FUNCTIONS
|- ============================================================================

|- Check if character can forage
Sub CanForage
   |- Classes that can forage: Ranger, Druid, Bard, Rogue (with AA)
   /if (${Select[${Me.Class.ShortName},RNG,DRU,BRD,ROG]}) {
      /return ${Me.Ability[forage]}
   }
/return FALSE

|- Get forage status
Sub GetForageStatus
   /echo [GZM_Forage] === Forage Status ===
   /echo DoForage: ${If[${DoForage},ON,OFF]}
   /echo Can Forage: ${If[${Me.Ability[forage]},Yes,No]}
   /echo Forage Ready: ${If[${Me.AbilityReady[forage]},Yes,No]}
   /echo ForageTimer: ${If[${ForageTimer},${ForageTimer},Ready]}
   /echo ========================
/return

|- Load forage settings from INI
Sub LoadForageSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /varset DoForage ${Ini[${IniFile},Forage,DoForage,${DoForage}]}
   /varset ForageAnnounce ${Ini[${IniFile},Forage,ForageAnnounce,${ForageAnnounce}]}

   |- Auto-enable for forage classes
   /if (${Select[${Me.Class.ShortName},RNG,DRU]}) {
      /if (!${Ini[${IniFile},Forage,DoForage].Length}) {
         /varset DoForage TRUE
      }
   }

   /if (${ForageDebug}) /echo [GZM_Forage] Settings loaded from ${IniFile}
/return

|- Save forage settings to INI
Sub SaveForageSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /ini "${IniFile}" "Forage" "DoForage" "${DoForage}"
   /ini "${IniFile}" "Forage" "ForageAnnounce" "${ForageAnnounce}"

   /echo [GZM_Forage] Settings saved to ${IniFile}
/return

