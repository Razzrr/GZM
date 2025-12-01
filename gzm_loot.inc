|- GronnzMaster Enhanced Loot Management Include
|- INI-based item routing with categories
|- Works alongside ninjadvloot.inc
|- Supports category-based item routing
|- Version 1.0

Sub InitLootVars
   |- Loot settings
   /if (!${Defined[LootRadius]}) /declare LootRadius int outer 50
   /if (!${Defined[LootDelay]}) /declare LootDelay int outer 3
   /if (!${Defined[AutoLearnSpells]}) /declare AutoLearnSpells bool outer TRUE
   /if (!${Defined[ConsolidateStacks]}) /declare ConsolidateStacks bool outer TRUE
   /if (!${Defined[CheckInventorySpace]}) /declare CheckInventorySpace bool outer TRUE

   |- Loot INI file
   /if (!${Defined[LootIniFile]}) /declare LootIniFile string outer ${MacroQuest.Path}\config\GZM_Loot_${Me.CleanName}.ini

   |- Loot actions: Keep, Sell, Destroy, Ignore, Quest
   |- Categories: Tradeskill, Spellscroll, Gem, Ore, etc.

   |- Item action tracking (runtime)
   /if (!${Defined[ItemActions]}) /declare ItemActions[200] string outer

   |- Category defaults
   /if (!${Defined[CategoryActions]}) /declare CategoryActions[20] string outer

   |- Loot statistics
   /if (!${Defined[LootedCount]}) /declare LootedCount int outer 0
   /if (!${Defined[DestroyedCount]}) /declare DestroyedCount int outer 0
   /if (!${Defined[SoldCount]}) /declare SoldCount int outer 0

   |- Debug
   /if (!${Defined[LootDebug]}) /declare LootDebug bool outer FALSE

   |- Load settings
   /call LoadLootSettings
/return

|- Get action for an item (Keep|Sell|Destroy|Ignore|Quest)
Sub GetItemAction(itemName)
   /declare i int local
   /declare action string local

   |- First check specific item settings
   /varset action ${Ini[${LootIniFile},Items,${itemName}]}
   /if (${action.Length} && ${action.NotEqual[NULL]}) {
      /if (${LootDebug}) /echo [GZM_Loot] Item action for ${itemName}: ${action}
      /return ${action}
   }

   |- Check category defaults based on item type
   /if (${FindItem[=${itemName}].Type.Find[Scroll]}) {
      /varset action ${Ini[${LootIniFile},Categories,Spellscroll,Keep]}
      /if (${action.Length} && ${action.NotEqual[NULL]}) /return ${action}
   }

   /if (${FindItem[=${itemName}].Type.Find[Tradeskill]}) {
      /varset action ${Ini[${LootIniFile},Categories,Tradeskill,Keep]}
      /if (${action.Length} && ${action.NotEqual[NULL]}) /return ${action}
   }

   |- Check value-based defaults
   /declare itemValue int local ${FindItem[=${itemName}].Value}
   /if (${itemValue}>=10000) {
      /varset action ${Ini[${LootIniFile},Categories,Valuable,Keep]}
      /if (${action.Length} && ${action.NotEqual[NULL]}) /return ${action}
   }

   |- Default action
   /varset action ${Ini[${LootIniFile},Settings,DefaultAction,Keep]}
/return ${action}

|- Set action for an item
Sub SetItemAction(itemName, action)
   /ini "${LootIniFile}" "Items" "${itemName}" "${action}"
   /echo [GZM_Loot] Set ${itemName} -> ${action}
/return

|- Process a looted item based on its action
Sub ProcessLootedItem(itemName)
   /declare action string local
   /call GetItemAction "${itemName}"
   /varset action ${Macro.Return}

   /if (${LootDebug}) /echo [GZM_Loot] Processing ${itemName}: ${action}

   /if (${action.Equal[Destroy]}) {
      /call DestroyItem "${itemName}"
   } else /if (${action.Equal[Sell]}) {
      |- Mark for sale (actual selling happens at vendor)
      /if (${Defined[AddSellItem]}) /call AddSellItem "${itemName}"
   } else /if (${action.Equal[Learn]} && ${AutoLearnSpells}) {
      /call LearnSpell "${itemName}"
   }
   |- Keep and Ignore items are left alone

   /varcalc LootedCount ${LootedCount}+1
/return

|- Destroy an item
Sub DestroyItem(itemName)
   /declare itemSlot int local ${FindItem[=${itemName}].InvSlot}
   /if (!${itemSlot}) {
      /if (${LootDebug}) /echo [GZM_Loot] Item not found to destroy: ${itemName}
      /return FALSE
   }

   /if (${LootDebug}) /echo [GZM_Loot] Destroying ${itemName}

   |- Pick up item
   /itemnotify ${itemSlot} leftmouseup
   /delay 5 ${Cursor.ID}

   /if (${Cursor.ID}) {
      |- Destroy it
      /destroy
      /delay 5 !${Cursor.ID}
      /varcalc DestroyedCount ${DestroyedCount}+1
      /return TRUE
   }
/return FALSE

|- Learn a spell scroll
Sub LearnSpell(spellName)
   /if (!${AutoLearnSpells}) /return FALSE

   /declare itemSlot int local ${FindItem[=${spellName}].InvSlot}
   /if (!${itemSlot}) /return FALSE

   |- Check if it's a spell scroll
   /if (!${FindItem[=${spellName}].Type.Find[Scroll]}) /return FALSE

   |- Check if we can scribe it
   /if (${Me.Book[${spellName}]}) {
      /if (${LootDebug}) /echo [GZM_Loot] Already know spell: ${spellName}
      /return FALSE
   }

   /echo [GZM_Loot] Learning spell: ${spellName}

   |- Right-click to scribe
   /itemnotify ${itemSlot} rightmouseup
   /delay 3s ${Me.Book[${spellName}]}

   /if (${Me.Book[${spellName}]}) {
      /echo [GZM_Loot] Successfully learned: ${spellName}
      /return TRUE
   }
/return FALSE

|- Check available inventory space
Sub CheckInvSpace
   /declare freeSlots int local 0
   /declare i int local

   |- Count free inventory slots
   /for i 1 to 10
      /if (!${Me.Inventory[pack${i}].ID}) {
         |- Empty bag slot
         /varcalc freeSlots ${freeSlots}+1
      } else /if (${Me.Inventory[pack${i}].Container}) {
         |- Check slots in bag
         /declare j int local
         /declare bagSlots int local ${Me.Inventory[pack${i}].Container}
         /for j 1 to ${bagSlots}
            /if (!${Me.Inventory[pack${i}].Item[${j}].ID}) {
               /varcalc freeSlots ${freeSlots}+1
            }
         /next j
      }
   /next i

/return ${freeSlots}

|- Should we loot this corpse?
Sub ShouldLootCorpse(corpseID)
   |- Check distance
   /if (${Spawn[${corpseID}].Distance}>${LootRadius}) /return FALSE

   |- Check if it's a valid corpse
   /if (!${Spawn[${corpseID}].Type.Equal[Corpse]}) /return FALSE

   |- Check if it's an NPC corpse (not player)
   /if (${Spawn[${corpseID}].Deity.ID}) /return FALSE

   |- Check inventory space
   /if (${CheckInventorySpace}) {
      /call CheckInvSpace
      /if (${Macro.Return}<2) {
         /if (${LootDebug}) /echo [GZM_Loot] Low inventory space, skipping corpse
         /return FALSE
      }
   }

/return TRUE

|- Consolidate item stacks in inventory
Sub ConsolidateStacks
   /if (!${ConsolidateStacks}) /return

   /if (${LootDebug}) /echo [GZM_Loot] Consolidating stacks...

   |- Use /autoinventory on any items that can stack
   /declare i int local
   /declare j int local

   /for i 1 to 10
      /if (${Me.Inventory[pack${i}].Container}) {
         /declare bagSlots int local ${Me.Inventory[pack${i}].Container}
         /for j 1 to ${bagSlots}
            /if (${Me.Inventory[pack${i}].Item[${j}].Stackable}) {
               /if (${Me.Inventory[pack${i}].Item[${j}].StackCount}<${Me.Inventory[pack${i}].Item[${j}].StackSize}) {
                  |- This stack isn't full, try to merge
                  /itemnotify in pack${i} ${j} leftmouseup
                  /delay 3 ${Cursor.ID}
                  /if (${Cursor.ID}) {
                     /autoinventory
                     /delay 3 !${Cursor.ID}
                  }
               }
            }
         /next j
      }
   /next i
/return

|- Quick add commonly destroyed items
Sub AddJunkItems
   |- Common junk items to destroy
   /call SetItemAction "Rusty Sword" Destroy
   /call SetItemAction "Rusty Axe" Destroy
   /call SetItemAction "Rusty Dagger" Destroy
   /call SetItemAction "Tattered Cloth" Destroy
   /call SetItemAction "Cracked Staff" Destroy
   /call SetItemAction "Raw-hide Tunic" Destroy
   /call SetItemAction "Leather Gloves" Destroy
   /echo [GZM_Loot] Added common junk items to destroy list
/return

|- Quick add commonly kept items
Sub AddKeepItems
   |- Common items to keep
   /call SetItemAction "Diamond" Keep
   /call SetItemAction "Black Sapphire" Keep
   /call SetItemAction "Blue Diamond" Keep
   /call SetItemAction "Kronos" Keep
   /echo [GZM_Loot] Added common keep items
/return

|- Load loot settings from INI
Sub LoadLootSettings
   /if (!${Ini[${LootIniFile}].Length}) {
      /if (${LootDebug}) /echo [GZM_Loot] No loot INI found, creating defaults
      /call SaveLootSettings
      /return
   }

   |- Load settings
   /varset LootRadius ${Ini[${LootIniFile},Settings,LootRadius,${LootRadius}]}
   /varset LootDelay ${Ini[${LootIniFile},Settings,LootDelay,${LootDelay}]}
   /varset AutoLearnSpells ${Ini[${LootIniFile},Settings,AutoLearnSpells,${AutoLearnSpells}]}
   /varset ConsolidateStacks ${Ini[${LootIniFile},Settings,ConsolidateStacks,${ConsolidateStacks}]}
   /varset CheckInventorySpace ${Ini[${LootIniFile},Settings,CheckInventorySpace,${CheckInventorySpace}]}

   /if (${LootDebug}) /echo [GZM_Loot] Settings loaded from ${LootIniFile}
/return

|- Save loot settings to INI
Sub SaveLootSettings
   /ini "${LootIniFile}" "Settings" "LootRadius" "${LootRadius}"
   /ini "${LootIniFile}" "Settings" "LootDelay" "${LootDelay}"
   /ini "${LootIniFile}" "Settings" "AutoLearnSpells" "${AutoLearnSpells}"
   /ini "${LootIniFile}" "Settings" "ConsolidateStacks" "${ConsolidateStacks}"
   /ini "${LootIniFile}" "Settings" "CheckInventorySpace" "${CheckInventorySpace}"
   /ini "${LootIniFile}" "Settings" "DefaultAction" "Keep"

   |- Save category defaults
   /ini "${LootIniFile}" "Categories" "Spellscroll" "Keep"
   /ini "${LootIniFile}" "Categories" "Tradeskill" "Keep"
   /ini "${LootIniFile}" "Categories" "Valuable" "Keep"
   /ini "${LootIniFile}" "Categories" "Gem" "Keep"
   /ini "${LootIniFile}" "Categories" "Ore" "Keep"

   /echo [GZM_Loot] Settings saved to ${LootIniFile}
/return

|- Show loot statistics
Sub ShowLootStats
   /echo [GZM_Loot] === Loot Statistics ===
   /echo Items looted: ${LootedCount}
   /echo Items destroyed: ${DestroyedCount}
   /echo Items sold: ${SoldCount}
   /echo Free inventory slots: ${Macro.Return}
   /call CheckInvSpace
   /echo Free slots: ${Macro.Return}
   /echo ========================
/return

|- Reset loot statistics
Sub ResetLootStats
   /varset LootedCount 0
   /varset DestroyedCount 0
   /varset SoldCount 0
   /echo [GZM_Loot] Statistics reset
/return
