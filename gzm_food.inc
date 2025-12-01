|- GronnzMaster Food/Drink Management Include
|- Auto-summon and consume food/drink
|- Supports summon spells and auto-consume
|- Version 1.0

|- Events for food/drink detection
#Event GMNeedFood "You are low on food."
#Event GMNeedFood "You are out of food."
#Event GMNeedDrink "You are low on drink."
#Event GMNeedDrink "You are out of drink."

Sub InitFoodVars
   |- Food/Drink settings
   /if (!${Defined[SummonFood]}) /declare SummonFood bool outer FALSE
   /if (!${Defined[SummonDrink]}) /declare SummonDrink bool outer FALSE

   |- Summoning spells (format: "SpellName|gem|casttime" or AA name)
   /if (!${Defined[FoodSpell]}) /declare FoodSpell string outer
   /if (!${Defined[DrinkSpell]}) /declare DrinkSpell string outer

   |- Timers to prevent spam (3 minutes)
   /if (!${Defined[NeedFoodTimer]}) /declare NeedFoodTimer timer outer
   /if (!${Defined[NeedDrinkTimer]}) /declare NeedDrinkTimer timer outer

   |- Thresholds
   /if (!${Defined[FoodThreshold]}) /declare FoodThreshold int outer 50
   /if (!${Defined[DrinkThreshold]}) /declare DrinkThreshold int outer 50

   |- Summoned item names (auto-detected or configured)
   /if (!${Defined[SummonedFoodName]}) /declare SummonedFoodName string outer
   /if (!${Defined[SummonedDrinkName]}) /declare SummonedDrinkName string outer

   |- Debug
   /if (!${Defined[FoodDebug]}) /declare FoodDebug bool outer FALSE
/return

|- Event handler for low food
Sub Event_GMNeedFood
   /if (!${SummonFood}) /return
   /if (${NeedFoodTimer}) /return

   /if (${FoodDebug}) /echo [GZM_Food] Food needed event triggered
   /varset NeedFoodTimer 1800
/return

|- Event handler for low drink
Sub Event_GMNeedDrink
   /if (!${SummonDrink}) /return
   /if (${NeedDrinkTimer}) /return

   /if (${FoodDebug}) /echo [GZM_Food] Drink needed event triggered
   /varset NeedDrinkTimer 1800
/return

|- Check food/drink status (call from main loop)
Sub CheckFoodDrink
   |- Skip if invisible or in combat
   /if (${Me.Invis}) /return
   /if (${Defined[InCombat]}) {
      /if (${InCombat}) /return
   }

   |- Skip if mobs nearby
   /if (${SpawnCount[npc radius 50 los]}) /return

   |- Check food timer
   /if (${NeedFoodTimer} && ${SummonFood}) {
      /if (${Me.Hunger}<${FoodThreshold}) {
         /call SummonFoodItem
      }
   }

   |- Check drink timer
   /if (${NeedDrinkTimer} && ${SummonDrink}) {
      /if (${Me.Thirst}<${DrinkThreshold}) {
         /call SummonDrinkItem
      }
   }

   |- Proactive check (not event-driven)
   /if (${Me.Hunger}<${FoodThreshold} && ${SummonFood} && !${NeedFoodTimer}) {
      /varset NeedFoodTimer 1800
      /call SummonFoodItem
   }

   /if (${Me.Thirst}<${DrinkThreshold} && ${SummonDrink} && !${NeedDrinkTimer}) {
      /varset NeedDrinkTimer 1800
      /call SummonDrinkItem
   }
/return

|- Summon food item
Sub SummonFoodItem
   /if (!${FoodSpell.Length}) {
      /if (${FoodDebug}) /echo [GZM_Food] No food spell configured
      /return
   }

   /echo [GZM_Food] Summoning food...

   |- Parse spell config
   /declare spellName string local ${FoodSpell.Arg[1,|]}
   /declare spellGem string local ${FoodSpell.Arg[2,|]}
   /declare castTime string local ${FoodSpell.Arg[3,|]}

   |- Check if it's an AA
   /if (${Me.AltAbility[${spellName}].ID}) {
      /alt activate ${Me.AltAbility[${spellName}].ID}
      /delay 3s
   } else /if (${Me.Book[${spellName}].ID}) {
      |- It's a spell
      /if (!${spellGem.Length}) /varset spellGem 8
      /if (!${castTime.Length}) /varset castTime 5s
      /casting "${spellName}" gem${spellGem} ${castTime}
      /delay ${castTime}
   } else {
      /echo [GZM_Food] Cannot find food spell: ${spellName}
      /return
   }

   |- Clear food timer after successful summon
   /varset NeedFoodTimer 0

   /if (${FoodDebug}) /echo [GZM_Food] Food summoned
/return

|- Summon drink item
Sub SummonDrinkItem
   /if (!${DrinkSpell.Length}) {
      /if (${FoodDebug}) /echo [GZM_Food] No drink spell configured
      /return
   }

   /echo [GZM_Food] Summoning drink...

   |- Parse spell config
   /declare spellName string local ${DrinkSpell.Arg[1,|]}
   /declare spellGem string local ${DrinkSpell.Arg[2,|]}
   /declare castTime string local ${DrinkSpell.Arg[3,|]}

   |- Check if it's an AA
   /if (${Me.AltAbility[${spellName}].ID}) {
      /alt activate ${Me.AltAbility[${spellName}].ID}
      /delay 3s
   } else /if (${Me.Book[${spellName}].ID}) {
      |- It's a spell
      /if (!${spellGem.Length}) /varset spellGem 8
      /if (!${castTime.Length}) /varset castTime 5s
      /casting "${spellName}" gem${spellGem} ${castTime}
      /delay ${castTime}
   } else {
      /echo [GZM_Food] Cannot find drink spell: ${spellName}
      /return
   }

   |- Clear drink timer after successful summon
   /varset NeedDrinkTimer 0

   /if (${FoodDebug}) /echo [GZM_Food] Drink summoned
/return

|- Use food from inventory
Sub UseFood
   /if (${Me.Hunger}>=100) /return

   |- Find food in inventory
   /declare foodSlot int local 0

   |- Check for summoned food first
   /if (${SummonedFoodName.Length}) {
      /varset foodSlot ${FindItem[=${SummonedFoodName}].InvSlot}
   }

   |- Otherwise find any food
   /if (!${foodSlot}) {
      /varset foodSlot ${FindItem[type food].InvSlot}
   }

   /if (${foodSlot}) {
      /itemnotify ${foodSlot} rightmouseup
      /delay 5
      /if (${FoodDebug}) /echo [GZM_Food] Ate food from slot ${foodSlot}
   }
/return

|- Use drink from inventory
Sub UseDrink
   /if (${Me.Thirst}>=100) /return

   |- Find drink in inventory
   /declare drinkSlot int local 0

   |- Check for summoned drink first
   /if (${SummonedDrinkName.Length}) {
      /varset drinkSlot ${FindItem[=${SummonedDrinkName}].InvSlot}
   }

   |- Otherwise find any drink
   /if (!${drinkSlot}) {
      /varset drinkSlot ${FindItem[type drink].InvSlot}
   }

   /if (${drinkSlot}) {
      /itemnotify ${drinkSlot} rightmouseup
      /delay 5
      /if (${FoodDebug}) /echo [GZM_Food] Drank from slot ${drinkSlot}
   }
/return

|- Get food/drink status
Sub GetFoodStatus
   /echo [GZM_Food] === Food/Drink Status ===
   /echo Hunger: ${Me.Hunger}/100 (threshold: ${FoodThreshold})
   /echo Thirst: ${Me.Thirst}/100 (threshold: ${DrinkThreshold})
   /echo Summon Food: ${If[${SummonFood},ON,OFF]} (${FoodSpell})
   /echo Summon Drink: ${If[${SummonDrink},ON,OFF]} (${DrinkSpell})
   /echo Food Timer: ${If[${NeedFoodTimer},${NeedFoodTimer},Ready]}
   /echo Drink Timer: ${If[${NeedDrinkTimer},${NeedDrinkTimer},Ready]}
   /echo ========================
/return

|- Load food settings from INI
Sub LoadFoodSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /varset SummonFood ${Ini[${IniFile},Food,SummonFood,${SummonFood}]}
   /varset SummonDrink ${Ini[${IniFile},Food,SummonDrink,${SummonDrink}]}
   /varset FoodSpell ${Ini[${IniFile},Food,FoodSpell,${FoodSpell}]}
   /varset DrinkSpell ${Ini[${IniFile},Food,DrinkSpell,${DrinkSpell}]}
   /varset FoodThreshold ${Ini[${IniFile},Food,FoodThreshold,${FoodThreshold}]}
   /varset DrinkThreshold ${Ini[${IniFile},Food,DrinkThreshold,${DrinkThreshold}]}
   /varset SummonedFoodName ${Ini[${IniFile},Food,SummonedFoodName,${SummonedFoodName}]}
   /varset SummonedDrinkName ${Ini[${IniFile},Food,SummonedDrinkName,${SummonedDrinkName}]}

   /if (${FoodDebug}) /echo [GZM_Food] Settings loaded from ${IniFile}
/return

|- Save food settings to INI
Sub SaveFoodSettings(IniFile)
   /if (!${IniFile.Length}) /return

   /ini "${IniFile}" "Food" "SummonFood" "${SummonFood}"
   /ini "${IniFile}" "Food" "SummonDrink" "${SummonDrink}"
   /ini "${IniFile}" "Food" "FoodSpell" "${FoodSpell}"
   /ini "${IniFile}" "Food" "DrinkSpell" "${DrinkSpell}"
   /ini "${IniFile}" "Food" "FoodThreshold" "${FoodThreshold}"
   /ini "${IniFile}" "Food" "DrinkThreshold" "${DrinkThreshold}"
   /ini "${IniFile}" "Food" "SummonedFoodName" "${SummonedFoodName}"
   /ini "${IniFile}" "Food" "SummonedDrinkName" "${SummonedDrinkName}"

   /echo [GZM_Food] Settings saved to ${IniFile}
/return
