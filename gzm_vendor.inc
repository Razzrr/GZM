|- GronnzMaster Vendor Automation Include
|- Buy, sell, and bank automation
|- Supports configurable sell lists and keep amounts
|- Version 1.0

Sub InitVendorVars
   |- Vendor settings
   /if (!${Defined[AutoSell]}) /declare AutoSell bool outer FALSE
   /if (!${Defined[AutoBuy]}) /declare AutoBuy bool outer FALSE
   /if (!${Defined[KeepPlat]}) /declare KeepPlat int outer 10000
   /if (!${Defined[VendorDistance]}) /declare VendorDistance int outer 20

   |- Vendor INI file
   /if (!${Defined[VendorIniFile]}) /declare VendorIniFile string outer ${MacroQuest.Path}\config\GZM_Vendor_${Me.CleanName}.ini

   |- Item lists (arrays for sell/buy)
   /if (!${Defined[SellItems]}) /declare SellItems[50] string outer
   /if (!${Defined[BuyItems]}) /declare BuyItems[50] string outer
   /if (!${Defined[BuyQty]}) /declare BuyQty[50] int outer

   |- Debug
   /if (!${Defined[VendorDebug]}) /declare VendorDebug bool outer FALSE
/return

|- Open vendor window with specified NPC
Sub OpenVendor(vendorName)
   /if (${VendorDebug}) /echo [GZM_Vendor] Opening vendor: ${vendorName}

   |- Find the vendor
   /declare vendorID int local ${Spawn[npc ${vendorName}].ID}
   /if (!${vendorID}) {
      /echo [GZM_Vendor] Vendor "${vendorName}" not found
      /return FALSE
   }

   |- Check distance
   /if (${Spawn[${vendorID}].Distance}>${VendorDistance}) {
      /echo [GZM_Vendor] Vendor too far (${Int[${Spawn[${vendorID}].Distance}]} away, max ${VendorDistance})
      /return FALSE
   }

   |- Target vendor
   /target id ${vendorID}
   /delay 2s ${Target.ID}==${vendorID}

   |- Right-click to open
   /if (${Target.ID}==${vendorID}) {
      /click right
      /delay 3s ${Window[MerchantWnd].Open}
      /if (${Window[MerchantWnd].Open}) {
         /if (${VendorDebug}) /echo [GZM_Vendor] Vendor window opened
         /return TRUE
      }
   }

   /echo [GZM_Vendor] Failed to open vendor window
/return FALSE

|- Close vendor window
Sub CloseVendor
   /if (${Window[MerchantWnd].Open}) {
      /notify MerchantWnd MW_Done_Button leftmouseup
      /delay 1s !${Window[MerchantWnd].Open}
   }
/return

|- Buy a specific item
Sub BuyItem(itemName, quantity)
   /if (!${Window[MerchantWnd].Open}) {
      /echo [GZM_Vendor] Vendor window not open
      /return FALSE
   }

   /if (!${quantity}) /varset quantity 1

   /if (${VendorDebug}) /echo [GZM_Vendor] Buying ${quantity}x ${itemName}

   |- Find item in merchant list
   /declare i int local
   /declare found bool local FALSE

   /for i 1 to ${Window[MerchantWnd].Child[ItemList].Items}
      /if (${Window[MerchantWnd].Child[ItemList].List[${i},2].Equal[${itemName}]}) {
         /varset found TRUE
         |- Select the item
         /notify MerchantWnd ItemList listselect ${i}
         /delay 5

         |- Set quantity if more than 1
         /if (${quantity}>1) {
            /notify MerchantWnd MW_Quantity_Slider newvalue ${quantity}
            /delay 5
         }

         |- Click buy button
         /notify MerchantWnd MW_Buy_Button leftmouseup
         /delay 10

         /if (${VendorDebug}) /echo [GZM_Vendor] Purchased ${quantity}x ${itemName}
         /return TRUE
      }
   /next i

   /if (!${found}) {
      /echo [GZM_Vendor] Item "${itemName}" not found at vendor
   }
/return FALSE

|- Sell a specific item from inventory
Sub SellItem(itemName)
   /if (!${Window[MerchantWnd].Open}) {
      /echo [GZM_Vendor] Vendor window not open
      /return FALSE
   }

   /if (${VendorDebug}) /echo [GZM_Vendor] Selling ${itemName}

   |- Find item in inventory
   /declare itemSlot int local ${FindItem[=${itemName}].InvSlot}
   /if (!${itemSlot}) {
      /if (${VendorDebug}) /echo [GZM_Vendor] Item "${itemName}" not in inventory
      /return FALSE
   }

   |- Pick up the item and sell
   /itemnotify ${itemSlot} leftmouseup
   /delay 5 ${Cursor.ID}
   /if (${Cursor.ID}) {
      /notify MerchantWnd MW_Sell_Button leftmouseup
      /delay 10 !${Cursor.ID}
      /if (!${Cursor.ID}) {
         /if (${VendorDebug}) /echo [GZM_Vendor] Sold ${itemName}
         /return TRUE
      } else {
         |- Put item back if sell failed
         /autoinventory
      }
   }

/return FALSE

|- Sell all items marked for sale in INI
Sub SellAll
   /if (!${Window[MerchantWnd].Open}) {
      /echo [GZM_Vendor] Vendor window not open - cannot sell
      /return
   }

   /echo [GZM_Vendor] Selling all marked items...

   /declare i int local
   /declare itemName string local
   /declare soldCount int local 0

   |- Loop through sell list
   /for i 1 to 50
      /if (${SellItems[${i}].Length}) {
         /varset itemName ${SellItems[${i}]}
         :SellLoop
         /if (${FindItem[=${itemName}].ID}) {
            /call SellItem "${itemName}"
            /if (${Macro.Return.Equal[TRUE]}) {
               /varcalc soldCount ${soldCount}+1
               /goto :SellLoop
            }
         }
      }
   /next i

   /echo [GZM_Vendor] Sold ${soldCount} items
/return

|- Buy all items in the buy list
Sub BuyAll
   /if (!${Window[MerchantWnd].Open}) {
      /echo [GZM_Vendor] Vendor window not open - cannot buy
      /return
   }

   /echo [GZM_Vendor] Buying items from list...

   /declare i int local
   /declare itemName string local
   /declare qty int local
   /declare boughtCount int local 0

   |- Loop through buy list
   /for i 1 to 50
      /if (${BuyItems[${i}].Length}) {
         /varset itemName ${BuyItems[${i}]}
         /varset qty ${BuyQty[${i}]}
         /if (!${qty}) /varset qty 1

         |- Check if we already have enough
         /if (${FindItemCount[=${itemName}]}<${qty}) {
            /declare needQty int local ${Math.Calc[${qty}-${FindItemCount[=${itemName}]}]}
            /call BuyItem "${itemName}" ${needQty}
            /if (${Macro.Return.Equal[TRUE]}) {
               /varcalc boughtCount ${boughtCount}+1
            }
         }
      }
   /next i

   /echo [GZM_Vendor] Bought ${boughtCount} items
/return

|- Full vendor routine (sell then buy)
Sub DoVendor(vendorName)
   /echo [GZM_Vendor] Starting vendor routine with ${vendorName}

   /call OpenVendor "${vendorName}"
   /if (${Macro.Return.NotEqual[TRUE]}) {
      /echo [GZM_Vendor] Failed to open vendor
      /return
   }

   |- Sell first
   /if (${AutoSell}) {
      /call SellAll
   }

   |- Then buy
   /if (${AutoBuy}) {
      /call BuyAll
   }

   /call CloseVendor
   /echo [GZM_Vendor] Vendor routine complete
/return

|- Open bank window
Sub OpenBank
   /if (${VendorDebug}) /echo [GZM_Vendor] Opening bank

   |- Find banker
   /declare bankerID int local ${Spawn[npc banker].ID}
   /if (!${bankerID}) {
      /echo [GZM_Vendor] No banker found nearby
      /return FALSE
   }

   |- Check distance
   /if (${Spawn[${bankerID}].Distance}>${VendorDistance}) {
      /echo [GZM_Vendor] Banker too far
      /return FALSE
   }

   |- Target and open
   /target id ${bankerID}
   /delay 2s ${Target.ID}==${bankerID}
   /if (${Target.ID}==${bankerID}) {
      /click right
      /delay 3s ${Window[BigBankWnd].Open} || ${Window[BankWnd].Open}
      /if (${Window[BigBankWnd].Open} || ${Window[BankWnd].Open}) {
         /if (${VendorDebug}) /echo [GZM_Vendor] Bank window opened
         /return TRUE
      }
   }

   /echo [GZM_Vendor] Failed to open bank
/return FALSE

|- Close bank window
Sub CloseBank
   /if (${Window[BigBankWnd].Open}) {
      /notify BigBankWnd BIGB_DoneButton leftmouseup
      /delay 1s !${Window[BigBankWnd].Open}
   } else /if (${Window[BankWnd].Open}) {
      /notify BankWnd BANK_DoneButton leftmouseup
      /delay 1s !${Window[BankWnd].Open}
   }
/return

|- Get item from bank
Sub GetBankItem(itemName, quantity)
   /if (!${Window[BigBankWnd].Open} && !${Window[BankWnd].Open}) {
      /echo [GZM_Vendor] Bank window not open
      /return FALSE
   }

   /if (!${quantity}) /varset quantity 1

   /if (${VendorDebug}) /echo [GZM_Vendor] Getting ${quantity}x ${itemName} from bank

   |- Find in bank
   /declare bankSlot int local ${FindItem[=${itemName}].InvSlot}
   /if (${bankSlot}<23) {
      /echo [GZM_Vendor] Item "${itemName}" not in bank
      /return FALSE
   }

   |- Withdraw item
   /itemnotify ${bankSlot} leftmouseup
   /delay 5 ${Cursor.ID}
   /if (${Cursor.ID}) {
      /autoinventory
      /delay 5 !${Cursor.ID}
      /if (${VendorDebug}) /echo [GZM_Vendor] Retrieved ${itemName} from bank
      /return TRUE
   }

/return FALSE

|- Deposit item to bank
Sub DepositItem(itemName)
   /if (!${Window[BigBankWnd].Open} && !${Window[BankWnd].Open}) {
      /echo [GZM_Vendor] Bank window not open
      /return FALSE
   }

   /if (${VendorDebug}) /echo [GZM_Vendor] Depositing ${itemName} to bank

   |- Find in inventory
   /declare itemSlot int local ${FindItem[=${itemName}].InvSlot}
   /if (!${itemSlot} || ${itemSlot}>22) {
      /echo [GZM_Vendor] Item "${itemName}" not in inventory
      /return FALSE
   }

   |- Pick up and deposit
   /itemnotify ${itemSlot} leftmouseup
   /delay 5 ${Cursor.ID}
   /if (${Cursor.ID}) {
      /notify BigBankWnd BIGB_AutoButton leftmouseup
      /delay 5 !${Cursor.ID}
      /if (!${Cursor.ID}) {
         /if (${VendorDebug}) /echo [GZM_Vendor] Deposited ${itemName}
         /return TRUE
      } else {
         /autoinventory
      }
   }

/return FALSE

|- Load vendor settings from INI
Sub LoadVendorSettings
   /if (!${Ini[${VendorIniFile}].Length}) {
      /if (${VendorDebug}) /echo [GZM_Vendor] No vendor INI found, using defaults
      /return
   }

   |- Load settings
   /varset AutoSell ${Ini[${VendorIniFile},Settings,AutoSell,${AutoSell}]}
   /varset AutoBuy ${Ini[${VendorIniFile},Settings,AutoBuy,${AutoBuy}]}
   /varset KeepPlat ${Ini[${VendorIniFile},Settings,KeepPlat,${KeepPlat}]}

   |- Load sell list
   /declare i int local
   /for i 1 to 50
      /varset SellItems[${i}] ${Ini[${VendorIniFile},SellItems,Item${i}]}
   /next i

   |- Load buy list
   /for i 1 to 50
      /declare buyEntry string local ${Ini[${VendorIniFile},BuyItems,Item${i}]}
      /if (${buyEntry.Length} && ${buyEntry.NotEqual[NULL]}) {
         /varset BuyItems[${i}] ${buyEntry.Arg[1,|]}
         /varset BuyQty[${i}] ${buyEntry.Arg[2,|]}
      }
   /next i

   /if (${VendorDebug}) /echo [GZM_Vendor] Settings loaded from ${VendorIniFile}
/return

|- Save vendor settings to INI
Sub SaveVendorSettings
   /ini "${VendorIniFile}" "Settings" "AutoSell" "${AutoSell}"
   /ini "${VendorIniFile}" "Settings" "AutoBuy" "${AutoBuy}"
   /ini "${VendorIniFile}" "Settings" "KeepPlat" "${KeepPlat}"

   /declare i int local
   /for i 1 to 50
      /if (${SellItems[${i}].Length}) {
         /ini "${VendorIniFile}" "SellItems" "Item${i}" "${SellItems[${i}]}"
      }
   /next i

   /for i 1 to 50
      /if (${BuyItems[${i}].Length}) {
         /ini "${VendorIniFile}" "BuyItems" "Item${i}" "${BuyItems[${i}]}|${BuyQty[${i}]}"
      }
   /next i

   /echo [GZM_Vendor] Settings saved to ${VendorIniFile}
/return

|- Add item to sell list
Sub AddSellItem(itemName)
   /declare i int local
   /for i 1 to 50
      /if (!${SellItems[${i}].Length}) {
         /varset SellItems[${i}] ${itemName}
         /echo [GZM_Vendor] Added "${itemName}" to sell list
         /return
      }
   /next i
   /echo [GZM_Vendor] Sell list full
/return

|- Add item to buy list
Sub AddBuyItem(itemName, quantity)
   /if (!${quantity}) /varset quantity 1
   /declare i int local
   /for i 1 to 50
      /if (!${BuyItems[${i}].Length}) {
         /varset BuyItems[${i}] ${itemName}
         /varset BuyQty[${i}] ${quantity}
         /echo [GZM_Vendor] Added "${itemName}" x${quantity} to buy list
         /return
      }
   /next i
   /echo [GZM_Vendor] Buy list full
/return
