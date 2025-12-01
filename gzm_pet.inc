|**
    GZM_pet.inc - GronnzMaster Pet Management System

    Features:
    - Auto pet summon on death
    - Pet buffing
    - Charm pet management (Enchanters)
    - Pet equipment (Mages) - PetToys system
    - Warder management (Beastlords)
    - Pet suspend/unsuspend
    - Pet Affinity detection for buff decisions
**|

| ============================================================================
| PET CONSTANTS
| ============================================================================
#DEFINE MAXPETBUFFS 8
#DEFINE MAXPETTOYS 5

| ============================================================================
| SUB: Initialize Pet System
| ============================================================================
Sub InitPetSystem
    /declare i int local

    | Pet state
    /declare PetOn int outer 1
    /declare PetHold int outer 0
    /declare PetTaunt int outer 0
    /declare PetAssist int outer 1
    /declare PetShrink int outer 0
    /declare PetSpell string outer

    | Pet buff arrays
    /declare PetBuff[${MAXPETBUFFS}] string outer
    /declare PetBuffCount int outer 0

    | Pet resummon tracking
    /declare PetWasDead int outer 0
    /declare PetResummonTimer timer outer 0
    /declare PetLastHP int outer 100

    | Pet heal
    /declare PetHealOn int outer 1
    /declare PetHealPct int outer 50
    /declare PetHealSpell string outer

    | Charm (Enchanter)
    /declare CharmOn int outer 0
    /declare CharmSpell string outer
    /declare CharmTarget int outer 0
    /declare CharmTimer timer outer 0
    /declare CharmBreakTimer timer outer 0
    /declare CharmMinLevel int outer 1
    /declare CharmMaxLevel int outer 999

    | Mage pet equipment (legacy)
    /declare PetWeapon string outer
    /declare PetMask string outer
    /declare PetArmor string outer
    /declare PetEquipTimer timer outer 0

    | PetToys System (Mage pet equipment)
    /declare DoPetToys int outer 0
    /declare PetToys1 string outer
    /declare PetToys2 string outer
    /declare PetToys3 string outer
    /declare PetToys4 string outer
    /declare PetToys5 string outer
    /declare PTSpell[${MAXPETTOYS}] string outer
    /declare PTItem1[${MAXPETTOYS}] string outer
    /declare PTItem2[${MAXPETTOYS}] string outer
    /declare PTCount int outer 0
    /declare PetToysTimer timer outer 0

    | Pet Affinity tracking (for deciding who can receive pet buffs)
    /declare HasPetAffinity int outer 0

    | Suspend
    /declare PetSuspendOn int outer 0
    /declare SuspendedPetID int outer 0

    | Load settings
    /call LoadPetSettings

    | Check Pet Affinity AA
    /call CheckPetAffinity

    | Parse Pet Toys if configured
    /if (${Me.Class.ShortName.Equal[MAG]}) {
        /call ParsePetToys
    }

    /echo \ag[GZM-Pet] Pet system initialized
/return

| ============================================================================
| SUB: Check Pet Affinity AA
| ============================================================================
Sub CheckPetAffinity
    /if (${Me.AltAbility[Pet Affinity].ID}) {
        /varset HasPetAffinity 1
        | Broadcast via DanNet so others know for buff decisions
        /if (${Plugin[MQ2DanNet].IsLoaded}) {
            /squelch /dobserve ${Me.CleanName} -q "HasPetAffinity"
        }
    } else {
        /varset HasPetAffinity 0
    }
/return

| ============================================================================
| SUB: Parse Pet Toy Configurations
| ============================================================================
Sub ParsePetToys
    /varset PTCount 0
    /declare a int local
    /declare toyConfig string local

    /for a 1 to ${MAXPETTOYS}
        /varset toyConfig ${PetToys${a}}
        /if (${toyConfig.Length} && ${toyConfig.NotEqual[NULL]} && ${toyConfig.NotEqual[ ]}) {
            /varcalc PTCount ${PTCount}+1
            /varset PTSpell[${PTCount}] ${toyConfig.Arg[1,,]}
            /varset PTItem1[${PTCount}] ${If[${toyConfig.Arg[2,,].Length},${toyConfig.Arg[2,,]},]}
            /varset PTItem2[${PTCount}] ${If[${toyConfig.Arg[3,,].Length},${toyConfig.Arg[3,,]},]}
        }
    /next a

    /if (${PTCount}) {
        /echo \ag[GZM-Pet] Parsed ${PTCount} pet toy configurations
    }
/return

| ============================================================================
| SUB: Load Pet Settings from INI
| ============================================================================
Sub LoadPetSettings
    /declare i int local
    /declare spell string local

    | Basic settings
    /varset PetOn ${Ini[${IniFileName},Pet,PetOn,1]}
    /varset PetHold ${Ini[${IniFileName},Pet,PetHold,0]}
    /varset PetTaunt ${Ini[${IniFileName},Pet,PetTaunt,0]}
    /varset PetAssist ${Ini[${IniFileName},Pet,PetAssist,1]}
    /varset PetShrink ${Ini[${IniFileName},Pet,PetShrink,0]}
    /varset PetSpell ${Ini[${IniFileName},Pet,PetSpell]}

    | Pet heal
    /varset PetHealOn ${Ini[${IniFileName},PetHeal,PetHealOn,1]}
    /varset PetHealPct ${Ini[${IniFileName},PetHeal,PetHealPct,50]}
    /varset PetHealSpell ${Ini[${IniFileName},PetHeal,PetHealSpell]}

    | Charm settings
    /varset CharmOn ${Ini[${IniFileName},Charm,CharmOn,0]}
    /varset CharmSpell ${Ini[${IniFileName},Charm,CharmSpell]}
    /varset CharmMinLevel ${Ini[${IniFileName},Charm,CharmMinLevel,1]}
    /varset CharmMaxLevel ${Ini[${IniFileName},Charm,CharmMaxLevel,999]}

    | Mage equipment (legacy)
    /varset PetWeapon ${Ini[${IniFileName},PetEquip,PetWeapon]}
    /varset PetMask ${Ini[${IniFileName},PetEquip,PetMask]}
    /varset PetArmor ${Ini[${IniFileName},PetEquip,PetArmor]}

    | PetToys system (Mages only)
    /if (${Me.Class.ShortName.Equal[MAG]}) {
        /varset DoPetToys ${Ini[${IniFileName},PetToys,DoPetToys,0]}
        /varset PetToys1 ${Ini[${IniFileName},PetToys,PetToys1]}
        /varset PetToys2 ${Ini[${IniFileName},PetToys,PetToys2]}
        /varset PetToys3 ${Ini[${IniFileName},PetToys,PetToys3]}
        /varset PetToys4 ${Ini[${IniFileName},PetToys,PetToys4]}
        /varset PetToys5 ${Ini[${IniFileName},PetToys,PetToys5]}
    }

    | Pet buffs
    /for i 1 to ${MAXPETBUFFS}
        /varset spell ${Ini[${IniFileName},PetBuffs,PetBuff${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset PetBuff[${i}] ${spell}
            /varcalc PetBuffCount ${PetBuffCount}+1
        }
    /next i

    | Suspend
    /varset PetSuspendOn ${Ini[${IniFileName},PetSuspend,PetSuspendOn,0]}
/return

| ============================================================================
| SUB: Main Pet Check (called from main loop)
| ============================================================================
Sub CheckPet
    /if (!${PetOn}) /return

    | Check if charm class
    /if (${CharmOn} && ${Select[${Me.Class.ShortName},ENC,DRU,NEC]}) {
        /call CheckCharmPet
        /return
    }

    | Regular pet classes
    | Priority 1: Check if pet died and needs resummon
    /if (!${Me.Pet.ID}) {
        /call CheckPetResummon
        /return
    }

    | Priority 2: Check pet health
    /if (${PetHealOn} && ${Me.Pet.PctHPs} < ${PetHealPct}) {
        /call HealPet
    }

    | Priority 3: Pet buffs (out of combat)
    /if (!${InCombat}) {
        /call CheckPetBuffs
    }

    | Priority 4: Pet equipment (Mages)
    /if (${Me.Class.ShortName.Equal[MAG]} && !${PetEquipTimer}) {
        /call CheckPetEquipment
    }

    | Priority 5: Pet combat commands
    /if (${InCombat} && ${PetAssist} && ${MyTargetID}) {
        /call PetCombatCommands
    }

    | Track pet HP for death detection
    /varset PetLastHP ${Me.Pet.PctHPs}
/return

| ============================================================================
| SUB: Check Pet Resummon
| ============================================================================
Sub CheckPetResummon
    /if (${PetResummonTimer}) /return
    /if (!${PetSpell.Length}) /return
    /if (${Me.Casting.ID}) /return
    /if (${InCombat} && !${Safe}) /return

    | Check if we have a pet spell/AA
    /if (${Me.Pet.ID}) /return

    GZMDEBUG_PET Pet died or missing - resummoning!
    /echo \ar[GZM-Pet] Pet died! Resummoning...

    /varset PetWasDead 1

    | Stand if sitting
    /if (${Me.Sitting}) /stand

    | Check if it's an AA
    /if (${Me.AltAbility[${PetSpell}].ID}) {
        /if (${Me.AltAbilityReady[${PetSpell}]}) {
            /alt act ${Me.AltAbility[${PetSpell}].ID}
            /delay 5s ${Me.Pet.ID}
        }
    } else {
        | Regular spell
        /if (${Me.SpellReady[${PetSpell}]}) {
            /cast "${PetSpell}"
            /delay 5s ${Me.Casting.ID}
            /delay 15s !${Me.Casting.ID}
        }
    }

    | Wait for pet
    /delay 20 ${Me.Pet.ID}

    /if (${Me.Pet.ID}) {
        /echo \ag[GZM-Pet] Pet summoned successfully!
        /varset PetResummonTimer 30
        /varset PetWasDead 0

        | Configure pet
        /call ConfigurePet
    } else {
        /echo \ar[GZM-Pet] Failed to summon pet!
        /varset PetResummonTimer 100
    }
/return

| ============================================================================
| SUB: Configure Pet (after summon)
| ============================================================================
Sub ConfigurePet
    /if (!${Me.Pet.ID}) /return

    GZMDEBUG_PET Configuring pet...

    | Pet hold
    /if (${PetHold}) {
        /pet hold
        /delay 2
    }

    | Pet taunt
    /if (!${PetTaunt}) {
        /pet notaunt
        /delay 2
    } else {
        /pet taunt
        /delay 2
    }

    | Pet shrink
    /if (${PetShrink} && ${Me.AltAbilityReady[Companion's Blessing]}) {
        /alt act ${Me.AltAbility[Companion's Blessing].ID}
        /delay 5
    }

    | Buff pet immediately
    /call CheckPetBuffs

    | Equip pet (Mages)
    /if (${Me.Class.ShortName.Equal[MAG]}) {
        /call EquipPet
    }
/return

| ============================================================================
| SUB: Heal Pet
| ============================================================================
Sub HealPet
    /if (!${Me.Pet.ID}) /return
    /if (!${PetHealSpell.Length}) /return

    GZMDEBUG_PET Healing pet at ${Me.Pet.PctHPs}%

    | Check if spell ready
    /if (${Me.SpellReady[${PetHealSpell}]}) {
        /target id ${Me.Pet.ID}
        /delay 3 ${Target.ID} == ${Me.Pet.ID}
        /if (${Me.Sitting}) /stand
        /cast "${PetHealSpell}"
        /delay 3s ${Me.Casting.ID}
        /delay 5s !${Me.Casting.ID}

        | Return to previous target
        /if (${MyTargetID}) {
            /target id ${MyTargetID}
        }
    }

    | Try AA heal
    /if (${Me.AltAbilityReady[Mend Companion]}) {
        /alt act ${Me.AltAbility[Mend Companion].ID}
    }
/return

| ============================================================================
| SUB: Check Pet Buffs
| ============================================================================
Sub CheckPetBuffs
    /if (!${Me.Pet.ID}) /return
    /if (${Me.Casting.ID}) /return

    /declare i int local
    /declare buff string local

    /for i 1 to ${PetBuffCount}
        /varset buff ${PetBuff[${i}]}
        /if (!${buff.Length}) /continue

        | Check if buff already on pet
        /if (${Me.Pet.Buff[${buff}].ID}) /continue
        /if (${Me.Pet.Buff[${Spell[${buff}].Trigger}].ID}) /continue

        | Cast buff
        /if (${Me.SpellReady[${buff}]}) {
            GZMDEBUG_PET Buffing pet with ${buff}
            /target id ${Me.Pet.ID}
            /delay 3 ${Target.ID} == ${Me.Pet.ID}
            /if (${Me.Sitting}) /stand
            /cast "${buff}"
            /delay 3s ${Me.Casting.ID}
            /delay 10s !${Me.Casting.ID}
            /return
        }
    /next i
/return

| ============================================================================
| SUB: Check Pet Equipment (Mages)
| ============================================================================
Sub CheckPetEquipment
    /if (!${Me.Pet.ID}) /return
    /if (!${Me.Class.ShortName.Equal[MAG]}) /return

    | Only check occasionally
    /varset PetEquipTimer 600

    /call EquipPet
/return

| ============================================================================
| SUB: Equip Pet (Mages)
| ============================================================================
Sub EquipPet
    /if (!${Me.Pet.ID}) /return

    GZMDEBUG_PET Equipping pet...

    | Summon and give weapon
    /if (${PetWeapon.Length} && ${Me.SpellReady[${PetWeapon}]}) {
        /cast "${PetWeapon}"
        /delay 3s ${Me.Casting.ID}
        /delay 10s !${Me.Casting.ID}
        /delay 5
        | Give to pet
        /if (${Cursor.ID}) {
            /target id ${Me.Pet.ID}
            /delay 3
            /click left target
            /delay 10
        }
    }

    | Summon and give mask
    /if (${PetMask.Length} && ${Me.SpellReady[${PetMask}]}) {
        /cast "${PetMask}"
        /delay 3s ${Me.Casting.ID}
        /delay 10s !${Me.Casting.ID}
        /delay 5
        /if (${Cursor.ID}) {
            /target id ${Me.Pet.ID}
            /delay 3
            /click left target
            /delay 10
        }
    }

    | Summon and give armor
    /if (${PetArmor.Length} && ${Me.SpellReady[${PetArmor}]}) {
        /cast "${PetArmor}"
        /delay 3s ${Me.Casting.ID}
        /delay 10s !${Me.Casting.ID}
        /delay 5
        /if (${Cursor.ID}) {
            /target id ${Me.Pet.ID}
            /delay 3
            /click left target
            /delay 10
        }
    }
/return

| ============================================================================
| SUB: Pet Combat Commands
| ============================================================================
Sub PetCombatCommands
    /if (!${Me.Pet.ID}) /return
    /if (!${MyTargetID}) /return

    | Send pet to attack if not already
    /if (${Me.Pet.Target.ID} != ${MyTargetID}) {
        /pet attack ${MyTargetID}
    }

    | Pet back off if target enraged
    /if (${Enraged} && ${Me.Pet.Combat}) {
        /pet back
    }
/return

| ============================================================================
| CHARM PET SYSTEM (Enchanters)
| ============================================================================

| ============================================================================
| SUB: Check Charm Pet
| ============================================================================
Sub CheckCharmPet
    /if (!${CharmOn}) /return
    /if (!${CharmSpell.Length}) /return

    | Priority 1: Check if charm broke
    /if (${CharmTarget} && !${Me.Pet.ID}) {
        GZMDEBUG_PET CHARM BROKE!
        /call ReCharm
        /return
    }

    | Priority 2: Check charm duration
    /if (${Me.Pet.ID} && ${CharmTimer} < 100) {
        GZMDEBUG_PET Charm wearing off - refreshing
        /call ReCharm
        /return
    }

    | Priority 3: Find charm target if we don't have one
    /if (!${Me.Pet.ID} && !${CharmBreakTimer}) {
        /call FindCharmTarget
    }
/return

| ============================================================================
| SUB: Find Charm Target
| ============================================================================
Sub FindCharmTarget
    /declare i int local
    /declare targetID int local
    /declare mobCount int local ${SpawnCount[npc radius ${MezRadius} targetable zradius ${MaxZRange}]}

    | Don't charm if only 1-2 mobs
    /if (${mobCount} < 3) /return

    /for i 1 to ${mobCount}
        /varset targetID ${NearestSpawn[${i},npc radius ${MezRadius} targetable zradius ${MaxZRange}].ID}

        | Skip main target
        /if (${targetID} == ${MyTargetID}) /continue

        | Check level range
        /if (${Spawn[${targetID}].Level} < ${CharmMinLevel} || ${Spawn[${targetID}].Level} > ${CharmMaxLevel}) /continue

        | Check if charmable (not named, not rooted, etc)
        /if (${Spawn[${targetID}].Named}) /continue

        GZMDEBUG_PET Found charm target: ${Spawn[${targetID}].CleanName}
        /call DoCharm ${targetID}
        /return
    /next i
/return

| ============================================================================
| SUB: Do Charm
| ============================================================================
Sub DoCharm(int TargetID)
    /if (!${TargetID}) /return

    GZMDEBUG_PET Charming ${Spawn[${TargetID}].CleanName}

    | Target
    /target id ${TargetID}
    /delay 5 ${Target.ID} == ${TargetID}

    | Stand
    /if (${Me.Sitting}) /stand

    | Cast charm
    /if (${Me.SpellReady[${CharmSpell}]}) {
        /cast "${CharmSpell}"
        /delay 5s ${Me.Casting.ID}
        /delay 15s !${Me.Casting.ID}

        | Check if charm succeeded
        /if (${Me.Pet.ID}) {
            /varset CharmTarget ${TargetID}
            /varset CharmTimer 1200 | 2 minute default
            /echo \ag[GZM-Pet] Charm successful: ${Spawn[${TargetID}].CleanName}

            | Configure charmed pet
            /pet hold
            /pet notaunt
        } else {
            /echo \ar[GZM-Pet] Charm FAILED on ${Spawn[${TargetID}].CleanName}
            /varset CharmBreakTimer 100
        }
    }
/return

| ============================================================================
| SUB: ReCharm
| ============================================================================
Sub ReCharm
    /if (!${CharmTarget}) /return

    | Check if old charm target still exists
    /if (${Spawn[${CharmTarget}].ID}) {
        GZMDEBUG_PET Recharming ${Spawn[${CharmTarget}].CleanName}

        | Root first if available
        /if (${Me.SpellReady[Root]}) {
            /target id ${CharmTarget}
            /delay 3
            /cast "Root"
            /delay 3s ${Me.Casting.ID}
            /delay 5s !${Me.Casting.ID}
        }

        /call DoCharm ${CharmTarget}
    } else {
        | Target gone, clear charm
        /varset CharmTarget 0
        /varset CharmTimer 0
        /varset CharmBreakTimer 100
    }
/return

| ============================================================================
| SUB: Release Charm
| ============================================================================
Sub ReleaseCharm
    /if (!${Me.Pet.ID}) /return
    /if (!${CharmTarget}) /return

    GZMDEBUG_PET Releasing charm pet

    | Kill the charmed pet
    /pet kill

    /varset CharmTarget 0
    /varset CharmTimer 0

    /echo \ay[GZM-Pet] Charm pet released
/return

| ============================================================================
| EVENT: Pet Died
| ============================================================================
Sub Event_PetDied
    /echo \ar[GZM-Pet] Pet has died!
    /varset PetWasDead 1
    /varset PetResummonTimer 0

    | Immediate resummon attempt
    /call CheckPetResummon
/return

| ============================================================================
| EVENT: Charm Broke
| ============================================================================
Sub Event_CharmBroke
    /echo \ar[GZM-Pet] CHARM BROKE!
    /varset CharmBreakTimer 0

    | Immediate recharm
    /call ReCharm
/return

| ============================================================================
| BINDS
| ============================================================================
Sub Bind_PetSummon
    /varset PetResummonTimer 0
    /call CheckPetResummon
/return

Sub Bind_PetHold
    /varset PetHold ${If[${PetHold},0,1]}
    /if (${PetHold}) {
        /pet hold
        /echo \ay[GZM-Pet] Pet HOLD enabled
    } else {
        /pet follow
        /echo \ay[GZM-Pet] Pet HOLD disabled
    }
/return

Sub Bind_ReleaseCharm
    /call ReleaseCharm
/return

Sub Bind_PetToys
    /if (!${Me.Class.ShortName.Equal[MAG]}) {
        /echo \ar[GZM-Pet] PetToys only available for Mages
        /return
    }
    /if (!${Me.Pet.ID}) {
        /echo \ar[GZM-Pet] No pet to equip
        /return
    }
    /call GivePetToys ${Me.Pet.ID}
/return

| ============================================================================
| MODBOT PETTOYS SYSTEM (Mages)
| ============================================================================

| Main PetToys routine - gives toys to specified pet
Sub GivePetToys(int petID)
    /if (!${DoPetToys} || !${PTCount}) /return FALSE
    /if (!${petID}) /return FALSE

    | Safety checks
    /if (${Me.Invis}) {
        /echo \ar[GZM-Pet] Cannot give toys while invisible
        /return FALSE
    }

    /if (${InCombat}) {
        /echo \ar[GZM-Pet] Cannot give toys in combat
        /return FALSE
    }

    /declare i int local
    /declare tPack int local 0
    /declare tItem int local 0
    /declare summonedID int local
    /declare tItemID int local

    | Verify pet exists
    /if (!${Spawn[pet id ${petID}].ID}) {
        /echo \ar[GZM-Pet] PetToys - Pet not found (ID: ${petID})
        /return FALSE
    }

    | Check distance to pet
    /if (${Spawn[id ${petID}].Distance3D} > 50) {
        /echo \ar[GZM-Pet] PetToys - Pet too far away
        /return FALSE
    }

    | Move closer if needed
    /if (${Spawn[id ${petID}].Distance3D} > 12) {
        /echo \ay[GZM-Pet] Moving closer to pet...
        /nav id ${petID}
        /delay 5s ${Spawn[id ${petID}].Distance3D} < 12 || !${Navigation.Active}
    }

    | Clear cursor before starting
    /if (${Cursor.ID}) {
        /call ClearCursor force
    }

    | Find an empty pack slot for summoned items
    /for i 1 to ${Me.NumBagSlots}
        /if (!${Me.Inventory[pack${i}].ID}) {
            /varset tPack ${i}
            /break
        }
        /if (${Me.Inventory[pack${i}].Items}) /continue
        /if (${Me.FreeInventory} > 1) /varset tPack ${i}
    /next i

    /if (!${tPack}) {
        /echo \ar[GZM-Pet] PetToys - No empty inventory slot
        /return FALSE
    }

    /echo \ag[GZM-Pet] Using pack${tPack} for toys - giving to ${Spawn[id ${petID}].CleanName}

    | Loop through configured toys
    /for i 1 to ${PTCount}
        /if (!${DoPetToys}) {
            /echo \ay[GZM-Pet] PetToys disabled - aborting
            /return FALSE
        }

        | Skip certain items for own pet
        /if (${Me.Pet.ID} == ${petID}) {
            /if (${Spell[${PTSpell[${i}].Arg[1,|]}].Level} >= 76) {
                /if (${PTSpell[${i}].Find[muzzle]} || ${PTSpell[${i}].Find[visor]} || ${PTSpell[${i}].Find[belt]}) {
                    /continue
                }
            }
        } else {
            /echo \ag[GZM-Pet] Giving toys to ${Spawn[id ${petID}].CleanName} (${i} of ${PTCount})
        }

        | Summon the item
        /if (${Me.Book[${PTSpell[${i}].Arg[1,|]}]}) {
            /cast "${PTSpell[${i}].Arg[1,|]}"
            /delay 3s ${Me.Casting.ID}
            /delay 15s !${Me.Casting.ID}
            /delay 15s ${Cursor.ID}

            | Wait for summoned item on cursor
            /declare waitTimer timer local 50
            /while (${waitTimer}) {
                /delay 1
                /if (!${Cursor.ID}) {
                    /echo \ar[GZM-Pet] Problem summoning pet toys - aborting
                    /return FALSE
                }
                /if (${Cursor.NoRent}) /break
                /if (${Cursor.ID}) /autoinv
                /delay 5s ${Cursor.ID}
            }

            /varset summonedID ${Cursor.ID}
            /delay 5

            | Place in pack slot
            /nomodkey /itemnotify pack${tPack} leftmouseup
            /delay 5s ${Cursor.ID} != ${summonedID}
            /delay 5

            /if (${Cursor.ID}) {
                /call ClearCursor force
            }

            /if (${Me.Inventory[pack${tPack}].ID} != ${summonedID} || ${Cursor.ID}) {
                /echo \ar[GZM-Pet] Problem with pet toys - aborting
                /return FALSE
            }

            | Handle folded packs (unfold first)
            /if (${Me.Inventory[pack${tPack}].Name.Find[folded]}) {
                /nomodkey /itemnotify pack${tPack} rightmouseup
                /echo \ay[GZM-Pet] Opening ${Me.Inventory[pack${tPack}].Name}
                /delay 15s ${Cursor.ID}
                /if (!${Cursor.ID}) /return FALSE
                /delay 1s
                /nomodkey /itemnotify pack${tPack} leftmouseup
                /delay 5s ${Me.Inventory[pack${tPack}].ID}

                /if (${Cursor.ID}) {
                    /call ClearCursor force
                }
            }

            | Process container or single item
            /if (${Me.Inventory[pack${tPack}].Container}) {
                /call GivePetToysFromBag ${tPack} ${petID} ${i}
            } else /if (${Me.Inventory[pack${tPack}].Name.Find[Summoned:]}) {
                /call GiveSinglePetToy ${tPack} ${petID}
            }
        }
    /next i

    /if (${Me.Pet.ID} != ${petID}) {
        /echo \ag[GZM-Pet] Finished giving toys to ${Spawn[id ${petID}].CleanName}
    }
/return TRUE

| Give items from a pet toy bag
Sub GivePetToysFromBag(int packSlot, int petID, int toyIndex)
    /declare tItem int local
    /declare tItemID int local

    /nomodkey /itemnotify pack${packSlot} rightmouseup
    /delay 1s

    /for tItem 1 to ${Me.Inventory[pack${packSlot}].Container}
        /if (!${DoPetToys}) {
            /echo \ay[GZM-Pet] PetToys disabled - aborting
            /return FALSE
        }

        /if (${Me.Inventory[pack${packSlot}].Item[${tItem}].ID}) {
            | Check if specific items configured
            /if (${PTItem1[${toyIndex}].Length}) {
                /if (!${Me.Inventory[pack${packSlot}].Item[${tItem}].Name.Equal[${PTItem1[${toyIndex}]}]}) {
                    /if (!${Me.Inventory[pack${packSlot}].Item[${tItem}].Name.Equal[${PTItem2[${toyIndex}]}]}) {
                        /continue
                    }
                }
            }

            /varset tItemID ${Me.Inventory[pack${packSlot}].Item[${tItem}].ID}

            | Pick up the item
            /nomodkey /itemnotify in pack${packSlot} ${tItem} leftmouseup
            /delay 2s ${Cursor.ID}

            | Give to pet
            /target id ${petID}
            /delay 3 ${Target.ID} == ${petID}
            /click left target
            /delay 3s ${Window[GiveWnd].Open}
            /if (${Window[GiveWnd].Open}) {
                /notify GiveWnd GVW_Give_Button leftmouseup
                /delay 2s !${Cursor.ID}
            }

            | Destroy rejected items
            /if (${Cursor.ID}) {
                /call DestroyPetToy
            }
        }
    /next tItem

    /delay 5

    | Destroy the empty bag
    /nomodkey /itemnotify pack${packSlot} leftmouseup
    /delay 5s ${Cursor.ID}
    /if (${Cursor.Name.Find[Phantom Satchel]} || ${Cursor.Name.Find[Pouch of Quellious]}) {
        /destroy
    }
    /delay 1s !${Cursor.ID}
/return TRUE

| Give a single pet toy item
Sub GiveSinglePetToy(int packSlot, int petID)
    /nomodkey /itemnotify pack${packSlot} leftmouseup
    /delay 2s ${Cursor.ID}

    /target id ${petID}
    /delay 3 ${Target.ID} == ${petID}
    /click left target
    /delay 3s ${Window[GiveWnd].Open}
    /if (${Window[GiveWnd].Open}) {
        /notify GiveWnd GVW_Give_Button leftmouseup
        /delay 2s !${Cursor.ID}
    }

    /if (${Cursor.ID}) {
        /call DestroyPetToy
    }
/return TRUE

| Destroy unwanted pet toy (pet didn't need it)
Sub DestroyPetToy
    /if (${Cursor.ID}) {
        /if (${Cursor.NoRent} || ${Cursor.Name.Find[muzzle]} || ${Cursor.Name.Find[visor]} || ${Cursor.Name.Find[belt]}) {
            /echo \ay[GZM-Pet] Pet didn't need ${Cursor.Name} - Destroying
            /destroy
            /delay 10
        }
    }
/return

| ============================================================================
| END OF PET INCLUDE
| ============================================================================
