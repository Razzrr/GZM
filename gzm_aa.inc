|- GronnzMaster AA Auto-Purchaser Include
|- Auto-purchase AAs from a prioritized list
|- Supports class-specific AA priorities
|- Version 1.0

|- Max AA entries in buy list
#DEFINE MAXAACOUNT 100

Sub InitAAVars
   |- AA purchasing settings
   /if (!${Defined[DoAA]}) /declare DoAA bool outer FALSE
   /if (!${Defined[AABank]}) /declare AABank int outer 0
   /if (!${Defined[AAWarning]}) /declare AAWarning int outer 0
   /if (!${Defined[AAtoNormal]}) /declare AAtoNormal bool outer FALSE
   /if (!${Defined[AAAnnounce]}) /declare AAAnnounce string outer
   /if (!${Defined[AAVerbose]}) /declare AAVerbose bool outer FALSE

   |- AA list and count
   /if (!${Defined[AACount]}) /declare AACount int outer 0
   /if (!${Defined[AA]}) /declare AA[${MAXAACOUNT}] string outer

   |- AA INI file
   /if (!${Defined[AAIniFile]}) /declare AAIniFile string outer
   /if (!${Defined[AAIniSection]}) /declare AAIniSection string outer AAtoBuy

   |- Timer
   /if (!${Defined[AACheckTimer]}) /declare AACheckTimer timer outer

   |- Window list name
   /if (!${Defined[ListNum]}) /declare ListNum string outer

   |- Debug
   /if (!${Defined[AADebug]}) /declare AADebug bool outer FALSE
/return

|- ============================================================================
|- AA INITIALIZATION
|- ============================================================================

|- Initialize AA system and load buy list
Sub AAInit
   /declare a int local

   |- Determine AA INI file path
   /if (!${AAIniFile.Length}) {
      /if (${Defined[IniFileName]}) {
         /varset AAIniFile ${IniFileName.Left[-4]}_AA.ini
      } else {
         /varset AAIniFile ${MacroQuest.Path}\config\AA_${Me.Name}.ini
      }
   }

   |- Create INI if doesn't exist
   /if (!${Ini.File["${AAIniFile}"].Exists}) {
      /echo [GZM_AA] Creating AA ini file: ${AAIniFile}
      /call AADump
   } else /if (!${Ini["${AAIniFile}","${AAIniSection}"].Length}) {
      /echo [GZM_AA] Creating section ${AAIniSection} in ${AAIniFile}
      /call AADump
   }

   |- Load AA count
   /varset AACount ${Ini[${AAIniFile},${AAIniSection},AACount,0]}

   |- Load AA list
   /if (${AACount}) {
      /for a 1 to ${AACount}
         /varset AA[${a}] ${Ini[${AAIniFile},${AAIniSection},AA${a}]}
      /next a
      /echo [GZM_AA] Loaded ${AACount} AAs to purchase
      /if (${DoAA}) /call AAPicker force
   }
/return

|- ============================================================================
|- AA PICKER - MAIN PURCHASE ROUTINE
|- ============================================================================

|- Check and purchase AAs from the prioritized list
|- Format in INI: AA1=Ability Name|Level|S
|- Level can be: number (max level to buy), M (max), L (max at current level), X (skip)
|- S flag = skip to next if can't afford (optional)
Sub AAPicker(aaParm)
   /if (!${DoAA} && !${aaParm.Equal[force]}) /return
   /if (!${AACount}) /return
   /if (${Me.AAPoints}<=0) /return

   |- Skip if confirmation box is open from something else
   /if (${Window[ConfirmationDialogBox].Open}) /return

   /varset AACheckTimer 5m

   /declare x int local
   /declare Cost int local
   /declare AALevelStr string local
   /declare curAALevel int local
   /declare maxAALevel int local
   /declare maxPurchaseLevel int local
   /declare nameAA string local
   /declare AAtoSpend int local
   /declare wantSomething bool local FALSE
   /declare doAnnounce string local ${If[${AAAnnounce.Length},${AAAnnounce} Msg:,]}
   /declare myMax int local ${Math.Calc[${Me.Level}*2]}

   /varcalc AAtoSpend ${Me.AAPoints}-${AABank}
   /if (${AAtoSpend}<=0) {
      /if (${doAnnounce.Length} && ${AAVerbose}) {
         /docommand ${doAnnounce} No spendable AA (keeping ${AABank} banked)
      }
      /return
   }

   /for x 1 to ${AACount}
      |- Parse AA entry
      /if (!${AA[${x}].Arg[1,|].Length}) /continue
      /if (${AA[${x}].Arg[2,|].Equal[X]}) /continue

      /varset nameAA ${AA[${x}].Arg[1,|]}

      |- Get list number for this AA type
      /varset ListNum List${AltAbility[${nameAA}].Type}

:BuyAdditional
      |- Reset filters
      /nomodkey /notify AAWindow ResetFilter leftmouseup
      /delay 2

      |- Check if AA exists
      /if (!${AltAbility[${nameAA}].ID}) {
         /if (${doAnnounce.Length}) {
            /docommand ${doAnnounce} ${nameAA} is not a valid ${Me.Class} ability
         }
         /continue
      }

      |- Get current and max levels
      /varset AALevelStr ${Window[AAWindow].Child[${ListNum}].List[${Window[AAWindow].Child[${ListNum}].List[=${nameAA}]}, 2]}
      /varset curAALevel ${AALevelStr.Arg[1,"/"]}
      /varset maxAALevel ${AALevelStr.Arg[2,"/"]}

      |- Determine max purchase level
      /if (${Select[${AA[${x}].Arg[2,|]},M,L]}) {
         /varset maxPurchaseLevel ${maxAALevel}
      } else {
         /varset maxPurchaseLevel ${AA[${x}].Arg[2,|]}
         /if (${maxPurchaseLevel}>${maxAALevel}) {
            /varset maxPurchaseLevel ${maxAALevel}
         }
      }

      |- Check if already at max
      /if (${curAALevel}>=${maxPurchaseLevel}) {
         |- Mark as done in INI
         /if (${Select[${AA[${x}].Arg[2,|]},M,L]}) {
            /ini "${AAIniFile}" "${AAIniSection}" "AA${x}" "${nameAA}|X"
         }
         /continue
      }

      /varset wantSomething TRUE

      |- Get cost
      /varset Cost ${Window[AAWindow].Child[${ListNum}].List[${Window[AAWindow].Child[${ListNum}].List[=${nameAA}]}, 3]}

      |- Enable can purchase filter
      /if (!${Window[AAWindow].Child[CanPurchaseFilter].Checked}) {
         /nomodkey /notify AAWindow CanPurchaseFilter leftmouseup
         /delay 2
      }

      |- Check if we can purchase it
      /if (!${Window[AAWindow].Child[${ListNum}].List[=${nameAA}]}==NULL) {
         /if (${Cost}<=${AAtoSpend}) {
            |- Purchase the AA
            /call AASelect "${nameAA}"
            /call AAPurchase

            /varcalc curAALevel ${curAALevel}+1
            /if (${doAnnounce.Length}) {
               /docommand ${doAnnounce} Purchased ${nameAA} (${curAALevel}/${maxAALevel}) for ${Cost} AA
            }
            /delay 1s

            |- Try to buy more if we have points
            /if (${Me.AAPoints}>${AABank}) {
               /varcalc AAtoSpend ${Me.AAPoints}-${AABank}
               /if (${AAtoSpend}>0 && ${curAALevel}<${maxPurchaseLevel}) {
                  /goto :BuyAdditional
               }
            }
            /return
         } else {
            /if (${doAnnounce.Length} && ${AAVerbose}) {
               /docommand ${doAnnounce} Can't afford ${nameAA} (need ${Cost}, have ${AAtoSpend})
            }
         }
      }

      |- Skip to next or stop
      /if (${AA[${x}].Arg[3,|].Equal[S]}) {
         /continue
      }
      /return
   /next x

   |- All done
   /if (!${wantSomething}) {
      /if (${doAnnounce.Length}) {
         /docommand ${doAnnounce} All defined AAs maxed. Disabling DoAA.
      }
      /varset DoAA FALSE
   }

   |- Warning if near max
   /if (${AAWarning} && ${Me.AAPoints}>=${AAWarning} && ${Me.AAPoints}<${myMax}) {
      /if (${doAnnounce.Length}) {
         /docommand ${doAnnounce} Warning! You have ${Me.AAPoints} banked AA.
      }
   }

   |- Turn off alt exp if maxed
   /if (${Me.AAPoints}>=${myMax} && ${AAtoNormal}) {
      /if (${doAnnounce.Length}) {
         /docommand ${doAnnounce} Max AA reached. Switching to normal XP.
      }
      /alt off
   }
/return

|- ============================================================================
|- AA HELPER FUNCTIONS
|- ============================================================================

|- Select an AA in the window
Sub AASelect(AAName)
   /nomodkey /notify AAWindow ResetFilter leftmouseup
   /delay 2
   /nomodkey /notify AAWindow AAW_Subwindows tabselect ${AltAbility[${AAName}].Type}
   /delay 2
   /nomodkey /notify AAWindow ${ListNum} listselect ${Window[AAWindow].Child[${ListNum}].List[=${AAName}]}
   /delay 2
   /nomodkey /notify AAWindow ${ListNum} leftmouse ${Window[AAWindow].Child[${ListNum}].List[=${AAName}]}
   /delay 2
/return

|- Purchase the selected AA
Sub AAPurchase
   |- Check fast-purchase setting
   /if (${Window[OptionsWindow].Child[OptionsGeneralPage].Child[OGP_AANoConfirmCheckbox].Checked}) {
      |- Fast purchase - just click train
      /nomodkey /notify AAWindow TrainButton leftmouseup
      /delay 10
   } else {
      |- Need confirmation
      /nomodkey /notify AAWindow TrainButton leftmouseup
      /delay 2s ${Window[ConfirmationDialogBox].Open}
      /if (${Window[ConfirmationDialogBox].Open}) {
         /nomodkey /notify ConfirmationDialogBox Yes_Button leftmouseup
         /delay 10
      }
   }
/return

|- Dump all AAs to INI file
Sub AADump
   /declare a int local 0
   /declare b int local
   /declare c int local 1
   /declare AAName string local
   /declare AALevel string local

   /echo [GZM_AA] Dumping available AAs to ${AAIniFile}...

   |- Iterate through AA window tabs
   /for b 1 to 10
      /nomodkey /notify AAWindow AAW_Subwindows tabselect ${b}
      /delay 3
      /varset ListNum List${b}

      /for c 1 to ${Window[AAWindow].Child[${ListNum}].Items}
         /varset AAName ${Window[AAWindow].Child[${ListNum}].List[${c}, 1]}
         /varset AALevel ${Window[AAWindow].Child[${ListNum}].List[${c}, 2]}

         /if (${AAName.Length} && ${AAName.NotEqual[NULL]}) {
            /varcalc a ${a}+1
            /ini "${AAIniFile}" "${AAIniSection}" "AA${a}" "${AAName}|M|S"
         }
      /next c
   /next b

   /ini "${AAIniFile}" "${AAIniSection}" "AACount" "${a}"
   /echo [GZM_AA] Dumped ${a} AAs to ${AAIniFile}
/return

|- ============================================================================
|- COMMANDS
|- ============================================================================

|- Force AA check
Sub ForceAACheck
   /call AAPicker force
/return

|- Add AA to buy list
Sub AddAAToBuy(AAName, level, skipFlag)
   /if (!${AAName.Length}) /return

   /varcalc AACount ${AACount}+1
   /varset AA[${AACount}] ${AAName}|${If[${level.Length},${level},M]}|${If[${skipFlag.Length},${skipFlag},S]}
   /ini "${AAIniFile}" "${AAIniSection}" "AA${AACount}" "${AA[${AACount}]}"
   /ini "${AAIniFile}" "${AAIniSection}" "AACount" "${AACount}"
   /echo [GZM_AA] Added ${AAName} to buy list
/return

|- ============================================================================
|- INI FUNCTIONS
|- ============================================================================

Sub LoadAASettings(IniFile)
   /if (!${IniFile.Length}) /return

   /varset DoAA ${Ini[${IniFile},AA,DoAA,${DoAA}]}
   /varset AABank ${Ini[${IniFile},AA,AABank,${AABank}]}
   /varset AAWarning ${Ini[${IniFile},AA,AAWarning,${AAWarning}]}
   /varset AAtoNormal ${Ini[${IniFile},AA,AAtoNormal,${AAtoNormal}]}
   /varset AAAnnounce ${Ini[${IniFile},AA,AAAnnounce,${AAAnnounce}]}
   /varset AAVerbose ${Ini[${IniFile},AA,AAVerbose,${AAVerbose}]}

   /if (${AADebug}) /echo [GZM_AA] Settings loaded from ${IniFile}
/return

Sub SaveAASettings(IniFile)
   /if (!${IniFile.Length}) /return

   /ini "${IniFile}" "AA" "DoAA" "${DoAA}"
   /ini "${IniFile}" "AA" "AABank" "${AABank}"
   /ini "${IniFile}" "AA" "AAWarning" "${AAWarning}"
   /ini "${IniFile}" "AA" "AAtoNormal" "${AAtoNormal}"
   /ini "${IniFile}" "AA" "AAAnnounce" "${AAAnnounce}"
   /ini "${IniFile}" "AA" "AAVerbose" "${AAVerbose}"

   /echo [GZM_AA] Settings saved to ${IniFile}
/return

|- Get AA status
Sub GetAAStatus
   /echo [GZM_AA] === AA Status ===
   /echo DoAA: ${If[${DoAA},ON,OFF]}
   /echo AA Points: ${Me.AAPoints}
   /echo AA Bank: ${AABank}
   /echo Spendable: ${Math.Calc[${Me.AAPoints}-${AABank}]}
   /echo AAs to Buy: ${AACount}
   /echo Next Check: ${If[${AACheckTimer},${AACheckTimer},Ready]}
   /echo ========================
/return

