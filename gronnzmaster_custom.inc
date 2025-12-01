|**
    GronnzMaster_custom.inc - Custom User Extensions for GronnzMaster

    This file is included optionally and will not be overwritten on updates.
    Add your custom subroutines, events, and modifications here.

    To use: Create custom subs and call them from main macro hooks
**|

| ============================================================================
| CUSTOM EVENTS - Add your own event triggers here
| ============================================================================

| Example: Custom raid warning
|#Event CustomRaidWarning "#*#RAID WARNING#*#"

| ============================================================================
| CUSTOM VARIABLES - Declare any custom variables here
| ============================================================================
Sub CustomDeclareVars
    | Example:
    | /declare MyCustomVar int outer 0
/return

| ============================================================================
| CUSTOM INITIALIZATION - Called during macro startup
| ============================================================================
Sub CustomInit
    /echo \aw[GZM-Custom] Custom initialization loaded

    | Add your startup code here
    | Example: Load custom settings
    | /call LoadCustomSettings
/return

| ============================================================================
| CUSTOM COMBAT HOOK - Called during combat loop
| ============================================================================
Sub CustomCombatHook
    | Add custom combat logic here
    | This is called every combat loop iteration

    | Example: Use a custom ability
    | /if (${Me.AltAbilityReady[My Custom AA]}) {
    |     /alt act ${Me.AltAbility[My Custom AA].ID}
    | }
/return

| ============================================================================
| CUSTOM BUFF HOOK - Called during buff checks
| ============================================================================
Sub CustomBuffHook
    | Add custom buff logic here

    | Example: Cast a custom buff
    | /if (!${Me.Buff[My Custom Buff].ID}) {
    |     /call GZMCast "My Custom Buff" ${Me.ID}
    | }
/return

| ============================================================================
| CUSTOM HEAL HOOK - Called during heal checks
| ============================================================================
Sub CustomHealHook
    | Add custom healing logic here
/return

| ============================================================================
| CUSTOM PULL HOOK - Called before pulling
| ============================================================================
Sub CustomPullHook
    | Add custom pull logic here

    | Example: Check for specific named before pulling
    | /if (${SpawnCount[npc ${MyNamedMob}]}) {
    |     /echo Named mob detected!
    | }
/return

| ============================================================================
| CUSTOM IDLE HOOK - Called during idle time
| ============================================================================
Sub CustomIdleHook
    | Add custom idle logic here

    | Example: Auto-tradeskill
    | Example: Auto-fishing
    | Example: Custom announcements
/return

| ============================================================================
| CUSTOM TRIGGERS - Add handlers for your custom events
| ============================================================================

|Sub Event_CustomRaidWarning
|    /echo \arRAID WARNING DETECTED!
|    | Add your raid warning response here
|/return

| ============================================================================
| CUSTOM UTILITY FUNCTIONS
| ============================================================================

| Example: Check if a specific buff is on target
Sub CustomHasBuff(int TargetID, string BuffName)
    /if (${Spawn[${TargetID}].Buff[${BuffName}].ID}) {
        /return 1
    }
/return 0

| Example: Custom announcement
Sub CustomAnnounce(string Message)
    /if (${UseDanNet}) {
        /dgae /echo [Custom] ${Message}
    } else {
        /gsay [Custom] ${Message}
    }
/return

| ============================================================================
| CLASS-SPECIFIC CUSTOMIZATIONS
| ============================================================================

| Add class-specific custom logic here
| Example for Bard:
Sub CustomBardTwist
    | Custom twist order
    | /if (${Me.Class.ShortName.Equal[BRD]}) {
    |     /twist 1 2 3 4 5
    | }
/return

| Example for Enchanter:
Sub CustomEnchanterAura
    | /if (${Me.Class.ShortName.Equal[ENC]}) {
    |     | Check aura
    | }
/return

| ============================================================================
| RAID-SPECIFIC CUSTOMIZATIONS
| ============================================================================

Sub CustomRaidCheck
    | Add raid-specific logic
    | Example: Check for raid targets
    | Example: Raid buff assignments
/return

| ============================================================================
| END OF CUSTOM INCLUDE
| ============================================================================
