// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_PLAYER_ATTACHED_EFFECTS_H
#define FS_PLAYER_ATTACHED_EFFECTS_H

#include "enums.h"

#include <memory>
#include <string>
#include <variant>

class Creature;
class Player;
struct Aura;
struct Effect;
struct Shader;
struct Wing;

class PlayerAttachedEffects
{
public:
	explicit PlayerAttachedEffects(Player& player);

	uint16_t getLastWing() const;
	uint16_t getCurrentWing() const;
	void setCurrentWing(uint16_t wingId);
	bool isWinged() const;
	bool toggleWing(bool wing);
	bool tameWing(uint16_t wingId);
	bool untameWing(uint16_t wingId);
	bool hasWing(const std::shared_ptr<Wing>& wing) const;
	bool hasAnyWing() const;
	uint16_t getRandomWingId() const;
	void diswing();

	uint16_t getLastAura() const;
	uint16_t getCurrentAura() const;
	void setCurrentAura(uint16_t auraId);
	bool isAuraed() const { return defaultOutfit.lookAura != 0; }
	bool toggleAura(bool aura);
	bool tameAura(uint16_t auraId);
	bool untameAura(uint16_t auraId);
	bool hasAura(const std::shared_ptr<Aura>& aura) const;
	bool hasAnyAura() const;
	uint16_t getRandomAuraId() const;
	void disaura();

	uint16_t getLastEffect() const;
	uint16_t getCurrentEffect() const;
	void setCurrentEffect(uint16_t effectId);
	bool isEffected() const { return defaultOutfit.lookEffect != 0; }
	bool toggleEffect(bool effect);
	bool tameEffect(uint16_t effectId);
	bool untameEffect(uint16_t effectId);
	bool hasEffect(const std::shared_ptr<Effect>& effect) const;
	bool hasAnyEffect() const;
	uint16_t getRandomEffectId() const;
	void diseffect();

	uint16_t getRandomShader() const;
	uint16_t getCurrentShader() const;
	void setCurrentShader(uint16_t shaderId);
	bool isShadered() const { return defaultOutfit.lookShader != 0; }
	bool toggleShader(bool shader);
	bool tameShader(uint16_t shaderId);
	bool untameShader(uint16_t shaderId);
	bool hasShader(const Shader* shader) const;
	bool hasShaders() const;
	void disshader();
	std::string getCurrentShaderName() const;

	bool addCustomOutfit(const std::string& type, const std::variant<uint16_t, std::string>& idOrName);
	bool removeCustomOutfit(const std::string& type, const std::variant<uint16_t, std::string>& idOrName);
	void applyCurrent();

	void sendAttachedEffect(const Creature* creature, uint16_t effectId) const;
	void sendDetachEffect(const Creature* creature, uint16_t effectId) const;
	void sendShader(const Creature* creature, const std::string& shaderName) const;
	void sendMapShader(const std::string& shaderName) const;

	const std::string& getMapShader() const { return mapShader; }
	void setMapShader(std::string_view shaderName) { mapShader = shaderName; }

	void setWasWinged(bool value) { wasWinged = value; }
	void setWasAuraed(bool value) { wasAuraed = value; }
	void setWasEffected(bool value) { wasEffected = value; }
	void setWasShadered(bool value) { wasShadered = value; }

private:
	std::shared_ptr<class KV> playerKV() const;
	uint16_t getLastFromStorageOrKV(uint32_t storageKey, const std::string& kvKey) const;
	void setStorage(uint32_t key, int64_t value);

	std::string mapShader;

	int64_t lastToggleWing = 0;
	int64_t lastToggleAura = 0;
	int64_t lastToggleEffect = 0;
	int64_t lastToggleShader = 0;

	uint16_t currentShader = 0;

	bool wasWinged = false;
	bool wasAuraed = false;
	bool wasEffected = false;
	bool wasShadered = false;

	Outfit_t defaultOutfit;
	Player& player;
};

#endif
