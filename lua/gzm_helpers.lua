--[[
    GZM Helpers - Lua Performance Module for GZM Macro
    Version 1.0

    This module provides high-performance data operations for GZM:
    - Hash table lookups instead of array iteration (O(1) vs O(n))
    - Immune tracking (mez, slow, charm immunities)
    - Mezzed mob tracking with timers
    - Slowed mob tracking
    - Spawn filtering and sorting

    Usage from macro:
        /lua run gzm_helpers
        /lua parse gzm_helpers.IsImmune("mob name", "mez")
        /lua parse gzm_helpers.AddImmune("mob name", "mez")

    The module runs persistently and responds to queries.
]]--

local mq = require('mq')

local VERSION = "1.0.0"

-- ============================================================================
-- DATA STRUCTURES (Hash tables for O(1) lookups)
-- ============================================================================

-- Immune tracking: immuneData[mobName][immuneType] = true
local immuneData = {}

-- Mezzed mobs: mezzedMobs[spawnID] = {name, expireTime}
local mezzedMobs = {}

-- Slowed mobs: slowedMobs[spawnID] = {name, expireTime, slowType}
local slowedMobs = {}

-- Charmed mobs: charmedMobs[spawnID] = {name, expireTime}
local charmedMobs = {}

-- Pull ignore list: pullIgnore[mobName] = true
local pullIgnore = {}

-- Current zone (to clear data on zone change)
local currentZone = ""

-- Debug flag
local debugMode = false

-- ============================================================================
-- UTILITY FUNCTIONS
-- ============================================================================

local function debug(msg)
    if debugMode then
        print(string.format("\ay[GZM-Lua] %s", msg))
    end
end

local function announce(msg)
    print(string.format("\ag[GZM-Lua] %s", msg))
end

-- Get current time in seconds
local function now()
    return os.time()
end

-- Check if zone changed and clear data if so
local function checkZone()
    local zone = mq.TLO.Zone.ShortName() or ""
    if zone ~= currentZone then
        debug("Zone changed from " .. currentZone .. " to " .. zone .. " - clearing data")
        currentZone = zone
        -- Clear zone-specific data
        mezzedMobs = {}
        slowedMobs = {}
        charmedMobs = {}
        -- Immune data persists (loaded from INI)
    end
end

-- ============================================================================
-- IMMUNE TRACKING (Hash table - O(1) lookups)
-- ============================================================================

-- Add immunity for a mob
local function addImmune(mobName, immuneType)
    if not mobName or mobName == "" then return false end
    immuneType = immuneType or "all"

    if not immuneData[mobName] then
        immuneData[mobName] = {}
    end

    if not immuneData[mobName][immuneType] then
        immuneData[mobName][immuneType] = true
        debug("Added immune: " .. mobName .. " -> " .. immuneType)
        return true
    end
    return false
end

-- Check if mob is immune
local function isImmune(mobName, immuneType)
    if not mobName or mobName == "" then return false end
    immuneType = immuneType or "all"

    if immuneData[mobName] then
        -- Check specific type or "all"
        if immuneData[mobName][immuneType] or immuneData[mobName]["all"] then
            return true
        end
    end
    return false
end

-- Remove immunity
local function removeImmune(mobName, immuneType)
    if not mobName or immuneData[mobName] == nil then return false end

    if immuneType then
        immuneData[mobName][immuneType] = nil
    else
        immuneData[mobName] = nil
    end
    return true
end

-- Clear all immunities
local function clearImmunes()
    immuneData = {}
    debug("Cleared all immunities")
end

-- Get immunity count
local function getImmuneCount()
    local count = 0
    for _ in pairs(immuneData) do
        count = count + 1
    end
    return count
end

-- Save immunities to INI
local function saveImmunes()
    local iniPath = mq.TLO.MacroQuest.Path() .. "\\config\\GZM_Immunes_" .. currentZone .. ".ini"
    local idx = 1

    for mobName, types in pairs(immuneData) do
        local typeList = {}
        for t, _ in pairs(types) do
            table.insert(typeList, t)
        end
        mq.cmdf('/ini "%s" "Immunes" "Mob%d" "%s"', iniPath, idx, mobName)
        mq.cmdf('/ini "%s" "%s" "Types" "%s"', iniPath, mobName, table.concat(typeList, "|"))
        idx = idx + 1
    end
    announce("Saved " .. (idx-1) .. " immunities for " .. currentZone)
end

-- Load immunities from INI
local function loadImmunes()
    local iniPath = mq.TLO.MacroQuest.Path() .. "\\config\\GZM_Immunes_" .. currentZone .. ".ini"
    immuneData = {}

    for i = 1, 100 do
        local mobName = mq.TLO.Ini(iniPath, "Immunes", "Mob" .. i)()
        if not mobName or mobName == "NULL" or mobName == "" then break end

        local types = mq.TLO.Ini(iniPath, mobName, "Types")() or "all"
        immuneData[mobName] = {}
        for t in string.gmatch(types, "[^|]+") do
            immuneData[mobName][t] = true
        end
    end

    local count = getImmuneCount()
    if count > 0 then
        debug("Loaded " .. count .. " immunities for " .. currentZone)
    end
end

-- ============================================================================
-- MEZZED MOB TRACKING
-- ============================================================================

-- Add mezzed mob
local function addMezzed(spawnID, duration)
    if not spawnID or spawnID == 0 then return false end
    duration = duration or 60

    local spawn = mq.TLO.Spawn(spawnID)
    local name = spawn.CleanName() or "Unknown"

    mezzedMobs[spawnID] = {
        name = name,
        expireTime = now() + duration
    }
    debug("Added mezzed: " .. name .. " (ID: " .. spawnID .. ") for " .. duration .. "s")
    return true
end

-- Check if mob is mezzed
local function isMezzed(spawnID)
    if not spawnID or not mezzedMobs[spawnID] then return false end

    -- Check if expired
    if now() >= mezzedMobs[spawnID].expireTime then
        mezzedMobs[spawnID] = nil
        return false
    end
    return true
end

-- Get mez time remaining
local function getMezTimeRemaining(spawnID)
    if not spawnID or not mezzedMobs[spawnID] then return 0 end

    local remaining = mezzedMobs[spawnID].expireTime - now()
    if remaining < 0 then
        mezzedMobs[spawnID] = nil
        return 0
    end
    return remaining
end

-- Remove mezzed mob
local function removeMezzed(spawnID)
    if mezzedMobs[spawnID] then
        mezzedMobs[spawnID] = nil
        return true
    end
    return false
end

-- Get mezzed mob count
local function getMezzedCount()
    local count = 0
    local expired = {}

    for id, data in pairs(mezzedMobs) do
        if now() < data.expireTime then
            count = count + 1
        else
            table.insert(expired, id)
        end
    end

    -- Clean expired
    for _, id in ipairs(expired) do
        mezzedMobs[id] = nil
    end

    return count
end

-- Get mezzed mob about to break (< threshold seconds)
local function getMezBreaking(threshold)
    threshold = threshold or 6
    local breaking = {}

    for id, data in pairs(mezzedMobs) do
        local remaining = data.expireTime - now()
        if remaining > 0 and remaining < threshold then
            -- Verify spawn still exists
            if mq.TLO.Spawn(id).ID() then
                table.insert(breaking, {id = id, name = data.name, remaining = remaining})
            else
                mezzedMobs[id] = nil
            end
        elseif remaining <= 0 then
            mezzedMobs[id] = nil
        end
    end

    -- Sort by time remaining (soonest first)
    table.sort(breaking, function(a, b) return a.remaining < b.remaining end)

    return breaking
end

-- ============================================================================
-- SLOWED MOB TRACKING
-- ============================================================================

-- Add slowed mob
local function addSlowed(spawnID, duration, slowType)
    if not spawnID or spawnID == 0 then return false end
    duration = duration or 120
    slowType = slowType or "slow"

    local spawn = mq.TLO.Spawn(spawnID)
    local name = spawn.CleanName() or "Unknown"

    slowedMobs[spawnID] = {
        name = name,
        expireTime = now() + duration,
        slowType = slowType
    }
    debug("Added slowed: " .. name .. " (ID: " .. spawnID .. ") - " .. slowType)
    return true
end

-- Check if mob is slowed
local function isSlowed(spawnID)
    if not spawnID or not slowedMobs[spawnID] then return false end

    if now() >= slowedMobs[spawnID].expireTime then
        slowedMobs[spawnID] = nil
        return false
    end
    return true
end

-- Remove slowed mob
local function removeSlowed(spawnID)
    if slowedMobs[spawnID] then
        slowedMobs[spawnID] = nil
        return true
    end
    return false
end

-- Get slowed count
local function getSlowedCount()
    local count = 0
    for id, data in pairs(slowedMobs) do
        if now() < data.expireTime then
            count = count + 1
        else
            slowedMobs[id] = nil
        end
    end
    return count
end

-- ============================================================================
-- PULL IGNORE LIST
-- ============================================================================

local function addPullIgnore(mobName)
    if not mobName or mobName == "" then return false end
    pullIgnore[mobName] = true
    return true
end

local function isPullIgnored(mobName)
    return pullIgnore[mobName] == true
end

local function removePullIgnore(mobName)
    pullIgnore[mobName] = nil
    return true
end

local function clearPullIgnore()
    pullIgnore = {}
end

-- ============================================================================
-- FIND BEST TARGET FUNCTIONS
-- ============================================================================

-- Find best mez target (not mezzed, not immune, in range)
local function findMezTarget(radius, minLevel, maxLevel)
    radius = radius or 60
    minLevel = minLevel or 1
    maxLevel = maxLevel or 999

    local bestTarget = nil
    local bestPriority = 0

    -- Scan XTarget first
    for i = 1, 20 do
        local xt = mq.TLO.Me.XTarget(i)
        if xt.ID() and xt.ID() > 0 then
            local id = xt.ID()
            local name = xt.CleanName() or ""
            local level = xt.Level() or 0
            local dist = xt.Distance() or 999
            local hp = xt.PctHPs() or 0

            -- Skip if: mezzed, immune, dead, out of range, wrong level, main assist target
            if dist <= radius and hp > 0 and level >= minLevel and level <= maxLevel then
                if not isMezzed(id) and not isImmune(name, "mez") then
                    -- Priority: lower level = easier to mez = higher priority
                    local priority = 100 - level
                    if priority > bestPriority then
                        bestPriority = priority
                        bestTarget = id
                    end
                end
            end
        end
    end

    return bestTarget
end

-- Find unmezzed adds count
local function countUnmezzedAdds(radius)
    radius = radius or 60
    local count = 0

    for i = 1, 20 do
        local xt = mq.TLO.Me.XTarget(i)
        if xt.ID() and xt.ID() > 0 then
            local id = xt.ID()
            local name = xt.CleanName() or ""
            local dist = xt.Distance() or 999
            local hp = xt.PctHPs() or 0

            if dist <= radius and hp > 0 and not isMezzed(id) and not isImmune(name, "mez") then
                count = count + 1
            end
        end
    end

    return count
end

-- ============================================================================
-- BIND COMMANDS (Called from macro via /lua parse)
-- ============================================================================

-- These functions are exposed globally so macro can call them

-- Format: Returns "TRUE" or "FALSE" for macro compatibility
local function boolToMacro(val)
    return val and "TRUE" or "FALSE"
end

-- Exposed functions for macro queries
_G.GZM = {
    -- Immune functions
    IsImmune = function(mobName, immuneType)
        return boolToMacro(isImmune(mobName, immuneType))
    end,
    AddImmune = function(mobName, immuneType)
        return boolToMacro(addImmune(mobName, immuneType))
    end,
    RemoveImmune = function(mobName, immuneType)
        return boolToMacro(removeImmune(mobName, immuneType))
    end,
    ClearImmunes = function()
        clearImmunes()
        return "TRUE"
    end,
    SaveImmunes = function()
        saveImmunes()
        return "TRUE"
    end,
    LoadImmunes = function()
        loadImmunes()
        return "TRUE"
    end,
    GetImmuneCount = function()
        return tostring(getImmuneCount())
    end,

    -- Mez tracking
    IsMezzed = function(spawnID)
        return boolToMacro(isMezzed(tonumber(spawnID)))
    end,
    AddMezzed = function(spawnID, duration)
        return boolToMacro(addMezzed(tonumber(spawnID), tonumber(duration)))
    end,
    RemoveMezzed = function(spawnID)
        return boolToMacro(removeMezzed(tonumber(spawnID)))
    end,
    GetMezTimeRemaining = function(spawnID)
        return tostring(getMezTimeRemaining(tonumber(spawnID)))
    end,
    GetMezzedCount = function()
        return tostring(getMezzedCount())
    end,
    GetMezBreaking = function(threshold)
        local breaking = getMezBreaking(tonumber(threshold))
        if #breaking > 0 then
            return tostring(breaking[1].id)
        end
        return "0"
    end,

    -- Slow tracking
    IsSlowed = function(spawnID)
        return boolToMacro(isSlowed(tonumber(spawnID)))
    end,
    AddSlowed = function(spawnID, duration, slowType)
        return boolToMacro(addSlowed(tonumber(spawnID), tonumber(duration), slowType))
    end,
    RemoveSlowed = function(spawnID)
        return boolToMacro(removeSlowed(tonumber(spawnID)))
    end,
    GetSlowedCount = function()
        return tostring(getSlowedCount())
    end,

    -- Pull ignore
    IsPullIgnored = function(mobName)
        return boolToMacro(isPullIgnored(mobName))
    end,
    AddPullIgnore = function(mobName)
        return boolToMacro(addPullIgnore(mobName))
    end,

    -- Target finding
    FindMezTarget = function(radius, minLevel, maxLevel)
        local target = findMezTarget(tonumber(radius), tonumber(minLevel), tonumber(maxLevel))
        return tostring(target or 0)
    end,
    CountUnmezzedAdds = function(radius)
        return tostring(countUnmezzedAdds(tonumber(radius)))
    end,

    -- Debug
    SetDebug = function(val)
        debugMode = (val == "1" or val == "true" or val == "TRUE")
        return boolToMacro(debugMode)
    end,

    -- Status
    Status = function()
        announce("GZM Helpers v" .. VERSION)
        announce("  Zone: " .. currentZone)
        announce("  Immunes: " .. getImmuneCount())
        announce("  Mezzed: " .. getMezzedCount())
        announce("  Slowed: " .. getSlowedCount())
        return "TRUE"
    end,

    -- Version
    Version = function()
        return VERSION
    end
}

-- ============================================================================
-- MAIN LOOP
-- ============================================================================

local function main()
    announce("GZM Helpers v" .. VERSION .. " loaded")
    announce("Use /lua parse GZM.Status() to check status")

    -- Initialize
    currentZone = mq.TLO.Zone.ShortName() or ""
    loadImmunes()

    -- Main loop - just keep alive and check for zone changes
    while true do
        checkZone()

        -- Cleanup expired mez/slow entries periodically
        getMezzedCount()
        getSlowedCount()

        mq.delay(1000)  -- Check every second
    end
end

-- Start
main()
