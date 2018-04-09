#pragma once
#include <vector>

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

struct starcraft_map
{
	uint32_t scenario_type;
	uint16_t version_code;

	uint8_t player_data[12];
	uint8_t races[12];

	uint16_t tileset;
	map_dimensions dimensions = { 0, 0 };
	uint16_t map_data[256 * 256];
	uint8_t map_visibility[256 * 256];
};

struct starcraft_map_file
{
	starcraft_map map;
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

struct starcraft_parse_status
{
	int error_code;
	uint16_t failedHeader;
};

void parse_starcraft_map(const char* mapfile, starcraft_map_file& scm, starcraft_parse_status& status);