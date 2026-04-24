local handler = PacketHandler(0xA0)

function handler.onReceive(player, msg)
	if msg:len() - msg:tell() < 3 then
		return
	end

	local stanceRaw = msg:getByte() -- 1 - offensive, 2 - balanced, 3 - defensive
	local chaseModeRaw = msg:getByte() -- 0 - stand while fighting, 1 - chase opponent
	local secureModeRaw = msg:getByte() -- 0 - cannot attack unmarked, 1 - can attack unmarked
	local secureMode = secureModeRaw ~= 0

	local stance = FIGHTMODE_DEFENSE
	if stanceRaw == 1 then
		stance = FIGHTMODE_ATTACK
	elseif stanceRaw == 2 then
		stance = FIGHTMODE_BALANCED
	end

	local pvpMode = player:getPvpMode()
	if msg:len() - msg:tell() >= 1 then
		local pvpModeRaw = msg:getByte()
		if pvpModeRaw == PVP_MODE_WHITE_HAND or pvpModeRaw == PVP_MODE_YELLOW_HAND or pvpModeRaw == PVP_MODE_RED_FIST then
			pvpMode = pvpModeRaw
		else
			pvpMode = PVP_MODE_DOVE
		end
	else
		pvpMode = secureMode and PVP_MODE_DOVE or PVP_MODE_RED_FIST
	end

	player:setPvpMode(pvpMode)
	player:setFightMode(stance, chaseModeRaw ~= 0, secureMode)
end

handler:register()
