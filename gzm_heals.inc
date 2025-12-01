|**
    GZM_heals.inc - GronnzMaster Healing System

    Comprehensive healing routines for all healing classes:
    - Cleric (CLR)
    - Druid (DRU)
    - Shaman (SHM)
    - Paladin (PAL) - off heals
    - Ranger (RNG) - off heals
    - Beastlord (BST) - off heals
    - Necromancer (NEC) - lifetaps

    Features:
    - XTarget healing support
    - Per-class heal thresholds
    - Prioritized heal targets
    - Heal announce/broadcast
**|

| ============================================================================
| HEALING CONSTANTS
| ============================================================================
#DEFINE MAXHEALSPELLS 12
#DEFINE MAXHOTSPELLS 4
#DEFINE MAXGROUPHEALS 4
#DEFINE MAXCURESPELLS 8
#DEFINE MAXREZSPELLS 2
#DEFINE MAXXTHEALTARGETS 10

| ============================================================================
| SUB: Initialize Healing System
| ============================================================================
Sub InitHealingSystem
    /declare i int local

    | Heal spell arrays
    /declare HealSpell[${MAXHEALSPELLS}] string outer
    /declare HealSpellPct[${MAXHEALSPELLS}] int outer
    /declare HealSpellGem[${MAXHEALSPELLS}] int outer
    /declare HealSpellType[${MAXHEALSPELLS}] string outer
    /declare HealSpellCount int outer 0

    | Group heal arrays
    /declare GroupHealSpell[${MAXGROUPHEALS}] string outer
    /declare GroupHealPct[${MAXGROUPHEALS}] int outer
    /declare GroupHealCount[${MAXGROUPHEALS}] int outer
    /declare GroupHealSpellCount int outer 0

    | HoT arrays
    /declare HoTSpell[${MAXHOTSPELLS}] string outer
    /declare HoTSpellPct[${MAXHOTSPELLS}] int outer
    /declare HoTSpellCount int outer 0

    | Cure arrays
    /declare CureSpell[${MAXCURESPELLS}] string outer
    /declare CureSpellType[${MAXCURESPELLS}] string outer
    /declare CureSpellCount int outer 0

    | Rez arrays
    /declare RezSpell[${MAXREZSPELLS}] string outer
    /declare RezSpellCount int outer 0

    | Healing state variables
    /declare HealingTarget int outer 0
    /declare LastHealTime timer outer 0
    /declare HealRotationIndex int outer 1
    /declare EmergencyHealSpell string outer
    /declare QuickHealSpell string outer
    /declare MainHealSpell string outer
    /declare GroupHealSpell string outer
    /declare PanicHealPct int outer 25
    /declare TankHealPct int outer 75
    /declare GroupMemberHealPct int outer 70
    /declare OOGHealPct int outer 60

    | Cure settings
    /declare CureOn int outer 1
    /declare CurePoison int outer 1
    /declare CureDisease int outer 1
    /declare CureCurse int outer 1
    /declare CureCorrupt int outer 1

    | Rez settings
    /declare RezOn int outer 1
    /declare RezOOC int outer 1
    /declare RezCombat int outer 0
    /declare RezRadius int outer 100

    | Promised heals
    /declare UsePromised int outer 1
    /declare PromisedSpell string outer
    /declare PromisedTarget int outer 0
    /declare PromisedTimer timer outer 0

    | Intervention/Divine Arb
    /declare UseIntervention int outer 1
    /declare InterventionSpell string outer
    /declare InterventionPct int outer 30

    | ========================================================================
    | XTARGET HEALING     | ========================================================================
    /declare XTargetHealOn int outer 1
    /declare XTargetHealPct int outer 70
    /declare XTargetHealRange int outer 200
    /declare XTargetHealSlots string outer 1 2 3 4 5

    | ========================================================================
    | PER-CLASS HEAL THRESHOLDS     | Different classes have different heal priority
    | ========================================================================
    /declare HealPct_WAR int outer 80
    /declare HealPct_PAL int outer 75
    /declare HealPct_SHD int outer 75
    /declare HealPct_CLR int outer 70
    /declare HealPct_DRU int outer 70
    /declare HealPct_SHM int outer 70
    /declare HealPct_ENC int outer 65
    /declare HealPct_WIZ int outer 60
    /declare HealPct_MAG int outer 60
    /declare HealPct_NEC int outer 55
    /declare HealPct_ROG int outer 65
    /declare HealPct_MNK int outer 65
    /declare HealPct_BER int outer 65
    /declare HealPct_RNG int outer 65
    /declare HealPct_BST int outer 65
    /declare HealPct_BRD int outer 60
    /declare HealPct_PET int outer 50

    | ========================================================================
    | HEAL PRIORITY ORDER
    | 1=highest priority (tanks), 5=lowest (pets)
    | ========================================================================
    /declare HealPriority_WAR int outer 1
    /declare HealPriority_PAL int outer 1
    /declare HealPriority_SHD int outer 1
    /declare HealPriority_CLR int outer 2
    /declare HealPriority_DRU int outer 3
    /declare HealPriority_SHM int outer 2
    /declare HealPriority_ENC int outer 3
    /declare HealPriority_WIZ int outer 4
    /declare HealPriority_MAG int outer 4
    /declare HealPriority_NEC int outer 4
    /declare HealPriority_ROG int outer 3
    /declare HealPriority_MNK int outer 3
    /declare HealPriority_BER int outer 3
    /declare HealPriority_RNG int outer 3
    /declare HealPriority_BST int outer 3
    /declare HealPriority_BRD int outer 3
    /declare HealPriority_PET int outer 5

    | ========================================================================
    | HEAL ANNOUNCE/BROADCAST
    | ========================================================================
    /declare HealAnnounce string outer
    /declare HealAnnounceChannel string outer
    /declare HealAnnounceMinPct int outer 30

    | Heal debug
    /declare HealDebug bool outer FALSE

    | Load class-specific heals
    /call LoadHealSpells

    /echo \ag[GZM-Heals] Healing system initialized - ${HealSpellCount} heal spells loaded
/return

| ============================================================================
| SUB: Load Heal Spells from INI
| ============================================================================
Sub LoadHealSpells
    /declare i int local
    /declare spell string local
    /declare pct int local
    /declare gem int local
    /declare stype string local

    | Load main heals
    /for i 1 to ${MAXHEALSPELLS}
        /varset spell ${Ini[${IniFileName},Heals,HealSpell${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset HealSpell[${i}] ${spell}
            /varset HealSpellPct[${i}] ${Ini[${IniFileName},Heals,HealSpell${i}Pct,70]}
            /varset HealSpellGem[${i}] ${Ini[${IniFileName},Heals,HealSpell${i}Gem,0]}
            /varset HealSpellType[${i}] ${Ini[${IniFileName},Heals,HealSpell${i}Type,Normal]}
            /varcalc HealSpellCount ${HealSpellCount}+1
            GZMDEBUG_HEAL Loaded heal ${i}: ${spell} at ${HealSpellPct[${i}]}%
        }
    /next i

    | Load group heals
    /for i 1 to ${MAXGROUPHEALS}
        /varset spell ${Ini[${IniFileName},Heals,GroupHeal${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset GroupHealSpell[${i}] ${spell}
            /varset GroupHealPct[${i}] ${Ini[${IniFileName},Heals,GroupHeal${i}Pct,70]}
            /varset GroupHealCount[${i}] ${Ini[${IniFileName},Heals,GroupHeal${i}Count,3]}
            /varcalc GroupHealSpellCount ${GroupHealSpellCount}+1
        }
    /next i

    | Load HoTs
    /for i 1 to ${MAXHOTSPELLS}
        /varset spell ${Ini[${IniFileName},Heals,HoT${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset HoTSpell[${i}] ${spell}
            /varset HoTSpellPct[${i}] ${Ini[${IniFileName},Heals,HoT${i}Pct,85]}
            /varcalc HoTSpellCount ${HoTSpellCount}+1
        }
    /next i

    | Load cures
    /for i 1 to ${MAXCURESPELLS}
        /varset spell ${Ini[${IniFileName},Cures,CureSpell${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset CureSpell[${i}] ${spell}
            /varset CureSpellType[${i}] ${Ini[${IniFileName},Cures,CureSpell${i}Type,All]}
            /varcalc CureSpellCount ${CureSpellCount}+1
        }
    /next i

    | Load rez spells
    /for i 1 to ${MAXREZSPELLS}
        /varset spell ${Ini[${IniFileName},Rez,RezSpell${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset RezSpell[${i}] ${spell}
            /varcalc RezSpellCount ${RezSpellCount}+1
        }
    /next i

    | Load special spells
    /varset EmergencyHealSpell ${Ini[${IniFileName},Heals,EmergencyHeal]}
    /varset QuickHealSpell ${Ini[${IniFileName},Heals,QuickHeal]}
    /varset MainHealSpell ${Ini[${IniFileName},Heals,MainHeal]}
    /varset PromisedSpell ${Ini[${IniFileName},Heals,PromisedHeal]}
    /varset InterventionSpell ${Ini[${IniFileName},Heals,Intervention]}

    | Load heal percentages
    /varset PanicHealPct ${Ini[${IniFileName},Heals,PanicHealPct,25]}
    /varset TankHealPct ${Ini[${IniFileName},Heals,TankHealPct,75]}
    /varset GroupMemberHealPct ${Ini[${IniFileName},Heals,GroupMemberHealPct,70]}
    /varset OOGHealPct ${Ini[${IniFileName},Heals,OOGHealPct,60]}
/return

| ============================================================================
| SUB: Main Healing Check (called from main loop)
| ============================================================================
Sub CheckHealing
    /if (!${HealsOn}) /return
    /if (${Me.Casting.ID} && !${Me.Class.ShortName.Equal[BRD]}) /return

    | Priority 1: Self panic heal
    /if (${Me.PctHPs} <= ${PanicHealPct}) {
        GZMDEBUG_HEAL PANIC: Self at ${Me.PctHPs}%!
        /call PanicHeal ${Me.ID}
        /return
    }

    | Priority 2: Tank panic heal
    /if (${Spawn[=${MainAssist}].PctHPs} <= ${PanicHealPct}) {
        GZMDEBUG_HEAL PANIC: Tank at ${Spawn[=${MainAssist}].PctHPs}%!
        /call PanicHeal ${Spawn[=${MainAssist}].ID}
        /return
    }

    | Priority 3: Check for intervention target
    /if (${UseIntervention} && ${InterventionSpell.Length}) {
        /call CheckIntervention
    }

    | Priority 4: Regular healing rotation
    /call HealRotation

    | Priority 5: Group heals if needed
    /call CheckGroupHeals

    | Priority 6: HoTs on tank
    /call CheckHoTs

    | Priority 7: Cures
    /if (${CureOn}) {
        /call CheckCures
    }

    | Priority 8: XTarget healing     /if (${XTargetHealOn}) {
        /call CheckXTargetHealing
    }

    | Priority 9: Rez check (if out of combat or RezCombat enabled)
    /if (${RezOn} && (!${InCombat} || ${RezCombat})) {
        /call CheckRez
    }

    | Priority 10: OOG Healing
    /if (${OOGHealOn}) {
        /call CheckOOGHealing
    }
/return

| ============================================================================
| SUB: Panic Heal (Emergency)
| ============================================================================
Sub PanicHeal(int TargetID)
    /declare healSpell string local

    GZMDEBUG_HEAL PANIC HEAL on ${Spawn[${TargetID}].CleanName}!

    | Try emergency heal first
    /if (${EmergencyHealSpell.Length} && ${Me.SpellReady[${EmergencyHealSpell}]}) {
        /varset healSpell ${EmergencyHealSpell}
    } else /if (${QuickHealSpell.Length} && ${Me.SpellReady[${QuickHealSpell}]}) {
        /varset healSpell ${QuickHealSpell}
    } else {
        | Find any ready heal
        /call FindReadyHeal
        /varset healSpell ${Macro.Return}
    }

    /if (${healSpell.Length}) {
        /call CastHeal "${healSpell}" ${TargetID}
    } else {
        GZMDEBUG_HEAL No heal spell ready for panic!
    }
/return

| ============================================================================
| SUB: Heal Rotation
| ============================================================================
Sub HealRotation
    /declare i int local
    /declare targetID int local 0
    /declare lowestPct int local 100
    /declare lowestID int local 0
    /declare checkPct int local

    | Build priority heal list
    | 1. Self
    /if (${Me.PctHPs} < ${GroupMemberHealPct}) {
        /varset lowestPct ${Me.PctHPs}
        /varset lowestID ${Me.ID}
    }

    | 2. Tank/MA
    /if (${Spawn[=${MainAssist}].ID} && ${Spawn[=${MainAssist}].Distance} < 200) {
        /varset checkPct ${Spawn[=${MainAssist}].PctHPs}
        /if (${checkPct} < ${TankHealPct} && ${checkPct} < ${lowestPct}) {
            /varset lowestPct ${checkPct}
            /varset lowestID ${Spawn[=${MainAssist}].ID}
        }
    }

    | 3. Group members
    /for i 1 to ${Group.Members}
        /if (${Group.Member[${i}].ID} && ${Group.Member[${i}].Distance} < 200) {
            /varset checkPct ${Group.Member[${i}].PctHPs}
            /if (${checkPct} < ${GroupMemberHealPct} && ${checkPct} < ${lowestPct}) {
                /varset lowestPct ${checkPct}
                /varset lowestID ${Group.Member[${i}].ID}
            }
        }
    /next i

    | 4. Group pets
    /for i 1 to ${Group.Members}
        /if (${Group.Member[${i}].Pet.ID} && ${Group.Member[${i}].Pet.Distance} < 200) {
            /varset checkPct ${Group.Member[${i}].Pet.PctHPs}
            /if (${checkPct} < 50 && ${checkPct} < ${lowestPct}) {
                /varset lowestPct ${checkPct}
                /varset lowestID ${Group.Member[${i}].Pet.ID}
            }
        }
    /next i

    | Heal the lowest
    /if (${lowestID} && ${lowestPct} < 100) {
        /call SelectHealSpell ${lowestID} ${lowestPct}
        /if (${Macro.Return.Length}) {
            /call CastHeal "${Macro.Return}" ${lowestID}
        }
    }
/return

| ============================================================================
| SUB: Select Appropriate Heal Spell
| ============================================================================
Sub SelectHealSpell(int TargetID, int TargetPct)
    /declare i int local
    /declare selectedSpell string local

    | Check promised heal first if target is tank and promised ready
    /if (${UsePromised} && ${PromisedSpell.Length} && ${TargetID} == ${Spawn[=${MainAssist}].ID}) {
        /if (!${PromisedTimer} && ${Me.SpellReady[${PromisedSpell}]}) {
            /return ${PromisedSpell}
        }
    }

    | Find appropriate heal based on target health
    /for i 1 to ${HealSpellCount}
        /if (${HealSpell[${i}].Length} && ${TargetPct} <= ${HealSpellPct[${i}]}) {
            /if (${Me.SpellReady[${HealSpell[${i}]}]}) {
                /return ${HealSpell[${i}]}
            }
        }
    /next i

    | Fallback to any ready heal
    /call FindReadyHeal
/return ${Macro.Return}

| ============================================================================
| SUB: Find Any Ready Heal
| ============================================================================
Sub FindReadyHeal
    /declare i int local

    /for i 1 to ${HealSpellCount}
        /if (${HealSpell[${i}].Length} && ${Me.SpellReady[${HealSpell[${i}]}]}) {
            /return ${HealSpell[${i}]}
        }
    /next i
/return

| ============================================================================
| SUB: Check Group Heals
| ============================================================================
Sub CheckGroupHeals
    /declare i int local
    /declare lowCount int local 0
    /declare lowPct int local 100

    | Count group members below threshold
    /for i 0 to ${Group.Members}
        /if (${Group.Member[${i}].ID} && ${Group.Member[${i}].Distance} < 100) {
            /if (${Group.Member[${i}].PctHPs} < ${GroupMemberHealPct}) {
                /varcalc lowCount ${lowCount}+1
                /if (${Group.Member[${i}].PctHPs} < ${lowPct}) {
                    /varset lowPct ${Group.Member[${i}].PctHPs}
                }
            }
        }
    /next i

    | Check if group heal is warranted
    /for i 1 to ${GroupHealSpellCount}
        /if (${lowCount} >= ${GroupHealCount[${i}]} && ${lowPct} <= ${GroupHealPct[${i}]}) {
            /if (${Me.SpellReady[${GroupHealSpell[${i}]}]}) {
                GZMDEBUG_HEAL Group heal: ${lowCount} members low
                /call CastHeal "${GroupHealSpell[${i}]}" ${Me.ID}
                /return
            }
        }
    /next i
/return

| ============================================================================
| SUB: Check HoTs
| ============================================================================
Sub CheckHoTs
    /declare i int local
    /declare tankID int local ${Spawn[=${MainAssist}].ID}

    /if (!${tankID} || ${Spawn[${tankID}].Distance} > 200) /return

    | Check if tank needs HoT
    /for i 1 to ${HoTSpellCount}
        /if (${Spawn[${tankID}].PctHPs} <= ${HoTSpellPct[${i}]}) {
            | Check if HoT already on tank
            /if (!${Spawn[${tankID}].Buff[${Spell[${HoTSpell[${i}]}].Trigger}].ID} && !${Spawn[${tankID}].Buff[${HoTSpell[${i}]}].ID}) {
                /if (${Me.SpellReady[${HoTSpell[${i}]}]}) {
                    GZMDEBUG_HEAL Applying HoT to tank
                    /call CastHeal "${HoTSpell[${i}]}" ${tankID}
                    /return
                }
            }
        }
    /next i
/return

| ============================================================================
| SUB: Check Intervention
| ============================================================================
Sub CheckIntervention
    /declare i int local
    /declare targetID int local 0

    | Look for intervention target (very low health)
    /for i 0 to ${Group.Members}
        /if (${Group.Member[${i}].ID} && ${Group.Member[${i}].PctHPs} <= ${InterventionPct}) {
            /if (${Group.Member[${i}].Distance} < 100) {
                /varset targetID ${Group.Member[${i}].ID}
                /break
            }
        }
    /next i

    /if (${targetID} && ${Me.AltAbilityReady[${InterventionSpell}]}) {
        GZMDEBUG_HEAL INTERVENTION on ${Spawn[${targetID}].CleanName}!
        /call CastHeal "${InterventionSpell}" ${targetID}
    }
/return

| ============================================================================
| SUB: Check Cures
| ============================================================================
Sub CheckCures
    /declare i int local
    /declare j int local
    /declare targetID int local 0
    /declare detrimental string local

    | Check self first
    /if (${Me.Poisoned} && ${CurePoison}) {
        /call FindCureSpell Poison
        /if (${Macro.Return.Length}) {
            /call CastHeal "${Macro.Return}" ${Me.ID}
            /return
        }
    }
    /if (${Me.Diseased} && ${CureDisease}) {
        /call FindCureSpell Disease
        /if (${Macro.Return.Length}) {
            /call CastHeal "${Macro.Return}" ${Me.ID}
            /return
        }
    }
    /if (${Me.Cursed} && ${CureCurse}) {
        /call FindCureSpell Curse
        /if (${Macro.Return.Length}) {
            /call CastHeal "${Macro.Return}" ${Me.ID}
            /return
        }
    }
    /if (${Me.Corrupted} && ${CureCorrupt}) {
        /call FindCureSpell Corrupt
        /if (${Macro.Return.Length}) {
            /call CastHeal "${Macro.Return}" ${Me.ID}
            /return
        }
    }

    | Check group members
    /for i 1 to ${Group.Members}
        /if (${Group.Member[${i}].ID} && ${Group.Member[${i}].Distance} < 200) {
            /if (${Group.Member[${i}].Poisoned} && ${CurePoison}) {
                /call FindCureSpell Poison
                /if (${Macro.Return.Length}) {
                    /call CastHeal "${Macro.Return}" ${Group.Member[${i}].ID}
                    /return
                }
            }
            /if (${Group.Member[${i}].Diseased} && ${CureDisease}) {
                /call FindCureSpell Disease
                /if (${Macro.Return.Length}) {
                    /call CastHeal "${Macro.Return}" ${Group.Member[${i}].ID}
                    /return
                }
            }
            /if (${Group.Member[${i}].Cursed} && ${CureCurse}) {
                /call FindCureSpell Curse
                /if (${Macro.Return.Length}) {
                    /call CastHeal "${Macro.Return}" ${Group.Member[${i}].ID}
                    /return
                }
            }
        }
    /next i
/return

| ============================================================================
| SUB: Find Cure Spell
| ============================================================================
Sub FindCureSpell(string CureType)
    /declare i int local

    /for i 1 to ${CureSpellCount}
        /if (${CureSpellType[${i}].Equal[${CureType}]} || ${CureSpellType[${i}].Equal[All]}) {
            /if (${Me.SpellReady[${CureSpell[${i}]}]}) {
                /return ${CureSpell[${i}]}
            }
        }
    /next i
/return

| ============================================================================
| SUB: Check Rez
| ============================================================================
Sub CheckRez
    /declare i int local
    /declare corpseID int local 0

    | Find group member corpse
    /for i 1 to ${Group.Members}
        /if (${Group.Member[${i}].Dead}) {
            /varset corpseID ${NearestSpawn[corpse ${Group.Member[${i}].CleanName}].ID}
            /if (${corpseID} && ${Spawn[${corpseID}].Distance} < ${RezRadius}) {
                /call DoRez ${corpseID}
                /return
            }
        }
    /next i

    | Check for fellowship corpses via DanNet
    /if (${UseDanNet} && ${RezOOC}) {
        | Query DanNet for dead fellowship members
    }
/return

| ============================================================================
| SUB: Do Rez
| ============================================================================
Sub DoRez(int CorpseID)
    /declare rezSpell string local

    /if (!${RezSpellCount}) {
        | Try AA rez
        /if (${Me.AltAbilityReady[Blessing of Resurrection]}) {
            /varset rezSpell "Blessing of Resurrection"
        } else /if (${Me.AltAbilityReady[Call of the Wild]}) {
            /varset rezSpell "Call of the Wild"
        }
    } else {
        /varset rezSpell ${RezSpell[1]}
    }

    /if (${rezSpell.Length} && ${Me.SpellReady[${rezSpell}]}) {
        GZMDEBUG_HEAL Rezzing ${Spawn[${CorpseID}].CleanName}
        /target id ${CorpseID}
        /delay 5 ${Target.ID} == ${CorpseID}
        /call CastSpell "${rezSpell}" ${CorpseID} 0
    }
/return

| ============================================================================
| SUB: Check XTarget Healing | Heals friendly targets in XTarget slots
| ============================================================================
Sub CheckXTargetHealing
    /if (!${XTargetHealOn}) /return

    /declare i int local
    /declare slot int local
    /declare xtID int local
    /declare xtHP int local
    /declare lowestID int local 0
    /declare lowestHP int local 100
    /declare lowestPriority int local 99
    /declare xtClass string local
    /declare classPct int local
    /declare classPriority int local

    | Parse XTarget heal slots
    /declare slotCount int local ${Math.Calc[${XTargetHealSlots.Count[ ]}+1]}

    /for i 1 to ${slotCount}
        /varset slot ${XTargetHealSlots.Arg[${i}, ]}
        /if (!${slot}) /continue

        | Get XTarget info
        /varset xtID ${Me.XTarget[${slot}].ID}
        /if (!${xtID}) /continue

        | Skip if not a PC or pet
        /if (!${Spawn[${xtID}].Type.Equal[PC]} && !${Spawn[${xtID}].Type.Equal[Pet]}) /continue

        | Skip if too far
        /if (${Spawn[${xtID}].Distance} > ${XTargetHealRange}) /continue

        | Get HP and class
        /varset xtHP ${Spawn[${xtID}].PctHPs}
        /varset xtClass ${Spawn[${xtID}].Class.ShortName}

        | Get class-specific threshold and priority
        /call GetClassHealPct ${xtClass}
        /varset classPct ${Macro.Return}
        /call GetClassHealPriority ${xtClass}
        /varset classPriority ${Macro.Return}

        | Check if needs healing based on class threshold
        /if (${xtHP} < ${classPct}) {
            | Priority-based selection (lower priority number = heal first)
            | If same priority, heal lowest HP
            /if (${classPriority} < ${lowestPriority} || (${classPriority} == ${lowestPriority} && ${xtHP} < ${lowestHP})) {
                /varset lowestID ${xtID}
                /varset lowestHP ${xtHP}
                /varset lowestPriority ${classPriority}
            }
        }
    /next i

    | Heal the selected target
    /if (${lowestID}) {
        /if (${HealDebug}) /echo [GZM_Heals] XTarget heal: ${Spawn[${lowestID}].CleanName} at ${lowestHP}%
        /call SelectHealSpell ${lowestID} ${lowestHP}
        /if (${Macro.Return.Length}) {
            /call CastHeal "${Macro.Return}" ${lowestID}
        }
    }
/return

| ============================================================================
| SUB: Get Class Heal Percentage Threshold
| ============================================================================
Sub GetClassHealPct(string ClassName)
    /if (${ClassName.Equal[WAR]}) /return ${HealPct_WAR}
    /if (${ClassName.Equal[PAL]}) /return ${HealPct_PAL}
    /if (${ClassName.Equal[SHD]}) /return ${HealPct_SHD}
    /if (${ClassName.Equal[CLR]}) /return ${HealPct_CLR}
    /if (${ClassName.Equal[DRU]}) /return ${HealPct_DRU}
    /if (${ClassName.Equal[SHM]}) /return ${HealPct_SHM}
    /if (${ClassName.Equal[ENC]}) /return ${HealPct_ENC}
    /if (${ClassName.Equal[WIZ]}) /return ${HealPct_WIZ}
    /if (${ClassName.Equal[MAG]}) /return ${HealPct_MAG}
    /if (${ClassName.Equal[NEC]}) /return ${HealPct_NEC}
    /if (${ClassName.Equal[ROG]}) /return ${HealPct_ROG}
    /if (${ClassName.Equal[MNK]}) /return ${HealPct_MNK}
    /if (${ClassName.Equal[BER]}) /return ${HealPct_BER}
    /if (${ClassName.Equal[RNG]}) /return ${HealPct_RNG}
    /if (${ClassName.Equal[BST]}) /return ${HealPct_BST}
    /if (${ClassName.Equal[BRD]}) /return ${HealPct_BRD}
    | Default for pets/unknown
/return ${HealPct_PET}

| ============================================================================
| SUB: Get Class Heal Priority (1=highest, 5=lowest)
| ============================================================================
Sub GetClassHealPriority(string ClassName)
    /if (${ClassName.Equal[WAR]}) /return ${HealPriority_WAR}
    /if (${ClassName.Equal[PAL]}) /return ${HealPriority_PAL}
    /if (${ClassName.Equal[SHD]}) /return ${HealPriority_SHD}
    /if (${ClassName.Equal[CLR]}) /return ${HealPriority_CLR}
    /if (${ClassName.Equal[DRU]}) /return ${HealPriority_DRU}
    /if (${ClassName.Equal[SHM]}) /return ${HealPriority_SHM}
    /if (${ClassName.Equal[ENC]}) /return ${HealPriority_ENC}
    /if (${ClassName.Equal[WIZ]}) /return ${HealPriority_WIZ}
    /if (${ClassName.Equal[MAG]}) /return ${HealPriority_MAG}
    /if (${ClassName.Equal[NEC]}) /return ${HealPriority_NEC}
    /if (${ClassName.Equal[ROG]}) /return ${HealPriority_ROG}
    /if (${ClassName.Equal[MNK]}) /return ${HealPriority_MNK}
    /if (${ClassName.Equal[BER]}) /return ${HealPriority_BER}
    /if (${ClassName.Equal[RNG]}) /return ${HealPriority_RNG}
    /if (${ClassName.Equal[BST]}) /return ${HealPriority_BST}
    /if (${ClassName.Equal[BRD]}) /return ${HealPriority_BRD}
    | Default for pets/unknown
/return ${HealPriority_PET}

| ============================================================================
| SUB: Check OOG Healing (Out of Group)
| ============================================================================
Sub CheckOOGHealing
    /if (!${UseDanNet}) /return

    /declare peerCount int local ${DanNet.PeerCount}
    /declare i int local
    /declare peerName string local
    /declare peerHP int local

    /for i 1 to ${peerCount}
        /varset peerName ${DanNet.Peers.Arg[${i}, ]}
        | Skip if in group
        /if (${Group.Member[${peerName}].ID}) /continue

        | Check their HP via observed variable
        /if (${DanNet[${peerName}].ObserveSet[Me.PctHPs]}) {
            /varset peerHP ${DanNet[${peerName}].Observe[Me.PctHPs]}
            /if (${peerHP} < ${OOGHealPct} && ${Spawn[=${peerName}].Distance} < ${OOGRange}) {
                /if (${HealDebug}) /echo [GZM_Heals] OOG Heal needed: ${peerName} at ${peerHP}%
                /call CastHeal "${MainHealSpell}" ${Spawn[=${peerName}].ID}
                /return
            }
        }
    /next i
/return

| ============================================================================
| SUB: Cast Heal
| ============================================================================
Sub CastHeal(string SpellName, int TargetID)
    /if (!${SpellName.Length}) /return
    /if (!${TargetID}) /return
    /if (${Spawn[${TargetID}].Distance} > 200) /return

    GZMDEBUG_HEAL Casting ${SpellName} on ${Spawn[${TargetID}].CleanName}

    | Target if needed
    /if (${Target.ID} != ${TargetID}) {
        /target id ${TargetID}
        /delay 5 ${Target.ID} == ${TargetID}
    }

    | Stand if sitting
    /if (${Me.Sitting}) /stand

    | Check if it's an AA
    /if (${Me.AltAbility[${SpellName}].ID}) {
        /if (${Me.AltAbilityReady[${SpellName}]}) {
            /alt act ${Me.AltAbility[${SpellName}].ID}
            /delay 5
        }
        /return
    }

    | Check if it's an item
    /if (${FindItem[${SpellName}].ID}) {
        /if (${FindItem[${SpellName}].TimerReady} == 0) {
            /useitem "${SpellName}"
            /delay 5
        }
        /return
    }

    | Regular spell
    /if (${Me.SpellReady[${SpellName}]}) {
        /cast "${SpellName}"
        /delay 3s ${Me.Casting.ID}
        /delay 3s !${Me.Casting.ID}
    }

    /varset LastHealTime 10
/return

| ============================================================================
| SUB: Cast Spell (Generic)
| ============================================================================
Sub CastSpell(string SpellName, int TargetID, int GemNum)
    /if (!${SpellName.Length}) /return

    | Target
    /if (${TargetID} && ${Target.ID} != ${TargetID}) {
        /target id ${TargetID}
        /delay 5 ${Target.ID} == ${TargetID}
    }

    | Stand
    /if (${Me.Sitting}) /stand

    | Cast
    /if (${GemNum}) {
        /cast ${GemNum}
    } else {
        /cast "${SpellName}"
    }

    /delay 5s ${Me.Casting.ID}
    /delay 10s !${Me.Casting.ID}
/return

| ============================================================================
| CLASS-SPECIFIC HEALING ROUTINES
| ============================================================================

| ============================================================================
| SUB: Cleric Specific
| ============================================================================
Sub ClericHealRoutine
    | Celestial Regeneration AA (group HoT)
    /if (${Me.PctMana} > 20 && ${Me.AltAbilityReady[Celestial Regeneration]}) {
        /declare lowCount int local 0
        /declare i int local
        /for i 0 to ${Group.Members}
            /if (${Group.Member[${i}].PctHPs} < 80) /varcalc lowCount ${lowCount}+1
        /next i
        /if (${lowCount} >= 3) {
            /alt act ${Me.AltAbility[Celestial Regeneration].ID}
            /return
        }
    }

    | Divine Arbitration
    /if (${Me.AltAbilityReady[Divine Arbitration]}) {
        /if (${Spawn[=${MainAssist}].PctHPs} < 25) {
            /target id ${Spawn[=${MainAssist}].ID}
            /delay 3
            /alt act ${Me.AltAbility[Divine Arbitration].ID}
            /return
        }
    }

    | Beacon of Life (rez AA)
    /if (${Me.AltAbilityReady[Beacon of Life]} && !${InCombat}) {
        /call CheckRez
    }
/return

| ============================================================================
| SUB: Druid Specific
| ============================================================================
Sub DruidHealRoutine
    | Convergence of Spirits
    /if (${Me.AltAbilityReady[Convergence of Spirits]}) {
        /declare lowCount int local 0
        /declare i int local
        /for i 0 to ${Group.Members}
            /if (${Group.Member[${i}].PctHPs} < 70) /varcalc lowCount ${lowCount}+1
        /next i
        /if (${lowCount} >= 3) {
            /alt act ${Me.AltAbility[Convergence of Spirits].ID}
            /return
        }
    }

    | Nature's Boon (emergency heal)
    /if (${Me.AltAbilityReady[Nature's Boon]}) {
        /if (${Me.PctHPs} < 30 || ${Spawn[=${MainAssist}].PctHPs} < 30) {
            /alt act ${Me.AltAbility[Nature's Boon].ID}
            /return
        }
    }
/return

| ============================================================================
| SUB: Shaman Specific
| ============================================================================
Sub ShamanHealRoutine
    | Ancestral Aid
    /if (${Me.AltAbilityReady[Ancestral Aid]}) {
        /if (${Me.PctHPs} < 40 || ${Spawn[=${MainAssist}].PctHPs} < 40) {
            /alt act ${Me.AltAbility[Ancestral Aid].ID}
            /return
        }
    }

    | Call of the Ancients
    /if (${Me.AltAbilityReady[Call of the Ancients]}) {
        /if (${Spawn[=${MainAssist}].PctHPs} < 30) {
            /target id ${Spawn[=${MainAssist}].ID}
            /delay 3
            /alt act ${Me.AltAbility[Call of the Ancients].ID}
            /return
        }
    }

    | Union of Spirits
    /if (${Me.AltAbilityReady[Union of Spirits]}) {
        /declare lowCount int local 0
        /declare i int local
        /for i 0 to ${Group.Members}
            /if (${Group.Member[${i}].PctHPs} < 70) /varcalc lowCount ${lowCount}+1
        /next i
        /if (${lowCount} >= 3) {
            /alt act ${Me.AltAbility[Union of Spirits].ID}
            /return
        }
    }
/return

| ============================================================================
| SUB: Load Heal Settings from INI
| ============================================================================
Sub LoadHealSettings(IniFile)
    /if (!${IniFile.Length}) /return

    | XTarget healing
    /varset XTargetHealOn ${Ini[${IniFile},Heals,XTargetHealOn,${XTargetHealOn}]}
    /varset XTargetHealPct ${Ini[${IniFile},Heals,XTargetHealPct,${XTargetHealPct}]}
    /varset XTargetHealRange ${Ini[${IniFile},Heals,XTargetHealRange,${XTargetHealRange}]}
    /varset XTargetHealSlots ${Ini[${IniFile},Heals,XTargetHealSlots,${XTargetHealSlots}]}

    | Per-class thresholds
    /varset HealPct_WAR ${Ini[${IniFile},HealThresholds,WAR,${HealPct_WAR}]}
    /varset HealPct_PAL ${Ini[${IniFile},HealThresholds,PAL,${HealPct_PAL}]}
    /varset HealPct_SHD ${Ini[${IniFile},HealThresholds,SHD,${HealPct_SHD}]}
    /varset HealPct_CLR ${Ini[${IniFile},HealThresholds,CLR,${HealPct_CLR}]}
    /varset HealPct_DRU ${Ini[${IniFile},HealThresholds,DRU,${HealPct_DRU}]}
    /varset HealPct_SHM ${Ini[${IniFile},HealThresholds,SHM,${HealPct_SHM}]}
    /varset HealPct_ENC ${Ini[${IniFile},HealThresholds,ENC,${HealPct_ENC}]}
    /varset HealPct_WIZ ${Ini[${IniFile},HealThresholds,WIZ,${HealPct_WIZ}]}
    /varset HealPct_MAG ${Ini[${IniFile},HealThresholds,MAG,${HealPct_MAG}]}
    /varset HealPct_NEC ${Ini[${IniFile},HealThresholds,NEC,${HealPct_NEC}]}
    /varset HealPct_ROG ${Ini[${IniFile},HealThresholds,ROG,${HealPct_ROG}]}
    /varset HealPct_MNK ${Ini[${IniFile},HealThresholds,MNK,${HealPct_MNK}]}
    /varset HealPct_BER ${Ini[${IniFile},HealThresholds,BER,${HealPct_BER}]}
    /varset HealPct_RNG ${Ini[${IniFile},HealThresholds,RNG,${HealPct_RNG}]}
    /varset HealPct_BST ${Ini[${IniFile},HealThresholds,BST,${HealPct_BST}]}
    /varset HealPct_BRD ${Ini[${IniFile},HealThresholds,BRD,${HealPct_BRD}]}
    /varset HealPct_PET ${Ini[${IniFile},HealThresholds,PET,${HealPct_PET}]}

    | Heal announce
    /varset HealAnnounce ${Ini[${IniFile},Heals,HealAnnounce,${HealAnnounce}]}
    /varset HealAnnounceChannel ${Ini[${IniFile},Heals,HealAnnounceChannel,${HealAnnounceChannel}]}
    /varset HealAnnounceMinPct ${Ini[${IniFile},Heals,HealAnnounceMinPct,${HealAnnounceMinPct}]}

    /if (${HealDebug}) /echo [GZM_Heals] Settings loaded from ${IniFile}
/return

| ============================================================================
| SUB: Save Heal Settings to INI
| ============================================================================
Sub SaveHealSettings(IniFile)
    /if (!${IniFile.Length}) /return

    | XTarget healing
    /ini "${IniFile}" "Heals" "XTargetHealOn" "${XTargetHealOn}"
    /ini "${IniFile}" "Heals" "XTargetHealPct" "${XTargetHealPct}"
    /ini "${IniFile}" "Heals" "XTargetHealRange" "${XTargetHealRange}"
    /ini "${IniFile}" "Heals" "XTargetHealSlots" "${XTargetHealSlots}"

    | Per-class thresholds
    /ini "${IniFile}" "HealThresholds" "WAR" "${HealPct_WAR}"
    /ini "${IniFile}" "HealThresholds" "PAL" "${HealPct_PAL}"
    /ini "${IniFile}" "HealThresholds" "SHD" "${HealPct_SHD}"
    /ini "${IniFile}" "HealThresholds" "CLR" "${HealPct_CLR}"
    /ini "${IniFile}" "HealThresholds" "DRU" "${HealPct_DRU}"
    /ini "${IniFile}" "HealThresholds" "SHM" "${HealPct_SHM}"
    /ini "${IniFile}" "HealThresholds" "ENC" "${HealPct_ENC}"
    /ini "${IniFile}" "HealThresholds" "WIZ" "${HealPct_WIZ}"
    /ini "${IniFile}" "HealThresholds" "MAG" "${HealPct_MAG}"
    /ini "${IniFile}" "HealThresholds" "NEC" "${HealPct_NEC}"
    /ini "${IniFile}" "HealThresholds" "ROG" "${HealPct_ROG}"
    /ini "${IniFile}" "HealThresholds" "MNK" "${HealPct_MNK}"
    /ini "${IniFile}" "HealThresholds" "BER" "${HealPct_BER}"
    /ini "${IniFile}" "HealThresholds" "RNG" "${HealPct_RNG}"
    /ini "${IniFile}" "HealThresholds" "BST" "${HealPct_BST}"
    /ini "${IniFile}" "HealThresholds" "BRD" "${HealPct_BRD}"
    /ini "${IniFile}" "HealThresholds" "PET" "${HealPct_PET}"

    | Heal announce
    /ini "${IniFile}" "Heals" "HealAnnounce" "${HealAnnounce}"
    /ini "${IniFile}" "Heals" "HealAnnounceChannel" "${HealAnnounceChannel}"
    /ini "${IniFile}" "Heals" "HealAnnounceMinPct" "${HealAnnounceMinPct}"

    /echo [GZM_Heals] Settings saved to ${IniFile}
/return

| ============================================================================
| SUB: Get Heal Status
| ============================================================================
Sub GetHealStatus
    /echo [GZM_Heals] === Heal System Status ===
    /echo Heals On: ${If[${HealsOn},Yes,No]}
    /echo Heal Spells Loaded: ${HealSpellCount}
    /echo Group Heals Loaded: ${GroupHealSpellCount}
    /echo HoTs Loaded: ${HoTSpellCount}
    /echo Cures Loaded: ${CureSpellCount}
    /echo XTarget Healing: ${If[${XTargetHealOn},Yes,No]} (Slots: ${XTargetHealSlots})
    /echo Tank Heal Pct: ${TankHealPct}%
    /echo Panic Heal Pct: ${PanicHealPct}%
    /echo ========================
/return

| ============================================================================
| END OF HEALING INCLUDE
| ============================================================================
