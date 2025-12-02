--[[
    GZM GUI - Simple, Functional Control Panel
    Version 1.0

    A minimal GUI focused on what you actually need during gameplay:
    - Status display (state, target, combat)
    - Toggle switches for all major features
    - Slider controls for key percentages
    - Quick buttons for common actions

    Usage: /lua run gzm_gui

    Design Philosophy:
    - Read macro variables directly (no complex sync)
    - Write changes via /varset commands (reliable)
    - Simple ImGui layout (no complex state)
    - Auto-hide when macro not running
]]--

local mq = require('mq')
local imgui = require('ImGui')

local VERSION = "1.0.0"

-- ============================================================================
-- GUI STATE
-- ============================================================================

local gui = {
    open = true,
    collapsed = false,
    showToggles = true,
    showSliders = true,
    showStatus = true,
    showQuickButtons = true,
}

-- Colors
local COLORS = {
    enabled = ImVec4(0.2, 0.8, 0.2, 1.0),      -- Green
    disabled = ImVec4(0.8, 0.2, 0.2, 1.0),     -- Red
    warning = ImVec4(0.9, 0.7, 0.0, 1.0),      -- Yellow
    combat = ImVec4(1.0, 0.3, 0.3, 1.0),       -- Red
    idle = ImVec4(0.3, 0.7, 1.0, 1.0),         -- Blue
    header = ImVec4(0.4, 0.6, 0.8, 1.0),       -- Light blue
    mana = ImVec4(0.3, 0.5, 0.9, 1.0),         -- Blue
    health = ImVec4(0.2, 0.8, 0.2, 1.0),       -- Green
    endurance = ImVec4(0.9, 0.7, 0.0, 1.0),    -- Yellow
}

-- ============================================================================
-- HELPER FUNCTIONS
-- ============================================================================

-- Check if GZM macro is running
local function isMacroRunning()
    local macroName = mq.TLO.Macro.Name()
    if not macroName then return false end
    return macroName:lower():find("gzm") ~= nil
end

-- Read macro variable (returns string or nil)
local function getVar(name)
    if not isMacroRunning() then return nil end
    local val = mq.TLO.Macro.Variable(name)()
    return val
end

-- Read macro variable as number
local function getVarNum(name, default)
    local val = getVar(name)
    if val then return tonumber(val) or default end
    return default
end

-- Read macro variable as boolean (1 = true)
local function getVarBool(name)
    local val = getVar(name)
    return val == "1" or val == "TRUE" or val == "true"
end

-- Set macro variable
local function setVar(name, value)
    mq.cmdf('/varset %s %s', name, tostring(value))
end

-- Toggle macro variable (0/1)
local function toggleVar(name)
    local current = getVarBool(name)
    setVar(name, current and "0" or "1")
end

-- Execute macro command
local function execCmd(cmd)
    mq.cmd(cmd)
end

-- ============================================================================
-- UI COMPONENTS
-- ============================================================================

-- Draw a toggle button with color feedback
local function drawToggle(label, varName, width)
    width = width or 80
    local isOn = getVarBool(varName)
    local color = isOn and COLORS.enabled or COLORS.disabled

    imgui.PushStyleColor(ImGuiCol.Button, color)
    imgui.PushStyleColor(ImGuiCol.ButtonHovered, ImVec4(color.x + 0.1, color.y + 0.1, color.z + 0.1, 1.0))

    if imgui.Button(label .. (isOn and " ON" or " OFF") .. "##" .. varName, ImVec2(width, 0)) then
        toggleVar(varName)
    end

    imgui.PopStyleColor(2)
    return isOn
end

-- Draw a slider that writes to macro variable
local function drawSlider(label, varName, min, max, width)
    width = width or 120
    local value = getVarNum(varName, min)

    imgui.SetNextItemWidth(width)
    local changed, newValue = imgui.SliderInt(label .. "##" .. varName, value, min, max)
    if changed then
        setVar(varName, newValue)
    end
    return newValue
end

-- Draw a command button
local function drawCmdButton(label, cmd, width, color)
    width = width or 80
    if color then
        imgui.PushStyleColor(ImGuiCol.Button, color)
    end

    if imgui.Button(label, ImVec2(width, 0)) then
        execCmd(cmd)
    end

    if color then
        imgui.PopStyleColor()
    end
end

-- Draw a progress bar
local function drawProgressBar(label, value, max, color, width)
    width = width or 200
    local pct = (value or 0) / (max or 100)

    if color then
        imgui.PushStyleColor(ImGuiCol.PlotHistogram, color)
    end

    imgui.ProgressBar(pct, ImVec2(width, 0), string.format("%s: %d%%", label, (value or 0)))

    if color then
        imgui.PopStyleColor()
    end
end

-- ============================================================================
-- MAIN UI SECTIONS
-- ============================================================================

local function drawStatusSection()
    if not imgui.CollapsingHeader("Status", ImGuiTreeNodeFlags.DefaultOpen) then return end

    -- Character info
    local hp = mq.TLO.Me.PctHPs() or 0
    local mana = mq.TLO.Me.PctMana() or 0
    local endur = mq.TLO.Me.PctEndurance() or 0

    drawProgressBar("HP", hp, 100, COLORS.health, 280)
    drawProgressBar("Mana", mana, 100, COLORS.mana, 280)
    drawProgressBar("End", endur, 100, COLORS.endurance, 280)

    imgui.Separator()

    -- Macro state
    local role = getVar("Role") or "Unknown"
    local ma = getVar("MainAssist") or "None"
    local assistAt = getVarNum("AssistAt", 98)
    local inCombat = getVarBool("InCombat")
    local paused = getVarBool("Paused")

    -- State indicator
    local stateColor = inCombat and COLORS.combat or COLORS.idle
    local stateText = inCombat and "COMBAT" or "IDLE"
    if paused then
        stateColor = COLORS.warning
        stateText = "PAUSED"
    end

    imgui.TextColored(stateColor, "State: " .. stateText)
    imgui.SameLine(150)
    imgui.Text("Role: " .. role)

    imgui.Text("MA: " .. ma)
    imgui.SameLine(150)
    imgui.Text("Assist@: " .. assistAt .. "%%")

    -- Target info
    if mq.TLO.Target.ID() then
        imgui.Separator()
        local tgtName = mq.TLO.Target.CleanName() or "Unknown"
        local tgtHP = mq.TLO.Target.PctHPs() or 0
        local tgtDist = mq.TLO.Target.Distance() or 0

        imgui.Text("Target: " .. tgtName)
        imgui.Text(string.format("HP: %d%%  Dist: %d", tgtHP, tgtDist))
    end
end

local function drawTogglesSection()
    if not imgui.CollapsingHeader("Toggles", ImGuiTreeNodeFlags.DefaultOpen) then return end

    -- Row 1: Core toggles
    drawToggle("Assist", "AssistOn", 75)
    imgui.SameLine()
    drawToggle("Melee", "MeleeOn", 75)
    imgui.SameLine()
    drawToggle("DPS", "DPSOn", 70)
    imgui.SameLine()
    drawToggle("Heals", "HealsOn", 75)

    -- Row 2: Feature toggles
    drawToggle("Buffs", "BuffsOn", 75)
    imgui.SameLine()
    drawToggle("Pet", "PetOn", 70)
    imgui.SameLine()
    drawToggle("Mez", "MezOn", 70)
    imgui.SameLine()
    drawToggle("Slow", "SlowOn", 70)

    -- Row 3: Utility toggles
    drawToggle("Pull", "PullOn", 70)
    imgui.SameLine()
    drawToggle("Loot", "LootOn", 70)
    imgui.SameLine()
    drawToggle("Items", "ItemClicksOn", 75)
    imgui.SameLine()
    drawToggle("Charm", "CharmOn", 80)

    -- Pause button (special)
    imgui.Separator()
    local paused = getVarBool("Paused")
    if paused then
        imgui.PushStyleColor(ImGuiCol.Button, COLORS.warning)
        if imgui.Button("RESUME", ImVec2(140, 25)) then
            setVar("Paused", "0")
        end
        imgui.PopStyleColor()
    else
        if imgui.Button("PAUSE", ImVec2(140, 25)) then
            setVar("Paused", "1")
        end
    end
    imgui.SameLine()
    drawCmdButton("BackOff", "/backoff", 140)
end

local function drawSlidersSection()
    if not imgui.CollapsingHeader("Settings") then return end

    -- Assist settings
    imgui.Text("Combat:")
    drawSlider("Assist %", "AssistAt", 1, 100, 150)
    imgui.SameLine()
    drawSlider("Stick Dist", "StickDistance", 5, 50, 150)

    -- Healing settings
    imgui.Text("Healing:")
    drawSlider("Heal %", "HealPct", 1, 99, 150)
    imgui.SameLine()
    drawSlider("Emergency %", "EmergencyHealPct", 1, 50, 150)

    drawSlider("Group Heal %", "GroupHealPct", 1, 99, 150)
    imgui.SameLine()
    drawSlider("OOG Heal %", "OOGHealPct", 1, 99, 150)

    -- DPS settings
    imgui.Text("DPS:")
    drawSlider("Nuke Mana %", "NukeManaPct", 1, 100, 150)
    imgui.SameLine()
    drawSlider("DoT Mana %", "DoTManaPct", 1, 100, 150)

    drawSlider("Stop DPS %", "StopDPSPct", 1, 50, 150)

    -- Pull settings
    imgui.Text("Pull:")
    drawSlider("Pull Range", "PullRange", 50, 500, 150)
    imgui.SameLine()
    drawSlider("Pull Z Range", "PullZRange", 10, 200, 150)

    -- Camp settings
    imgui.Text("Camp:")
    drawSlider("Camp Radius", "CampRadius", 10, 200, 150)
    imgui.SameLine()
    drawSlider("Mez Radius", "MezRadius", 20, 150, 150)
end

local function drawQuickButtonsSection()
    if not imgui.CollapsingHeader("Quick Actions", ImGuiTreeNodeFlags.DefaultOpen) then return end

    -- Camp controls
    drawCmdButton("Make Camp", "/makecamp on", 90)
    imgui.SameLine()
    drawCmdButton("Clear Camp", "/makecamp off", 90)
    imgui.SameLine()
    drawCmdButton("Nav Stop", "/nav stop", 90)

    -- Movement
    drawCmdButton("Chase MA", "/chaseme", 90)
    imgui.SameLine()
    drawCmdButton("Stop Chase", "/stayhere", 90)
    imgui.SameLine()
    drawCmdButton("Stick", "/stick", 90)

    imgui.Separator()

    -- Combat
    drawCmdButton("BURN!", "/burn", 90, COLORS.combat)
    imgui.SameLine()
    drawCmdButton("Attack", "/attack on", 90)
    imgui.SameLine()
    drawCmdButton("Back Off", "/attack off", 90)

    -- Pet controls
    drawCmdButton("Pet Atk", "/pet attack", 90)
    imgui.SameLine()
    drawCmdButton("Pet Back", "/pet back off", 90)
    imgui.SameLine()
    drawCmdButton("Pet Hold", "/varset PetHold 1", 90)

    imgui.Separator()

    -- DanNet commands
    imgui.Text("Group Commands:")
    drawCmdButton("Grp Pause", "/dgge /varset Paused 1", 90)
    imgui.SameLine()
    drawCmdButton("Grp Resume", "/dgge /varset Paused 0", 90)
    imgui.SameLine()
    drawCmdButton("Grp Burn", "/dgge /burn", 90)
end

local function drawCHChainSection()
    -- Only show if CH chain variables exist
    local chActive = getVar("CHChainActive")
    if chActive == nil then return end

    if not imgui.CollapsingHeader("CH Chain") then return end

    local isActive = getVarBool("CHChainActive")
    local target = getVar("CHTargetName") or "None"
    local mySlot = getVarNum("CHMySlot", 0)
    local totalClerics = getVarNum("CHClericCount", 0)

    -- Status
    local statusColor = isActive and COLORS.enabled or COLORS.disabled
    imgui.TextColored(statusColor, "CH Chain: " .. (isActive and "ACTIVE" or "INACTIVE"))

    if isActive then
        imgui.Text("Target: " .. target)
        imgui.Text(string.format("My Slot: %d of %d", mySlot, totalClerics))

        imgui.Separator()
        drawCmdButton("Stop CH", "/stopch", 100)
    else
        imgui.Text("Not in a CH chain")
        imgui.Separator()

        -- Quick start buttons
        imgui.Text("Start with clerics:")
        -- Would need to know cleric names to populate this
    end
end

local function drawMezTrackerSection()
    -- Only show for enchanters/bards with mezzing on
    if not getVarBool("MezOn") then return end

    if not imgui.CollapsingHeader("Mez Tracker") then return end

    local mezCount = getVarNum("MezTargetCount", 0)

    if mezCount == 0 then
        imgui.Text("No mobs mezzed")
        return
    end

    imgui.Text(string.format("Mezzed: %d", mezCount))

    -- Show each mezzed mob
    for i = 1, 5 do
        local id = getVarNum("MezTarget" .. i .. "ID", 0)
        if id > 0 then
            local name = getVar("MezTarget" .. i .. "Name") or "Unknown"
            local timer = getVarNum("MezTarget" .. i .. "Timer", 0) / 10  -- Convert to seconds

            local timerColor = timer < 10 and COLORS.combat or COLORS.enabled
            imgui.TextColored(timerColor, string.format("%d. %s (%.1fs)", i, name, timer))
        end
    end
end

-- ============================================================================
-- MAIN GUI RENDER
-- ============================================================================

local function renderGUI()
    if not gui.open then return end

    -- Check if macro is running
    if not isMacroRunning() then
        imgui.SetNextWindowSize(ImVec2(200, 80), ImGuiCond.FirstUseEver)
        gui.open, _ = imgui.Begin("GZM Control", gui.open, ImGuiWindowFlags.NoResize)
        imgui.TextColored(COLORS.warning, "GZM Macro not running")
        imgui.Text("Start with: /mac gzm")
        imgui.End()
        return
    end

    -- Main window
    imgui.SetNextWindowSize(ImVec2(320, 500), ImGuiCond.FirstUseEver)
    gui.open, _ = imgui.Begin("GZM Control v" .. VERSION, gui.open)

    -- Draw all sections
    drawStatusSection()
    imgui.Separator()
    drawTogglesSection()
    imgui.Separator()
    drawSlidersSection()
    imgui.Separator()
    drawQuickButtonsSection()
    drawCHChainSection()
    drawMezTrackerSection()

    imgui.End()
end

-- ============================================================================
-- MAIN LOOP
-- ============================================================================

local function main()
    print("\ag[GZM-GUI] GZM Control Panel v" .. VERSION .. " loaded")
    print("\ag[GZM-GUI] Close window or /lua stop gzm_gui to exit")

    -- Register ImGui callback
    mq.imgui.init('GZM_GUI', renderGUI)

    -- Main loop
    while gui.open do
        mq.delay(100)
    end

    print("\ay[GZM-GUI] GUI closed")
end

main()
