// Copyright 2023 The Forgotten Server Authors. All rights reserved.
// Use of this source code is governed by the GPL-2.0 License that can be found in the LICENSE file.

#pragma once

#include <cstdint>

enum itemgroup_t
{
	ITEM_GROUP_NONE,

	ITEM_GROUP_GROUND,
	ITEM_GROUP_CONTAINER,
	ITEM_GROUP_WEAPON,     // deprecated
	ITEM_GROUP_AMMUNITION, // deprecated
	ITEM_GROUP_ARMOR,      // deprecated
	ITEM_GROUP_CHARGES,
	ITEM_GROUP_TELEPORT,   // deprecated
	ITEM_GROUP_MAGICFIELD, // deprecated
	ITEM_GROUP_WRITEABLE,  // deprecated
	ITEM_GROUP_KEY,        // deprecated
	ITEM_GROUP_SPLASH,
	ITEM_GROUP_FLUID,
	ITEM_GROUP_DOOR, // deprecated
	ITEM_GROUP_DEPRECATED,

	ITEM_GROUP_LAST
};

// 8.6 .dat flags
enum class ItemDatFlag : uint8_t {
	Ground           = 0,
	GroundBorder     = 1,
	OnBottom         = 2,
	OnTop            = 3,
	Container        = 4,
	Stackable        = 5,
	ForceUse         = 6,
	MultiUse         = 7,
	Writable         = 8,
	WritableOnce     = 9,
	FluidContainer   = 10,
	Fluid            = 11,
	IsUnpassable     = 12,
	IsUnmoveable     = 13,
	BlockMissiles    = 14,
	BlockPathfinder  = 15,
	Pickupable       = 16,
	Hangable         = 17,
	IsHorizontal     = 18,
	IsVertical       = 19,
	Rotatable        = 20,
	HasLight         = 21,
	DontHide         = 22,
	Translucent      = 23,
	HasOffset        = 24,
	HasElevation     = 25,
	Lying            = 26,
	AnimateAlways    = 27,
	Minimap          = 28,
	LensHelp         = 29,
	FullGround       = 30,
	IgnoreLook       = 31,
	Cloth            = 32,
	MarketItem       = 33,
	LastFlag         = 255
};
