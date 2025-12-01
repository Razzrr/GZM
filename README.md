# GZM - GronnzMaster

**The Ultimate EQ Boxing Macro**

Version 1.0.1 - December 2024

## Overview

GZM (GronnzMaster) is a comprehensive boxing macro for EverQuest featuring:

- **DanNet/EQBC Communication** - Seamless multi-boxing coordination
- **10-State Machine** - Robust state management for reliable operation
- **CH Rotation System** - Complete Heal chain for raid healing
- **Event-Driven Architecture** - Customizable triggers and hooks
- **OOG Support** - Out-of-group healing and buffing
- **Nav Integration** - MQ2Nav pathing for movement

### Project Stats
| Metric | Value |
|--------|-------|
| Main Macro | 6,100+ lines |
| Include Files | 25 files |
| Total Code | ~17,000 lines |
| Subroutines | 150+ routines |
| Event Handlers | 60+ events |
| Supported Classes | All 16 EQ classes |

## Features

### Core Systems
- **Multi-Boxing Support** - Control multiple characters via DanNet/EQBC
- **Role-Based Operation** - Tank, Healer, DPS, Puller, CC roles
- **Smart Combat** - Assist-based targeting with configurable assist percentage
- **State Machine** - 10-state robust management (IDLE, COMBAT, FOLLOWING, CAMPING, ZONING, DEAD, REZZING, LOOTING, BUFFING, PULLING)
- **Timer Throttling** - 16 named timers with 4 priority tiers

### Combat & DPS
- **Class-Specific Rotations** - Combat routines for all 16 classes
- **Ability Management** - AA, disc, spell, and item rotations
- **Debuff System** - Resist tracking, cooldowns, priority debuffing
- **Combat Hooks** - Extensible entry/exit/loop hooks

### Healing
- **Group Healing** - Priority-based group healing
- **Raid Healing** - Extended target healing support
- **XTarget Healing** - Heal friendly targets in XTarget slots
- **CH Chain** - Complete Heal chain rotation for raids (fully implemented)
- **Per-Class Thresholds** - Configurable heal % per class

### CH Chain System
- Multi-cleric rotation (up to 6 clerics)
- DanNet synchronized timing with GO signal pattern
- Configurable delay between casts
- Fallback handling for missed turns
- Duck heals when target is high HP
- Target death detection and chain pausing

### Crowd Control
- **Mezzing** - Priority-based mez with resist tracking and immune learning
- **Charm** - Charm pet system for ENC/NEC/DRU with break recovery
- **Slow** - Priority slow system with tash/malo debuffing

### Pulling
- **Smart Pulling** - Configurable pull paths and priorities
- **Pull Ranking** - Prioritize targets by level/type
- **Chain Pull** - Continuous pulling mode
- **Hunter Mode** - Navigate and pull in configured areas
- **Safe Pull Checks** - Avoid pulling when group is low

### Pet Management
- **Auto-Summon** - Resummon pets when dead
- **Pet Buffs** - Automatic pet buffing
- **Pet Heals** - Pet healing integration
- **Pet Hold/Attack** - Combat pet management
- **PetToys** - Pet weapons/haste items

### Utility Systems
- **Navigation** - MQ2Nav integration for pathing
- **Camping** - Return to camp functionality with stuck recovery
- **Looting** - NinjaAdvLoot integration with categories
- **Buffing** - Self, group, pet, and OOG buffs
- **Forage** - Auto-forage for RNG/DRU/BRD/ROG
- **Vendor** - Auto buy/sell/bank automation
- **AA Purchasing** - Auto-buy AAs from priority list
- **Waypoints** - Record/playback path system

## Requirements

### Required Plugins
| Plugin | Purpose |
|--------|---------|
| MQ2DanNet | Primary communication between characters |
| MQ2Nav | Navigation and pathing |
| MQ2MoveUtils | Movement and positioning |
| MQ2Exchange | Item/gear swapping |
| MQ2Rez | Resurrection handling |
| MQ2Twist | Bard melody twisting |

### Optional Plugins
| Plugin | Purpose |
|--------|---------|
| MQ2Cast | Enhanced spell casting |
| MQ2Melee | Enhanced melee combat |
| MQ2EQBC | Backup communication |
| MQ2AdvPath | Path recording/playback |
| MQ2DPSAdv | DPS tracking |
| MQ2SpawnMaster | Spawn alerts |
| MQ2Events | Custom event handling |

## Installation

1. Copy all files to your MacroQuest macros folder:
   ```
   MQ2/macros/
   ├── GZM.mac
   ├── gronnzmaster_custom.inc
   └── gzm_*.inc (23 include files)
   ```

2. Ensure required plugins are loaded in MacroQuest

3. (Optional) Copy `gronnzmaster_custom.inc` to add your own customizations

## Quick Start

### Basic Usage
```
/mac gzm [role] [mainassist] [assistat]
```

### Examples
```
| Start as DPS, assist "TankName" at 95%
/mac gzm assist TankName 95

| Start as tank
/mac gzm tank

| Start as healer, assist "MainTank" at 100%
/mac gzm healer MainTank 100

| Start as puller
/mac gzm puller TankName 95
```

### Roles
| Role | Description |
|------|-------------|
| `tank` | Main tank - holds aggro, doesn't assist |
| `assist` | DPS - assists main assist |
| `healer` | Healer - prioritizes healing over DPS |
| `puller` | Puller - pulls mobs to camp |
| `cc` | Crowd Control - mezzing/rooting |

## Configuration

Configuration is stored in INI files:
```
MQ2/config/GZM_ServerName_CharName.ini
```

### Key Settings
| Setting | Description | Default |
|---------|-------------|---------|
| `MainAssist` | Character to assist | - |
| `AssistAt` | HP % to start assisting | 95 |
| `CampRadius` | Radius to return to camp | 100 |
| `HealPct` | HP % to start healing | 70 |
| `MezOn` | Enable mezzing | FALSE |
| `DoBuffs` | Enable buffing | TRUE |
| `DoLoot` | Enable looting | FALSE |

## File Structure

| File | Description |
|------|-------------|
| `GZM.mac` | Main macro file |
| `gronnzmaster_custom.inc` | User customization hooks |
| `gzm_aa.inc` | AA auto-purchase system |
| `gzm_buffs.inc` | Buff management |
| `gzm_charm.inc` | Charm pet system |
| `gzm_chchain.inc` | Complete Heal chain |
| `gzm_combat.inc` | Combat/DPS routines |
| `gzm_comm.inc` | Communication (DanNet/EQBC) |
| `gzm_cursor.inc` | Cursor item handling |
| `gzm_death.inc` | Death/resurrection handling |
| `gzm_food.inc` | Food/drink management |
| `gzm_forage.inc` | Forage system |
| `gzm_group.inc` | Group management |
| `gzm_heals.inc` | Healing routines |
| `gzm_items.inc` | Item click rotations |
| `gzm_loot.inc` | Looting system |
| `gzm_mez.inc` | Mezzing/CC |
| `gzm_pet.inc` | Pet management |
| `gzm_pull.inc` | Pulling system |
| `gzm_slow.inc` | Slow/debuff system |
| `gzm_state.inc` | State machine |
| `gzm_timers.inc` | Timer management |
| `gzm_vendor.inc` | Vendor interaction |
| `gzm_waypoint.inc` | Waypoint/path system |
| `gzm_zone.inc` | Zone handling |

## Customization

Edit `gronnzmaster_custom.inc` to add your own:
- Custom events
- Combat hooks
- Buff hooks
- Idle tasks
- Class-specific logic

This file is not overwritten on updates.

## Commands

### Core Commands
| Command | Description |
|---------|-------------|
| `/gzmhelp` | Show all available commands |
| `/gzmstatus` | Show current macro status |
| `/gzmreload` | Reload INI settings |
| `/gzmdebug [type]` | Toggle debug output |
| `/backoff` | Stop all combat |
| `/burn` | Activate burn abilities |

### Movement & Positioning
| Command | Description |
|---------|-------------|
| `/makecamphere` | Set current location as camp |
| `/chaseme` | Make others chase you |
| `/stayhere` | Stop and hold position |
| `/navto [loc/id]` | Navigate to location |

### Combat & Assist
| Command | Description |
|---------|-------------|
| `/switchma [name]` | Change main assist |
| `/pullhold` | Toggle pull hold |
| `/addpull [name]` | Add mob to pull list |
| `/addignore [name]` | Add mob to ignore list |

### CH Chain Commands (Raid Healing)
| Command | Description |
|---------|-------------|
| `/startch c1 c2 c3...` | Start CH rotation with listed clerics |
| `/stopch` | Stop the CH rotation |
| `/chtarget [name]` | Set/change the CH target |
| `/chdelay [tenths]` | View/set delay before GO signal |
| `/chduck [on\|off]` | Toggle duck heals (stop if target high HP) |
| `/chstatus` | Show current CH chain status |

### Toggle Commands
| Command | Description |
|---------|-------------|
| `/buffson` | Toggle buffs on/off |
| `/meleeon` | Toggle melee on/off |
| `/healson` | Toggle heals on/off |
| `/dpson` | Toggle DPS on/off |
| `/peton` | Toggle pet on/off |
| `/mezon` | Toggle mez on/off |
| `/pullon` | Toggle pulling on/off |
| `/looton` | Toggle looting on/off |

### Variable Commands
```
/varset MainAssist NewTankName    | Change main assist
/varset AssistAt 90               | Change assist percentage
/varset BuffsOn FALSE             | Disable buffing
/varset HealPct 60                | Change heal threshold
/varset Debug TRUE                | Enable debug output
```

## Troubleshooting

### Macro won't start
- Ensure all required plugins are loaded
- Check that `ninjadvloot.inc` is in macros folder

### Character not assisting
- Verify MainAssist name is correct
- Check AssistAt percentage
- Ensure target is an NPC

### Healing issues
- Check HealPct settings
- Verify healer has mana
- Check spell gems are loaded

### Navigation problems
- Ensure MQ2Nav mesh is loaded for zone
- Check camp location is valid

## Credits

GZM (GronnzMaster) was developed for the EverQuest boxing community.

## License

This macro is provided as-is for personal use in EverQuest.
