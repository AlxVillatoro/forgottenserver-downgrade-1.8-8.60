// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#include "otpch.h"

#include "items.h"

#include "movement.h"
#include "pugicast.h"
#include "script.h"
#include "scriptmanager.h"
#include "spells.h"
#include "weapons.h"
#include "logger.h"
#include <fmt/format.h>
#include <fstream>
#include <cstring>
#include <limits>
#include <type_traits>
#include "configmanager.h"
#include "itemloader.h"

namespace {

const std::unordered_map<std::string, ItemParseAttributes_t> ItemParseAttributesMap = {
    {"type", ITEM_PARSE_TYPE},
    {"description", ITEM_PARSE_DESCRIPTION},
    {"runespellname", ITEM_PARSE_RUNESPELLNAME},
	{"weight", ITEM_PARSE_WEIGHT},
	{"weightreduction", ITEM_PARSE_WEIGHTREDUCTION},
    {"showcount", ITEM_PARSE_SHOWCOUNT},
    {"armor", ITEM_PARSE_ARMOR},
    {"defense", ITEM_PARSE_DEFENSE},
    {"extradef", ITEM_PARSE_EXTRADEF},
    {"attack", ITEM_PARSE_ATTACK},
    {"attackspeed", ITEM_PARSE_ATTACK_SPEED},
    {"classification", ITEM_PARSE_CLASSIFICATION},
    {"tier", ITEM_PARSE_TIER},
    {"rotateto", ITEM_PARSE_ROTATETO},
    {"moveable", ITEM_PARSE_MOVEABLE},
    {"movable", ITEM_PARSE_MOVEABLE},
    {"blockprojectile", ITEM_PARSE_BLOCKPROJECTILE},
    {"ignoreblocking", ITEM_PARSE_IGNOREBLOCKING},
    {"allowpickupable", ITEM_PARSE_PICKUPABLE},
    {"pickupable", ITEM_PARSE_PICKUPABLE},
    {"forceserialize", ITEM_PARSE_FORCESERIALIZE},
    {"forcesave", ITEM_PARSE_FORCESERIALIZE},
    {"floorchange", ITEM_PARSE_FLOORCHANGE},
    {"corpsetype", ITEM_PARSE_CORPSETYPE},
    {"containersize", ITEM_PARSE_CONTAINERSIZE},
    {"fluidsource", ITEM_PARSE_FLUIDSOURCE},
    {"readable", ITEM_PARSE_READABLE},
    {"writeable", ITEM_PARSE_WRITEABLE},
    {"maxtextlen", ITEM_PARSE_MAXTEXTLEN},
    {"writeonceitemid", ITEM_PARSE_WRITEONCEITEMID},
    {"weapontype", ITEM_PARSE_WEAPONTYPE},
    {"slottype", ITEM_PARSE_SLOTTYPE},
    {"ammotype", ITEM_PARSE_AMMOTYPE},
    {"shoottype", ITEM_PARSE_SHOOTTYPE},
    {"effect", ITEM_PARSE_EFFECT},
    {"range", ITEM_PARSE_RANGE},
    {"stopduration", ITEM_PARSE_STOPDURATION},
    {"decayto", ITEM_PARSE_DECAYTO},
    {"transformequipto", ITEM_PARSE_TRANSFORMEQUIPTO},
    {"transformdeequipto", ITEM_PARSE_TRANSFORMDEEQUIPTO},
    {"duration", ITEM_PARSE_DURATION},
    {"showduration", ITEM_PARSE_SHOWDURATION},
    {"charges", ITEM_PARSE_CHARGES},
    {"showcharges", ITEM_PARSE_SHOWCHARGES},
    {"showattributes", ITEM_PARSE_SHOWATTRIBUTES},
    {"hitchance", ITEM_PARSE_HITCHANCE},
    {"maxhitchance", ITEM_PARSE_MAXHITCHANCE},
    {"invisible", ITEM_PARSE_INVISIBLE},
    {"speed", ITEM_PARSE_SPEED},
    {"healthgain", ITEM_PARSE_HEALTHGAIN},
    {"healthgainpercent", ITEM_PARSE_HEALTHGAINPERCENT},
    {"healthticks", ITEM_PARSE_HEALTHTICKS},
    {"managain", ITEM_PARSE_MANAGAIN},
    {"managainpercent", ITEM_PARSE_MANAGAINPERCENT},
    {"manaticks", ITEM_PARSE_MANATICKS},
    {"manashield", ITEM_PARSE_MANASHIELD},
    {"skillsword", ITEM_PARSE_SKILLSWORD},
    {"skillaxe", ITEM_PARSE_SKILLAXE},
    {"skillclub", ITEM_PARSE_SKILLCLUB},
    {"skilldist", ITEM_PARSE_SKILLDIST},
    {"skillfish", ITEM_PARSE_SKILLFISH},
    {"skillshield", ITEM_PARSE_SKILLSHIELD},
    {"skillfist", ITEM_PARSE_SKILLFIST},
    {"maxhitpoints", ITEM_PARSE_MAXHITPOINTS},
    {"maxhitpointspercent", ITEM_PARSE_MAXHITPOINTSPERCENT},
    {"maxmanapoints", ITEM_PARSE_MAXMANAPOINTS},
    {"maxmanapointspercent", ITEM_PARSE_MAXMANAPOINTSPERCENT},
    {"magicpoints", ITEM_PARSE_MAGICPOINTS},
    {"magiclevelpoints", ITEM_PARSE_MAGICPOINTS},
    {"magicpointspercent", ITEM_PARSE_MAGICPOINTSPERCENT},
    {"criticalhitchance", ITEM_PARSE_CRITICALHITCHANCE},
    {"criticalhitamount", ITEM_PARSE_CRITICALHITAMOUNT},
    {"lifeleechchance", ITEM_PARSE_LIFELEECHCHANCE},
    {"lifeleechamount", ITEM_PARSE_LIFELEECHAMOUNT},
    {"manaleechchance", ITEM_PARSE_MANALEECHCHANCE},
    {"manaleechamount", ITEM_PARSE_MANALEECHAMOUNT},
    {"fieldabsorbpercentenergy", ITEM_PARSE_FIELDABSORBPERCENTENERGY},
    {"fieldabsorbpercentfire", ITEM_PARSE_FIELDABSORBPERCENTFIRE},
    {"fieldabsorbpercentpoison", ITEM_PARSE_FIELDABSORBPERCENTPOISON},
    {"fieldabsorbpercentearth", ITEM_PARSE_FIELDABSORBPERCENTPOISON},
    {"absorbpercentall", ITEM_PARSE_ABSORBPERCENTALL},
    {"absorbpercentallelements", ITEM_PARSE_ABSORBPERCENTALL},
    {"absorbpercentelements", ITEM_PARSE_ABSORBPERCENTELEMENTS},
    {"absorbpercentmagic", ITEM_PARSE_ABSORBPERCENTMAGIC},
    {"absorbpercentenergy", ITEM_PARSE_ABSORBPERCENTENERGY},
    {"absorbpercentfire", ITEM_PARSE_ABSORBPERCENTFIRE},
    {"absorbpercentpoison", ITEM_PARSE_ABSORBPERCENTPOISON},
    {"absorbpercentearth", ITEM_PARSE_ABSORBPERCENTPOISON},
    {"absorbpercentice", ITEM_PARSE_ABSORBPERCENTICE},
    {"absorbpercentholy", ITEM_PARSE_ABSORBPERCENTHOLY},
    {"absorbpercentdeath", ITEM_PARSE_ABSORBPERCENTDEATH},
    {"absorbpercentlifedrain", ITEM_PARSE_ABSORBPERCENTLIFEDRAIN},
    {"absorbpercentmanadrain", ITEM_PARSE_ABSORBPERCENTMANADRAIN},
    {"absorbpercentdrown", ITEM_PARSE_ABSORBPERCENTDROWN},
    {"absorbpercentphysical", ITEM_PARSE_ABSORBPERCENTPHYSICAL},
    {"absorbpercenthealing", ITEM_PARSE_ABSORBPERCENTHEALING},
    {"absorbpercentundefined", ITEM_PARSE_ABSORBPERCENTUNDEFINED},
    {"reflectpercentall", ITEM_PARSE_REFLECTPERCENTALL},
    {"reflectpercentallelements", ITEM_PARSE_REFLECTPERCENTALL},
    {"reflectpercentelements", ITEM_PARSE_REFLECTPERCENTELEMENTS},
    {"reflectpercentmagic", ITEM_PARSE_REFLECTPERCENTMAGIC},
    {"reflectpercentenergy", ITEM_PARSE_REFLECTPERCENTENERGY},
    {"reflectpercentfire", ITEM_PARSE_REFLECTPERCENTFIRE},
    {"reflectpercentpoison", ITEM_PARSE_REFLECTPERCENTEARTH},
    {"reflectpercentearth", ITEM_PARSE_REFLECTPERCENTEARTH},
    {"reflectpercentice", ITEM_PARSE_REFLECTPERCENTICE},
    {"reflectpercentholy", ITEM_PARSE_REFLECTPERCENTHOLY},
    {"reflectpercentdeath", ITEM_PARSE_REFLECTPERCENTDEATH},
    {"reflectpercentlifedrain", ITEM_PARSE_REFLECTPERCENTLIFEDRAIN},
    {"reflectpercentmanadrain", ITEM_PARSE_REFLECTPERCENTMANADRAIN},
    {"reflectpercentdrown", ITEM_PARSE_REFLECTPERCENTDROWN},
    {"reflectpercentphysical", ITEM_PARSE_REFLECTPERCENTPHYSICAL},
    {"reflectpercenthealing", ITEM_PARSE_REFLECTPERCENTHEALING},
    {"reflectchanceall", ITEM_PARSE_REFLECTCHANCEALL},
    {"reflectchanceallelements", ITEM_PARSE_REFLECTCHANCEALL},
    {"reflectchanceelements", ITEM_PARSE_REFLECTCHANCEELEMENTS},
    {"reflectchancemagic", ITEM_PARSE_REFLECTCHANCEMAGIC},
    {"reflectchanceenergy", ITEM_PARSE_REFLECTCHANCEENERGY},
    {"reflectchancefire", ITEM_PARSE_REFLECTCHANCEFIRE},
    {"reflectchancepoison", ITEM_PARSE_REFLECTCHANCEEARTH},
    {"reflectchanceearth", ITEM_PARSE_REFLECTCHANCEEARTH},
    {"reflectchanceice", ITEM_PARSE_REFLECTCHANCEICE},
    {"reflectchanceholy", ITEM_PARSE_REFLECTCHANCEHOLY},
    {"reflectchancedeath", ITEM_PARSE_REFLECTCHANCEDEATH},
    {"reflectchancelifedrain", ITEM_PARSE_REFLECTCHANCELIFEDRAIN},
    {"reflectchancemanadrain", ITEM_PARSE_REFLECTCHANCEMANADRAIN},
    {"reflectchancedrown", ITEM_PARSE_REFLECTCHANCEDROWN},
    {"reflectchancephysical", ITEM_PARSE_REFLECTCHANCEPHYSICAL},
    {"reflectchancehealing", ITEM_PARSE_REFLECTCHANCEHEALING},
    {"boostpercentall", ITEM_PARSE_BOOSTPERCENTALL},
    {"boostpercentallelements", ITEM_PARSE_BOOSTPERCENTALL},
    {"boostpercentelements", ITEM_PARSE_BOOSTPERCENTELEMENTS},
    {"boostpercentmagic", ITEM_PARSE_BOOSTPERCENTMAGIC},
    {"boostpercentenergy", ITEM_PARSE_BOOSTPERCENTENERGY},
    {"boostpercentfire", ITEM_PARSE_BOOSTPERCENTFIRE},
    {"boostpercentpoison", ITEM_PARSE_BOOSTPERCENTEARTH},
    {"boostpercentearth", ITEM_PARSE_BOOSTPERCENTEARTH},
    {"boostpercentice", ITEM_PARSE_BOOSTPERCENTICE},
    {"boostpercentholy", ITEM_PARSE_BOOSTPERCENTHOLY},
    {"boostpercentdeath", ITEM_PARSE_BOOSTPERCENTDEATH},
    {"boostpercentlifedrain", ITEM_PARSE_BOOSTPERCENTLIFEDRAIN},
    {"boostpercentmanadrain", ITEM_PARSE_BOOSTPERCENTMANADRAIN},
    {"boostpercentdrown", ITEM_PARSE_BOOSTPERCENTDROWN},
    {"boostpercentphysical", ITEM_PARSE_BOOSTPERCENTPHYSICAL},
    {"boostpercenthealing", ITEM_PARSE_BOOSTPERCENTHEALING},
    {"magiclevelenergy", ITEM_PARSE_MAGICLEVELENERGY},
    {"magiclevelfire", ITEM_PARSE_MAGICLEVELFIRE},
    {"magiclevelpoison", ITEM_PARSE_MAGICLEVELPOISON},
    {"magiclevelearth", ITEM_PARSE_MAGICLEVELPOISON},
    {"magiclevelice", ITEM_PARSE_MAGICLEVELICE},
    {"magiclevelholy", ITEM_PARSE_MAGICLEVELHOLY},
    {"magicleveldeath", ITEM_PARSE_MAGICLEVELDEATH},
    {"magiclevellifedrain", ITEM_PARSE_MAGICLEVELLIFEDRAIN},
    {"magiclevelmanadrain", ITEM_PARSE_MAGICLEVELMANADRAIN},
    {"magicleveldrown", ITEM_PARSE_MAGICLEVELDROWN},
    {"magiclevelphysical", ITEM_PARSE_MAGICLEVELPHYSICAL},
    {"magiclevelhealing", ITEM_PARSE_MAGICLEVELHEALING},
    {"magiclevelundefined", ITEM_PARSE_MAGICLEVELUNDEFINED},
    {"suppressdrunk", ITEM_PARSE_SUPPRESSDRUNK},
    {"suppressenergy", ITEM_PARSE_SUPPRESSENERGY},
    {"suppressfire", ITEM_PARSE_SUPPRESSFIRE},
    {"suppresspoison", ITEM_PARSE_SUPPRESSPOISON},
    {"suppressdrown", ITEM_PARSE_SUPPRESSDROWN},
    {"suppressphysical", ITEM_PARSE_SUPPRESSPHYSICAL},
    {"suppressfreeze", ITEM_PARSE_SUPPRESSFREEZE},
    {"suppressdazzle", ITEM_PARSE_SUPPRESSDAZZLE},
    {"suppresscurse", ITEM_PARSE_SUPPRESSCURSE},
    {"field", ITEM_PARSE_FIELD},
    {"replaceable", ITEM_PARSE_REPLACEABLE},
    {"partnerdirection", ITEM_PARSE_PARTNERDIRECTION},
    {"leveldoor", ITEM_PARSE_LEVELDOOR},
    {"maletransformto", ITEM_PARSE_MALETRANSFORMTO},
    {"malesleeper", ITEM_PARSE_MALETRANSFORMTO},
    {"femaletransformto", ITEM_PARSE_FEMALETRANSFORMTO},
    {"femalesleeper", ITEM_PARSE_FEMALETRANSFORMTO},
    {"transformto", ITEM_PARSE_TRANSFORMTO},
    {"destroyto", ITEM_PARSE_DESTROYTO},
    {"elementice", ITEM_PARSE_ELEMENTICE},
    {"elementearth", ITEM_PARSE_ELEMENTEARTH},
    {"elementfire", ITEM_PARSE_ELEMENTFIRE},
    {"elementenergy", ITEM_PARSE_ELEMENTENERGY},
    {"elementdeath", ITEM_PARSE_ELEMENTDEATH},
    {"elementholy", ITEM_PARSE_ELEMENTHOLY},
    {"walkstack", ITEM_PARSE_WALKSTACK},
    {"blocking", ITEM_PARSE_BLOCKING},
    {"allowdistread", ITEM_PARSE_ALLOWDISTREAD},
    {"storeitem", ITEM_PARSE_STOREITEM},
    {"worth", ITEM_PARSE_WORTH},
    {"imbuementslot", ITEM_PARSE_IMBUEMENTSLOT},
    {"wrapableto", ITEM_PARSE_WRAPABLETO},
    {"stacksize", ITEM_PARSE_STACKSIZE},
    {"supply", ITEM_PARSE_SUPPLY},
    {"experienceratebase", ITEM_PARSE_EXPERIENCERATE_BASE},
    {"experienceratelowlevel", ITEM_PARSE_EXPERIENCERATE_LOW_LEVEL},
    {"experienceratebonus", ITEM_PARSE_EXPERIENCERATE_BONUS},
    {"experienceratestamina", ITEM_PARSE_EXPERIENCERATE_STAMINA},
    {"reduceskillloss", ITEM_PARSE_REDUCESKILLLOSS},
	{"drop", ITEM_PARSE_DROPBONUS},
    {"elementalbond", ITEM_PARSE_ELEMENTALBOND},
    {"script", ITEM_PARSE_SCRIPT},
};

const std::unordered_map<std::string, ItemTypes_t> ItemTypesMap = {
	{"key", ITEM_TYPE_KEY},
	{"magicfield", ITEM_TYPE_MAGICFIELD},
	{"container", ITEM_TYPE_CONTAINER},
	{"depot", ITEM_TYPE_DEPOT},
	{"mailbox", ITEM_TYPE_MAILBOX},
	{"trashholder", ITEM_TYPE_TRASHHOLDER},
	{"teleport", ITEM_TYPE_TELEPORT},
	{"door", ITEM_TYPE_DOOR},
	{"bed", ITEM_TYPE_BED},
	{"rune", ITEM_TYPE_RUNE},
	{"rewardchest", ITEM_TYPE_REWARDCHEST}
};

const std::unordered_map<std::string, tileflags_t> TileStatesMap = {
    {"down", TILESTATE_FLOORCHANGE_DOWN},        {"north", TILESTATE_FLOORCHANGE_NORTH},
    {"south", TILESTATE_FLOORCHANGE_SOUTH},      {"southalt", TILESTATE_FLOORCHANGE_SOUTH_ALT},
    {"west", TILESTATE_FLOORCHANGE_WEST},        {"east", TILESTATE_FLOORCHANGE_EAST},
    {"eastalt", TILESTATE_FLOORCHANGE_EAST_ALT},
};

const std::unordered_map<std::string, RaceType_t> RaceTypesMap = {
    {"venom", RACE_VENOM}, {"blood", RACE_BLOOD},   {"undead", RACE_UNDEAD},
    {"fire", RACE_FIRE},   {"energy", RACE_ENERGY}, {"ink", RACE_INK},
};

const std::unordered_map<std::string, WeaponType_t> WeaponTypesMap = {
    {"sword", WEAPON_SWORD},     {"club", WEAPON_CLUB},         {"axe", WEAPON_AXE},
    {"shield", WEAPON_SHIELD},   {"distance", WEAPON_DISTANCE}, {"wand", WEAPON_WAND},
    {"ammunition", WEAPON_AMMO}, {"quiver", WEAPON_QUIVER},     {"fist", WEAPON_FIST},
};

const std::unordered_map<std::string, FluidTypes_t> FluidTypesMap = {
    {"water", FLUID_WATER},
    {"blood", FLUID_BLOOD},
    {"beer", FLUID_BEER},
    {"slime", FLUID_SLIME},
    {"lemonade", FLUID_LEMONADE},
    {"milk", FLUID_MILK},
    {"mana", FLUID_MANA},
    {"life", FLUID_LIFE},
    {"oil", FLUID_OIL},
    {"urine", FLUID_URINE},
    {"coconut", FLUID_COCONUTMILK},
    {"wine", FLUID_WINE},
    {"mud", FLUID_MUD},
    {"fruitjuice", FLUID_FRUITJUICE},
    {"lava", FLUID_LAVA},
    {"rum", FLUID_RUM},
    {"swamp", FLUID_SWAMP},
    {"tea", FLUID_TEA},
    {"mead", FLUID_MEAD},
    {"ink", FLUID_INK},
};

const std::unordered_map<std::string_view, Direction> DirectionsMap = {
    {"north", DIRECTION_NORTH},
    {"n", DIRECTION_NORTH},
    {"0", DIRECTION_NORTH},
    {"east", DIRECTION_EAST},
    {"e", DIRECTION_EAST},
    {"1", DIRECTION_EAST},
    {"south", DIRECTION_SOUTH},
    {"s", DIRECTION_SOUTH},
    {"2", DIRECTION_SOUTH},
    {"west", DIRECTION_WEST},
    {"w", DIRECTION_WEST},
    {"3", DIRECTION_WEST},
    {"southwest", DIRECTION_SOUTHWEST},
    {"south west", DIRECTION_SOUTHWEST},
    {"south-west", DIRECTION_SOUTHWEST},
    {"sw", DIRECTION_SOUTHWEST},
    {"4", DIRECTION_SOUTHWEST},
    {"southeast", DIRECTION_SOUTHEAST},
    {"south east", DIRECTION_SOUTHEAST},
    {"south-east", DIRECTION_SOUTHEAST},
    {"se", DIRECTION_SOUTHEAST},
    {"5", DIRECTION_SOUTHEAST},
    {"northwest", DIRECTION_NORTHWEST},
    {"north west", DIRECTION_NORTHWEST},
    {"north-west", DIRECTION_NORTHWEST},
    {"nw", DIRECTION_NORTHWEST},
    {"6", DIRECTION_NORTHWEST},
    {"northeast", DIRECTION_NORTHEAST},
    {"north east", DIRECTION_NORTHEAST},
    {"north-east", DIRECTION_NORTHEAST},
    {"ne", DIRECTION_NORTHEAST},
    {"7", DIRECTION_NORTHEAST},
};

Direction getDirection(std::string_view string)
{
	if (auto it = DirectionsMap.find(string); it != DirectionsMap.end()) {
		return it->second;
	}
	fmt::print("[Warning - getDirection] Invalid direction: {}\n", string);
	return DIRECTION_NORTH;
}

class DatReader
{
public:
	explicit DatReader(const std::vector<uint8_t>& buffer) : buffer{buffer} {}

	size_t position() const { return offset; }
	size_t remaining() const { return buffer.size() - offset; }

	bool skip(size_t count)
	{
		if (count > remaining()) {
			return false;
		}

		offset += count;
		return true;
	}

	template <typename T>
	bool read(T& value)
	{
		static_assert(std::is_trivially_copyable_v<T>);
		if (sizeof(T) > remaining()) {
			return false;
		}

		std::memcpy(&value, buffer.data() + offset, sizeof(T));
		offset += sizeof(T);
		return true;
	}

private:
	const std::vector<uint8_t>& buffer;
	size_t offset = 0;
};

bool skipDatMarketItem(DatReader& reader)
{
	uint16_t nameLength = 0;
	return reader.skip(6) && reader.read(nameLength) && reader.skip(static_cast<size_t>(nameLength) + 4);
}

bool readDatAttributes(ItemType& iType, DatReader& reader, bool logErrors)
{
	uint8_t rawFlag = 0;
	do {
		if (!reader.read(rawFlag)) {
			return false;
		}

		switch (static_cast<ItemDatFlag>(rawFlag)) {
			case ItemDatFlag::Ground: {
				uint16_t groundSpeed = 0;
				if (!reader.read(groundSpeed)) {
					return false;
				}

				iType.group = ITEM_GROUP_GROUND;
				iType.speed = groundSpeed;
				break;
			}

			case ItemDatFlag::GroundBorder:
				iType.alwaysOnTopOrder = 1;
				break;

			case ItemDatFlag::OnBottom:
				iType.alwaysOnTopOrder = 2;
				break;

			case ItemDatFlag::OnTop:
				iType.alwaysOnTopOrder = 3;
				break;

			case ItemDatFlag::Container:
				iType.group = ITEM_GROUP_CONTAINER;
				iType.type = ITEM_TYPE_CONTAINER;
				break;

			case ItemDatFlag::Stackable:
				iType.stackable = true;
				break;

			case ItemDatFlag::ForceUse:
				iType.forceUse = true;
				break;

			case ItemDatFlag::MultiUse:
				iType.useable = true;
				break;

			case ItemDatFlag::Writable: {
				uint16_t maxTextLength = 0;
				if (!reader.read(maxTextLength)) {
					return false;
				}

				iType.canWriteText = true;
				iType.canReadText = true;
				iType.maxTextLen = maxTextLength;
				break;
			}

			case ItemDatFlag::WritableOnce: {
				uint16_t maxTextLength = 0;
				if (!reader.read(maxTextLength)) {
					return false;
				}

				iType.canReadText = true;
				iType.maxTextLen = maxTextLength;
				break;
			}

			case ItemDatFlag::FluidContainer:
				iType.group = ITEM_GROUP_FLUID;
				break;

			case ItemDatFlag::Fluid:
				iType.group = ITEM_GROUP_SPLASH;
				break;

			case ItemDatFlag::IsUnpassable:
				iType.blockSolid = true;
				break;

			case ItemDatFlag::IsUnmoveable:
				iType.moveable = false;
				break;

			case ItemDatFlag::BlockMissiles:
				iType.blockProjectile = true;
				break;

			case ItemDatFlag::BlockPathfinder:
				iType.blockPathFind = true;
				break;

			case ItemDatFlag::Pickupable:
				iType.pickupable = true;
				break;

			case ItemDatFlag::Hangable:
				iType.isHangable = true;
				break;

			case ItemDatFlag::IsHorizontal:
				iType.isHorizontal = true;
				break;

			case ItemDatFlag::IsVertical:
				iType.isVertical = true;
				break;

			case ItemDatFlag::Rotatable:
				iType.rotatable = true;
				break;

			case ItemDatFlag::HasLight: {
				uint16_t lightLevel = 0;
				uint16_t lightColor = 0;
				if (!reader.read(lightLevel) || !reader.read(lightColor)) {
					return false;
				}

				iType.lightLevel = static_cast<uint8_t>(lightLevel);
				iType.lightColor = static_cast<uint8_t>(lightColor);
				break;
			}

			case ItemDatFlag::DontHide:
			case ItemDatFlag::Translucent:
			case ItemDatFlag::Lying:
			case ItemDatFlag::AnimateAlways:
			case ItemDatFlag::FullGround:
				break;

			case ItemDatFlag::HasOffset:
				if (!reader.skip(4)) {
					return false;
				}
				break;

			case ItemDatFlag::HasElevation:
				if (!reader.skip(2)) {
					return false;
				}
				iType.hasHeight = true;
				break;

			case ItemDatFlag::Minimap:
				if (!reader.skip(2)) {
					return false;
				}
				break;

			case ItemDatFlag::LensHelp: {
				uint16_t lensHelp = 0;
				if (!reader.read(lensHelp)) {
					return false;
				}

				if (lensHelp == 1112) {
					iType.canReadText = true;
				}
				break;
			}

			case ItemDatFlag::IgnoreLook:
				iType.lookThrough = true;
				break;

			case ItemDatFlag::Cloth:
				if (!reader.skip(2)) {
					return false;
				}
				break;

			case ItemDatFlag::MarketItem:
				if (!skipDatMarketItem(reader)) {
					return false;
				}
				break;

			case ItemDatFlag::LastFlag:
				break;

			default:
				if (logErrors) {
					LOG_ERROR(fmt::format("[Error - Items::loadFromDat] Unknown flag {} at item id {}.", rawFlag, iType.id));
				}
				return false;
		}
	} while (rawFlag != static_cast<uint8_t>(ItemDatFlag::LastFlag));

	iType.alwaysOnTop = iType.alwaysOnTopOrder != 0;
	return true;
}

bool skipDatSpriteLayout(DatReader& reader, size_t spriteIdBytes, uint8_t* frameCount = nullptr)
{
	uint8_t width = 0;
	uint8_t height = 0;
	uint8_t layers = 0;
	uint8_t patternX = 0;
	uint8_t patternY = 0;
	uint8_t patternZ = 0;
	uint8_t frames = 0;
	if (!reader.read(width) || !reader.read(height)) {
		return false;
	}

	if (width == 0 || height == 0) {
		return false;
	}

	if ((width > 1 || height > 1) && !reader.skip(1)) {
		return false;
	}

	if (!reader.read(layers) || !reader.read(patternX) || !reader.read(patternY) || !reader.read(patternZ) ||
	    !reader.read(frames)) {
		return false;
	}

	if (layers == 0 || patternX == 0 || patternY == 0 || patternZ == 0 || frames == 0) {
		return false;
	}

	const uint64_t spriteCount = static_cast<uint64_t>(width) * height * layers * patternX * patternY * patternZ * frames;
	if (spriteCount > (std::numeric_limits<size_t>::max)() / spriteIdBytes) {
		return false;
	}

	if (frameCount) {
		*frameCount = frames;
	}
	return reader.skip(static_cast<size_t>(spriteCount) * spriteIdBytes);
}

bool parseDatBuffer(const std::vector<uint8_t>& buffer, size_t spriteIdBytes, std::vector<ItemType>& parsedItems,
                    uint16_t& itemCount, bool logErrors)
{
	if (spriteIdBytes != sizeof(uint16_t) && spriteIdBytes != sizeof(uint32_t)) {
		return false;
	}

	DatReader reader{buffer};
	uint32_t signature = 0;
	uint16_t outfitCount = 0;
	uint16_t effectCount = 0;
	uint16_t distanceEffectCount = 0;
	if (!reader.read(signature) || !reader.read(itemCount) || !reader.read(outfitCount) || !reader.read(effectCount) ||
	    !reader.read(distanceEffectCount)) {
		return false;
	}

	if (itemCount < 100) {
		return false;
	}

	std::vector<ItemType> itemsBuffer(itemCount + 1);
	for (uint32_t id = 100; id <= itemCount; ++id) {
		ItemType& iType = itemsBuffer[id];
		iType.id = static_cast<uint16_t>(id);

		if (!readDatAttributes(iType, reader, logErrors)) {
			return false;
		}

		uint8_t frames = 0;
		if (!skipDatSpriteLayout(reader, spriteIdBytes, &frames)) {
			return false;
		}
		iType.isAnimation = frames > 1;
	}

	parsedItems = std::move(itemsBuffer);
	return true;
}

} // namespace

Items::Items()
{
	items.reserve(30000);
	nameToItems.reserve(30000);
}

void Items::clear()
{
	items.clear();
	nameToItems.clear();
	currencyItems.clear();
	inventory.clear();
}

bool Items::reload()
{
	clear();
	if (!loadFromDat(getString(ConfigManager::ASSETS_DAT_PATH))) {
		return false;
	}

	g_moveEvents->reload();
	g_weapons->reload();

	if (!loadFromXml()) {
		return false;
	}

	g_scripts->loadScripts("items", false, true);
	g_weapons->loadDefaults();
	return true;
}

bool Items::loadFromDat(std::string_view file)
{
	const std::string filePath{file};
	std::ifstream fin(filePath, std::ios::binary | std::ios::ate);
	if (!fin) {
		LOG_ERROR(fmt::format("[Error - Items::loadFromDat] Unable to load assets.dat from path: {}", filePath));
		LOG_ERROR("[Error - Items::loadFromDat] Copy Tibia.dat from your client folder, rename it to assets.dat and place it in data/items/.");
		return false;
	}

	const std::streamoff endPosition = fin.tellg();
	if (endPosition <= 0 || static_cast<uint64_t>(endPosition) > (std::numeric_limits<size_t>::max)()) {
		LOG_ERROR(fmt::format("[Error - Items::loadFromDat] Invalid assets.dat size: {}", filePath));
		return false;
	}

	const auto fileSize = static_cast<size_t>(endPosition);
	fin.seekg(0, std::ios::beg);

	std::vector<uint8_t> buffer(fileSize);
	if (!fin.read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(buffer.size()))) {
		LOG_ERROR(fmt::format("[Error - Items::loadFromDat] Failed to read assets.dat: {}", filePath));
		return false;
	}

	std::vector<ItemType> parsedItems;
	uint16_t itemCount = 0;
	size_t spriteIdBytes = sizeof(uint16_t);
	if (!parseDatBuffer(buffer, sizeof(uint16_t), parsedItems, itemCount, false)) {
		parsedItems.clear();
		if (!parseDatBuffer(buffer, sizeof(uint32_t), parsedItems, itemCount, true)) {
			LOG_ERROR(fmt::format("[Error - Items::loadFromDat] Invalid or unsupported assets.dat: {}", filePath));
			return false;
		}

		spriteIdBytes = sizeof(uint32_t);
	}

	items = std::move(parsedItems);
	items.shrink_to_fit();
	LOG_INFO(fmt::format(">> Assets DAT: {:d} item client ids loaded (sprite ids: uint{:d}).", itemCount - 99,
	                     static_cast<uint32_t>(spriteIdBytes * 8)));
	return true;
}

bool Items::loadFromXml()
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file("data/items/items.xml");
	if (!result) {
		printXMLError("Error - Items::loadFromXml", "data/items/items.xml", result);
		return false;
	}

	for (auto itemNode : doc.child("items").children()) {
		pugi::xml_attribute idAttribute = itemNode.attribute("id");
		if (idAttribute) {
			parseItemNode(itemNode, pugi::cast<uint16_t>(idAttribute.value()));
			continue;
		}

		pugi::xml_attribute fromIdAttribute = itemNode.attribute("fromid");
		if (!fromIdAttribute) {
			LOG_WARN("[Warning - Items::loadFromXml] No item id found");
			continue;
		}

		pugi::xml_attribute toIdAttribute = itemNode.attribute("toid");
		if (!toIdAttribute) {
			LOG_WARN(fmt::format("[Warning - Items::loadFromXml] fromid ({}) without toid", fromIdAttribute.value()));
			continue;
		}

		uint16_t id = pugi::cast<uint16_t>(fromIdAttribute.value());
		uint16_t toId = pugi::cast<uint16_t>(toIdAttribute.value());
		while (id <= toId) {
			parseItemNode(itemNode, id++);
		}
	}
	return true;
}

void Items::parseItemNode(const pugi::xml_node& itemNode, uint16_t id)
{
	if (id > 0 && id < 100) {
		ItemType& iType = items[id];
		iType.id = id;
	}

	ItemType& it = getItemType(id);
	if (it.id == 0) {
		return;
	}

	if (!it.name.empty()) {
		LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Duplicate item with id: {}", id));
		return;
	}

	it.name = itemNode.attribute("name").as_string();

	if (!it.name.empty()) {
		std::string lowerCaseName = boost::algorithm::to_lower_copy(it.name);
		if (!nameToItems.contains(lowerCaseName)) {
			nameToItems.emplace(std::move(lowerCaseName), id);
		}
	}

	pugi::xml_attribute articleAttribute = itemNode.attribute("article");
	if (articleAttribute) {
		it.article = articleAttribute.as_string();
	}

	pugi::xml_attribute pluralAttribute = itemNode.attribute("plural");
	if (pluralAttribute) {
		it.pluralName = pluralAttribute.as_string();
	}

	Abilities& abilities = it.getAbilities();

	for (auto attributeNode : itemNode.children()) {
		pugi::xml_attribute keyAttribute = attributeNode.attribute("key");
		if (!keyAttribute) {
			continue;
		}

		pugi::xml_attribute valueAttribute = attributeNode.attribute("value");
		pugi::xml_attribute maxValueAttr;
		if (!valueAttribute) {
			valueAttribute = attributeNode.attribute("minvalue");
			if (!valueAttribute) {
				continue;
			}

			maxValueAttr = attributeNode.attribute("maxvalue");
			if (!maxValueAttr) {
				continue;
			}
		}

		std::string tmpStrValue = boost::algorithm::to_lower_copy<std::string>(keyAttribute.as_string());
		auto parseAttribute = ItemParseAttributesMap.find(tmpStrValue);
		if (parseAttribute != ItemParseAttributesMap.end()) {
			ItemParseAttributes_t parseType = parseAttribute->second;
			switch (parseType) {
				case ITEM_PARSE_TYPE: {
					tmpStrValue = boost::algorithm::to_lower_copy<std::string>(valueAttribute.as_string());
					auto it2 = ItemTypesMap.find(tmpStrValue);
					if (it2 != ItemTypesMap.end()) {
						it.type = it2->second;
						if (it.type == ITEM_TYPE_CONTAINER) {
							it.group = ITEM_GROUP_CONTAINER;
						}
					} else {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Unknown type: {}", valueAttribute.as_string()));
					}
					break;
				}

				case ITEM_PARSE_DESCRIPTION: {
					it.description = valueAttribute.as_string();
					break;
				}

				case ITEM_PARSE_RUNESPELLNAME: {
					it.runeSpellName = valueAttribute.as_string();
					break;
				}

				case ITEM_PARSE_WEIGHT: {
					it.weight = pugi::cast<uint32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_WEIGHTREDUCTION: {
					const int32_t intValue = pugi::cast<int32_t>(valueAttribute.value());
					it.weightReduction = static_cast<uint8_t>(std::min<int32_t>(std::max<int32_t>(intValue, 0), 100));
					break;
				}

				case ITEM_PARSE_SHOWCOUNT: {
					it.showCount = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_SUPPLY: {
					it.supply = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_ARMOR: {
					it.armor = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_DEFENSE: {
					it.defense = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_EXTRADEF: {
					it.extraDefense = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ATTACK: {
					it.attack = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ATTACK_SPEED: {
					it.attackSpeed = pugi::cast<uint32_t>(valueAttribute.value());
					if (it.attackSpeed > 0 && it.attackSpeed < 100) {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] AttackSpeed lower than 100 for item: {}", it.id));
						it.attackSpeed = 100;
					}
					break;
				}

				case ITEM_PARSE_CLASSIFICATION: {
					it.classification = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_TIER: {
					it.tier = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ROTATETO: {
					it.rotateTo = pugi::cast<uint16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MOVEABLE: {
					it.moveable = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_BLOCKPROJECTILE: {
					it.blockProjectile = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_IGNOREBLOCKING: {
					it.ignoreBlocking = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_PICKUPABLE: {
					it.allowPickupable = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_FORCESERIALIZE: {
					it.forceSerialize = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_FLOORCHANGE: {
					tmpStrValue = boost::algorithm::to_lower_copy<std::string>(valueAttribute.as_string());
					auto it2 = TileStatesMap.find(tmpStrValue);
					if (it2 != TileStatesMap.end()) {
						it.floorChange |= it2->second;
					} else {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Unknown floorChange: {}", valueAttribute.as_string()));
					}
					break;
				}

				case ITEM_PARSE_CORPSETYPE: {
					tmpStrValue = boost::algorithm::to_lower_copy<std::string>(valueAttribute.as_string());
					auto it2 = RaceTypesMap.find(tmpStrValue);
					if (it2 != RaceTypesMap.end()) {
						it.corpseType = it2->second;
					} else {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Unknown corpseType: {}", valueAttribute.as_string()));
					}
					break;
				}

				case ITEM_PARSE_CONTAINERSIZE: {
					it.maxItems = pugi::cast<uint16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_FLUIDSOURCE: {
					tmpStrValue = boost::algorithm::to_lower_copy<std::string>(valueAttribute.as_string());
					auto it2 = FluidTypesMap.find(tmpStrValue);
					if (it2 != FluidTypesMap.end()) {
						it.fluidSource = it2->second;
					} else {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Unknown fluidSource: {}", valueAttribute.as_string()));
					}
					break;
				}

				case ITEM_PARSE_READABLE: {
					it.canReadText = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_WRITEABLE: {
					it.canWriteText = valueAttribute.as_bool();
					it.canReadText = it.canWriteText;
					break;
				}

				case ITEM_PARSE_MAXTEXTLEN: {
					it.maxTextLen = pugi::cast<uint16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_WRITEONCEITEMID: {
					it.writeOnceItemId = pugi::cast<uint16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_WEAPONTYPE: {
					tmpStrValue = boost::algorithm::to_lower_copy<std::string>(valueAttribute.as_string());
					auto it2 = WeaponTypesMap.find(tmpStrValue);
					if (it2 != WeaponTypesMap.end()) {
						it.weaponType = it2->second;
					} else {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Unknown weaponType: {}", valueAttribute.as_string()));
					}
					break;
				}

				case ITEM_PARSE_SLOTTYPE: {
					tmpStrValue = boost::algorithm::to_lower_copy<std::string>(valueAttribute.as_string());
					if (tmpStrValue == "head") {
						it.slotPosition |= SLOTP_HEAD;
					} else if (tmpStrValue == "body") {
						it.slotPosition |= SLOTP_ARMOR;
					} else if (tmpStrValue == "legs") {
						it.slotPosition |= SLOTP_LEGS;
					} else if (tmpStrValue == "feet") {
						it.slotPosition |= SLOTP_FEET;
					} else if (tmpStrValue == "backpack") {
						it.slotPosition |= SLOTP_BACKPACK;
					} else if (tmpStrValue == "two-handed") {
						it.slotPosition |= SLOTP_TWO_HAND;
					} else if (tmpStrValue == "right-hand") {
						it.slotPosition &= ~SLOTP_LEFT;
					} else if (tmpStrValue == "left-hand") {
						it.slotPosition &= ~SLOTP_RIGHT;
					} else if (tmpStrValue == "necklace") {
						it.slotPosition |= SLOTP_NECKLACE;
					} else if (tmpStrValue == "ring") {
						it.slotPosition |= SLOTP_RING;
					} else if (tmpStrValue == "ammo") {
						it.slotPosition |= SLOTP_AMMO;
					} else if (tmpStrValue == "hand") {
						it.slotPosition |= SLOTP_HAND;
					} else {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Unknown slotType: {}", valueAttribute.as_string()));
					}
					break;
				}

				case ITEM_PARSE_AMMOTYPE: {
					it.ammoType = getAmmoType(boost::algorithm::to_lower_copy<std::string>(valueAttribute.as_string()));
					if (it.ammoType == AMMO_NONE) {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Unknown ammoType: {}", valueAttribute.as_string()));
					}
					break;
				}

				case ITEM_PARSE_SHOOTTYPE: {
					ShootType_t shoot =
					    getShootType(boost::algorithm::to_lower_copy<std::string>(valueAttribute.as_string()));
					if (shoot != CONST_ANI_NONE) {
						it.shootType = shoot;
					} else {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Unknown shootType: {}", valueAttribute.as_string()));
					}
					break;
				}

				case ITEM_PARSE_EFFECT: {
					MagicEffectClasses effect =
					    getMagicEffect(boost::algorithm::to_lower_copy<std::string>(valueAttribute.as_string()));
					if (effect != CONST_ME_NONE) {
						it.magicEffect = effect;
					} else {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Unknown effect: {}", valueAttribute.as_string()));
					}
					break;
				}

				case ITEM_PARSE_RANGE: {
					it.shootRange = pugi::cast<uint16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_STOPDURATION: {
					it.stopTime = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_DECAYTO: {
					it.decayTo = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_TRANSFORMEQUIPTO: {
					it.transformEquipTo = pugi::cast<uint16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_TRANSFORMDEEQUIPTO: {
					it.transformDeEquipTo = pugi::cast<uint16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_DURATION: {
					it.decayTimeMin = pugi::cast<uint32_t>(valueAttribute.value());

					if (maxValueAttr) {
						it.decayTimeMax = pugi::cast<uint32_t>(maxValueAttr.value());
					}
					break;
				}

				case ITEM_PARSE_SHOWDURATION: {
					it.showDuration = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_CHARGES: {
					it.charges = pugi::cast<uint32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_SHOWCHARGES: {
					it.showCharges = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_SHOWATTRIBUTES: {
					it.showAttributes = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_HITCHANCE: {
					it.hitChance =
					    std::min<int8_t>(100, std::max<int8_t>(-100, pugi::cast<int16_t>(valueAttribute.value())));
					break;
				}

				case ITEM_PARSE_MAXHITCHANCE: {
					it.maxHitChance = std::min<uint32_t>(100, pugi::cast<uint32_t>(valueAttribute.value()));
					break;
				}

				case ITEM_PARSE_INVISIBLE: {
					abilities.invisible = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_SPEED: {
					abilities.speed = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_HEALTHGAIN: {
					abilities.regeneration = true;
					abilities.healthGain = pugi::cast<uint32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_HEALTHGAINPERCENT: {
					abilities.regeneration = true;
					abilities.healthGainPercent = pugi::cast<uint32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_HEALTHTICKS: {
					abilities.regeneration = true;
					abilities.healthTicks = pugi::cast<uint32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MANAGAIN: {
					abilities.regeneration = true;
					abilities.manaGain = pugi::cast<uint32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MANAGAINPERCENT: {
					abilities.regeneration = true;
					abilities.manaGainPercent = pugi::cast<uint32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MANATICKS: {
					abilities.regeneration = true;
					abilities.manaTicks = pugi::cast<uint32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MANASHIELD: {
					abilities.manaShield = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_SKILLSWORD: {
					abilities.skills[SKILL_SWORD] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_SKILLAXE: {
					abilities.skills[SKILL_AXE] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_SKILLCLUB: {
					abilities.skills[SKILL_CLUB] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_SKILLDIST: {
					abilities.skills[SKILL_DISTANCE] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_SKILLFISH: {
					abilities.skills[SKILL_FISHING] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_SKILLSHIELD: {
					abilities.skills[SKILL_SHIELD] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_SKILLFIST: {
					abilities.skills[SKILL_FIST] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_CRITICALHITAMOUNT: {
					abilities.specialSkills[SPECIALSKILL_CRITICALHITAMOUNT] =
					    pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_CRITICALHITCHANCE: {
					abilities.specialSkills[SPECIALSKILL_CRITICALHITCHANCE] =
					    pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MANALEECHAMOUNT: {
					abilities.specialSkills[SPECIALSKILL_MANALEECHAMOUNT] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MANALEECHCHANCE: {
					abilities.specialSkills[SPECIALSKILL_MANALEECHCHANCE] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_LIFELEECHAMOUNT: {
					abilities.specialSkills[SPECIALSKILL_LIFELEECHAMOUNT] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_LIFELEECHCHANCE: {
					abilities.specialSkills[SPECIALSKILL_LIFELEECHCHANCE] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAXHITPOINTS: {
					abilities.stats[STAT_MAXHITPOINTS] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAXHITPOINTSPERCENT: {
					abilities.statsPercent[STAT_MAXHITPOINTS] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAXMANAPOINTS: {
					abilities.stats[STAT_MAXMANAPOINTS] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAXMANAPOINTSPERCENT: {
					abilities.statsPercent[STAT_MAXMANAPOINTS] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICPOINTS: {
					abilities.stats[STAT_MAGICPOINTS] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICPOINTSPERCENT: {
					abilities.statsPercent[STAT_MAGICPOINTS] = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_FIELDABSORBPERCENTENERGY: {
					abilities.fieldAbsorbPercent[combatTypeToIndex(COMBAT_ENERGYDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_FIELDABSORBPERCENTFIRE: {
					abilities.fieldAbsorbPercent[combatTypeToIndex(COMBAT_FIREDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_FIELDABSORBPERCENTPOISON: {
					abilities.fieldAbsorbPercent[combatTypeToIndex(COMBAT_EARTHDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTALL: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					for (auto& i : abilities.absorbPercent) {
						i += value;
					}
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTELEMENTS: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					abilities.absorbPercent[combatTypeToIndex(COMBAT_ENERGYDAMAGE)] += value;
					abilities.absorbPercent[combatTypeToIndex(COMBAT_FIREDAMAGE)] += value;
					abilities.absorbPercent[combatTypeToIndex(COMBAT_EARTHDAMAGE)] += value;
					abilities.absorbPercent[combatTypeToIndex(COMBAT_ICEDAMAGE)] += value;
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTMAGIC: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					abilities.absorbPercent[combatTypeToIndex(COMBAT_ENERGYDAMAGE)] += value;
					abilities.absorbPercent[combatTypeToIndex(COMBAT_FIREDAMAGE)] += value;
					abilities.absorbPercent[combatTypeToIndex(COMBAT_EARTHDAMAGE)] += value;
					abilities.absorbPercent[combatTypeToIndex(COMBAT_ICEDAMAGE)] += value;
					abilities.absorbPercent[combatTypeToIndex(COMBAT_HOLYDAMAGE)] += value;
					abilities.absorbPercent[combatTypeToIndex(COMBAT_DEATHDAMAGE)] += value;
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTENERGY: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_ENERGYDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTFIRE: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_FIREDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTPOISON: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_EARTHDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTICE: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_ICEDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTHOLY: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_HOLYDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTDEATH: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_DEATHDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTLIFEDRAIN: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_LIFEDRAIN)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTMANADRAIN: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_MANADRAIN)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTDROWN: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_DROWNDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTPHYSICAL: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_PHYSICALDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTHEALING: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_HEALING)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ABSORBPERCENTUNDEFINED: {
					abilities.absorbPercent[combatTypeToIndex(COMBAT_UNDEFINEDDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTALL: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					for (auto& i : abilities.reflect) {
						i.percent += value;
					}
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTELEMENTS: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					abilities.reflect[combatTypeToIndex(COMBAT_ENERGYDAMAGE)].percent += value;
					abilities.reflect[combatTypeToIndex(COMBAT_FIREDAMAGE)].percent += value;
					abilities.reflect[combatTypeToIndex(COMBAT_EARTHDAMAGE)].percent += value;
					abilities.reflect[combatTypeToIndex(COMBAT_ICEDAMAGE)].percent += value;
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTMAGIC: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					abilities.reflect[combatTypeToIndex(COMBAT_ENERGYDAMAGE)].percent += value;
					abilities.reflect[combatTypeToIndex(COMBAT_FIREDAMAGE)].percent += value;
					abilities.reflect[combatTypeToIndex(COMBAT_EARTHDAMAGE)].percent += value;
					abilities.reflect[combatTypeToIndex(COMBAT_ICEDAMAGE)].percent += value;
					abilities.reflect[combatTypeToIndex(COMBAT_HOLYDAMAGE)].percent += value;
					abilities.reflect[combatTypeToIndex(COMBAT_DEATHDAMAGE)].percent += value;
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTENERGY: {
					abilities.reflect[combatTypeToIndex(COMBAT_ENERGYDAMAGE)].percent +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTFIRE: {
					abilities.reflect[combatTypeToIndex(COMBAT_FIREDAMAGE)].percent +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTEARTH: {
					abilities.reflect[combatTypeToIndex(COMBAT_EARTHDAMAGE)].percent +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTICE: {
					abilities.reflect[combatTypeToIndex(COMBAT_ICEDAMAGE)].percent +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTHOLY: {
					abilities.reflect[combatTypeToIndex(COMBAT_HOLYDAMAGE)].percent +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTDEATH: {
					abilities.reflect[combatTypeToIndex(COMBAT_DEATHDAMAGE)].percent +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTLIFEDRAIN: {
					abilities.reflect[combatTypeToIndex(COMBAT_LIFEDRAIN)].percent +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTMANADRAIN: {
					abilities.reflect[combatTypeToIndex(COMBAT_MANADRAIN)].percent +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTDROWN: {
					abilities.reflect[combatTypeToIndex(COMBAT_DROWNDAMAGE)].percent +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTPHYSICAL: {
					abilities.reflect[combatTypeToIndex(COMBAT_PHYSICALDAMAGE)].percent +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTPERCENTHEALING: {
					abilities.reflect[combatTypeToIndex(COMBAT_HEALING)].percent +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEALL: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					for (auto& i : abilities.reflect) {
						i.chance += value;
					}
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEELEMENTS: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					abilities.reflect[combatTypeToIndex(COMBAT_ENERGYDAMAGE)].chance += value;
					abilities.reflect[combatTypeToIndex(COMBAT_FIREDAMAGE)].chance += value;
					abilities.reflect[combatTypeToIndex(COMBAT_EARTHDAMAGE)].chance += value;
					abilities.reflect[combatTypeToIndex(COMBAT_ICEDAMAGE)].chance += value;
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEMAGIC: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					abilities.reflect[combatTypeToIndex(COMBAT_ENERGYDAMAGE)].chance += value;
					abilities.reflect[combatTypeToIndex(COMBAT_FIREDAMAGE)].chance += value;
					abilities.reflect[combatTypeToIndex(COMBAT_EARTHDAMAGE)].chance += value;
					abilities.reflect[combatTypeToIndex(COMBAT_ICEDAMAGE)].chance += value;
					abilities.reflect[combatTypeToIndex(COMBAT_HOLYDAMAGE)].chance += value;
					abilities.reflect[combatTypeToIndex(COMBAT_DEATHDAMAGE)].chance += value;
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEENERGY: {
					abilities.reflect[combatTypeToIndex(COMBAT_ENERGYDAMAGE)].chance +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEFIRE: {
					abilities.reflect[combatTypeToIndex(COMBAT_FIREDAMAGE)].chance +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEEARTH: {
					abilities.reflect[combatTypeToIndex(COMBAT_EARTHDAMAGE)].chance +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEICE: {
					abilities.reflect[combatTypeToIndex(COMBAT_ICEDAMAGE)].chance +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEHOLY: {
					abilities.reflect[combatTypeToIndex(COMBAT_HOLYDAMAGE)].chance +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEDEATH: {
					abilities.reflect[combatTypeToIndex(COMBAT_DEATHDAMAGE)].chance +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTCHANCELIFEDRAIN: {
					abilities.reflect[combatTypeToIndex(COMBAT_LIFEDRAIN)].chance +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEMANADRAIN: {
					abilities.reflect[combatTypeToIndex(COMBAT_MANADRAIN)].chance +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEDROWN: {
					abilities.reflect[combatTypeToIndex(COMBAT_DROWNDAMAGE)].chance +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEPHYSICAL: {
					abilities.reflect[combatTypeToIndex(COMBAT_PHYSICALDAMAGE)].chance +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_REFLECTCHANCEHEALING: {
					abilities.reflect[combatTypeToIndex(COMBAT_HEALING)].chance +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTALL: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					for (auto& i : abilities.boostPercent) {
						i += value;
					}
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTELEMENTS: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					abilities.boostPercent[combatTypeToIndex(COMBAT_ENERGYDAMAGE)] += value;
					abilities.boostPercent[combatTypeToIndex(COMBAT_FIREDAMAGE)] += value;
					abilities.boostPercent[combatTypeToIndex(COMBAT_EARTHDAMAGE)] += value;
					abilities.boostPercent[combatTypeToIndex(COMBAT_ICEDAMAGE)] += value;
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTMAGIC: {
					int16_t value = pugi::cast<int16_t>(valueAttribute.value());
					abilities.boostPercent[combatTypeToIndex(COMBAT_ENERGYDAMAGE)] += value;
					abilities.boostPercent[combatTypeToIndex(COMBAT_FIREDAMAGE)] += value;
					abilities.boostPercent[combatTypeToIndex(COMBAT_EARTHDAMAGE)] += value;
					abilities.boostPercent[combatTypeToIndex(COMBAT_ICEDAMAGE)] += value;
					abilities.boostPercent[combatTypeToIndex(COMBAT_HOLYDAMAGE)] += value;
					abilities.boostPercent[combatTypeToIndex(COMBAT_DEATHDAMAGE)] += value;
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTENERGY: {
					abilities.boostPercent[combatTypeToIndex(COMBAT_ENERGYDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTFIRE: {
					abilities.boostPercent[combatTypeToIndex(COMBAT_FIREDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTEARTH: {
					abilities.boostPercent[combatTypeToIndex(COMBAT_EARTHDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTICE: {
					abilities.boostPercent[combatTypeToIndex(COMBAT_ICEDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTHOLY: {
					abilities.boostPercent[combatTypeToIndex(COMBAT_HOLYDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTDEATH: {
					abilities.boostPercent[combatTypeToIndex(COMBAT_DEATHDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTLIFEDRAIN: {
					abilities.boostPercent[combatTypeToIndex(COMBAT_LIFEDRAIN)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTMANADRAIN: {
					abilities.boostPercent[combatTypeToIndex(COMBAT_MANADRAIN)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTDROWN: {
					abilities.boostPercent[combatTypeToIndex(COMBAT_DROWNDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTPHYSICAL: {
					abilities.boostPercent[combatTypeToIndex(COMBAT_PHYSICALDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_BOOSTPERCENTHEALING: {
					abilities.boostPercent[combatTypeToIndex(COMBAT_HEALING)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELENERGY: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_ENERGYDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELFIRE: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_FIREDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELPOISON: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_EARTHDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELICE: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_ICEDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELHOLY: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_HOLYDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELDEATH: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_DEATHDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELLIFEDRAIN: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_LIFEDRAIN)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELMANADRAIN: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_MANADRAIN)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELDROWN: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_DROWNDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELPHYSICAL: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_PHYSICALDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELHEALING: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_HEALING)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MAGICLEVELUNDEFINED: {
					abilities.specialMagicLevelSkill[combatTypeToIndex(COMBAT_UNDEFINEDDAMAGE)] +=
					    pugi::cast<int16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_SUPPRESSDRUNK: {
					if (valueAttribute.as_bool()) {
						abilities.conditionSuppressions |= CONDITION_DRUNK;
					}
					break;
				}

				case ITEM_PARSE_SUPPRESSENERGY: {
					if (valueAttribute.as_bool()) {
						abilities.conditionSuppressions |= CONDITION_ENERGY;
					}
					break;
				}

				case ITEM_PARSE_SUPPRESSFIRE: {
					if (valueAttribute.as_bool()) {
						abilities.conditionSuppressions |= CONDITION_FIRE;
					}
					break;
				}

				case ITEM_PARSE_SUPPRESSPOISON: {
					if (valueAttribute.as_bool()) {
						abilities.conditionSuppressions |= CONDITION_POISON;
					}
					break;
				}

				case ITEM_PARSE_SUPPRESSDROWN: {
					if (valueAttribute.as_bool()) {
						abilities.conditionSuppressions |= CONDITION_DROWN;
					}
					break;
				}

				case ITEM_PARSE_SUPPRESSPHYSICAL: {
					if (valueAttribute.as_bool()) {
						abilities.conditionSuppressions |= CONDITION_BLEEDING;
					}
					break;
				}

				case ITEM_PARSE_SUPPRESSFREEZE: {
					if (valueAttribute.as_bool()) {
						abilities.conditionSuppressions |= CONDITION_FREEZING;
					}
					break;
				}

				case ITEM_PARSE_SUPPRESSDAZZLE: {
					if (valueAttribute.as_bool()) {
						abilities.conditionSuppressions |= CONDITION_DAZZLED;
					}
					break;
				}

				case ITEM_PARSE_SUPPRESSCURSE: {
					if (valueAttribute.as_bool()) {
						abilities.conditionSuppressions |= CONDITION_CURSED;
					}
					break;
				}

				case ITEM_PARSE_FIELD: {
					it.group = ITEM_GROUP_MAGICFIELD;
					it.type = ITEM_TYPE_MAGICFIELD;

					CombatType_t combatType = COMBAT_NONE;
					std::unique_ptr<ConditionDamage> conditionDamage;

					tmpStrValue = boost::algorithm::to_lower_copy<std::string>(valueAttribute.as_string());
					if (tmpStrValue == "fire") {
						conditionDamage = std::make_unique<ConditionDamage>(CONDITIONID_COMBAT, CONDITION_FIRE);
						combatType = COMBAT_FIREDAMAGE;
					} else if (tmpStrValue == "energy") {
						conditionDamage = std::make_unique<ConditionDamage>(CONDITIONID_COMBAT, CONDITION_ENERGY);
						combatType = COMBAT_ENERGYDAMAGE;
					} else if (tmpStrValue == "poison") {
						conditionDamage = std::make_unique<ConditionDamage>(CONDITIONID_COMBAT, CONDITION_POISON);
						combatType = COMBAT_EARTHDAMAGE;
					} else if (tmpStrValue == "drown") {
						conditionDamage = std::make_unique<ConditionDamage>(CONDITIONID_COMBAT, CONDITION_DROWN);
						combatType = COMBAT_DROWNDAMAGE;
					} else if (tmpStrValue == "physical") {
						conditionDamage = std::make_unique<ConditionDamage>(CONDITIONID_COMBAT, CONDITION_BLEEDING);
						combatType = COMBAT_PHYSICALDAMAGE;
					} else if (tmpStrValue == "agony") {
						conditionDamage = std::make_unique<ConditionDamage>(CONDITIONID_COMBAT, CONDITION_AGONY);
						combatType = COMBAT_AGONYDAMAGE;
					} else {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Unknown field value: {}", valueAttribute.as_string()));
					}

					if (combatType != COMBAT_NONE) {
						it.combatType = combatType;

						uint32_t ticks = 0;
						int32_t start = 0;
						int32_t count = 1;
						int32_t initDamage = -1;
						int32_t damage = 0;
						for (auto subAttributeNode : attributeNode.children()) {
							pugi::xml_attribute subKeyAttribute = subAttributeNode.attribute("key");
							if (!subKeyAttribute) {
								continue;
							}

							pugi::xml_attribute subValueAttribute = subAttributeNode.attribute("value");
							if (!subValueAttribute) {
								continue;
							}

							tmpStrValue = boost::algorithm::to_lower_copy<std::string>(subKeyAttribute.as_string());
							if (tmpStrValue == "initdamage") {
								initDamage = pugi::cast<int32_t>(subValueAttribute.value());
							} else if (tmpStrValue == "ticks") {
								ticks = pugi::cast<uint32_t>(subValueAttribute.value());
							} else if (tmpStrValue == "count") {
								count = std::max<int32_t>(1, pugi::cast<int32_t>(subValueAttribute.value()));
							} else if (tmpStrValue == "start") {
								start = std::max<int32_t>(0, pugi::cast<int32_t>(subValueAttribute.value()));
							} else if (tmpStrValue == "damage") {
								damage = -pugi::cast<int32_t>(subValueAttribute.value());
								if (start > 0) {
									const int32_t damageEnd = std::max<int32_t>(0, -damage);
									const int32_t tickInterval = 1000;
									const int32_t tickCount = std::max<int32_t>(1, static_cast<int32_t>(ticks / tickInterval));

									conditionDamage->setInitDamage(-start);
									for (int32_t i = 1; i <= tickCount; ++i) {
										const int32_t damageValue = start - ((start - damageEnd) * i / tickCount);
										conditionDamage->addDamage(1, tickInterval, -std::max<int32_t>(damageEnd, damageValue));
									}

									start = 0;
									initDamage = 0;
								} else {
									conditionDamage->addDamage(count, ticks, damage);
								}
							}
						}

						// datapack compatibility, presume damage to be initialdamage if initialdamage is not declared.
						// initDamage = 0 (don't override initDamage with damage, don't set any initDamage)
						// initDamage = -1 (undefined, override initDamage with damage)
						if (initDamage > 0 || initDamage < -1) {
							conditionDamage->setInitDamage(-initDamage);
						} else if (initDamage == -1 && start != 0) {
							conditionDamage->setInitDamage(start);
						} else if (initDamage == -1 && damage != 0) {
							conditionDamage->setInitDamage(damage);
						}

						conditionDamage->setParam(CONDITION_PARAM_FIELD, 1);

						if (conditionDamage->getTotalDamage() > 0) {
							conditionDamage->setParam(CONDITION_PARAM_FORCEUPDATE, 1);
						}

						it.conditionDamage = std::move(conditionDamage);
					}
					break;
				}

				case ITEM_PARSE_REPLACEABLE: {
					it.replaceable = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_PARTNERDIRECTION: {
					it.bedPartnerDir = getDirection(valueAttribute.as_string());
					break;
				}

				case ITEM_PARSE_LEVELDOOR: {
					it.levelDoor = pugi::cast<uint32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_MALETRANSFORMTO: {
					uint16_t value = pugi::cast<uint16_t>(valueAttribute.value());
					it.transformToOnUse[PLAYERSEX_MALE] = value;
					ItemType& other = getItemType(value);
					if (other.transformToFree == 0) {
						other.transformToFree = it.id;
					}

					if (it.transformToOnUse[PLAYERSEX_FEMALE] == 0) {
						it.transformToOnUse[PLAYERSEX_FEMALE] = value;
					}
					break;
				}

				case ITEM_PARSE_FEMALETRANSFORMTO: {
					uint16_t value = pugi::cast<uint16_t>(valueAttribute.value());
					it.transformToOnUse[PLAYERSEX_FEMALE] = value;

					ItemType& other = getItemType(value);
					if (other.transformToFree == 0) {
						other.transformToFree = it.id;
					}

					if (it.transformToOnUse[PLAYERSEX_MALE] == 0) {
						it.transformToOnUse[PLAYERSEX_MALE] = value;
					}
					break;
				}

				case ITEM_PARSE_TRANSFORMTO: {
					it.transformToFree = pugi::cast<uint16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_DESTROYTO: {
					it.destroyTo = pugi::cast<uint16_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ELEMENTICE: {
					abilities.elementDamage = pugi::cast<uint16_t>(valueAttribute.value());
					abilities.elementType = COMBAT_ICEDAMAGE;
					break;
				}

				case ITEM_PARSE_ELEMENTEARTH: {
					abilities.elementDamage = pugi::cast<uint16_t>(valueAttribute.value());
					abilities.elementType = COMBAT_EARTHDAMAGE;
					break;
				}

				case ITEM_PARSE_ELEMENTFIRE: {
					abilities.elementDamage = pugi::cast<uint16_t>(valueAttribute.value());
					abilities.elementType = COMBAT_FIREDAMAGE;
					break;
				}

				case ITEM_PARSE_ELEMENTENERGY: {
					abilities.elementDamage = pugi::cast<uint16_t>(valueAttribute.value());
					abilities.elementType = COMBAT_ENERGYDAMAGE;
					break;
				}

				case ITEM_PARSE_ELEMENTDEATH: {
					abilities.elementDamage = pugi::cast<uint16_t>(valueAttribute.value());
					abilities.elementType = COMBAT_DEATHDAMAGE;
					break;
				}

				case ITEM_PARSE_ELEMENTHOLY: {
					abilities.elementDamage = pugi::cast<uint16_t>(valueAttribute.value());
					abilities.elementType = COMBAT_HOLYDAMAGE;
					break;
				}

				case ITEM_PARSE_WALKSTACK: {
					it.walkStack = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_BLOCKING: {
					it.blockSolid = valueAttribute.as_bool();
					break;
				}

				case ITEM_PARSE_ALLOWDISTREAD: {
					it.allowDistRead = booleanString(valueAttribute.as_string());
					break;
				}

				case ITEM_PARSE_STOREITEM: {
					it.storeItem = booleanString(valueAttribute.as_string());
					break;
				}

				case ITEM_PARSE_WORTH: {
					uint64_t worth = pugi::cast<uint64_t>(valueAttribute.value());
					if (currencyItems.contains(worth)) {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Duplicated currency worth. Item {} redefines worth {}", id, worth));
					} else {
						currencyItems.insert(CurrencyMap::value_type(worth, id));
						it.worth = worth;
					}
					break;
				}

				case ITEM_PARSE_EXPERIENCERATE_BASE: {
					int32_t rate = pugi::cast<int32_t>(valueAttribute.value());
					abilities.experienceRate[static_cast<size_t>(ExperienceRateType::BASE)] = rate;
					break;
				}

				case ITEM_PARSE_EXPERIENCERATE_LOW_LEVEL: {
					int32_t rate = pugi::cast<int32_t>(valueAttribute.value());
					abilities.experienceRate[static_cast<size_t>(ExperienceRateType::LOW_LEVEL)] = rate;
					break;
				}

				case ITEM_PARSE_EXPERIENCERATE_BONUS: {
					int32_t rate = pugi::cast<int32_t>(valueAttribute.value());
					abilities.experienceRate[static_cast<size_t>(ExperienceRateType::BONUS)] = rate;
					break;
				}

				case ITEM_PARSE_EXPERIENCERATE_STAMINA: {
					int32_t rate = pugi::cast<int32_t>(valueAttribute.value());
					abilities.experienceRate[static_cast<size_t>(ExperienceRateType::STAMINA)] = rate;
					break;
				}

				case ITEM_PARSE_REDUCESKILLLOSS: {
					it.reduceSkillLoss = pugi::cast<int32_t>(valueAttribute.value());
					abilities.reduceSkillLoss = pugi::cast<int32_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_DROPBONUS: {
					int32_t value = pugi::cast<int32_t>(valueAttribute.value());
					if (value < 0 || value > 100) {
						LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Drop bonus out of range (0-100) for item: {}", it.id));
						value = std::clamp(value, 0, 100);
					}
					it.dropBonus = value;
					abilities.dropBonus = value;
					break;
				}

				case ITEM_PARSE_STACKSIZE: {
					it.stackSize = pugi::cast<uint8_t>(valueAttribute.value());
					break;
				}

				case ITEM_PARSE_ELEMENTALBOND: {
					it.elementalBond = valueAttribute.as_string();
					break;
				}

				case ITEM_PARSE_SCRIPT: {
					parseScriptAttribute(it, attributeNode, valueAttribute);
					break;
				}

				case ITEM_PARSE_WRAPABLETO: {
					it.wrapableTo = pugi::cast<uint16_t>(valueAttribute.value());
					break;
				}
				case ITEM_PARSE_IMBUEMENTSLOT: {
					it.imbuementSlot = pugi::cast<uint16_t>(valueAttribute.value());
					for (auto subAttributeNode : attributeNode.children()) {
						pugi::xml_attribute subKeyAttribute = subAttributeNode.attribute("key");
						if (!subKeyAttribute) {
							continue;
						}
						pugi::xml_attribute subValueAttribute = subAttributeNode.attribute("value");
						if (!subValueAttribute) {
							continue;
						}
						std::string subKey = boost::algorithm::to_lower_copy<std::string>(subKeyAttribute.as_string());
						uint8_t maxTier = pugi::cast<uint16_t>(subValueAttribute.value()) & 0xFF;
						if (maxTier > 0) {
							it.imbuementAllowedTypes[subKey] = maxTier;
						}
					}
					break;
				}

				default: {
					// It should not ever get to here, only if you add a new key to the map and don't configure a case
					// for it.
					// for it.
					LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Not configured key value: {}", keyAttribute.as_string()));
					break;
				}
			}
		} else {
			LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Unknown key value: {}", keyAttribute.as_string()));
		}
	}

	// check bed items
	if ((it.transformToFree != 0 || it.transformToOnUse[PLAYERSEX_FEMALE] != 0 ||
	     it.transformToOnUse[PLAYERSEX_MALE] != 0) &&
	    it.type != ITEM_TYPE_BED) {
		LOG_WARN(fmt::format("[Warning - Items::parseItemNode] Item {} is not set as a bed-type", it.id));
	}
}

void Items::parseScriptAttribute(ItemType& it, const pugi::xml_node& attributeNode, const pugi::xml_attribute& valueAttribute)
{
	std::string scriptName = valueAttribute.as_string();
	std::vector<std::string> tokens;
	{
		std::istringstream iss(scriptName);
		std::string token;
		while (std::getline(iss, token, ';')) {
			if (!token.empty()) {
				tokens.push_back(token);
			}
		}
	}

	for (const auto& scriptToken : tokens) {
		if (scriptToken == "moveevent") {
			// Determine event type from sub-attributes, default to equip/deequip
			MoveEvent_t eventType = MOVE_EVENT_NONE;
			for (const auto& subNode : attributeNode.children()) {
				pugi::xml_attribute subKey = subNode.attribute("key");
				if (!subKey) continue;
				pugi::xml_attribute subValue = subNode.attribute("value");
				if (!subValue) continue;

				std::string key = boost::algorithm::to_lower_copy<std::string>(subKey.as_string());
				if (key == "eventtype") {
					std::string evtName = boost::algorithm::to_lower_copy<std::string>(subValue.as_string());
					if (evtName == "stepin") eventType = MOVE_EVENT_STEP_IN;
					else if (evtName == "stepout") eventType = MOVE_EVENT_STEP_OUT;
					else if (evtName == "equip") eventType = MOVE_EVENT_EQUIP;
					else if (evtName == "deequip") eventType = MOVE_EVENT_DEEQUIP;
					else if (evtName == "additem") eventType = MOVE_EVENT_ADD_ITEM;
					else if (evtName == "removeitem") eventType = MOVE_EVENT_REMOVE_ITEM;
					break;
				}
			}

			auto createMoveEvent = [&](MoveEvent_t type) {
				// We create a MoveEvent on the stack using moveEvents' script interface
				MoveEvent moveevent(g_moveEvents->getScriptInterfacePtr());
				moveevent.setEventType(type);
				moveevent.fromItem = true;
				moveevent.addItemId(it.id);

				if (type == MOVE_EVENT_EQUIP) {
					moveevent.equipFunction = MoveEvent::EquipItem;
				} else if (type == MOVE_EVENT_DEEQUIP) {
					moveevent.equipFunction = MoveEvent::DeEquipItem;
				} else if (type == MOVE_EVENT_STEP_IN) {
					moveevent.stepFunction = MoveEvent::StepInField;
				} else if (type == MOVE_EVENT_STEP_OUT) {
					moveevent.stepFunction = MoveEvent::StepOutField;
				} else if (type == MOVE_EVENT_ADD_ITEM_ITEMTILE) {
					moveevent.moveFunction = MoveEvent::AddItemField;
				} else if (type == MOVE_EVENT_REMOVE_ITEM) {
					moveevent.moveFunction = MoveEvent::RemoveItemField;
				}

				// Parse sub-attributes
				std::list<std::string> vocStringList;
				for (const auto& subNode : attributeNode.children()) {
					pugi::xml_attribute subKey = subNode.attribute("key");
					if (!subKey) continue;
					pugi::xml_attribute subValue = subNode.attribute("value");
					if (!subValue) continue;

					std::string key = boost::algorithm::to_lower_copy<std::string>(subKey.as_string());

					if (key == "slot" && (type == MOVE_EVENT_EQUIP || type == MOVE_EVENT_DEEQUIP)) {
						std::string slotName = boost::algorithm::to_lower_copy<std::string>(subValue.as_string());
						if (slotName == "head") moveevent.setSlot(SLOTP_HEAD);
						else if (slotName == "necklace") moveevent.setSlot(SLOTP_NECKLACE);
						else if (slotName == "backpack") moveevent.setSlot(SLOTP_BACKPACK);
						else if (slotName == "armor" || slotName == "body") moveevent.setSlot(SLOTP_ARMOR);
						else if (slotName == "right-hand") moveevent.setSlot(SLOTP_RIGHT);
						else if (slotName == "left-hand") moveevent.setSlot(SLOTP_LEFT);
						else if (slotName == "hand" || slotName == "shield") moveevent.setSlot(SLOTP_RIGHT | SLOTP_LEFT);
						else if (slotName == "legs") moveevent.setSlot(SLOTP_LEGS);
						else if (slotName == "feet") moveevent.setSlot(SLOTP_FEET);
						else if (slotName == "ring") moveevent.setSlot(SLOTP_RING);
						else if (slotName == "ammo") moveevent.setSlot(SLOTP_AMMO);
						else if (slotName == "two-handed") moveevent.setSlot(SLOTP_TWO_HAND);

						if (type == MoveEvent_t::MOVE_EVENT_EQUIP && moveevent.getSlot() != SlotPositionBits::SLOTP_WHEREEVER) {
							it.slotPosition = moveevent.getSlot();
						}
					} else if (key == "level") {
						moveevent.setRequiredLevel(subValue.as_uint());
						moveevent.setWieldInfo(WIELDINFO_LEVEL);
					} else if (key == "maglevel") {
						moveevent.setRequiredMagLevel(subValue.as_uint());
						moveevent.setWieldInfo(WIELDINFO_MAGLV);
					} else if (key == "premium") {
						if (subValue.as_bool()) {
							moveevent.setNeedPremium(true);
							moveevent.setWieldInfo(WIELDINFO_PREMIUM);
						}
					} else if (key == "reset") {
						it.minReqReset = subValue.as_uint();
						moveevent.setWieldInfo(WIELDINFO_RESETS);
					} else if (key == "vocation") {
						std::string vocations = subValue.as_string();
						std::istringstream vss(vocations);
						std::string vtoken;
						while (std::getline(vss, vtoken, ',')) {
							// trim
							vtoken.erase(vtoken.begin(), std::find_if(vtoken.begin(), vtoken.end(), [](unsigned char ch) { return !std::isspace(ch); }));
							vtoken.erase(std::find_if(vtoken.rbegin(), vtoken.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), vtoken.end());

							// format: "vocname;true" or just "vocname"
							std::string vocName;
							bool showInDescription = false;
							std::istringstream inner(vtoken);
							std::getline(inner, vocName, ';');
							std::string showStr;
							std::getline(inner, showStr, ';');
							showInDescription = (showStr == "true");

							moveevent.addVocationEquipSet(vocName);
							moveevent.setWieldInfo(WIELDINFO_VOCREQ);

							if (showInDescription) {
								vocStringList.push_back(boost::algorithm::to_lower_copy(vocName) + "s");
							}
						}
					}
				}

				// Build vocation string
				if (!vocStringList.empty()) {
					std::string vocStr;
					for (auto it2 = vocStringList.begin(); it2 != vocStringList.end(); ++it2) {
						if (!vocStr.empty()) {
							auto next = it2;
							++next;
							if (next == vocStringList.end()) {
								vocStr += " and ";
							} else {
								vocStr += ", ";
							}
						}
						vocStr += *it2;
					}
					moveevent.setVocationString(vocStr);
				}

				// Update ItemType wield info from the equip moveevent
				if (type == MOVE_EVENT_EQUIP) {
					it.wieldInfo = moveevent.getWieldInfo();
					it.minReqLevel = moveevent.getReqLevel();
					it.minReqMagicLevel = moveevent.getReqMagLv();
					it.vocationString = moveevent.getVocationString();
				}

				// Register the moveevent
				g_moveEvents->addEvent(std::move(moveevent), it.id, g_moveEvents->getItemIdMap());
			};

			if (eventType == MOVE_EVENT_NONE) {
				// Default: create both equip and deequip
				createMoveEvent(MOVE_EVENT_EQUIP);
				createMoveEvent(MOVE_EVENT_DEEQUIP);
			} else {
				createMoveEvent(eventType);
			}

		} else if (scriptToken == "weapon") {
			// Determine weapon type from sub-attributes
			WeaponType_t weaponType = it.weaponType;
			for (const auto& subNode : attributeNode.children()) {
				pugi::xml_attribute subKey = subNode.attribute("key");
				if (!subKey) continue;
				pugi::xml_attribute subValue = subNode.attribute("value");
				if (!subValue) continue;

				std::string key = boost::algorithm::to_lower_copy<std::string>(subKey.as_string());
				if (key == "weapontype") {
					std::string wt = boost::algorithm::to_lower_copy<std::string>(subValue.as_string());
					auto found = WeaponTypesMap.find(wt);
					if (found != WeaponTypesMap.end()) {
						weaponType = found->second;
					}
					break;
				}
			}

			if (weaponType == WEAPON_NONE) {
				LOG_WARN(fmt::format("[Warning - Items::parseScriptAttribute] No weapon type for item id: {}", it.id));
				continue;
			}

			LuaScriptInterface* weaponInterface = g_weapons->getScriptInterfacePtr();
			std::unique_ptr<Weapon> weapon;

			if (weaponType == WEAPON_DISTANCE || weaponType == WEAPON_AMMO) {
				weapon = std::make_unique<WeaponDistance>(weaponInterface);
			} else if (weaponType == WEAPON_WAND) {
				weapon = std::make_unique<WeaponWand>(weaponInterface);
			} else {
				weapon = std::make_unique<WeaponMelee>(weaponInterface);
			}

			weapon->weaponType = weaponType;
			it.weaponType = weaponType;
			weapon->configureWeapon(it);

			// Parse sub-attributes for weapon
			int32_t fromDamage = 0;
			int32_t toDamage = 0;
			std::list<std::string> vocStringList;

			for (const auto& subNode : attributeNode.children()) {
				pugi::xml_attribute subKey = subNode.attribute("key");
				if (!subKey) continue;
				pugi::xml_attribute subValue = subNode.attribute("value");
				if (!subValue) continue;

				std::string key = boost::algorithm::to_lower_copy<std::string>(subKey.as_string());

				if (key == "level") {
					weapon->setRequiredLevel(subValue.as_uint());
					weapon->setWieldInfo(WIELDINFO_LEVEL);
				} else if (key == "maglevel") {
					weapon->setRequiredMagLevel(subValue.as_uint());
					weapon->setWieldInfo(WIELDINFO_MAGLV);
				} else if (key == "unproperly") {
					weapon->setWieldUnproperly(subValue.as_bool());
				} else if (key == "action") {
					std::string action = boost::algorithm::to_lower_copy<std::string>(subValue.as_string());
					if (action == "removecharge") weapon->action = WEAPONACTION_REMOVECHARGE;
					else if (action == "removecount") weapon->action = WEAPONACTION_REMOVECOUNT;
					else if (action == "move") weapon->action = WEAPONACTION_MOVE;
				} else if (key == "breakchance") {
					weapon->setBreakChance(std::min<uint8_t>(100, static_cast<uint8_t>(subValue.as_uint())));
				} else if (key == "mana") {
					weapon->setMana(subValue.as_uint());
				} else if (key == "soul") {
					weapon->setSoul(subValue.as_uint());
				} else if (key == "premium") {
					weapon->setNeedPremium(subValue.as_bool());
					if (subValue.as_bool()) {
						weapon->setWieldInfo(WIELDINFO_PREMIUM);
					}
				} else if (key == "fromdamage") {
					fromDamage = subValue.as_int();
				} else if (key == "todamage") {
					toDamage = subValue.as_int();
				} else if (key == "wandtype") {
					std::string elementName = boost::algorithm::to_lower_copy<std::string>(subValue.as_string());
					if (elementName == "earth") weapon->params.combatType = COMBAT_EARTHDAMAGE;
					else if (elementName == "ice") weapon->params.combatType = COMBAT_ICEDAMAGE;
					else if (elementName == "energy") weapon->params.combatType = COMBAT_ENERGYDAMAGE;
					else if (elementName == "fire") weapon->params.combatType = COMBAT_FIREDAMAGE;
					else if (elementName == "death") weapon->params.combatType = COMBAT_DEATHDAMAGE;
					else if (elementName == "holy") weapon->params.combatType = COMBAT_HOLYDAMAGE;
				} else if (key == "slot") {
					std::string slotName = boost::algorithm::to_lower_copy<std::string>(subValue.as_string());
					if (slotName == "two-handed") {
						it.slotPosition = SLOTP_TWO_HAND;
					} else {
						it.slotPosition = SLOTP_HAND;
					}
				} else if (key == "vocation") {
					std::string vocations = subValue.as_string();
					std::istringstream vss(vocations);
					std::string vtoken;
					while (std::getline(vss, vtoken, ',')) {
						vtoken.erase(vtoken.begin(), std::find_if(vtoken.begin(), vtoken.end(), [](unsigned char ch) { return !std::isspace(ch); }));
						vtoken.erase(std::find_if(vtoken.rbegin(), vtoken.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), vtoken.end());

						std::string vocName;
						bool showInDescription = false;
						std::istringstream inner(vtoken);
						std::getline(inner, vocName, ';');
						std::string showStr;
						std::getline(inner, showStr, ';');
						showInDescription = (showStr == "true");

						weapon->addVocationWeaponSet(vocName);
						weapon->setWieldInfo(WIELDINFO_VOCREQ);

						if (showInDescription) {
							vocStringList.push_back(boost::algorithm::to_lower_copy(vocName) + "s");
						}
					}
				} else if (key == "reset") {
					weapon->setRequiredReset(subValue.as_uint());
					weapon->setWieldInfo(WIELDINFO_RESETS);
				}
			}

			// Configure wand damage
			if (WeaponWand* wand = dynamic_cast<WeaponWand*>(weapon.get())) {
				wand->setMinChange(fromDamage);
				wand->setMaxChange(toDamage);
				wand->configureWeapon(it);
			}

			// Build vocation string
			if (!vocStringList.empty()) {
				std::string vocStr;
				for (auto it2 = vocStringList.begin(); it2 != vocStringList.end(); ++it2) {
					if (!vocStr.empty()) {
						auto next = it2;
						++next;
						if (next == vocStringList.end()) {
							vocStr += " and ";
						} else {
							vocStr += ", ";
						}
					}
					vocStr += *it2;
				}
				weapon->setVocationString(vocStr);
			}

			// Update ItemType with wield info
			if (weapon->getWieldInfo() != 0) {
				it.wieldInfo = weapon->getWieldInfo();
				it.vocationString = weapon->getVocationString();
				it.minReqLevel = weapon->getReqLevel();
				it.minReqMagicLevel = weapon->getReqMagLv();
			}

			// Register weapon
			weapon->fromItem = true;
			g_weapons->registerLuaEvent(std::move(weapon));
		}
	}
}

void Items::buildInventoryList()
{
	inventory.reserve(items.size());
	for (const auto& type : items) {
		if (type.weaponType != WEAPON_NONE || type.ammoType != AMMO_NONE || type.attack != 0 || type.defense != 0 ||
		    type.extraDefense != 0 || type.armor != 0 || type.slotPosition & SLOTP_NECKLACE ||
		    type.slotPosition & SLOTP_RING || type.slotPosition & SLOTP_AMMO || type.slotPosition & SLOTP_FEET ||
		    type.slotPosition & SLOTP_HEAD || type.slotPosition & SLOTP_ARMOR || type.slotPosition & SLOTP_LEGS) {
			inventory.push_back(type.id);
		}
	}
	inventory.shrink_to_fit();
	std::ranges::sort(inventory);
}

ItemType& Items::getItemType(size_t id)
{
	if (id < items.size()) {
		return items[id];
	}
	return items.front();
}

const ItemType& Items::getItemType(size_t id) const
{
	if (id < items.size()) {
		return items[id];
	}
	return items.front();
}

uint16_t Items::getItemIdByName(const std::string& name)
{
	if (name.empty()) {
		return 0;
	}

	auto result = nameToItems.find(boost::algorithm::to_lower_copy<std::string>(name));
	if (result == nameToItems.end()) return 0;

	return result->second;
}

std::string ItemType::pluralString;

std::string_view ItemType::getPluralName() const
{
	if (!pluralName.empty()) {
		return pluralName;
	}

	if (showCount == 0) {
		return name;
	}

	if (name.empty() || name.back() == 's') {
		return name;
	}

	pluralString.reserve(name.size() + 1);
	pluralString.assign(name);
	pluralString.push_back('s');
	return pluralString;
}

Items::~Items()
{
	nameToItems.clear();
	currencyItems.clear();
	items.clear();
	inventory.clear();
}
