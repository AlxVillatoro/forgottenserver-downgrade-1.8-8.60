// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "attached_effects.h"

#include "pugicast.h"
#include "tools.h"

namespace {
template <typename T>
std::shared_ptr<T> findById(const std::vector<std::shared_ptr<T>>& values, uint16_t id)
{
	const auto it = std::ranges::find_if(values, [id](const auto& value) { return value->id == id; });
	return it != values.end() ? *it : nullptr;
}

template <typename T>
std::shared_ptr<T> findByName(const std::vector<std::shared_ptr<T>>& values, std::string_view name)
{
	const auto it = std::ranges::find_if(values, [name](const auto& value) {
		return caseInsensitiveEqual(name, value->name);
	});
	return it != values.end() ? *it : nullptr;
}
}

bool AttachedEffects::reload()
{
	auras.clear();
	effects.clear();
	shaders.clear();
	wings.clear();
	return loadFromXml();
}

bool AttachedEffects::loadFromXml()
{
	pugi::xml_document doc;
	const std::string file = "data/XML/attachedeffects.xml";
	const pugi::xml_parse_result result = doc.load_file(file.c_str());
	if (!result) {
		printXMLError(__FUNCTION__, file, result);
		return false;
	}

	for (const auto node : doc.child("attachedeffects").children("aura")) {
		auras.emplace_back(std::make_shared<Aura>(pugi::cast<uint16_t>(node.attribute("id").value()),
		                                          node.attribute("name").as_string()));
	}

	for (const auto node : doc.child("attachedeffects").children("effect")) {
		effects.emplace_back(std::make_shared<Effect>(pugi::cast<uint16_t>(node.attribute("id").value()),
		                                              node.attribute("name").as_string()));
	}

	for (const auto node : doc.child("attachedeffects").children("shader")) {
		shaders.emplace_back(std::make_shared<Shader>(pugi::cast<uint16_t>(node.attribute("id").value()),
		                                              node.attribute("name").as_string()));
	}

	for (const auto node : doc.child("attachedeffects").children("wing")) {
		wings.emplace_back(std::make_shared<Wing>(pugi::cast<uint16_t>(node.attribute("id").value()),
		                                          node.attribute("name").as_string()));
	}

	return true;
}

std::shared_ptr<Aura> AttachedEffects::getAuraByID(uint16_t id) const { return findById(auras, id); }
std::shared_ptr<Effect> AttachedEffects::getEffectByID(uint16_t id) const { return findById(effects, id); }
std::shared_ptr<Shader> AttachedEffects::getShaderByID(uint16_t id) const { return findById(shaders, id); }
std::shared_ptr<Wing> AttachedEffects::getWingByID(uint16_t id) const { return findById(wings, id); }

std::shared_ptr<Aura> AttachedEffects::getAuraByName(std::string_view name) const { return findByName(auras, name); }
std::shared_ptr<Effect> AttachedEffects::getEffectByName(std::string_view name) const { return findByName(effects, name); }
std::shared_ptr<Shader> AttachedEffects::getShaderByName(std::string_view name) const { return findByName(shaders, name); }
std::shared_ptr<Wing> AttachedEffects::getWingByName(std::string_view name) const { return findByName(wings, name); }
