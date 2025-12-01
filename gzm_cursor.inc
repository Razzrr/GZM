|- GronnzMaster Cursor & Stuck Gem Management Include
|- Handles stuck cursor items and gem recovery
|- Prevents inventory lockups and stuck spell gems
|- Version 1.0

Sub InitCursorVars
   |- Stuck Gems Settings
   /if (!${Defined[DoStuckGems]}) /declare DoStuckGems bool outer TRUE
   /if (!${Defined[StuckGemsAA]}) /declare StuckGemsAA string outer

   |- Stuck Gem Tracking
   /if (!${Defined[GemStuckTimer]}) /declare GemStuckTimer timer outer
   /if (!${Defined[GemStuckID]}) /declare GemStuckID int outer 0
   /if (!${Defined[GemStuckRetry]}) /declare GemStuckRetry timer outer

   |- Cursor Management
   /if (!${Defined[CursorCheckTimer]}) /declare CursorCheckTimer timer outer

   |- Debug
   /if (!${Defined[CursorDebug]}) /declare CursorDebug bool outer FALSE
/return

|- ============================================================================
|- CURSOR MANAGEMENT
|- Clear items stuck on cursor safely
|- ============================================================================

|- Clear cursor with optional force parameter
|- Returns TRUE if cursor cleared, FALSE if couldn't clear
Sub ClearCursor(ccParm)
   /if (!${Cursor.ID}) /return TRUE

   |- Skip if in trade/combine windows unless forced
   /if (${Window[InventoryWindow].Open} || ${Window[BigBankWnd].Open}) {
      /if (!${ccParm.Find[force]}) {
         /if (${CursorDebug}) /echo [GZM_Cursor] Skipping clear - window open
         /return FALSE
      }
   }

   /declare x int local
   /declare tTimer timer local 100
   /declare origCursorID int local ${Cursor.ID}

   /while (${Cursor.ID}) {
      |- If it's a container, try to place in empty pack slot
      /if (${Cursor.Items}) {
         /for x 1 to ${Me.NumBagSlots}
            /if (!${Me.Inventory[pack${x}].ID}) {
               /nomodkey /itemnotify pack${x} leftmouseup
               /delay 5
               /break
            }
         /next x
         |- If still on cursor (no empty slot), can't clear
         /if (${Cursor.Container}) /break
      } else {
         |- Regular item - autoinventory it
         /if (${CursorDebug}) /echo [GZM_Cursor] Auto-storing: ${Cursor.Name}
         /autoinventory
         /delay 2s !${Cursor.ID}
         /delay 5
      }
      /delay 2
      /if (!${tTimer}) /break
   }

   /if (${Cursor.ID}) {
      /echo [GZM_Cursor] Unable to clear cursor - ${Cursor.Name}
      /return FALSE
   }

   /if (${CursorDebug} && ${origCursorID}) /echo [GZM_Cursor] Cursor cleared
/return TRUE

|- Quick cursor check - call frequently in main loop
Sub CheckCursor
   /if (!${Cursor.ID}) /return
   /if (${CursorCheckTimer}) /return

   /varset CursorCheckTimer 5s
   /call ClearCursor
/return

|- ============================================================================
|- STUCK GEM DETECTION AND RECOVERY
|- Detects when spell gems are stuck and attempts recovery
|- ============================================================================

|- Check for stuck gems - call after any spell cast attempt
Sub StuckGemsCheck
   /if (!${DoStuckGems}) /return

   |- Track casting state changes
   /if (!${Me.Casting.ID} || !${GemStuckID} || ${Me.Casting.ID}!=${GemStuckID}) {
      |- Reset timer when casting state changes
      /varset GemStuckTimer 25s
      /varset GemStuckID ${Me.Casting.ID}
   } else /if (!${GemStuckTimer}) {
      |- Timer expired while still casting same spell = stuck
      /if (${CursorDebug}) {
         /echo [GZM_Cursor] Stuck check - Casting: ${Me.Casting.ID} Timer expired
      }
      /call StuckGems
   }
/return

|- Attempt to unstick spell gems
Sub StuckGems
   /if (!${Me.Casting.ID}) /return

   /if (!${DoStuckGems}) {
      /if (!${GemStuckRetry}) {
         /echo [GZM_Cursor] Spell gems may be stuck, but DoStuckGems is FALSE
         /varset GemStuckRetry 3s
      }
      /return
   }

   /if (!${GemStuckRetry}) {
      /beep
      /echo [GZM_Cursor] *** SPELL GEMS MAY BE STUCK ***

      |- Broadcast to group
      /if (${Plugin[MQ2DanNet].IsLoaded}) {
         /dgt Warning: ${Me.CleanName}'s spell gems may be stuck!
      }

      /delay 1s

      |- Try to unstick using configured AA
      /if (${StuckGemsAA.Length}>2) {
         /if (${Me.AltAbilityReady[${StuckGemsAA}]}) {
            /echo [GZM_Cursor] Attempting to unstick with ${StuckGemsAA}
            /target id ${Me.ID}
            /delay 5
            /alt activate ${Me.AltAbility[${StuckGemsAA}].ID}
            /delay 5s !${Me.Casting.ID}

            /if (!${Me.Casting.ID}) {
               /echo [GZM_Cursor] Successfully unstuck gems
            } else {
               /echo [GZM_Cursor] Unstick attempt failed - still casting
            }
         } else {
            /echo [GZM_Cursor] Waiting for ${StuckGemsAA} to be ready
         }
      } else {
         /echo [GZM_Cursor] No StuckGemsAA configured - trying /stopcast
         /stopcast
         /delay 2s !${Me.Casting.ID}

         /if (!${Me.Casting.ID}) {
            /echo [GZM_Cursor] /stopcast worked
         } else {
            /echo [GZM_Cursor] Cannot unstick - configure StuckGemsAA in INI
         }
      }

      /varset GemStuckRetry 2s
   }
/return

|- ============================================================================
|- UTILITY FUNCTIONS
|- ============================================================================

|- Pick up item from inventory by ID and slot
Sub PickUpItem(itemID, itemSlot, itemSlot2, stackCount)
   /if (!${itemID}) /return FALSE

   |- Clear cursor first
   /if (${Cursor.ID}) {
      /call ClearCursor force
      /if (!${Macro.Return}) /return FALSE
   }

   /declare tPack string local pack${Math.Calc[${itemSlot}-22].Int}
   /declare tSlot int local ${If[${itemSlot2}>=0,${Math.Calc[${itemSlot2}+1].Int},0]}

   |- Pick up the item
   /nomodkey /itemnotify ${If[${tSlot},in ,]}${tPack} ${If[${tSlot},${tSlot} ,]}leftmouseup
   /delay 2s ${Cursor.ID}==${itemID}

   /if (${Cursor.ID}==${itemID}) {
      /if (${CursorDebug}) /echo [GZM_Cursor] Picked up: ${Cursor.Name}
      /return TRUE
   }

   /echo [GZM_Cursor] Failed to pick up item ID ${itemID}
/return FALSE

|- Give item on cursor to target
Sub GiveItem(targetID, itemCount)
   /if (!${Cursor.ID}) /return FALSE
   /if (!${targetID}) /return FALSE

   /declare cursorItem string local ${Cursor.Name}
   /declare tSpawn int local ${Spawn[id ${targetID}].ID}

   /if (!${tSpawn}) {
      /echo [GZM_Cursor] Give target not found
      /return FALSE
   }

   |- Target the recipient
   /target id ${targetID}
   /delay 2s ${Target.ID}==${targetID}

   /if (${Target.ID}!=${targetID}) {
      /echo [GZM_Cursor] Could not target give recipient
      /return FALSE
   }

   |- Open trade window
   /click left target
   /delay 3s ${Window[GiveWnd].Open} || ${Window[TradeWnd].Open}

   /if (${Window[GiveWnd].Open}) {
      |- NPC give window
      /nomodkey /notify GiveWnd GVW_Give_Button leftmouseup
      /delay 2s !${Cursor.ID}
   } else /if (${Window[TradeWnd].Open}) {
      |- Player/Pet trade window - put item in slot
      /nomodkey /itemnotify in TradeWnd TWSlot0 leftmouseup
      /delay 5
      /nomodkey /notify TradeWnd TRDW_Trade_Button leftmouseup
      /delay 3s !${Window[TradeWnd].Open}
   } else {
      /echo [GZM_Cursor] Could not open give/trade window
      /return FALSE
   }

   /if (!${Cursor.ID}) {
      /if (${CursorDebug}) /echo [GZM_Cursor] Successfully gave ${cursorItem}
      /return TRUE
   }

   /echo [GZM_Cursor] Give failed - item still on cursor
/return FALSE

|- Destroy summoned/temporary item on cursor
Sub DestroyCursorItem
   /if (!${Cursor.ID}) /return

   |- Only destroy no-rent (summoned) items
   /if (${Cursor.NoRent}) {
      /echo [GZM_Cursor] Destroying summoned item: ${Cursor.Name}
      /destroy
      /delay 10
   } else {
      /echo [GZM_Cursor] Will not destroy ${Cursor.Name} - not summoned
   }
/return

|- Load cursor settings from INI
Sub LoadCursorSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /varset DoStuckGems ${Ini[${IniFile},Cursor,DoStuckGems,${DoStuckGems}]}
   /varset StuckGemsAA ${Ini[${IniFile},Cursor,StuckGemsAA,${StuckGemsAA}]}

   |- Auto-detect StuckGemsAA if not set
   /if (!${StuckGemsAA.Length} || ${StuckGemsAA.Equal[ ]}) {
      /call DetectStuckGemsAA
   }

   /if (${CursorDebug}) /echo [GZM_Cursor] Settings loaded from ${IniFile}
/return

|- Save cursor settings to INI
Sub SaveCursorSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /ini "${IniFile}" "Cursor" "DoStuckGems" "${DoStuckGems}"
   /ini "${IniFile}" "Cursor" "StuckGemsAA" "${StuckGemsAA}"

   /echo [GZM_Cursor] Settings saved to ${IniFile}
/return

|- Auto-detect best AA for unsticking gems
Sub DetectStuckGemsAA
   |- Check for common instant-cast AAs that can interrupt stuck gems
   /if (${Me.AltAbility[Origin].ID}) {
      |- Origin is instant and self-targeted - good option
      /varset StuckGemsAA Origin
   } else /if (${Me.AltAbility[Throne of Heroes].ID}) {
      /varset StuckGemsAA Throne of Heroes
   } else /if (${Me.AltAbility[Armor of Experience].ID}) {
      /varset StuckGemsAA Armor of Experience
   }

   /if (${StuckGemsAA.Length}>2) {
      /echo [GZM_Cursor] Auto-detected StuckGemsAA: ${StuckGemsAA}
   }
/return

|- Get cursor status report
Sub GetCursorStatus
   /echo [GZM_Cursor] === Cursor Status ===
   /echo Cursor: ${If[${Cursor.ID},${Cursor.Name} (ID: ${Cursor.ID}),Empty]}
   /echo DoStuckGems: ${If[${DoStuckGems},ON,OFF]}
   /echo StuckGemsAA: ${If[${StuckGemsAA.Length}>2,${StuckGemsAA},Not Set]}
   /echo Currently Casting: ${If[${Me.Casting.ID},${Me.Casting} (${Me.Casting.ID}),No]}
   /echo GemStuckTimer: ${If[${GemStuckTimer},${GemStuckTimer},Expired/Ready]}
   /echo ========================
/return

