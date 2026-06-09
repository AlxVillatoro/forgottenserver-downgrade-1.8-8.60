local talk = TalkAction("/condition")

function talk.onSay(player, words, param)
	if not player:getGroup():hasFlag(PlayerFlag_CanAnswerRuleViolations) then
		return false
	end

	if param == "" then
		player:sendTextMessage(MESSAGE_ADMIN, "Usage: /condition root|fear|agony|bleed, seconds, [playername]")
		return false
	end

	local parts = {}
	for part in string.gmatch(param, "[^,]+") do
		parts[#parts + 1] = part:trim()
	end

	if #parts < 2 then
		player:sendTextMessage(MESSAGE_ADMIN, "Usage: /condition root|fear|agony|bleed, seconds, [playername]")
		return false
	end

	local condName = parts[1]:lower()
	local seconds = tonumber(parts[2]) or 10
	local targetName = parts[3]
	local target = targetName and Player(targetName) or player

	if not target then
		player:sendTextMessage(MESSAGE_ADMIN, "Player not found.")
		return false
	end

	local conditionType
	if condName == "root" or condName == "rooted" then
		conditionType = CONDITION_ROOTED
	elseif condName == "fear" or condName == "feared" then
		conditionType = CONDITION_FEARED
	elseif condName == "agony" then
		conditionType = CONDITION_AGONY
	elseif condName == "bleed" or condName == "bleeding" then
		conditionType = CONDITION_BLEEDING
	else
		player:sendTextMessage(MESSAGE_ADMIN, "Unknown condition: " .. condName)
		return false
	end

	local ticks = seconds * 1000
	local condition = Condition(conditionType, CONDITIONID_DEFAULT, ticks)
	target:addCondition(condition)
	player:sendTextMessage(MESSAGE_ADMIN, "Applied " .. condName .. " to " .. target:getName() .. " for " .. seconds .. "s")
	return false
end

talk:separator(" ")
talk:register()
