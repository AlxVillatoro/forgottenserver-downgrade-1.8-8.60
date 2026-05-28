local OPCODE_WHEEL_OPEN = 0x61
local OPCODE_WHEEL_SAVE = 0x62
local OPCODE_WHEEL_GEM_ACTION = 0xE7
local OPCODE_WHEEL_WINDOW = 0x5F
local OPCODE_RESOURCE_BALANCE = 0xEE

local WHEEL_MIN_LEVEL = 51
local WHEEL_POINTS_PER_LEVEL = 1
local WHEEL_SLOT_COUNT = 36
local WHEEL_NO_GEM = 0

local RESOURCE_BANK = 0
local RESOURCE_INVENTORY = 1
local RESOURCE_LESSER_GEMS = 81
local RESOURCE_REGULAR_GEMS = 82
local RESOURCE_GREATER_GEMS = 83
local RESOURCE_LESSER_FRAGMENTS = 84
local RESOURCE_GREATER_FRAGMENTS = 85

local ITEM_LESSER_FRAGMENT = 46625
local ITEM_GREATER_FRAGMENT = 46626

local GEM_ITEMS = {
	[1] = { 44602, 44603, 44604 }, -- Knight
	[2] = { 44605, 44606, 44607 }, -- Paladin
	[3] = { 44608, 44609, 44610 }, -- Sorcerer
	[4] = { 44611, 44612, 44613 }, -- Druid
	[5] = { 49371, 49372, 49373 }, -- Monk
}

local WHEEL_SLOT_MAX_POINTS = {
	200, 150, 100, 100, 150, 200, 150, 100, 75,
	75, 100, 150, 100, 75, 50, 50, 75, 100,
	100, 75, 50, 50, 75, 100, 150, 100, 75,
	75, 100, 150, 200, 150, 100, 100, 150, 200
}

local WHEEL_SLOT_DOMAINS = {
	1, 1, 1, 2, 2, 2, 1, 1, 1,
	2, 2, 2, 1, 1, 1, 2, 2, 2,
	3, 3, 4, 4, 4, 4, 3, 3, 3,
	4, 4, 4, 3, 3, 3, 4, 4, 4
}

local WHEEL_SLOT_PREREQUISITES = {
	[1] = { 2, 7 },
	[2] = { 3, 8, 7, 1 },
	[3] = { 8, 9, 4, 2 },
	[4] = { 3, 10, 11, 5 },
	[5] = { 4, 11, 12, 6 },
	[6] = { 12, 5 },
	[7] = { 8, 13, 2, 1 },
	[8] = { 14, 9, 13, 3, 7, 2 },
	[9] = { 14, 15, 10, 3, 8 },
	[10] = { 9, 16, 17, 4, 11 },
	[11] = { 10, 17, 4, 18, 5, 12 },
	[12] = { 11, 18, 5, 6 },
	[13] = { 8, 14, 19, 7 },
	[14] = { 9, 15, 20, 13, 8 },
	[17] = { 10, 16, 23, 18, 11 },
	[18] = { 17, 11, 24, 12 },
	[19] = { 13, 20, 26, 25 },
	[20] = { 21, 14, 27, 19, 26 },
	[23] = { 22, 28, 17, 24, 29 },
	[24] = { 23, 18, 29, 30 },
	[25] = { 19, 26, 32, 31 },
	[26] = { 27, 20, 19, 25, 32, 33 },
	[27] = { 21, 28, 20, 26, 33 },
	[28] = { 22, 23, 27, 29, 34 },
	[29] = { 23, 28, 24, 34, 30, 35 },
	[30] = { 24, 29, 35, 36 },
	[31] = { 25, 32 },
	[32] = { 26, 25, 33, 31 },
	[33] = { 27, 34, 26, 32 },
	[34] = { 28, 33, 29, 35 },
	[35] = { 29, 34, 30, 36 },
	[36] = { 30, 35 },
}

local function supportsCustomNetwork(player)
	return player and player.isUsingOtClient and player:isUsingOtClient()
end

local function wheelKV(player)
	return player:kv():scoped("wheel")
end

local function clampU16(value)
	value = math.floor(tonumber(value) or 0)
	if value < 0 then
		return 0
	end
	if value > 0xFFFF then
		return 0xFFFF
	end
	return value
end

local function getWheelVocation(player)
	local vocation = player:getVocation()
	local clientId = vocation and vocation:getClientId() or 0
	if clientId == 1 or clientId == 11 then
		return 1
	elseif clientId == 2 or clientId == 12 then
		return 2
	elseif clientId == 3 or clientId == 13 then
		return 3
	elseif clientId == 4 or clientId == 14 then
		return 4
	elseif clientId == 5 or clientId == 15 then
		return 5
	end
	return 0
end

local function getWheelPoints(player)
	return clampU16(math.max(0, (player:getLevel() - (WHEEL_MIN_LEVEL - 1)) * WHEEL_POINTS_PER_LEVEL))
end

local function hasWheelPremium(player)
	return not player.isPremium or player:isPremium()
end

local function isWheelPromoted(player)
	return not player.isPromoted or player:isPromoted()
end

local function canOpenWheel(player)
	return getWheelVocation(player) > 0 and player:getLevel() >= WHEEL_MIN_LEVEL and hasWheelPremium(player) and
	       isWheelPromoted(player)
end

local function emptyPoints()
	local points = {}
	for slot = 1, WHEEL_SLOT_COUNT do
		points[slot] = 0
	end
	return points
end

local function emptyGems()
	return { WHEEL_NO_GEM, WHEEL_NO_GEM, WHEEL_NO_GEM, WHEEL_NO_GEM }
end

local function normalizePointTable(points)
	local normalized = emptyPoints()
	if type(points) ~= "table" then
		return normalized
	end

	for slot = 1, WHEEL_SLOT_COUNT do
		normalized[slot] = clampU16(points[slot])
	end
	return normalized
end

local function normalizeGemTable(gems)
	local normalized = emptyGems()
	if type(gems) ~= "table" then
		return normalized
	end

	for index = 1, 4 do
		normalized[index] = clampU16(gems[index])
	end
	return normalized
end

local function calculateDomainPoints(points)
	local domains = { 0, 0, 0, 0 }
	for slot = 1, WHEEL_SLOT_COUNT do
		local domain = WHEEL_SLOT_DOMAINS[slot]
		domains[domain] = domains[domain] + (points[slot] or 0)
	end
	return domains
end

local function getStage(points)
	if points >= 1000 then
		return 3
	elseif points >= 500 then
		return 2
	elseif points >= 250 then
		return 1
	end
	return 0
end

local function buildRevelationStages(domainPoints)
	local domain1 = getStage(domainPoints[1] or 0)
	local domain2 = getStage(domainPoints[2] or 0)
	local domain3 = getStage(domainPoints[3] or 0)
	local domain4 = getStage(domainPoints[4] or 0)

	return {
		["Gift of Life"] = domain1,
		["Executioner's Throw"] = domain2,
		["Divine Grenade"] = domain2,
		["Beam Mastery"] = domain2,
		["Blessing of the Grove"] = domain2,
		["Spiritual Outburst"] = domain2,
		["Combat Mastery"] = domain3,
		["Divine Empowerment"] = domain3,
		["Drain Body"] = domain3,
		["Twin Bursts"] = domain3,
		["Ascetic"] = domain3,
		["Avatar of Steel"] = domain4,
		["Avatar of Light"] = domain4,
		["Avatar of Storm"] = domain4,
		["Avatar of Nature"] = domain4,
		["Avatar of Balance"] = domain4,
	}
end

local function loadProfile(player)
	local store = wheelKV(player)
	return {
		points = normalizePointTable(store:get("points")),
		gems = normalizeGemTable(store:get("gems")),
	}
end

local function saveProfile(player, points, gems)
	local domainPoints = calculateDomainPoints(points)
	local stages = buildRevelationStages(domainPoints)
	local usedPoints = 0
	for slot = 1, WHEEL_SLOT_COUNT do
		usedPoints = usedPoints + (points[slot] or 0)
	end

	local store = wheelKV(player)
	store:set("version", 1)
	store:set("points", points)
	store:set("gems", gems)
	store:set("domainPoints", domainPoints)
	store:set("revelationStages", stages)
	store:set("usedPoints", usedPoints)
	store:set("vocation", getWheelVocation(player))
	store:set("savedAt", os.time())
end

local function validatePoints(player, points)
	local total = 0
	for slot = 1, WHEEL_SLOT_COUNT do
		local value = points[slot] or 0
		if value > WHEEL_SLOT_MAX_POINTS[slot] then
			return false, "Invalid wheel slot points."
		end
		total = total + value
	end

	if total > getWheelPoints(player) then
		return false, "Not enough promotion points."
	end

	for slot = 1, WHEEL_SLOT_COUNT do
		local value = points[slot] or 0
		if value > 0 and WHEEL_SLOT_MAX_POINTS[slot] ~= 50 then
			local prerequisites = WHEEL_SLOT_PREREQUISITES[slot]
			if prerequisites and #prerequisites > 0 then
				local unlocked = false
				for _, prerequisite in ipairs(prerequisites) do
					if (points[prerequisite] or 0) >= WHEEL_SLOT_MAX_POINTS[prerequisite] then
						unlocked = true
						break
					end
				end
				if not unlocked then
					return false, "Wheel path is not connected."
				end
			end
		end
	end

	return true
end

local function sendResourceBalance(player, resourceType, value)
	if not supportsCustomNetwork(player) then
		return false
	end

	local out = NetworkMessage(player)
	out:addByte(OPCODE_RESOURCE_BALANCE)
	out:addByte(resourceType)
	out:addU64(math.max(0, tonumber(value) or 0))
	return out:sendToPlayer(player)
end

local function sendWheelResources(player, vocationId)
	local gemItems = GEM_ITEMS[vocationId] or {}
	sendResourceBalance(player, RESOURCE_BANK, player:getBankBalance())
	sendResourceBalance(player, RESOURCE_INVENTORY, player:getMoney())
	sendResourceBalance(player, RESOURCE_LESSER_GEMS, gemItems[1] and player:getItemCount(gemItems[1]) or 0)
	sendResourceBalance(player, RESOURCE_REGULAR_GEMS, gemItems[2] and player:getItemCount(gemItems[2]) or 0)
	sendResourceBalance(player, RESOURCE_GREATER_GEMS, gemItems[3] and player:getItemCount(gemItems[3]) or 0)
	sendResourceBalance(player, RESOURCE_LESSER_FRAGMENTS, player:getItemCount(ITEM_LESSER_FRAGMENT))
	sendResourceBalance(player, RESOURCE_GREATER_FRAGMENTS, player:getItemCount(ITEM_GREATER_FRAGMENT))
end

local function sendWheelWindow(player, ownerId)
	if not supportsCustomNetwork(player) then
		return false
	end

	ownerId = tonumber(ownerId) or player:getId()
	local vocationId = getWheelVocation(player)
	local canView = canOpenWheel(player)
	sendWheelResources(player, vocationId)

	local out = NetworkMessage(player)
	out:addByte(OPCODE_WHEEL_WINDOW)
	out:addU32(ownerId)
	out:addByte(canView and 1 or 0)
	if not canView then
		return out:sendToPlayer(player)
	end

	local profile = loadProfile(player)
	local canEdit = ownerId == player:getId()
	out:addByte(canEdit and 1 or 0)
	out:addByte(vocationId)
	out:addU16(getWheelPoints(player))
	out:addU16(0) -- extra points from gems/scrolls are not generated by this 8.60 adapter.

	for slot = 1, WHEEL_SLOT_COUNT do
		out:addU16(profile.points[slot] or 0)
	end

	out:addU16(0) -- promotion scroll count
	out:addByte(0) -- active gem count
	out:addU16(0) -- revealed gem count
	out:addByte(0) -- basic upgrade count
	out:addByte(0) -- supreme upgrade count

	return out:sendToPlayer(player)
end

local function readSaveGems(msg)
	local gems = emptyGems()
	for index = 1, 4 do
		if msg:len() - msg:tell() < 1 then
			return gems
		end

		local hasGem = msg:getByte() ~= 0
		if hasGem then
			if msg:len() - msg:tell() < 2 then
				return nil
			end
			gems[index] = msg:getU16()
		end
	end
	return gems
end

local openHandler = PacketHandler(OPCODE_WHEEL_OPEN)

function openHandler.onReceive(player, msg)
	if msg:len() - msg:tell() < 4 then
		return
	end

	sendWheelWindow(player, msg:getU32())
end

openHandler:register()

local saveHandler = PacketHandler(OPCODE_WHEEL_SAVE)

function saveHandler.onReceive(player, msg)
	if msg:len() - msg:tell() < WHEEL_SLOT_COUNT * 2 then
		return
	end

	if not canOpenWheel(player) then
		sendWheelWindow(player, player:getId())
		return
	end

	local points = {}
	for slot = 1, WHEEL_SLOT_COUNT do
		points[slot] = msg:getU16()
	end

	local gems = readSaveGems(msg)
	if not gems then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, "Invalid wheel packet.")
		sendWheelWindow(player, player:getId())
		return
	end

	local valid, reason = validatePoints(player, points)
	if not valid then
		player:sendTextMessage(MESSAGE_STATUS_SMALL, reason)
		sendWheelWindow(player, player:getId())
		return
	end

	saveProfile(player, points, gems)
	player:reloadData()
	sendWheelWindow(player, player:getId())
end

saveHandler:register()

local gemActionHandler = PacketHandler(OPCODE_WHEEL_GEM_ACTION)

function gemActionHandler.onReceive(player, msg)
	if msg:len() - msg:tell() < 2 then
		return
	end

	msg:getByte() -- action type
	msg:getByte() -- parameter
	if msg:len() - msg:tell() >= 1 then
		msg:getByte() -- optional position for grade improvement
	end

	sendWheelWindow(player, player:getId())
end

gemActionHandler:register()
