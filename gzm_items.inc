|**
    GZM_items.inc - GronnzMaster Item Click System

    Comprehensive item clicking for:
    - Combat clicks (during combat rotation)
    - Buff clicks (self buffs with duration tracking)
    - DoT clicks (apply DoT to target)
    - Pet buff clicks (buff pet with item)
    - Worn clicks (equip then click via MQ2Exchange)
    - Burn clicks (during burn phase)
    - Instant clicks (no target needed)

    Class-specific examples:
    - Monk: Celestial Fists (every 18s), Epic 2.0 (3min CD)
    - Shaman: Epic 1.0 DoT, Epic 2.0 group buff
    - Mage: Epic 2.0 pet proc buff
    - Necro: Epic worn focus (passive)
    - All classes: Aug clicks, gear clicks, etc.

    Requires: MQ2Exchange for worn item swapping
**|

| ============================================================================
| ITEM CLICK CONSTANTS
| ============================================================================
#DEFINE MAXCOMBATCLICKS 10
#DEFINE MAXBUFFCLICKS 10
#DEFINE MAXDOTCLICKS 5
#DEFINE MAXPETCLICKS 5
#DEFINE MAXWORNCLICKS 5
#DEFINE MAXBURNCLICKS 10
#DEFINE MAXINSTANTCLICKS 5

| ============================================================================
| SUB: Initialize Item System
| ============================================================================
Sub InitItemSystem
    /declare i int local

    | Combat click arrays (clicked during combat rotation)
    /declare CombatClickItem[${MAXCOMBATCLICKS}] string outer
    /declare CombatClickTimer[${MAXCOMBATCLICKS}] timer outer
    /declare CombatClickRecast[${MAXCOMBATCLICKS}] int outer
    /declare CombatClickMinEnd[${MAXCOMBATCLICKS}] int outer
    /declare CombatClickCount int outer 0

    | Buff click arrays (self buffs with duration)
    /declare BuffClickItem[${MAXBUFFCLICKS}] string outer
    /declare BuffClickTimer[${MAXBUFFCLICKS}] timer outer
    /declare BuffClickDuration[${MAXBUFFCLICKS}] int outer
    /declare BuffClickRecast[${MAXBUFFCLICKS}] int outer
    /declare BuffClickSpell[${MAXBUFFCLICKS}] string outer
    /declare BuffClickCount int outer 0

    | DoT click arrays (apply DoT to target)
    /declare DoTClickItem[${MAXDOTCLICKS}] string outer
    /declare DoTClickTimer[${MAXDOTCLICKS}] timer outer
    /declare DoTClickRecast[${MAXDOTCLICKS}] int outer
    /declare DoTClickMinHP[${MAXDOTCLICKS}] int outer
    /declare DoTClickCount int outer 0

    | Pet buff click arrays
    /declare PetClickItem[${MAXPETCLICKS}] string outer
    /declare PetClickTimer[${MAXPETCLICKS}] timer outer
    /declare PetClickDuration[${MAXPETCLICKS}] int outer
    /declare PetClickRecast[${MAXPETCLICKS}] int outer
    /declare PetClickCount int outer 0

    | Worn click arrays (must equip via MQ2Exchange)
    /declare WornClickItem[${MAXWORNCLICKS}] string outer
    /declare WornClickSlot[${MAXWORNCLICKS}] string outer
    /declare WornClickTimer[${MAXWORNCLICKS}] timer outer
    /declare WornClickRecast[${MAXWORNCLICKS}] int outer
    /declare WornClickSwapBack[${MAXWORNCLICKS}] string outer
    /declare WornClickCount int outer 0

    | Burn click arrays
    /declare BurnClickItem[${MAXBURNCLICKS}] string outer
    /declare BurnClickTimer[${MAXBURNCLICKS}] timer outer
    /declare BurnClickCount int outer 0

    | Instant click arrays (no target needed, not combat dependent)
    /declare InstantClickItem[${MAXINSTANTCLICKS}] string outer
    /declare InstantClickTimer[${MAXINSTANTCLICKS}] timer outer
    /declare InstantClickRecast[${MAXINSTANTCLICKS}] int outer
    /declare InstantClickCombatOnly[${MAXINSTANTCLICKS}] int outer
    /declare InstantClickCount int outer 0

    | Global settings
    /declare ItemClicksOn int outer 1
    /declare UseExchange int outer 1
    /declare ClickDelay int outer 5

    | Load settings
    /call LoadItemSettings

    /echo \ag[GZM-Items] Item system initialized - ${CombatClickCount} combat, ${BuffClickCount} buff, ${DoTClickCount} DoT clicks loaded
/return

| ============================================================================
| SUB: Load Item Settings from INI
| ============================================================================
Sub LoadItemSettings
    /declare i int local
    /declare item string local

    | Load combat clicks
    /for i 1 to ${MAXCOMBATCLICKS}
        /varset item ${Ini[${IniFileName},CombatClicks,CombatClick${i}]}
        /if (${item.Length} && ${item.NotEqual[NULL]}) {
            /varset CombatClickItem[${i}] ${item}
            /varset CombatClickRecast[${i}] ${Ini[${IniFileName},CombatClicks,CombatClick${i}Recast,180]}
            /varset CombatClickMinEnd[${i}] ${Ini[${IniFileName},CombatClicks,CombatClick${i}MinEnd,0]}
            /varcalc CombatClickCount ${CombatClickCount}+1
            GZMDEBUG_ITEM Loaded combat click ${i}: ${item} (${CombatClickRecast[${i}]}s recast)
        }
    /next i

    | Load buff clicks
    /for i 1 to ${MAXBUFFCLICKS}
        /varset item ${Ini[${IniFileName},BuffClicks,BuffClick${i}]}
        /if (${item.Length} && ${item.NotEqual[NULL]}) {
            /varset BuffClickItem[${i}] ${item}
            /varset BuffClickDuration[${i}] ${Ini[${IniFileName},BuffClicks,BuffClick${i}Duration,1800]}
            /varset BuffClickRecast[${i}] ${Ini[${IniFileName},BuffClicks,BuffClick${i}Recast,0]}
            /varset BuffClickSpell[${i}] ${Ini[${IniFileName},BuffClicks,BuffClick${i}Spell]}
            /varcalc BuffClickCount ${BuffClickCount}+1
            GZMDEBUG_ITEM Loaded buff click ${i}: ${item} (${BuffClickDuration[${i}]}s duration)
        }
    /next i

    | Load DoT clicks
    /for i 1 to ${MAXDOTCLICKS}
        /varset item ${Ini[${IniFileName},DoTClicks,DoTClick${i}]}
        /if (${item.Length} && ${item.NotEqual[NULL]}) {
            /varset DoTClickItem[${i}] ${item}
            /varset DoTClickRecast[${i}] ${Ini[${IniFileName},DoTClicks,DoTClick${i}Recast,60]}
            /varset DoTClickMinHP[${i}] ${Ini[${IniFileName},DoTClicks,DoTClick${i}MinHP,20]}
            /varcalc DoTClickCount ${DoTClickCount}+1
        }
    /next i

    | Load pet buff clicks
    /for i 1 to ${MAXPETCLICKS}
        /varset item ${Ini[${IniFileName},PetClicks,PetClick${i}]}
        /if (${item.Length} && ${item.NotEqual[NULL]}) {
            /varset PetClickItem[${i}] ${item}
            /varset PetClickDuration[${i}] ${Ini[${IniFileName},PetClicks,PetClick${i}Duration,600]}
            /varset PetClickRecast[${i}] ${Ini[${IniFileName},PetClicks,PetClick${i}Recast,300]}
            /varcalc PetClickCount ${PetClickCount}+1
        }
    /next i

    | Load worn clicks (require MQ2Exchange)
    /for i 1 to ${MAXWORNCLICKS}
        /varset item ${Ini[${IniFileName},WornClicks,WornClick${i}]}
        /if (${item.Length} && ${item.NotEqual[NULL]}) {
            /varset WornClickItem[${i}] ${item}
            /varset WornClickSlot[${i}] ${Ini[${IniFileName},WornClicks,WornClick${i}Slot,hands]}
            /varset WornClickRecast[${i}] ${Ini[${IniFileName},WornClicks,WornClick${i}Recast,180]}
            /varset WornClickSwapBack[${i}] ${Ini[${IniFileName},WornClicks,WornClick${i}SwapBack]}
            /varcalc WornClickCount ${WornClickCount}+1
            GZMDEBUG_ITEM Loaded worn click ${i}: ${item} (slot: ${WornClickSlot[${i}]})
        }
    /next i

    | Load burn clicks
    /for i 1 to ${MAXBURNCLICKS}
        /varset item ${Ini[${IniFileName},BurnClicks,BurnClick${i}]}
        /if (${item.Length} && ${item.NotEqual[NULL]}) {
            /varset BurnClickItem[${i}] ${item}
            /varcalc BurnClickCount ${BurnClickCount}+1
        }
    /next i

    | Load instant clicks
    /for i 1 to ${MAXINSTANTCLICKS}
        /varset item ${Ini[${IniFileName},InstantClicks,InstantClick${i}]}
        /if (${item.Length} && ${item.NotEqual[NULL]}) {
            /varset InstantClickItem[${i}] ${item}
            /varset InstantClickRecast[${i}] ${Ini[${IniFileName},InstantClicks,InstantClick${i}Recast,0]}
            /varset InstantClickCombatOnly[${i}] ${Ini[${IniFileName},InstantClicks,InstantClick${i}CombatOnly,0]}
            /varcalc InstantClickCount ${InstantClickCount}+1
        }
    /next i

    | Global settings
    /varset ItemClicksOn ${Ini[${IniFileName},Items,ItemClicksOn,1]}
    /varset UseExchange ${Ini[${IniFileName},Items,UseExchange,1]}
/return

| ============================================================================
| SUB: Check Combat Clicks (called during combat)
| ============================================================================
Sub CheckCombatClicks
    /if (!${ItemClicksOn}) /return
    /if (!${CombatClickCount}) /return
    /if (!${InCombat}) /return
    /if (${Me.Casting.ID}) /return

    /declare i int local

    /for i 1 to ${CombatClickCount}
        /if (!${CombatClickItem[${i}].Length}) /continue

        | Check timer
        /if (${CombatClickTimer[${i}]}) /continue

        | Check endurance requirement
        /if (${CombatClickMinEnd[${i}]} && ${Me.PctEndurance} < ${CombatClickMinEnd[${i}]}) /continue

        | Check if item exists and is ready
        /if (${FindItem[=${CombatClickItem[${i}]}].ID}) {
            /if (${FindItem[=${CombatClickItem[${i}]}].TimerReady} == 0) {
                GZMDEBUG_ITEM Combat clicking: ${CombatClickItem[${i}]}
                /call ClickItem "${CombatClickItem[${i}]}"
                /varset CombatClickTimer[${i}] ${CombatClickRecast[${i}]}0
                /return
            }
        }
    /next i
/return

| ============================================================================
| SUB: Check Buff Clicks (maintain self buffs)
| ============================================================================
Sub CheckBuffClicks
    /if (!${ItemClicksOn}) /return
    /if (!${BuffClickCount}) /return
    /if (${Me.Casting.ID}) /return

    /declare i int local
    /declare buffName string local

    /for i 1 to ${BuffClickCount}
        /if (!${BuffClickItem[${i}].Length}) /continue

        | Check if buff is already on
        /if (${BuffClickSpell[${i}].Length}) {
            /varset buffName ${BuffClickSpell[${i}]}
            /if (${Me.Buff[${buffName}].ID}) /continue
        }

        | Check duration timer (if using timer instead of buff check)
        /if (${BuffClickTimer[${i}]}) /continue

        | Check if item exists and is ready
        /if (${FindItem[=${BuffClickItem[${i}]}].ID}) {
            /if (${FindItem[=${BuffClickItem[${i}]}].TimerReady} == 0) {
                GZMDEBUG_ITEM Buff clicking: ${BuffClickItem[${i}]}
                /call ClickItem "${BuffClickItem[${i}]}"
                /varset BuffClickTimer[${i}] ${BuffClickDuration[${i}]}0
                /return
            }
        }
    /next i
/return

| ============================================================================
| SUB: Check DoT Clicks (apply DoT to target)
| ============================================================================
Sub CheckDoTClicks
    /if (!${ItemClicksOn}) /return
    /if (!${DoTClickCount}) /return
    /if (!${InCombat}) /return
    /if (!${Target.ID}) /return
    /if (${Me.Casting.ID}) /return

    /declare i int local

    /for i 1 to ${DoTClickCount}
        /if (!${DoTClickItem[${i}].Length}) /continue

        | Check timer
        /if (${DoTClickTimer[${i}]}) /continue

        | Check target HP (don't waste on low HP)
        /if (${Target.PctHPs} < ${DoTClickMinHP[${i}]}) /continue

        | Check if item exists and is ready
        /if (${FindItem[=${DoTClickItem[${i}]}].ID}) {
            /if (${FindItem[=${DoTClickItem[${i}]}].TimerReady} == 0) {
                GZMDEBUG_ITEM DoT clicking: ${DoTClickItem[${i}]} on ${Target.CleanName}
                /call ClickItem "${DoTClickItem[${i}]}"
                /varset DoTClickTimer[${i}] ${DoTClickRecast[${i}]}0
                /return
            }
        }
    /next i
/return

| ============================================================================
| SUB: Check Pet Clicks (buff pet with item)
| ============================================================================
Sub CheckPetClicks
    /if (!${ItemClicksOn}) /return
    /if (!${PetClickCount}) /return
    /if (!${Me.Pet.ID}) /return
    /if (${Me.Casting.ID}) /return

    /declare i int local
    /declare oldTarget int local ${Target.ID}

    /for i 1 to ${PetClickCount}
        /if (!${PetClickItem[${i}].Length}) /continue

        | Check timer
        /if (${PetClickTimer[${i}]}) /continue

        | Check if item exists and is ready
        /if (${FindItem[=${PetClickItem[${i}]}].ID}) {
            /if (${FindItem[=${PetClickItem[${i}]}].TimerReady} == 0) {
                GZMDEBUG_ITEM Pet clicking: ${PetClickItem[${i}]} on pet
                | Target pet
                /target id ${Me.Pet.ID}
                /delay 5 ${Target.ID} == ${Me.Pet.ID}
                /call ClickItem "${PetClickItem[${i}]}"
                /varset PetClickTimer[${i}] ${PetClickDuration[${i}]}0
                | Restore target
                /if (${oldTarget}) {
                    /target id ${oldTarget}
                }
                /return
            }
        }
    /next i
/return

| ============================================================================
| SUB: Check Worn Clicks (equip, click, swap back)
| ============================================================================
Sub CheckWornClicks
    /if (!${ItemClicksOn}) /return
    /if (!${WornClickCount}) /return
    /if (!${UseExchange}) /return
    /if (${Me.Casting.ID}) /return

    /declare i int local
    /declare currentItem string local

    /for i 1 to ${WornClickCount}
        /if (!${WornClickItem[${i}].Length}) /continue

        | Check timer
        /if (${WornClickTimer[${i}]}) /continue

        | Check if item exists
        /if (!${FindItem[=${WornClickItem[${i}]}].ID}) /continue

        | Check if item is ready
        /if (${FindItem[=${WornClickItem[${i}]}].TimerReady} != 0) /continue

        GZMDEBUG_ITEM Worn click: ${WornClickItem[${i}]} (slot: ${WornClickSlot[${i}]})

        | Store current item in slot
        /varset currentItem ${Me.Inventory[${WornClickSlot[${i}]}].Name}

        | Equip the click item
        /if (!${Me.Inventory[${WornClickSlot[${i}]}].Name.Equal[${WornClickItem[${i}]}]}) {
            /exchange "${WornClickItem[${i}]}" ${WornClickSlot[${i}]}
            /delay 10 ${Me.Inventory[${WornClickSlot[${i}]}].Name.Equal[${WornClickItem[${i}]}]}
        }

        | Click it
        /call ClickItem "${WornClickItem[${i}]}"

        | Swap back
        /if (${WornClickSwapBack[${i}].Length}) {
            /exchange "${WornClickSwapBack[${i}]}" ${WornClickSlot[${i}]}
            /delay 10
        } else /if (${currentItem.Length}) {
            /exchange "${currentItem}" ${WornClickSlot[${i}]}
            /delay 10
        }

        /varset WornClickTimer[${i}] ${WornClickRecast[${i}]}0
        /return
    /next i
/return

| ============================================================================
| SUB: Check Burn Clicks (during burn phase)
| ============================================================================
Sub CheckBurnClicks
    /if (!${ItemClicksOn}) /return
    /if (!${BurnClickCount}) /return

    /declare i int local

    GZMDEBUG_ITEM Activating burn clicks...

    /for i 1 to ${BurnClickCount}
        /if (!${BurnClickItem[${i}].Length}) /continue
        /if (${BurnClickTimer[${i}]}) /continue

        | Check if item exists and is ready
        /if (${FindItem[=${BurnClickItem[${i}]}].ID}) {
            /if (${FindItem[=${BurnClickItem[${i}]}].TimerReady} == 0) {
                GZMDEBUG_ITEM Burn clicking: ${BurnClickItem[${i}]}
                /call ClickItem "${BurnClickItem[${i}]}"
                /varset BurnClickTimer[${i}] 3000
                /delay ${ClickDelay}
            }
        }
    /next i
/return

| ============================================================================
| SUB: Check Instant Clicks
| ============================================================================
Sub CheckInstantClicks
    /if (!${ItemClicksOn}) /return
    /if (!${InstantClickCount}) /return
    /if (${Me.Casting.ID}) /return

    /declare i int local

    /for i 1 to ${InstantClickCount}
        /if (!${InstantClickItem[${i}].Length}) /continue
        /if (${InstantClickTimer[${i}]}) /continue

        | Check combat only
        /if (${InstantClickCombatOnly[${i}]} && !${InCombat}) /continue

        | Check if item exists and is ready
        /if (${FindItem[=${InstantClickItem[${i}]}].ID}) {
            /if (${FindItem[=${InstantClickItem[${i}]}].TimerReady} == 0) {
                GZMDEBUG_ITEM Instant clicking: ${InstantClickItem[${i}]}
                /call ClickItem "${InstantClickItem[${i}]}"
                /if (${InstantClickRecast[${i}]}) {
                    /varset InstantClickTimer[${i}] ${InstantClickRecast[${i}]}0
                }
                /return
            }
        }
    /next i
/return

| ============================================================================
| SUB: Click Item (main click routine)
| ============================================================================
Sub ClickItem(string ItemName)
    /if (!${ItemName.Length}) /return

    | Stand if sitting
    /if (${Me.Sitting}) /stand

    | Find the item
    /if (!${FindItem[=${ItemName}].ID}) {
        GZMDEBUG_ITEM Item not found: ${ItemName}
        /return
    }

    | Check if worn or in inventory
    /if (${FindItem[=${ItemName}].WornSlot[1].ID}) {
        | Item is worn - click directly
        /useitem "${ItemName}"
    } else {
        | Item in inventory - need to find and click
        /useitem "${ItemName}"
    }

    /delay ${ClickDelay}
/return

| ============================================================================
| SUB: Force Click Item (bypass timers)
| ============================================================================
Sub ForceClickItem(string ItemName)
    /if (!${ItemName.Length}) /return

    /if (${FindItem[=${ItemName}].ID}) {
        /if (${FindItem[=${ItemName}].TimerReady} == 0) {
            /echo \ag[GZM-Items] Force clicking: ${ItemName}
            /useitem "${ItemName}"
        } else {
            /echo \ar[GZM-Items] Item not ready: ${ItemName} (${FindItem[=${ItemName}].TimerReady}s)
        }
    } else {
        /echo \ar[GZM-Items] Item not found: ${ItemName}
    }
/return

| ============================================================================
| CLASS-SPECIFIC ITEM ROUTINES
| ============================================================================

| ============================================================================
| SUB: Monk Epic Click (Celestial Fists - every 18 seconds)
| ============================================================================
Sub MonkEpicClick
    /if (!${Me.Class.ShortName.Equal[MNK]}) /return

    | Celestial Fists - must be worn, click every 18s for haste buff
    /declare epicName string local "Celestial Fists"

    | Check if we have it equipped
    /if (${Me.Inventory[hands].Name.Equal[${epicName}]}) {
        /if (${FindItem[=${epicName}].TimerReady} == 0) {
            | Check if buff is missing or about to fade
            /if (!${Me.Buff[Celestial Tranquility].ID} || ${Me.Buff[Celestial Tranquility].Duration} < 3) {
                GZMDEBUG_ITEM Monk epic click: ${epicName}
                /useitem "${epicName}"
            }
        }
    }
/return

| ============================================================================
| SUB: Shaman Epic Click
| ============================================================================
Sub ShamanEpicClick
    /if (!${Me.Class.ShortName.Equal[SHM]}) /return
    /if (!${InCombat}) /return
    /if (!${Target.ID}) /return

    | Spear of Fate (1.0) - DoT effect on target
    /if (${FindItem[=Spear of Fate].ID}) {
        /if (${FindItem[=Spear of Fate].TimerReady} == 0) {
            /if (${Target.PctHPs} > 20) {
                GZMDEBUG_ITEM Shaman 1.0 epic DoT click
                /useitem "Spear of Fate"
            }
        }
    }

    | Blessed Spiritstaff of the Heyokah (2.0) - Group crit buff
    /if (${FindItem[=Blessed Spiritstaff of the Heyokah].ID}) {
        /if (${FindItem[=Blessed Spiritstaff of the Heyokah].TimerReady} == 0) {
            GZMDEBUG_ITEM Shaman 2.0 epic group buff click
            /useitem "Blessed Spiritstaff of the Heyokah"
        }
    }
/return

| ============================================================================
| SUB: Mage Epic Click (Epic 1.0 - Orb of Mastery)
| ============================================================================
Sub MageEpicClick
    /if (!${Me.Class.ShortName.Equal[MAG]}) /return

    | ========================================
    | EPIC 1.0: Orb of Mastery - Manifest Elements
    | Summons the legendary "Fluffy" epic pet
    | Level 49 pet with:
    | - 81 damage quad kicks
    | - 140 DD stun spell
    | - 160 DD proc
    | - 4300 HP, 50 damage shield
    | Better than regular pets until PoP!
    | ========================================

    | Check if we should use epic pet
    /declare preferEpic int local ${Ini[${IniFileName},EpicPet,PreferEpicPet,1]}
    /declare epicItem string local ${Ini[${IniFileName},EpicPet,EpicPetItem,Orb of Mastery]}

    | Only summon if we have no pet
    /if (${Me.Pet.ID}) /return

    | Check if we have the Orb
    /if (${FindItem[=${epicItem}].ID}) {
        /if (${FindItem[=${epicItem}].TimerReady} == 0) {
            GZMDEBUG_ITEM Mage Epic 1.0: Summoning EPIC PET via ${epicItem}!
            /echo \ag[GZM-Items] Summoning Mage Epic Pet (Fluffy)!
            /useitem "${epicItem}"
            /delay 10s ${Me.Pet.ID}
            /if (${Me.Pet.ID}) {
                /echo \ag[GZM-Items] Epic Pet summoned successfully!
                | Configure pet
                /pet hold
                /delay 2
                /pet notaunt
            }
        }
    }
/return

| ============================================================================
| SUB: Necromancer Click Items
| ============================================================================
Sub NecroItemClick
    /if (!${Me.Class.ShortName.Equal[NEC]}) /return
    /if (!${InCombat}) /return
    /if (!${Target.ID}) /return

    | OOW Robe - 100% crit rate for DoTs
    /if (${FindItem[=Blightbringer's Tunic of the Grave].ID}) {
        /if (${FindItem[=Blightbringer's Tunic of the Grave].TimerReady} == 0) {
            GZMDEBUG_ITEM Necro OOW robe click
            /useitem "Blightbringer's Tunic of the Grave"
        }
    }
/return

| ============================================================================
| BINDS
| ============================================================================
Sub Bind_ClickItem(string ItemName)
    /call ForceClickItem "${ItemName}"
/return

Sub Bind_ItemsOn
    /varset ItemClicksOn ${If[${ItemClicksOn},0,1]}
    /echo \ay[GZM-Items] Item clicks: ${If[${ItemClicksOn},ENABLED,DISABLED]}
/return

Sub Bind_ListItems
    /declare i int local

    /echo \ag=== Combat Clicks ===
    /for i 1 to ${CombatClickCount}
        /echo ${i}: ${CombatClickItem[${i}]} (${CombatClickRecast[${i}]}s)
    /next i

    /echo \ag=== Buff Clicks ===
    /for i 1 to ${BuffClickCount}
        /echo ${i}: ${BuffClickItem[${i}]} (${BuffClickDuration[${i}]}s)
    /next i

    /echo \ag=== Worn Clicks ===
    /for i 1 to ${WornClickCount}
        /echo ${i}: ${WornClickItem[${i}]} (slot: ${WornClickSlot[${i}]})
    /next i
/return

| ============================================================================
| END OF ITEMS INCLUDE
| ============================================================================
