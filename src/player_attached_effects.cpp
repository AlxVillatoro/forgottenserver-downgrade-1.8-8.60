// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "player_attached_effects.h"

#include "attached_effects.h"
#include "const.h"
#include "game.h"
#include "kv/kv.h"
#include "player.h"
#include "protocolgame.h"

extern Game g_game;

namespace {
template <typename T, typename HasFn>
uint16_t randomAttachedId(const std::vector<std::shared_ptr<T>>& values, HasFn has)
{
	std::vector<uint16_t> ids;
	for (const auto& value : values) {
		if (has(value)) {
			ids.emplace_back(value->id);
		}
	}

	if (ids.empty()) {
		return 0;
	}
	return ids[uniform_random(0, static_cast<int32_t>(ids.size() - 1))];
}

bool hasBitStorage(const Player& player, uint32_t baseKey, uint16_t id)
{
	if (id == 0) {
		return false;
	}
	const uint16_t tmpId = id - 1;
	const int64_t value = player.getStorageValue(baseKey + (tmpId / 31)).value_or(-1);
	return value != -1 && ((int64_t{1} << (tmpId % 31)) & value) != 0;
}

void setBitStorage(Player& player, uint32_t baseKey, uint16_t id, bool enabled)
{
	if (id == 0) {
		return;
	}
	const uint16_t tmpId = id - 1;
	const uint32_t key = baseKey + (tmpId / 31);
	int64_t value = player.getStorageValue(key).value_or(-1);
	if (value == -1) {
		value = 0;
	}
	if (enabled) {
		value |= (int64_t{1} << (tmpId % 31));
	} else {
		value &= ~(int64_t{1} << (tmpId % 31));
	}
	player.setStorageValue(key, value);
}
}

PlayerAttachedEffects::PlayerAttachedEffects(Player& initPlayer) : defaultOutfit(initPlayer.getDefaultOutfit()), player(initPlayer) {}

std::shared_ptr<KV> PlayerAttachedEffects::playerKV() const
{
	return KVStore::getInstance().scoped("player")->scoped(fmt::format("{}", player.getGUID()));
}

uint16_t PlayerAttachedEffects::getLastFromStorageOrKV(uint32_t storageKey, const std::string& kvKey) const
{
	const int64_t storageValue = player.getStorageValue(storageKey).value_or(-1);
	if (storageValue > 0) {
		return static_cast<uint16_t>(storageValue);
	}

	const auto value = playerKV()->get(kvKey);
	if (!value.has_value()) {
		return 0;
	}
	return static_cast<uint16_t>(value->get<int32_t>());
}

void PlayerAttachedEffects::setStorage(uint32_t key, int64_t value) { player.setStorageValue(key, value); }

uint16_t PlayerAttachedEffects::getLastWing() const { return getLastFromStorageOrKV(PSTRG_WING_CURRENTWING, "last-wing"); }
uint16_t PlayerAttachedEffects::getCurrentWing() const { return static_cast<uint16_t>(player.getStorageValue(PSTRG_WING_CURRENTWING).value_or(0)); }
void PlayerAttachedEffects::setCurrentWing(uint16_t wingId) { setStorage(PSTRG_WING_CURRENTWING, wingId); }
bool PlayerAttachedEffects::isWinged() const { return defaultOutfit.lookWing != 0; }
bool PlayerAttachedEffects::hasWing(const std::shared_ptr<Wing>& wing) const { return wing && (player.isAccessPlayer() || hasBitStorage(player, PSTRG_WING_RANGE_START, wing->id)); }
bool PlayerAttachedEffects::hasAnyWing() const { return std::ranges::any_of(g_game.getAttachedEffects().getWings(), [&](const auto& wing) { return hasWing(wing); }); }
uint16_t PlayerAttachedEffects::getRandomWingId() const { return randomAttachedId(g_game.getAttachedEffects().getWings(), [&](const auto& wing) { return hasWing(wing); }); }

bool PlayerAttachedEffects::toggleWing(bool wing)
{
	if ((OTSYS_TIME() - lastToggleWing) < 3000 && !wasWinged) {
		player.sendCancelMessage(RETURNVALUE_YOUAREEXHAUSTED);
		return false;
	}

	if (wing) {
		if (isWinged()) {
			return false;
		}
		uint16_t currentWingId = getLastWing();
		if (currentWingId == 0) {
			player.sendOutfitWindow();
			return false;
		}
		if (player.isRandomMounted()) {
			currentWingId = getRandomWingId();
		}
		const auto currentWing = g_game.getAttachedEffects().getWingByID(currentWingId);
		if (!currentWing || !hasWing(currentWing) || player.hasCondition(CONDITION_OUTFIT)) {
			return false;
		}
		defaultOutfit.lookWing = currentWing->id;
		player.setDefaultOutfit(defaultOutfit);
		setCurrentWing(currentWing->id);
		playerKV()->set("last-wing", static_cast<int32_t>(currentWing->id));
	} else {
		if (!isWinged()) {
			return false;
		}
		diswing();
	}

	g_game.internalCreatureChangeOutfit(&player, defaultOutfit);
	lastToggleWing = OTSYS_TIME();
	return true;
}

bool PlayerAttachedEffects::tameWing(uint16_t wingId)
{
	if (!g_game.getAttachedEffects().getWingByID(wingId)) {
		return false;
	}
	setBitStorage(player, PSTRG_WING_RANGE_START, wingId, true);
	return true;
}

bool PlayerAttachedEffects::untameWing(uint16_t wingId)
{
	if (!g_game.getAttachedEffects().getWingByID(wingId)) {
		return false;
	}
	setBitStorage(player, PSTRG_WING_RANGE_START, wingId, false);
	if (getCurrentWing() == wingId) {
		diswing();
		g_game.internalCreatureChangeOutfit(&player, defaultOutfit);
		setCurrentWing(0);
		playerKV()->set("last-wing", 0);
	}
	return true;
}

void PlayerAttachedEffects::diswing()
{
	defaultOutfit.lookWing = 0;
	player.setDefaultOutfit(defaultOutfit);
}

uint16_t PlayerAttachedEffects::getLastAura() const { return getLastFromStorageOrKV(PSTRG_AURA_CURRENTAURA, "last-aura"); }
uint16_t PlayerAttachedEffects::getCurrentAura() const { return static_cast<uint16_t>(player.getStorageValue(PSTRG_AURA_CURRENTAURA).value_or(0)); }
void PlayerAttachedEffects::setCurrentAura(uint16_t auraId) { setStorage(PSTRG_AURA_CURRENTAURA, auraId); }
bool PlayerAttachedEffects::hasAura(const std::shared_ptr<Aura>& aura) const { return aura && (player.isAccessPlayer() || hasBitStorage(player, PSTRG_AURA_RANGE_START, aura->id)); }
bool PlayerAttachedEffects::hasAnyAura() const { return std::ranges::any_of(g_game.getAttachedEffects().getAuras(), [&](const auto& aura) { return hasAura(aura); }); }
uint16_t PlayerAttachedEffects::getRandomAuraId() const { return randomAttachedId(g_game.getAttachedEffects().getAuras(), [&](const auto& aura) { return hasAura(aura); }); }

bool PlayerAttachedEffects::toggleAura(bool aura)
{
	if ((OTSYS_TIME() - lastToggleAura) < 3000 && !wasAuraed) {
		player.sendCancelMessage(RETURNVALUE_YOUAREEXHAUSTED);
		return false;
	}

	if (aura) {
		if (isAuraed()) {
			return false;
		}
		uint16_t currentAuraId = getLastAura();
		if (currentAuraId == 0) {
			player.sendOutfitWindow();
			return false;
		}
		if (player.isRandomMounted()) {
			currentAuraId = getRandomAuraId();
		}
		const auto currentAura = g_game.getAttachedEffects().getAuraByID(currentAuraId);
		if (!currentAura || !hasAura(currentAura) || player.hasCondition(CONDITION_OUTFIT)) {
			return false;
		}
		defaultOutfit.lookAura = currentAura->id;
		player.setDefaultOutfit(defaultOutfit);
		setCurrentAura(currentAura->id);
		playerKV()->set("last-aura", static_cast<int32_t>(currentAura->id));
	} else {
		if (!isAuraed()) {
			return false;
		}
		disaura();
	}

	g_game.internalCreatureChangeOutfit(&player, defaultOutfit);
	lastToggleAura = OTSYS_TIME();
	return true;
}

bool PlayerAttachedEffects::tameAura(uint16_t auraId)
{
	if (!g_game.getAttachedEffects().getAuraByID(auraId)) {
		return false;
	}
	setBitStorage(player, PSTRG_AURA_RANGE_START, auraId, true);
	return true;
}

bool PlayerAttachedEffects::untameAura(uint16_t auraId)
{
	if (!g_game.getAttachedEffects().getAuraByID(auraId)) {
		return false;
	}
	setBitStorage(player, PSTRG_AURA_RANGE_START, auraId, false);
	if (getCurrentAura() == auraId) {
		disaura();
		g_game.internalCreatureChangeOutfit(&player, defaultOutfit);
		setCurrentAura(0);
		playerKV()->set("last-aura", 0);
	}
	return true;
}

void PlayerAttachedEffects::disaura()
{
	defaultOutfit.lookAura = 0;
	player.setDefaultOutfit(defaultOutfit);
}

uint16_t PlayerAttachedEffects::getLastEffect() const { return getLastFromStorageOrKV(PSTRG_EFFECT_CURRENTEFFECT, "last-effect"); }
uint16_t PlayerAttachedEffects::getCurrentEffect() const { return static_cast<uint16_t>(player.getStorageValue(PSTRG_EFFECT_CURRENTEFFECT).value_or(0)); }
void PlayerAttachedEffects::setCurrentEffect(uint16_t effectId) { setStorage(PSTRG_EFFECT_CURRENTEFFECT, effectId); }
bool PlayerAttachedEffects::hasEffect(const std::shared_ptr<Effect>& effect) const { return effect && (player.isAccessPlayer() || hasBitStorage(player, PSTRG_EFFECT_RANGE_START, effect->id)); }
bool PlayerAttachedEffects::hasAnyEffect() const { return std::ranges::any_of(g_game.getAttachedEffects().getEffects(), [&](const auto& effect) { return hasEffect(effect); }); }
uint16_t PlayerAttachedEffects::getRandomEffectId() const { return randomAttachedId(g_game.getAttachedEffects().getEffects(), [&](const auto& effect) { return hasEffect(effect); }); }

bool PlayerAttachedEffects::toggleEffect(bool effect)
{
	if ((OTSYS_TIME() - lastToggleEffect) < 3000 && !wasEffected) {
		player.sendCancelMessage(RETURNVALUE_YOUAREEXHAUSTED);
		return false;
	}

	if (effect) {
		if (isEffected()) {
			return false;
		}
		uint16_t currentEffectId = getLastEffect();
		if (currentEffectId == 0) {
			player.sendOutfitWindow();
			return false;
		}
		if (player.isRandomMounted()) {
			currentEffectId = getRandomEffectId();
		}
		const auto currentEffect = g_game.getAttachedEffects().getEffectByID(currentEffectId);
		if (!currentEffect || !hasEffect(currentEffect) || player.hasCondition(CONDITION_OUTFIT)) {
			return false;
		}
		defaultOutfit.lookEffect = currentEffect->id;
		player.setDefaultOutfit(defaultOutfit);
		setCurrentEffect(currentEffect->id);
		playerKV()->set("last-effect", static_cast<int32_t>(currentEffect->id));
	} else {
		if (!isEffected()) {
			return false;
		}
		diseffect();
	}

	g_game.internalCreatureChangeOutfit(&player, defaultOutfit);
	lastToggleEffect = OTSYS_TIME();
	return true;
}

bool PlayerAttachedEffects::tameEffect(uint16_t effectId)
{
	if (!g_game.getAttachedEffects().getEffectByID(effectId)) {
		return false;
	}
	setBitStorage(player, PSTRG_EFFECT_RANGE_START, effectId, true);
	return true;
}

bool PlayerAttachedEffects::untameEffect(uint16_t effectId)
{
	if (!g_game.getAttachedEffects().getEffectByID(effectId)) {
		return false;
	}
	setBitStorage(player, PSTRG_EFFECT_RANGE_START, effectId, false);
	if (getCurrentEffect() == effectId) {
		diseffect();
		g_game.internalCreatureChangeOutfit(&player, defaultOutfit);
		setCurrentEffect(0);
		playerKV()->set("last-effect", 0);
	}
	return true;
}

void PlayerAttachedEffects::diseffect()
{
	defaultOutfit.lookEffect = 0;
	player.setDefaultOutfit(defaultOutfit);
}

uint16_t PlayerAttachedEffects::getRandomShader() const
{
	return randomAttachedId(g_game.getAttachedEffects().getShaders(), [&](const auto& shader) { return hasShader(shader.get()); });
}

uint16_t PlayerAttachedEffects::getCurrentShader() const
{
	if (currentShader != 0) {
		return currentShader;
	}
	return static_cast<uint16_t>(player.getStorageValue(PSTRG_SHADER_CURRENTSHADER).value_or(0));
}
void PlayerAttachedEffects::setCurrentShader(uint16_t shaderId)
{
	currentShader = shaderId;
	setStorage(PSTRG_SHADER_CURRENTSHADER, shaderId);
}
bool PlayerAttachedEffects::hasShader(const Shader* shader) const { return shader && (player.isAccessPlayer() || hasBitStorage(player, PSTRG_SHADER_RANGE_START, shader->id)); }
bool PlayerAttachedEffects::hasShaders() const { return std::ranges::any_of(g_game.getAttachedEffects().getShaders(), [&](const auto& shader) { return hasShader(shader.get()); }); }

bool PlayerAttachedEffects::toggleShader(bool shader)
{
	if ((OTSYS_TIME() - lastToggleShader) < 3000 && !wasShadered) {
		player.sendCancelMessage(RETURNVALUE_YOUAREEXHAUSTED);
		return false;
	}

	if (shader) {
		if (isShadered()) {
			return false;
		}
		uint16_t currentShaderId = getCurrentShader();
		if (currentShaderId == 0) {
			player.sendOutfitWindow();
			return false;
		}
		const auto currentShader = g_game.getAttachedEffects().getShaderByID(currentShaderId);
		if (!currentShader || !hasShader(currentShader.get()) || player.hasCondition(CONDITION_OUTFIT)) {
			return false;
		}
		defaultOutfit.lookShader = currentShader->id;
		player.setDefaultOutfit(defaultOutfit);
	} else {
		if (!isShadered()) {
			return false;
		}
		disshader();
	}

	g_game.internalCreatureChangeOutfit(&player, defaultOutfit);
	lastToggleShader = OTSYS_TIME();
	return true;
}

bool PlayerAttachedEffects::tameShader(uint16_t shaderId)
{
	if (!g_game.getAttachedEffects().getShaderByID(shaderId) || hasBitStorage(player, PSTRG_SHADER_RANGE_START, shaderId)) {
		return false;
	}
	setBitStorage(player, PSTRG_SHADER_RANGE_START, shaderId, true);
	return true;
}

bool PlayerAttachedEffects::untameShader(uint16_t shaderId)
{
	if (!g_game.getAttachedEffects().getShaderByID(shaderId) || !hasBitStorage(player, PSTRG_SHADER_RANGE_START, shaderId)) {
		return false;
	}
	setBitStorage(player, PSTRG_SHADER_RANGE_START, shaderId, false);
	if (getCurrentShader() == shaderId) {
		disshader();
		g_game.internalCreatureChangeOutfit(&player, defaultOutfit);
		setCurrentShader(0);
	}
	return true;
}

void PlayerAttachedEffects::disshader()
{
	defaultOutfit.lookShader = 0;
	player.setDefaultOutfit(defaultOutfit);
}

std::string PlayerAttachedEffects::getCurrentShaderName() const
{
	const auto shader = g_game.getAttachedEffects().getShaderByID(getCurrentShader());
	return shader ? shader->name : "Outfit - Default";
}

bool PlayerAttachedEffects::addCustomOutfit(const std::string& type, const std::variant<uint16_t, std::string>& idOrName)
{
	uint16_t id = 0;
	if (std::holds_alternative<uint16_t>(idOrName)) {
		id = std::get<uint16_t>(idOrName);
	} else {
		const auto& name = std::get<std::string>(idOrName);
		if (type == "wing") {
			const auto value = g_game.getAttachedEffects().getWingByName(name);
			id = value ? value->id : 0;
		} else if (type == "aura") {
			const auto value = g_game.getAttachedEffects().getAuraByName(name);
			id = value ? value->id : 0;
		} else if (type == "effect") {
			const auto value = g_game.getAttachedEffects().getEffectByName(name);
			id = value ? value->id : 0;
		} else if (type == "shader") {
			const auto value = g_game.getAttachedEffects().getShaderByName(name);
			id = value ? value->id : 0;
		}
	}

	if (type == "wing") return tameWing(id);
	if (type == "aura") return tameAura(id);
	if (type == "effect") return tameEffect(id);
	if (type == "shader") return tameShader(id);
	return false;
}

bool PlayerAttachedEffects::removeCustomOutfit(const std::string& type, const std::variant<uint16_t, std::string>& idOrName)
{
	uint16_t id = 0;
	if (std::holds_alternative<uint16_t>(idOrName)) {
		id = std::get<uint16_t>(idOrName);
	} else {
		const auto& name = std::get<std::string>(idOrName);
		if (type == "wing" || type == "wings") {
			const auto value = g_game.getAttachedEffects().getWingByName(name);
			id = value ? value->id : 0;
		} else if (type == "aura") {
			const auto value = g_game.getAttachedEffects().getAuraByName(name);
			id = value ? value->id : 0;
		} else if (type == "effect") {
			const auto value = g_game.getAttachedEffects().getEffectByName(name);
			id = value ? value->id : 0;
		} else if (type == "shader") {
			const auto value = g_game.getAttachedEffects().getShaderByName(name);
			id = value ? value->id : 0;
		}
	}

	if (type == "wing" || type == "wings") return untameWing(id);
	if (type == "aura") return untameAura(id);
	if (type == "effect") return untameEffect(id);
	if (type == "shader") return untameShader(id);
	return false;
}

void PlayerAttachedEffects::applyCurrent()
{
	defaultOutfit = player.getDefaultOutfit();

	const auto wing = g_game.getAttachedEffects().getWingByID(getCurrentWing());
	if (wing && hasWing(wing)) {
		defaultOutfit.lookWing = wing->id;
		player.attachEffectById(wing->id);
	}

	const auto aura = g_game.getAttachedEffects().getAuraByID(getCurrentAura());
	if (aura && hasAura(aura)) {
		defaultOutfit.lookAura = aura->id;
		player.attachEffectById(aura->id);
	}

	const auto effect = g_game.getAttachedEffects().getEffectByID(getCurrentEffect());
	if (effect && hasEffect(effect)) {
		defaultOutfit.lookEffect = effect->id;
		player.attachEffectById(effect->id);
	}

	const auto shader = g_game.getAttachedEffects().getShaderByID(getCurrentShader());
	if (shader && hasShader(shader.get())) {
		defaultOutfit.lookShader = shader->id;
		player.setShader(shader->name);
		sendShader(&player, shader->name);
	}

	player.setDefaultOutfit(defaultOutfit);
}

void PlayerAttachedEffects::sendAttachedEffect(const Creature* creature, uint16_t effectId) const
{
	if (player.client && creature) {
		player.client->sendAttachedEffect(creature, effectId);
	}
}

void PlayerAttachedEffects::sendDetachEffect(const Creature* creature, uint16_t effectId) const
{
	if (player.client && creature) {
		player.client->sendDetachEffect(creature, effectId);
	}
}

void PlayerAttachedEffects::sendShader(const Creature* creature, const std::string& shaderName) const
{
	if (player.client && creature) {
		player.client->sendShader(creature, shaderName);
	}
}

void PlayerAttachedEffects::sendMapShader(const std::string& shaderName) const
{
	if (player.client) {
		player.client->sendMapShader(shaderName);
	}
}
