|**
    GZM_buffs.inc - GronnzMaster Buff System

    Comprehensive buffing routines including:
    - Self buffs
    - Group buffs
    - Pet buffs
    - Out-of-group buffs via DanNet
    - Blocked buff handling
    - Rebuffing
**|

| ============================================================================
| BUFF CONSTANTS
| ============================================================================
#DEFINE MAXSELFBUFFS 20
#DEFINE MAXGROUPBUFFS 15
#DEFINE MAXPETBUFFS 10
#DEFINE MAXOOGBUFFS 15
#DEFINE MAXAURABUFFS 5
#DEFINE MAXCOMBATBUFFS 10

| ============================================================================
| SUB: Initialize Buff System
| ============================================================================
Sub InitBuffSystem
    /declare i int local

    | Self buff arrays
    /declare SelfBuff[${MAXSELFBUFFS}] string outer
    /declare SelfBuffGem[${MAXSELFBUFFS}] int outer
    /declare SelfBuffCount int outer 0

    | Group buff arrays
    /declare GroupBuff[${MAXGROUPBUFFS}] string outer
    /declare GroupBuffClass[${MAXGROUPBUFFS}] string outer
    /declare GroupBuffGem[${MAXGROUPBUFFS}] int outer
    /declare GroupBuffCount int outer 0

    | Pet buff arrays
    /declare PetBuff[${MAXPETBUFFS}] string outer
    /declare PetBuffCount int outer 0

    | OOG buff arrays
    /declare OOGBuff[${MAXOOGBUFFS}] string outer
    /declare OOGBuffCount int outer 0

    | Aura arrays
    /declare AuraBuff[${MAXAURABUFFS}] string outer
    /declare AuraBuffCount int outer 0

    | Combat buff arrays (short duration/combat only)
    /declare CombatBuff[${MAXCOMBATBUFFS}] string outer
    /declare CombatBuffCount int outer 0

    | Buff settings
    /declare BuffCheckInterval int outer 30
    /declare MinBuffDuration int outer 180
    /declare RebuffPct int outer 20
    /declare BuffGroupPets int outer 1
    /declare UseBlockedBuffs int outer 1
    /declare BlockedBuffList string outer

    | Timers
    /declare BuffCheckTimer timer outer 0

    | Load buff spells
    /call LoadBuffSpells

    /echo \ag[GZM-Buffs] Buff system initialized - ${SelfBuffCount} self, ${GroupBuffCount} group, ${PetBuffCount} pet buffs loaded
/return

| ============================================================================
| SUB: Load Buff Spells from INI
| ============================================================================
Sub LoadBuffSpells
    /declare i int local
    /declare spell string local

    | Load self buffs
    /for i 1 to ${MAXSELFBUFFS}
        /varset spell ${Ini[${IniFileName},SelfBuffs,SelfBuff${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset SelfBuff[${i}] ${spell}
            /varset SelfBuffGem[${i}] ${Ini[${IniFileName},SelfBuffs,SelfBuff${i}Gem,0]}
            /varcalc SelfBuffCount ${SelfBuffCount}+1
            GZMDEBUG_BUFF Loaded self buff ${i}: ${spell}
        }
    /next i

    | Load group buffs
    /for i 1 to ${MAXGROUPBUFFS}
        /varset spell ${Ini[${IniFileName},GroupBuffs,GroupBuff${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset GroupBuff[${i}] ${spell}
            /varset GroupBuffClass[${i}] ${Ini[${IniFileName},GroupBuffs,GroupBuff${i}Class,All]}
            /varset GroupBuffGem[${i}] ${Ini[${IniFileName},GroupBuffs,GroupBuff${i}Gem,0]}
            /varcalc GroupBuffCount ${GroupBuffCount}+1
            GZMDEBUG_BUFF Loaded group buff ${i}: ${spell}
        }
    /next i

    | Load pet buffs
    /for i 1 to ${MAXPETBUFFS}
        /varset spell ${Ini[${IniFileName},PetBuffs,PetBuff${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset PetBuff[${i}] ${spell}
            /varcalc PetBuffCount ${PetBuffCount}+1
        }
    /next i

    | Load OOG buffs
    /for i 1 to ${MAXOOGBUFFS}
        /varset spell ${Ini[${IniFileName},OOGBuffs,OOGBuff${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset OOGBuff[${i}] ${spell}
            /varcalc OOGBuffCount ${OOGBuffCount}+1
        }
    /next i

    | Load auras
    /for i 1 to ${MAXAURABUFFS}
        /varset spell ${Ini[${IniFileName},Auras,Aura${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset AuraBuff[${i}] ${spell}
            /varcalc AuraBuffCount ${AuraBuffCount}+1
        }
    /next i

    | Load combat buffs
    /for i 1 to ${MAXCOMBATBUFFS}
        /varset spell ${Ini[${IniFileName},CombatBuffs,CombatBuff${i}]}
        /if (${spell.Length} && ${spell.NotEqual[NULL]}) {
            /varset CombatBuff[${i}] ${spell}
            /varcalc CombatBuffCount ${CombatBuffCount}+1
        }
    /next i

    | Load blocked buff list
    /varset BlockedBuffList ${Ini[${IniFileName},Buffs,BlockedBuffs]}

    | Load settings
    /varset BuffCheckInterval ${Ini[${IniFileName},Buffs,BuffCheckInterval,30]}
    /varset MinBuffDuration ${Ini[${IniFileName},Buffs,MinBuffDuration,180]}
    /varset RebuffPct ${Ini[${IniFileName},Buffs,RebuffPct,20]}
/return

| ============================================================================
| SUB: Main Buff Check (called from main loop)
| ============================================================================
Sub CheckBuffs
    /if (!${BuffsOn}) /return
    /if (${Me.Moving}) /return
    /if (${Me.Casting.ID}) /return
    /if (${BuffCheckTimer}) /return
    /if (${InCombat}) /return

    GZMDEBUG_BUFF Checking buffs...

    | Priority 1: Auras
    /call CheckAuras

    | Priority 2: Self buffs
    /call CheckSelfBuffs

    | Priority 3: Group buffs
    /call CheckGroupBuffs

    | Priority 4: Pet buffs
    /if (${Me.Pet.ID}) {
        /call CheckMyPetBuffs
    }

    | Priority 5: Group pet buffs
    /if (${BuffGroupPets}) {
        /call CheckGroupPetBuffs
    }

    | Priority 6: OOG buffs
    /if (${OOGBuffOn}) {
        /call CheckOOGBuffs
    }

    /varset BuffCheckTimer ${BuffCheckInterval}
/return

| ============================================================================
| SUB: Check Combat Buffs (called during combat)
| ============================================================================
Sub CheckCombatBuffs
    /declare i int local

    /for i 1 to ${CombatBuffCount}
        /if (!${CombatBuff[${i}].Length}) /continue

        | Check if buff is up
        /if (${Me.Buff[${CombatBuff[${i}]}].ID}) /continue
        /if (${Me.Song[${CombatBuff[${i}]}].ID}) /continue

        | Cast combat buff
        /call CastBuff "${CombatBuff[${i}]}" ${Me.ID}
    /next i
/return

| ============================================================================
| SUB: Check Auras
| ============================================================================
Sub CheckAuras
    /declare i int local

    /for i 1 to ${AuraBuffCount}
        /if (!${AuraBuff[${i}].Length}) /continue

        | Check if aura is active
        /if (${Me.Aura[1].Name.Find[${AuraBuff[${i}]}]}) /continue
        /if (${Me.Aura[2].Name.Find[${AuraBuff[${i}]}]}) /continue

        | Activate aura
        /if (${Me.AltAbility[${AuraBuff[${i}]}].ID}) {
            /if (${Me.AltAbilityReady[${AuraBuff[${i}]}]}) {
                GZMDEBUG_BUFF Activating aura: ${AuraBuff[${i}]}
                /alt act ${Me.AltAbility[${AuraBuff[${i}]}].ID}
                /delay 10
                /return
            }
        } else /if (${Me.Book[${AuraBuff[${i}]}]}) {
            /if (${Me.SpellReady[${AuraBuff[${i}]}]}) {
                GZMDEBUG_BUFF Casting aura: ${AuraBuff[${i}]}
                /cast "${AuraBuff[${i}]}"
                /delay 3s ${Me.Casting.ID}
                /delay 5s !${Me.Casting.ID}
                /return
            }
        }
    /next i
/return

| ============================================================================
| SUB: Check Self Buffs
| ============================================================================
Sub CheckSelfBuffs
    /declare i int local
    /declare buffName string local

    /for i 1 to ${SelfBuffCount}
        /if (!${SelfBuff[${i}].Length}) /continue

        | Get the buff name (could be triggered buff)
        /varset buffName ${SelfBuff[${i}]}
        /if (${Spell[${SelfBuff[${i}]}].Trigger.Length}) {
            /varset buffName ${Spell[${SelfBuff[${i}]}].Trigger}
        }

        | Check if buff is up
        /if (${Me.Buff[${buffName}].ID}) {
            | Check duration
            /if (${Me.Buff[${buffName}].Duration.TotalSeconds} > ${MinBuffDuration}) {
                /continue
            }
        }

        | Check if blocked
        /if (${BlockedBuffList.Find[${buffName}]}) /continue

        | Cast buff
        /call CastBuff "${SelfBuff[${i}]}" ${Me.ID}
        /if (${Macro.Return}) /return
    /next i
/return

| ============================================================================
| SUB: Check Group Buffs
| ============================================================================
Sub CheckGroupBuffs
    /declare i int local
    /declare j int local
    /declare buffName string local
    /declare memberID int local
    /declare memberClass string local
    /declare needsBuff int local

    /for i 1 to ${GroupBuffCount}
        /if (!${GroupBuff[${i}].Length}) /continue

        | Get the buff name
        /varset buffName ${GroupBuff[${i}]}
        /if (${Spell[${GroupBuff[${i}]}].Trigger.Length}) {
            /varset buffName ${Spell[${GroupBuff[${i}]}].Trigger}
        }

        | Check each group member
        /for j 0 to ${Group.Members}
            /varset memberID ${Group.Member[${j}].ID}
            /if (!${memberID}) /continue
            /if (${Spawn[${memberID}].Distance} > 200) /continue

            | Check class restriction
            /varset memberClass ${Spawn[${memberID}].Class.ShortName}
            /if (!${GroupBuffClass[${i}].Equal[All]}) {
                /if (!${GroupBuffClass[${i}].Find[${memberClass}]}) /continue
            }

            | Check if they need the buff
            /call NeedsBuff ${memberID} "${buffName}"
            /if (!${Macro.Return}) /continue

            | Check if blocked for this member
            /if (${UseBlockedBuffs}) {
                /call IsBuffBlocked ${memberID} "${buffName}"
                /if (${Macro.Return}) /continue
            }

            | Cast buff
            GZMDEBUG_BUFF Buffing ${Spawn[${memberID}].CleanName} with ${GroupBuff[${i}]}
            /call CastBuff "${GroupBuff[${i}]}" ${memberID}
            /if (${Macro.Return}) /return
        /next j
    /next i
/return

| ============================================================================
| SUB: Check My Pet Buffs
| ============================================================================
Sub CheckMyPetBuffs
    /declare i int local
    /declare buffName string local

    /if (!${Me.Pet.ID}) /return

    /for i 1 to ${PetBuffCount}
        /if (!${PetBuff[${i}].Length}) /continue

        | Get buff name
        /varset buffName ${PetBuff[${i}]}
        /if (${Spell[${PetBuff[${i}]}].Trigger.Length}) {
            /varset buffName ${Spell[${PetBuff[${i}]}].Trigger}
        }

        | Check if pet has buff
        /if (${Me.Pet.Buff[${buffName}].ID}) {
            /if (${Me.Pet.Buff[${buffName}].Duration.TotalSeconds} > ${MinBuffDuration}) {
                /continue
            }
        }

        | Cast buff on pet
        GZMDEBUG_PET Buffing pet with ${PetBuff[${i}]}
        /call CastBuff "${PetBuff[${i}]}" ${Me.Pet.ID}
        /if (${Macro.Return}) /return
    /next i
/return

| ============================================================================
| SUB: Check Group Pet Buffs
| ============================================================================
Sub CheckGroupPetBuffs
    /declare i int local
    /declare j int local
    /declare petID int local
    /declare buffName string local

    /for i 1 to ${PetBuffCount}
        /if (!${PetBuff[${i}].Length}) /continue

        /varset buffName ${PetBuff[${i}]}
        /if (${Spell[${PetBuff[${i}]}].Trigger.Length}) {
            /varset buffName ${Spell[${PetBuff[${i}]}].Trigger}
        }

        /for j 1 to ${Group.Members}
            /varset petID ${Group.Member[${j}].Pet.ID}
            /if (!${petID}) /continue
            /if (${Spawn[${petID}].Distance} > 200) /continue

            | Check if pet needs buff
            /call NeedsBuff ${petID} "${buffName}"
            /if (!${Macro.Return}) /continue

            | Cast buff
            GZMDEBUG_PET Buffing ${Spawn[${petID}].CleanName} with ${PetBuff[${i}]}
            /call CastBuff "${PetBuff[${i}]}" ${petID}
            /if (${Macro.Return}) /return
        /next j
    /next i
/return

| ============================================================================
| SUB: Check OOG Buffs (Out of Group)
| ============================================================================
Sub CheckOOGBuffs
    /if (!${UseDanNet}) /return
    /if (!${OOGBuffCount}) /return

    /declare i int local
    /declare j int local
    /declare peerName string local
    /declare peerID int local
    /declare buffName string local
    /declare hasBuff int local

    | Iterate through DanNet peers
    /for j 1 to ${DanNet.PeerCount}
        /varset peerName ${DanNet.Peers.Arg[${j}, ]}

        | Skip if in group
        /if (${Group.Member[${peerName}].ID}) /continue

        | Get their spawn ID
        /varset peerID ${Spawn[=${peerName}].ID}
        /if (!${peerID}) /continue
        /if (${Spawn[${peerID}].Distance} > ${OOGRange}) /continue

        | Check each OOG buff
        /for i 1 to ${OOGBuffCount}
            /if (!${OOGBuff[${i}].Length}) /continue

            /varset buffName ${OOGBuff[${i}]}
            /if (${Spell[${OOGBuff[${i}]}].Trigger.Length}) {
                /varset buffName ${Spell[${OOGBuff[${i}]}].Trigger}
            }

            | Query DanNet if they have the buff
            /if (${DanNet[${peerName}].ObserveSet[Me.Buff[${buffName}].ID]}) {
                /varset hasBuff ${DanNet[${peerName}].Observe[Me.Buff[${buffName}].ID]}
                /if (${hasBuff}) /continue
            }

            | Cast OOG buff
            GZMDEBUG_OOG OOG Buffing ${peerName} with ${OOGBuff[${i}]}
            /call CastBuff "${OOGBuff[${i}]}" ${peerID}
            /if (${Macro.Return}) /return
        /next i
    /next j
/return

| ============================================================================
| SUB: Needs Buff - Check if target needs the buff
| ============================================================================
Sub NeedsBuff(int TargetID, string BuffName)
    /declare duration int local 0

    | Check buff by ID
    /if (${Spawn[${TargetID}].Type.Equal[PC]}) {
        | Query via DanNet if possible
        /if (${UseDanNet}) {
            /if (${DanNet[${Spawn[${TargetID}].CleanName}].ObserveSet[Me.Buff[${BuffName}].Duration.TotalSeconds]}) {
                /varset duration ${DanNet[${Spawn[${TargetID}].CleanName}].Observe[Me.Buff[${BuffName}].Duration.TotalSeconds]}
                /if (${duration} > ${MinBuffDuration}) {
                    /return 0
                }
            }
        }

        | Direct check if we can target them
        /if (${Target.ID} == ${TargetID}) {
            /if (${Target.Buff[${BuffName}].ID}) {
                /if (${Target.Buff[${BuffName}].Duration.TotalSeconds} > ${MinBuffDuration}) {
                    /return 0
                }
            }
        }
    } else /if (${Spawn[${TargetID}].Type.Equal[Pet]}) {
        | Pet buff check
        /if (${Spawn[${TargetID}].Master.CleanName.Equal[${Me.CleanName}]}) {
            /if (${Me.Pet.Buff[${BuffName}].ID}) {
                /if (${Me.Pet.Buff[${BuffName}].Duration.TotalSeconds} > ${MinBuffDuration}) {
                    /return 0
                }
            }
        }
    }

/return 1

| ============================================================================
| SUB: Is Buff Blocked
| ============================================================================
Sub IsBuffBlocked(int TargetID, string BuffName)
    | Check global blocked list
    /if (${BlockedBuffList.Find[${BuffName}]}) {
        /return 1
    }

    | Check target's blocked buffs via DanNet
    /if (${UseDanNet} && ${Spawn[${TargetID}].Type.Equal[PC]}) {
        /declare targetName string local ${Spawn[${TargetID}].CleanName}
        /if (${DanNet[${targetName}].ObserveSet[Me.BlockedBuff[${BuffName}]]}) {
            /if (${DanNet[${targetName}].Observe[Me.BlockedBuff[${BuffName}]]}) {
                /return 1
            }
        }
    }

/return 0

| ============================================================================
| SUB: Cast Buff
| ============================================================================
Sub CastBuff(string SpellName, int TargetID)
    /if (!${SpellName.Length}) /return 0
    /if (!${TargetID}) /return 0
    /if (${Spawn[${TargetID}].Distance} > 200) /return 0

    | Target if needed
    /if (${Target.ID} != ${TargetID}) {
        /target id ${TargetID}
        /delay 10 ${Target.ID} == ${TargetID}
        /if (${Target.ID} != ${TargetID}) /return 0
    }

    | Stand if sitting
    /if (${Me.Sitting}) /stand

    | Check if it's an AA
    /if (${Me.AltAbility[${SpellName}].ID}) {
        /if (${Me.AltAbilityReady[${SpellName}]}) {
            GZMDEBUG_BUFF Casting AA buff: ${SpellName}
            /alt act ${Me.AltAbility[${SpellName}].ID}
            /delay 30 !${Me.Casting.ID}
            /return 1
        }
        /return 0
    }

    | Check if it's an item
    /if (${FindItem[${SpellName}].ID}) {
        /if (${FindItem[${SpellName}].TimerReady} == 0) {
            GZMDEBUG_BUFF Using item buff: ${SpellName}
            /useitem "${SpellName}"
            /delay 30 !${Me.Casting.ID}
            /return 1
        }
        /return 0
    }

    | Regular spell
    /if (${Me.SpellReady[${SpellName}]}) {
        GZMDEBUG_BUFF Casting spell buff: ${SpellName}
        /cast "${SpellName}"
        /delay 5s ${Me.Casting.ID}
        /delay 15s !${Me.Casting.ID}
        /return 1
    }

/return 0

| ============================================================================
| SUB: Bind - Buff Group Now
| ============================================================================
Sub Bind_BuffGroup
    /echo \ayBuffing group now...
    /varset BuffCheckTimer 0
    /call CheckBuffs
    /echo \agGroup buff check complete
/return

| ============================================================================
| CLASS-SPECIFIC BUFF ROUTINES
| ============================================================================

Sub ClericBuffRoutine
    | Armor of the Faithful
    /if (${Me.AltAbilityReady[Armor of the Faithful]} && !${Me.Buff[Armor of the Faithful].ID}) {
        /alt act ${Me.AltAbility[Armor of the Faithful].ID}
    }

    | Divine Aura (emergency)
    | Only use if about to die
/return

Sub EnchanterBuffRoutine
    | Twincast aura
    /if (!${Me.Aura.Find[Twincast]}) {
        /if (${Me.SpellReady[Twincast Aura]}) {
            /cast "Twincast Aura"
        }
    }

    | Mana regen aura
/return

Sub ShamanBuffRoutine
    | Ancestral Guard
    /if (${Me.AltAbilityReady[Ancestral Guard]} && !${Me.Buff[Ancestral Guard].ID}) {
        /alt act ${Me.AltAbility[Ancestral Guard].ID}
    }
/return

Sub DruidBuffRoutine
    | Group wolf form
    /if (${Me.AltAbilityReady[Pack Spirit]} && !${Me.Buff[Pack Spirit].ID}) {
        /alt act ${Me.AltAbility[Pack Spirit].ID}
    }
/return

Sub BardBuffRoutine
    | Handled by twist
/return

| ============================================================================
| END OF BUFF INCLUDE
| ============================================================================
