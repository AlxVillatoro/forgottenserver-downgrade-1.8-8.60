// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#ifndef FS_ATTACHED_EFFECTS_H
#define FS_ATTACHED_EFFECTS_H

#include <memory>
#include <string>
#include <vector>

struct Aura
{
	Aura(uint16_t initId, std::string initName) : id(initId), name(std::move(initName)) {}

	uint16_t id = 0;
	std::string name;
};

struct Effect
{
	Effect(uint16_t initId, std::string initName) : id(initId), name(std::move(initName)) {}

	uint16_t id = 0;
	std::string name;
};

struct Shader
{
	Shader(uint16_t initId, std::string initName) : id(initId), name(std::move(initName)) {}

	uint16_t id = 0;
	std::string name;
};

struct Wing
{
	Wing(uint16_t initId, std::string initName) : id(initId), name(std::move(initName)) {}

	uint16_t id = 0;
	std::string name;
};

class AttachedEffects
{
public:
	bool reload();
	bool loadFromXml();

	std::shared_ptr<Aura> getAuraByID(uint16_t id) const;
	std::shared_ptr<Effect> getEffectByID(uint16_t id) const;
	std::shared_ptr<Shader> getShaderByID(uint16_t id) const;
	std::shared_ptr<Wing> getWingByID(uint16_t id) const;

	std::shared_ptr<Aura> getAuraByName(std::string_view name) const;
	std::shared_ptr<Effect> getEffectByName(std::string_view name) const;
	std::shared_ptr<Shader> getShaderByName(std::string_view name) const;
	std::shared_ptr<Wing> getWingByName(std::string_view name) const;

	const std::vector<std::shared_ptr<Aura>>& getAuras() const { return auras; }
	const std::vector<std::shared_ptr<Effect>>& getEffects() const { return effects; }
	const std::vector<std::shared_ptr<Shader>>& getShaders() const { return shaders; }
	const std::vector<std::shared_ptr<Wing>>& getWings() const { return wings; }

private:
	std::vector<std::shared_ptr<Aura>> auras;
	std::vector<std::shared_ptr<Effect>> effects;
	std::vector<std::shared_ptr<Shader>> shaders;
	std::vector<std::shared_ptr<Wing>> wings;
};

#endif
