|- GZM Lua Integration Layer
|- Provides hybrid Lua/Macro operation for performance
|- Version 1.0
|-
|- This include provides wrapper subs that:
|- 1. Check if Lua helper is running
|- 2. Use Lua for O(1) hash lookups when available
|- 3. Fall back to macro arrays when Lua unavailable
|-
|- Usage: Just include this file - it auto-detects Lua availability

|- ============================================================================
|- CONFIGURATION
|- ============================================================================

#DEFINE GZM_LUA_SCRIPT gzm_helpers

|- ============================================================================
|- INITIALIZATION
|- ============================================================================

Sub InitLuaIntegration
    /if (!${Defined[GZMLuaEnabled]}) /declare GZMLuaEnabled bool outer FALSE
    /if (!${Defined[GZMLuaChecked]}) /declare GZMLuaChecked bool outer FALSE
    /if (!${Defined[GZMLuaCheckTimer]}) /declare GZMLuaCheckTimer timer outer 0

    |- Check if Lua is available
    /call CheckLuaStatus
/return

|- ============================================================================
|- LUA STATUS CHECK
|- ============================================================================

Sub CheckLuaStatus
    |- Only check every 30 seconds to avoid spam
    /if (${GZMLuaChecked} && ${GZMLuaCheckTimer}) /return

    /declare luaRunning bool local FALSE

    |- Check if our Lua script is running
    /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
        /varset luaRunning TRUE
    }

    /if (${luaRunning} && !${GZMLuaEnabled}) {
        /echo \ag[GZM-Lua] Lua helpers detected - hybrid mode ENABLED
        /varset GZMLuaEnabled TRUE
    } else /if (!${luaRunning} && ${GZMLuaEnabled}) {
        /echo \ay[GZM-Lua] Lua helpers stopped - falling back to macro mode
        /varset GZMLuaEnabled FALSE
    }

    /varset GZMLuaChecked TRUE
    /varset GZMLuaCheckTimer 300
/return

|- Start Lua helpers if not running
Sub StartLuaHelpers
    /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
        /echo \ag[GZM-Lua] Lua helpers already running
        /return
    }

    /lua run gzm_helpers
    /delay 5
    /call CheckLuaStatus
/return

|- Stop Lua helpers
Sub StopLuaHelpers
    /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
        /lua stop gzm_helpers
        /varset GZMLuaEnabled FALSE
        /echo \ay[GZM-Lua] Lua helpers stopped
    }
/return

|- ============================================================================
|- IMMUNE TRACKING WRAPPERS
|- ============================================================================

|- Check if mob is immune (uses Lua hash table if available)
Sub Lua_IsImmune(mobName, immuneType)
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            |- Use Lua O(1) lookup
            /declare result string local ${Lua.Parse[GZM.IsImmune("${mobName}", "${immuneType}")]}
            /if (${result.Equal[TRUE]}) /return TRUE
            /return FALSE
        }
    }

    |- Fall back to macro array scan
    /call IsImmune ${mobName} ${immuneType}
/return ${Macro.Return}

|- Add immune entry (uses Lua if available)
Sub Lua_AddImmune(mobName, immuneType)
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            /lua parse GZM.AddImmune("${mobName}", "${immuneType}")
            /return
        }
    }

    |- Fall back to macro
    /call AddImmune ${mobName} ${immuneType}
/return

|- Save immunities
Sub Lua_SaveImmunes
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            /lua parse GZM.SaveImmunes()
            /return
        }
    }

    |- Fall back to macro
    /call SaveImmunes
/return

|- Load immunities
Sub Lua_LoadImmunes
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            /lua parse GZM.LoadImmunes()
            /return
        }
    }

    |- Fall back to macro
    /call LoadImmunes
/return

|- ============================================================================
|- MEZ TRACKING WRAPPERS
|- ============================================================================

|- Check if mob is mezzed
Sub Lua_IsMezzed(spawnID)
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            /declare result string local ${Lua.Parse[GZM.IsMezzed(${spawnID})]}
            /if (${result.Equal[TRUE]}) /return TRUE
            /return FALSE
        }
    }

    |- Fall back to macro array
    /declare i int local
    /for i 1 to ${MezzedMobCount}
        /if (${MezzedMob[${i}]}==${spawnID} && ${MezzedMobTimer[${i}]}) {
            /return TRUE
        }
    /next i
/return FALSE

|- Add mezzed mob
Sub Lua_AddMezzed(spawnID, duration)
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            /lua parse GZM.AddMezzed(${spawnID}, ${duration})
            /return
        }
    }

    |- Fall back to macro array
    /declare i int local
    /for i 1 to 20
        /if (!${MezzedMob[${i}]} || !${MezzedMobTimer[${i}]}) {
            /varset MezzedMob[${i}] ${spawnID}
            /varset MezzedMobTimer[${i}] ${Math.Calc[${duration}*10]}
            /if (${i}>${MezzedMobCount}) /varset MezzedMobCount ${i}
            /return
        }
    /next i
/return

|- Get mez time remaining
Sub Lua_GetMezTimeRemaining(spawnID)
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            /return ${Lua.Parse[GZM.GetMezTimeRemaining(${spawnID})]}
        }
    }

    |- Fall back to macro
    /declare i int local
    /for i 1 to ${MezzedMobCount}
        /if (${MezzedMob[${i}]}==${spawnID}) {
            /return ${Math.Calc[${MezzedMobTimer[${i}]}/10]}
        }
    /next i
/return 0

|- Get mob about to break mez
Sub Lua_GetMezBreaking(threshold)
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            /return ${Lua.Parse[GZM.GetMezBreaking(${threshold})]}
        }
    }

    |- Fall back to macro scan
    /declare i int local
    /declare thresholdTicks int local ${Math.Calc[${threshold}*10]}
    /for i 1 to ${MezzedMobCount}
        /if (${MezzedMob[${i}]} && ${MezzedMobTimer[${i}]} < ${thresholdTicks} && ${MezzedMobTimer[${i}]} > 0) {
            /if (${Spawn[${MezzedMob[${i}]}].ID}) {
                /return ${MezzedMob[${i}]}
            }
        }
    /next i
/return 0

|- ============================================================================
|- SLOW TRACKING WRAPPERS
|- ============================================================================

|- Check if mob is slowed
Sub Lua_IsSlowed(spawnID)
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            /declare result string local ${Lua.Parse[GZM.IsSlowed(${spawnID})]}
            /if (${result.Equal[TRUE]}) /return TRUE
            /return FALSE
        }
    }

    |- Fall back to macro array
    /declare i int local
    /for i 1 to ${SlowedMobCount}
        /if (${SlowedMobID[${i}]}==${spawnID}) {
            /return TRUE
        }
    /next i
/return FALSE

|- Add slowed mob
Sub Lua_AddSlowed(spawnID, duration, slowType)
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            /lua parse GZM.AddSlowed(${spawnID}, ${duration}, "${slowType}")
            /return
        }
    }

    |- Fall back to macro array
    /declare i int local
    /for i 1 to 30
        /if (!${SlowedMobID[${i}]}) {
            /varset SlowedMobID[${i}] ${spawnID}
            /if (${i}>${SlowedMobCount}) /varset SlowedMobCount ${i}
            /return
        }
    /next i
/return

|- ============================================================================
|- TARGET FINDING WRAPPERS
|- ============================================================================

|- Find best mez target using Lua (much faster XTarget scan)
Sub Lua_FindMezTarget(radius, minLevel, maxLevel)
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            /return ${Lua.Parse[GZM.FindMezTarget(${radius}, ${minLevel}, ${maxLevel})]}
        }
    }

    |- Fall back to macro scan - implemented in gzm_mez.inc
/return 0

|- Count unmezzed adds
Sub Lua_CountUnmezzedAdds(radius)
    /if (${GZMLuaEnabled}) {
        /if (${Lua.Script[gzm_helpers].Status.Equal[RUNNING]}) {
            /return ${Lua.Parse[GZM.CountUnmezzedAdds(${radius})]}
        }
    }

    |- Fall back - count would be done in mez check
/return 0

|- ============================================================================
|- UTILITY COMMANDS
|- ============================================================================

|- Show Lua helper status
Sub Lua_Status
    /call CheckLuaStatus

    /if (${GZMLuaEnabled}) {
        /lua parse GZM.Status()
    } else {
        /echo \ay[GZM-Lua] Lua helpers NOT running - using macro mode
        /echo \ay[GZM-Lua] Start with: /lua run gzm_helpers
    }
/return

|- Toggle debug mode
Sub Lua_SetDebug(enabled)
    /if (${GZMLuaEnabled}) {
        /lua parse GZM.SetDebug("${enabled}")
    }
/return

|- ============================================================================
|- BIND COMMAND HANDLERS
|- ============================================================================

|- /gzmlua - Show Lua helper status
Sub Bind_LuaStatus
    /call Lua_Status
/return

|- /gzmluastart - Start Lua helpers
Sub Bind_LuaStart
    /call StartLuaHelpers
/return

|- /gzmlualstop - Stop Lua helpers
Sub Bind_LuaStop
    /call StopLuaHelpers
/return
