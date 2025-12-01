# GZM (GronnzMaster) User Guide

Complete documentation for setting up and using the GZM boxing macro.

---

## Table of Contents

1. [Getting Started](#getting-started)
2. [Initial Setup](#initial-setup)
3. [Roles and Configuration](#roles-and-configuration)
4. [Class-Specific Setup](#class-specific-setup)
5. [Combat System](#combat-system)
6. [Healing System](#healing-system)
7. [Crowd Control](#crowd-control)
8. [Pet Management](#pet-management)
9. [Pulling System](#pulling-system)
10. [Buff System](#buff-system)
11. [AA Auto-Purchase](#aa-auto-purchase)
12. [Navigation and Movement](#navigation-and-movement)
13. [Communication System](#communication-system)
14. [Custom Hooks](#custom-hooks)
15. [INI Reference](#ini-reference)
16. [Command Reference](#command-reference)
17. [Troubleshooting](#troubleshooting)

---

## Getting Started

### Prerequisites

Before using GZM, ensure you have:

1. **MacroQuest** installed and running
2. **Required plugins** loaded:
   - MQ2DanNet
   - MQ2Nav
   - MQ2MoveUtils
   - MQ2Exchange
   - MQ2Rez
   - MQ2Twist (for Bards)

3. **Navigation meshes** for your zones (via MQ2Nav)

### Installation

1. Copy all GZM files to your MacroQuest macros folder:
   ```
   C:\Users\[YourName]\E3_MQ2\macros\
   ```

2. Verify the following files exist:
   - `GZM.mac` (main macro)
   - `gronnzmaster_custom.inc` (custom hooks)
   - All `gzm_*.inc` files (23 include files)
   - `ninjadvloot.inc` (looting - usually already present)

3. Load required plugins in MacroQuest:
   ```
   /plugin MQ2DanNet
   /plugin MQ2Nav
   /plugin MQ2MoveUtils
   /plugin MQ2Exchange
   /plugin MQ2Rez
   ```

---

## Initial Setup

### First Run

1. Log into your character
2. Start the macro with a basic command:
   ```
   /mac gzm
   ```
3. The macro will create a default INI file at:
   ```
   MQ2/config/GZM_[Server]_[CharName].ini
   ```
4. End the macro: `/endmac`
5. Edit the INI file to configure your character

### Basic Startup Commands

```
| Basic startup - no role, will use defaults
/mac gzm

| Start as DPS assisting "TankName" at 95% mob HP
/mac gzm assist TankName 95

| Start as tank
/mac gzm tank

| Start as healer
/mac gzm healer MainTank 100

| Start as puller
/mac gzm puller TankName 95
```

---

## Roles and Configuration

### Available Roles

| Role | Description | Typical Classes |
|------|-------------|-----------------|
| `tank` | Main tank, generates aggro | WAR, PAL, SHD |
| `assist` | DPS, assists the main assist | All DPS classes |
| `healer` | Prioritizes healing over DPS | CLR, DRU, SHM |
| `puller` | Pulls mobs to camp | MNK, BRD, SK, Ranger |
| `cc` | Crowd control (mez/root) | ENC, BRD |

### Role-Specific Behavior

#### Tank Role
- Does not assist others
- Uses aggro abilities
- Stays in melee range
- Taunts to maintain aggro

#### Assist Role
- Waits for mob HP to reach `AssistAt` percentage
- Follows Main Assist's target
- Executes DPS rotation
- Backs off if aggro is too high

#### Healer Role
- Prioritizes healing over DPS
- Monitors group/raid health
- Casts heals based on priority
- Will DPS when healing not needed

#### Puller Role
- Pulls mobs to camp location
- Uses configured pull abilities
- Checks group readiness before pulling
- Returns to camp after pull

#### CC Role
- Prioritizes mezzing adds
- Maintains mez on multiple targets
- Tracks mez timers
- Re-mezzes before break

---

## Class-Specific Setup

### Warriors (WAR)
```ini
[Combat]
UseDiscs=TRUE
UseTaunt=TRUE
UseProvoke=TRUE
```
- Enable defensive/offensive disc rotations
- Configure taunt behavior
- Set up aggro abilities

### Paladins (PAL)
```ini
[Combat]
UseDiscs=TRUE
UseStuns=TRUE
[Heals]
SelfHealPct=50
GroupHealOn=TRUE
```
- Configure stun rotations
- Set up self-healing thresholds
- Enable group healing

### Shadow Knights (SHD)
```ini
[Combat]
UseDiscs=TRUE
UseLifetaps=TRUE
[Pet]
DoPet=TRUE
```
- Enable lifetap abilities
- Configure pet usage
- Set up aggro abilities

### Clerics (CLR)
```ini
[Heals]
HealPct=70
GroupHealPct=80
RezOn=TRUE
CHChainOn=FALSE
[Buffs]
DoBuffs=TRUE
```
- Configure heal thresholds
- Enable/disable CH chain
- Set up buff rotations

### Druids (DRU)
```ini
[Heals]
HealPct=65
[Combat]
DoNukes=TRUE
DoDots=TRUE
[Charm]
DoCharm=FALSE
```
- Balance healing vs DPS
- Configure dot/nuke usage
- Set up charm if desired

### Shamans (SHM)
```ini
[Heals]
HealPct=70
[Slow]
DoSlow=TRUE
SlowPct=95
[Buffs]
DoBuffs=TRUE
```
- Configure slow priority
- Set up canni rotation
- Enable buff rotations

### Enchanters (ENC)
```ini
[Mez]
MezOn=TRUE
MezRadius=100
MaxMezTargets=3
[Charm]
DoCharm=FALSE
[Buffs]
DoHaste=TRUE
```
- Configure mez settings
- Set up charm if desired
- Enable haste/buff rotations

### Magicians (MAG)
```ini
[Pet]
DoPet=TRUE
PetHealPct=50
PetBuffs=TRUE
[Combat]
DoNukes=TRUE
```
- Configure pet management
- Set up nuke rotations
- Enable pet toys

### Necromancers (NEC)
```ini
[Pet]
DoPet=TRUE
[Combat]
DoDots=TRUE
DoLifetaps=TRUE
[Charm]
DoCharm=FALSE
```
- Configure dot rotations
- Set up lifetap usage
- Enable pet management

### Wizards (WIZ)
```ini
[Combat]
DoNukes=TRUE
NukePct=90
UseHarvest=TRUE
```
- Configure nuke rotations
- Set mana thresholds
- Enable harvest

### Monks (MNK)
```ini
[Combat]
UseDiscs=TRUE
UseFD=TRUE
[Pull]
DoPull=FALSE
PullAbility=Distant Strike
```
- Configure disc rotations
- Set up FD usage
- Configure pulling

### Rogues (ROG)
```ini
[Combat]
UseDiscs=TRUE
UsePoison=TRUE
UseBackstab=TRUE
```
- Configure backstab usage
- Set up poison application
- Enable disc rotations

### Berserkers (BER)
```ini
[Combat]
UseDiscs=TRUE
UseFrenzy=TRUE
```
- Configure frenzy usage
- Set up disc rotations

### Rangers (RNG)
```ini
[Combat]
UseArchery=FALSE
DoDots=TRUE
DoNukes=TRUE
[Forage]
DoForage=TRUE
```
- Choose melee vs archery
- Configure dot/nuke usage
- Enable forage

### Bards (BRD)
```ini
[Twist]
DoTwist=TRUE
TwistList=1,2,3,4,5
MeleeList=6,7,8,9,10
[Mez]
MezOn=TRUE
MezSong=Wave of Sleep
```
- Configure twist lists
- Set up mez song
- Configure melee songs

### Beastlords (BST)
```ini
[Pet]
DoPet=TRUE
PetHealPct=60
[Combat]
DoNukes=TRUE
[Slow]
DoSlow=TRUE
```
- Configure pet management
- Set up slow priority
- Enable nuke rotations

---

## Combat System

### Assist Configuration

```ini
[Assist]
MainAssist=TankName
AssistAt=95
AssistRange=200
StickDistance=15
StickBehind=TRUE
```

| Setting | Description |
|---------|-------------|
| `MainAssist` | Character to assist |
| `AssistAt` | Mob HP % to start DPS |
| `AssistRange` | Max range to assist |
| `StickDistance` | Distance to maintain |
| `StickBehind` | Stay behind target |

### Combat Abilities

Configure abilities in INI:
```ini
[Combat]
Ability1=Ability Name|Conditions
Ability2=Another Ability|Conditions
...
```

#### Condition Examples
```ini
| Use when mob below 50%
Ability1=Execute|${Target.PctHPs}<50

| Use on cooldown
Ability2=Rampage|ready

| Use when above 50 endurance
Ability3=Mighty Strike|${Me.PctEndurance}>50
```

### Debuff System

```ini
[Debuffs]
DoDebuffs=TRUE
Debuff1=Malo|${Target.Named}
Debuff2=Slow|TRUE
DebuffMaxResists=3
DebuffRecastTime=30
```

| Setting | Description |
|---------|-------------|
| `DoDebuffs` | Enable debuffing |
| `DebuffMaxResists` | Stop after X resists |
| `DebuffRecastTime` | Seconds between attempts |

### Combat Hooks

The combat system provides hooks for customization:

- `OnCombatEnter` - Called when entering combat
- `OnCombatExit` - Called when leaving combat
- `OnCombatLoop` - Called each combat iteration

See [Custom Hooks](#custom-hooks) for usage.

---

## Healing System

### Basic Configuration

```ini
[Heals]
HealPct=70
GroupHealPct=80
HealRange=200
OOCHealPct=90
```

| Setting | Description |
|---------|-------------|
| `HealPct` | HP % to start healing |
| `GroupHealPct` | HP % for group heals |
| `HealRange` | Max healing range |
| `OOCHealPct` | Out of combat heal % |

### Heal Spell Configuration

```ini
[Heals]
MainHeal=Complete Heal
QuickHeal=Remedy
GroupHeal=Word of Vivification
HoT=Elixir of Healing
```

### Per-Class Thresholds

GZM supports different heal thresholds per class:

```ini
[Heals]
HealPct_WAR=80
HealPct_PAL=75
HealPct_CLR=70
HealPct_WIZ=65
HealPct_PET=50
```

### Heal Priority

Configure who to heal first:

```ini
[Heals]
HealPriority_WAR=1
HealPriority_CLR=2
HealPriority_DPS=3
HealPriority_PET=4
```

### XTarget Healing

Heal friendly targets in XTarget slots:

```ini
[Heals]
XTargetHealOn=TRUE
XTargetHealPct=70
XTargetHealRange=200
XTargetHealSlots=1,2,3,4,5
```

### Complete Heal Chain

For raid healing with CLR/DRU/SHM:

```ini
[CHChain]
CHChainOn=TRUE
CHSpell=Complete Heal
CHTarget=TankName
CHPosition=1
CHTotal=4
```

---

## Crowd Control

### Mezzing Configuration

```ini
[Mez]
MezOn=TRUE
MezSpell=Bewildering Wave
MezRadius=100
MaxMezTargets=3
MezImmune=golem,construct
AEMezOn=TRUE
AEMezCount=3
```

| Setting | Description |
|---------|-------------|
| `MezOn` | Enable mezzing |
| `MezSpell` | Primary mez spell |
| `MezRadius` | Range to mez targets |
| `MaxMezTargets` | Max mobs to mez |
| `MezImmune` | Mobs to skip |
| `AEMezOn` | Use AE mez |
| `AEMezCount` | Mob count for AE mez |

### Mez Priority

```ini
[Mez]
MezPriority1=a_caster
MezPriority2=a_healer
MezIgnore=pet,familiar
```

---

## Pet Management

### Basic Pet Configuration

```ini
[Pet]
DoPet=TRUE
PetSpell=Minion of Darkness
PetHealPct=50
PetBuffPct=90
AutoShrink=TRUE
```

| Setting | Description |
|---------|-------------|
| `DoPet` | Enable pet system |
| `PetSpell` | Pet summon spell |
| `PetHealPct` | HP % to heal pet |
| `PetBuffPct` | Buff if above this HP |
| `AutoShrink` | Auto-shrink pet |

### Pet Buffs

```ini
[Pet]
PetBuff1=Burnout
PetBuff2=Velocity
PetBuff3=Aegis of Kildrukaun
```

### Pet Combat

```ini
[Pet]
PetAssist=TRUE
PetTaunt=FALSE
PetHold=FALSE
PetRange=200
```

### Pet Resummon

```ini
[Pet]
ResummonPet=TRUE
ResummonDelay=5
MinManaForPet=40
```

---

## Pulling System

### Basic Configuration

```ini
[Pull]
DoPull=TRUE
PullRadius=200
PullZRadius=50
PullDelay=5
ReturnToCamp=TRUE
```

### Pull Abilities

```ini
[Pull]
PullAbility=Distant Strike
PullSpell=Snare
UseBow=FALSE
```

### Pull Safety

```ini
[Pull]
MinGroupHP=60
MinMana=30
MaxMobsInCamp=3
PullIfGroupSize=3
```

| Setting | Description |
|---------|-------------|
| `MinGroupHP` | Min group HP % to pull |
| `MinMana` | Min mana % to pull |
| `MaxMobsInCamp` | Stop if this many mobs |
| `PullIfGroupSize` | Min group size to pull |

### Pull Path

```ini
[Pull]
UsePullPath=TRUE
PullPath1=100,200,0
PullPath2=150,250,0
PullPath3=200,300,0
```

---

## Buff System

### Basic Configuration

```ini
[Buffs]
DoBuffs=TRUE
BuffCheckTimer=30
RebuffPct=20
```

### Self Buffs

```ini
[Buffs]
SelfBuff1=Shield of Fate
SelfBuff2=Armor of the Zealot
SelfBuff3=Symbol of Marzin
```

### Group Buffs

```ini
[Buffs]
GroupBuff1=Blessing of Aegolism
GroupBuff2=Virtue
GroupBuff3=Haste
```

### Raid Buffs

```ini
[Buffs]
RaidBuff1=Aegolism
RaidBuffClasses1=WAR,PAL,SHD
RaidBuff2=Focus of Spirit
RaidBuffClasses2=ALL
```

### Buff Conditions

```ini
[Buffs]
| Only buff warriors
GroupBuff1=Aegolism|WAR

| Buff if not already buffed
SelfBuff1=Shield|!${Me.Buff[Shield].ID}
```

---

## AA Auto-Purchase

### Configuration

```ini
[AA]
DoAA=TRUE
AABank=0
AAWarning=50
AAtoNormal=TRUE
```

| Setting | Description |
|---------|-------------|
| `DoAA` | Enable AA purchasing |
| `AABank` | AAs to keep banked |
| `AAWarning` | Warn at this many AAs |
| `AAtoNormal` | Switch to normal XP when maxed |

### AA Buy List

Create/edit the AA file:
```
MQ2/config/GZM_Server_CharName_AA.ini
```

```ini
[AAtoBuy]
AACount=5
AA1=Combat Agility|M|S
AA2=Combat Stability|M|S
AA3=Natural Durability|5|S
AA4=Innate Defense|M|S
AA5=Planar Power|M
```

#### Format
```
AAName|Level|SkipFlag
```

| Level | Description |
|-------|-------------|
| `M` | Max level |
| `L` | Max at current level |
| `X` | Skip |
| `5` | Buy up to rank 5 |

| SkipFlag | Description |
|----------|-------------|
| `S` | Skip to next if can't afford |
| (blank) | Wait until can afford |

---

## Navigation and Movement

### Camp Configuration

```ini
[Camp]
ReturnToCamp=TRUE
CampRadius=100
LeashLength=200
MaxZRange=50
```

### Navigation

```ini
[Nav]
UseNav=TRUE
NavStopDistance=15
NavTimeout=30
```

### Stick Settings

```ini
[Stick]
StickDistance=15
StickBehind=TRUE
StickPin=FALSE
StickSnaproll=FALSE
```

---

## Communication System

### DanNet (Primary)

```ini
[Comm]
UseDanNet=TRUE
DanNetChannel=group
```

#### DanNet Commands
```
| Observe a variable on another character
/dobserve CharName -q "${Target.ID}"

| Execute command on all group members
/dgae /mac gzm assist TankName 95

| Execute on specific character
/dex CharName /mac gzm assist TankName 95
```

### EQBC (Backup)

```ini
[Comm]
UseEQBC=FALSE
EQBCChannel=group
```

---

## Custom Hooks

### Overview

Edit `gronnzmaster_custom.inc` to add custom functionality without modifying core files.

### Available Hooks

#### CustomInit
Called during macro startup:
```
Sub CustomInit
    /echo [GZM-Custom] My custom init
    | Load custom settings, etc.
/return
```

#### CustomCombatHook
Called every combat loop:
```
Sub CustomCombatHook
    | Use custom ability
    /if (${Me.AltAbilityReady[My AA]}) {
        /alt act ${Me.AltAbility[My AA].ID}
    }
/return
```

#### CustomBuffHook
Called during buff checks:
```
Sub CustomBuffHook
    | Cast custom buff
    /if (!${Me.Buff[My Buff].ID}) {
        /call GZMCast "My Buff" ${Me.ID}
    }
/return
```

#### CustomHealHook
Called during heal checks:
```
Sub CustomHealHook
    | Custom heal logic
/return
```

#### CustomIdleHook
Called during idle time:
```
Sub CustomIdleHook
    | Auto-tradeskill, fishing, etc.
/return
```

#### CustomPullHook
Called before pulling:
```
Sub CustomPullHook
    | Check for named before pulling
    /if (${SpawnCount[npc ${NamedMob}]}) {
        /echo Named detected!
    }
/return
```

### Custom Events

Add custom event triggers:
```
#Event MyCustomEvent "#*#RAID WARNING#*#"

Sub Event_MyCustomEvent
    /echo Raid warning detected!
    | React to event
/return
```

---

## INI Reference

### Complete INI Structure

```ini
[General]
Debug=FALSE
Announce=group
Role=assist

[Assist]
MainAssist=TankName
AssistAt=95
AssistRange=200

[Camp]
ReturnToCamp=TRUE
CampRadius=100
CampX=0
CampY=0

[Combat]
DoMelee=TRUE
DoRanged=FALSE
UseDiscs=TRUE
Ability1=...

[Heals]
HealPct=70
GroupHealPct=80
MainHeal=...

[Buffs]
DoBuffs=TRUE
SelfBuff1=...
GroupBuff1=...

[Pet]
DoPet=TRUE
PetSpell=...

[Pull]
DoPull=FALSE
PullRadius=200

[Mez]
MezOn=FALSE
MezSpell=...

[Slow]
DoSlow=FALSE
SlowSpell=...

[Debuffs]
DoDebuffs=FALSE
Debuff1=...

[AA]
DoAA=FALSE
AABank=0

[Comm]
UseDanNet=TRUE
UseEQBC=FALSE
```

---

## Command Reference

### Runtime Variable Changes

```
| Change main assist
/varset MainAssist NewTankName

| Change assist percentage
/varset AssistAt 90

| Toggle features
/varset DoBuffs FALSE
/varset MezOn TRUE
/varset DoPull FALSE

| Change thresholds
/varset HealPct 60
/varset MezRadius 150

| Debug modes
/varset Debug TRUE
/varset DebugCombat TRUE
/varset DebugHeal TRUE
```

### Macro Control

```
| End macro
/endmac

| Pause macro
/mqpause

| Resume macro
/mqpause
```

### Status Commands

```
| AA Status
/call GetAAStatus

| Heal Status
/call GetHealStatus

| Combat Status
/call GetCombatHookStatus

| Debuff Status
/call GetDebuffStatus
```

---

## Troubleshooting

### Common Issues

#### Macro won't start
**Symptoms:** Error on /mac gzm

**Solutions:**
1. Check all required plugins are loaded
2. Verify `ninjadvloot.inc` exists
3. Check for syntax errors in INI

#### Character not assisting
**Symptoms:** Character stands idle during combat

**Solutions:**
1. Verify `MainAssist` is spelled correctly
2. Check `AssistAt` percentage
3. Ensure target is an NPC (not player pet)
4. Verify `AssistRange` is adequate

#### Not healing
**Symptoms:** Healer not casting heals

**Solutions:**
1. Check `HealPct` setting
2. Verify heal spells are memorized
3. Check mana levels
4. Verify `HealRange` setting

#### Navigation not working
**Symptoms:** Character doesn't move to camp/target

**Solutions:**
1. Verify MQ2Nav is loaded
2. Check mesh is loaded for zone: `/nav reload`
3. Verify camp location is set

#### Mezzing issues
**Symptoms:** Mobs not being mezzed

**Solutions:**
1. Check `MezOn=TRUE`
2. Verify mez spell is memorized
3. Check `MezRadius` setting
4. Verify mob isn't in `MezImmune` list

#### Pet not summoning
**Symptoms:** Pet class without pet

**Solutions:**
1. Check `DoPet=TRUE`
2. Verify `PetSpell` is correct
3. Check mana for summon
4. Check for reagents if needed

### Debug Mode

Enable debug output to troubleshoot:

```
/varset Debug TRUE
/varset DebugCombat TRUE
/varset DebugHeal TRUE
/varset DebugMez TRUE
/varset DebugPet TRUE
/varset DebugPull TRUE
```

Debug output format:
```
[GZM-DEBUG] L:123 T:45.6 Message here
```
- `L:` = Line number in macro
- `T:` = Macro runtime in seconds

### Log Files

Check MQ2 log files for errors:
```
MQ2/Logs/
```

### Getting Help

1. Check RedGuides forums: https://www.redguides.com/community/
2. Review debug output
3. Verify INI settings
4. Test with minimal configuration

---

## Appendix: Quick Reference Card

### Startup
```
/mac gzm [role] [mainassist] [assistat]
```

### Roles
- `tank` - Main tank
- `assist` - DPS
- `healer` - Healer
- `puller` - Puller
- `cc` - Crowd control

### Common Variables
| Variable | Description |
|----------|-------------|
| `MainAssist` | Who to assist |
| `AssistAt` | HP % to assist |
| `HealPct` | HP % to heal |
| `MezOn` | Enable mez |
| `DoPull` | Enable pulling |
| `DoBuffs` | Enable buffs |
| `Debug` | Debug mode |

### Emergency Commands
```
/endmac          | Stop macro
/mqpause         | Pause/unpause
/varset Debug 1  | Enable debug
```
