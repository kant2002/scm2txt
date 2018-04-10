#pragma once
#include <array>
#include <vector>
#include <functional>

struct cv5_entry {
	uint16_t flags;
	std::array<uint16_t, 16> mega_tile_index;
};
struct vf4_entry {
	enum flags_t : uint16_t {
		flag_walkable = 1,
		flag_middle = 2,
		flag_high = 4,
		flag_very_high = 8
	};

	std::array<uint16_t, 16> flags;
};

struct tileset_data
{
	std::vector<cv5_entry> cv5;
	std::vector<vf4_entry> vf4;
};

constexpr int StarcraftTilesetParse_Success = 0;
constexpr int StarcraftTilesetParse_FileOpenError = 1;
constexpr int StarcraftTilesetParse_InvalidStarcraftDirectory = 2;
constexpr int StarcraftTilesetParse_CV5Missing = 3;
constexpr int StarcraftTilesetParse_VF4Missing = 4;
constexpr int StarcraftTilesetParse_ExtractCV5Failed = 5;
constexpr int StarcraftTilesetParse_ExtractVF4Failed = 6;

struct starcraft_tileset_parse_status
{
	int error_code;
};

typedef std::function<void(int tilesetIndex, tileset_data& tileset, starcraft_tileset_parse_status& status)> tileset_provider;

void parse_starcraft_tileset(const char* starcraftDir, int tilesetIndex, tileset_data& tileset, starcraft_tileset_parse_status& status);