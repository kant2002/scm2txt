#pragma once
#include <vector>
#include "tilesetreader.h"

struct map_dimensions
{
	uint16_t width;
	uint16_t height;
};

struct unit_data
{
	uint32_t dwIndex;
	uint16_t x;
	uint16_t y;
	uint16_t wType;
	uint16_t wRelation;
	uint16_t wValidFlags;
	uint16_t wValidProperties;
	uint8_t  bOwner;
	uint8_t  bHitPoints;
	uint8_t  bShields;
	uint8_t  bEnergy;
	uint16_t wResources;
	uint16_t wUnused1;
	uint16_t wHanger;
	uint16_t wFlags;
	uint32_t dwUnused2;
	uint32_t dwRelatedTo;
};

struct map_location
{
	uint32_t left;
	uint32_t top;
	uint32_t right;
	uint32_t bottom;
	uint16_t nameIndex;
	uint16_t elevationFlags;
};

struct tile_id {
	uint16_t raw_value = 0;
	tile_id() = default;
	explicit tile_id(uint16_t raw_value) : raw_value(raw_value) {}
	explicit tile_id(size_t group_index, size_t subtile_index) : raw_value((uint16_t)(group_index << 4 | subtile_index)) {}
	bool has_creep() const {
		return ((raw_value >> 4) & 0x8000) != 0;
	}
	size_t group_index() const {
		return (raw_value >> 4) & 0x7ff;
	}
	size_t subtile_index() const {
		return raw_value & 0xf;
	}
	explicit operator bool() const {
		return raw_value != 0;
	}
};

struct tile_t {
	enum {
		flag_walkable = 1,
		flag_unk0 = 2,
		flag_unwalkable = 4,
		flag_unk1 = 8,
		flag_provides_cover = 0x10,
		flag_unk3 = 0x20,
		flag_has_creep = 0x40,
		flag_unbuildable = 0x80,
		flag_very_high = 0x100,
		flag_middle = 0x200,
		flag_high = 0x400,
		flag_occupied = 0x800,
		flag_creep_receding = 0x1000,
		flag_partially_walkable = 0x2000,
		flag_temporary_creep = 0x4000,
		flag_unk4 = 0x8000
	};
	uint8_t visible;
	uint8_t explored;
	uint16_t flags;
};

struct starcraft_map
{
	uint32_t scenario_type;
	uint16_t version_code;

	uint8_t player_data[12];
	uint8_t races[12];

	uint16_t tileset;

	map_dimensions dimensions = { 0, 0 };
	uint16_t map_data[256 * 256]; // same as tschmoo gfx_tiles
	uint8_t map_visibility[256 * 256];
};

struct starcraft_map_info
{
	tileset_data tileset_data;
	std::vector<uint16_t> mega_tile_flags;
	std::vector<tile_t> tiles;
	std::vector<uint16_t> tiles_mega_tile_index;
};

struct starcraft_map_file
{
	starcraft_map map;
	starcraft_map_info info;
	std::vector<unit_data> units;
};

constexpr uint16_t MapVersionStarcraft = 59;
constexpr uint16_t MapVersionHybrid = 63;
constexpr uint16_t MapVersionBroodWar = 205;

constexpr int StarcraftMapParse_Success = 0;
constexpr int StarcraftMapParse_FileOpenError = 1;
constexpr int StarcraftMapParse_InvalidMapFormat = 2;
constexpr int StarcraftMapParse_ExtractMapFailed = 3;
constexpr int StarcraftMapParse_UnexpectedFileHeader = 4;
constexpr int StarcraftMapParse_TilesetParseError = 5;

struct starcraft_parse_status
{
	int error_code;
	uint16_t failedHeader;
};

void parse_starcraft_map(const char* mapfile, tileset_provider provider, starcraft_map_file& scm, starcraft_parse_status& status);