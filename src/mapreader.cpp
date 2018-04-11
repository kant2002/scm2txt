#include "mapreader.h"
#include <iosfwd>
#include <iostream>
#include <StormLib.h>
#include "mpq.h"
using namespace std;

constexpr uint32_t to_code(char section_name[4])
{
	return (section_name[3] << 24) +
		(section_name[2] << 16) +
		(section_name[1] << 8) +
		(section_name[0] << 0);
}

struct chunkheader
{
	uint32_t szName;
	uint32_t size;
};

struct map_version
{
	uint16_t code;
};
struct verification_code
{
	uint32_t seed[265];
	uint8_t opcodes[16];
};

const char* SCM_INTERNAL_FILE = "staredit\\scenario.chk";



template <typename T>
void read_data(mpq_file_stream& map, T& data)
{
	map.read(reinterpret_cast<char*>(&data), sizeof(T));
}

void skip_header(mpq_file_stream& map, chunkheader& header)
{
#if USE_BOOST_STREAMS
	boost::iostreams::seek(map, header.size, std::ios_base::cur);
#else
	map.seekg(header.size, std::ios_base::cur);
#endif
}

void tiles_flags_and(starcraft_map& scm, starcraft_map_info& info, size_t offset_x, size_t offset_y, size_t width, size_t height, int flags) {
	for (size_t y = offset_y; y != offset_y + height; ++y) {
		for (size_t x = offset_x; x != offset_x + width; ++x) {
			info.tiles[x + y * scm.dimensions.width].flags &= flags;
		}
	}
}
void tiles_flags_or(starcraft_map& scm, starcraft_map_info& info, size_t offset_x, size_t offset_y, size_t width, size_t height, int flags) {
	for (size_t y = offset_y; y != offset_y + height; ++y) {
		for (size_t x = offset_x; x != offset_x + width; ++x) {
			info.tiles[x + y * scm.dimensions.width].flags |= flags;
		}
	}
}

void parse_race(mpq_file_stream& map, starcraft_map& scm)
{
	read_data(map, scm.races);
}

void parse_map_type(mpq_file_stream& map, starcraft_map& scm)
{
	read_data(map, scm.scenario_type);
}

void parse_map_version(mpq_file_stream& map, starcraft_map& scm)
{
	map_version version;
	read_data(map, version);
	scm.version_code = version.code;
}

void parse_player_types(mpq_file_stream& map, starcraft_map& scm)
{
	read_data(map, scm.player_data);
}

void parse_tileset(mpq_file_stream& map, starcraft_map& scm)
{
	read_data(map, scm.tileset);
}

void parse_map_data(mpq_file_stream& map, starcraft_map& scm, starcraft_map_info& info)
{
	map.read(reinterpret_cast<char*>(scm.map_data), sizeof(scm.map_data[0]) * scm.dimensions.width * scm.dimensions.height);
	info.tiles.resize(scm.dimensions.width * scm.dimensions.height);
	info.tiles_mega_tile_index.resize(scm.dimensions.width * scm.dimensions.height);

	for (size_t i = 0; i != scm.dimensions.width * scm.dimensions.height; ++i) {
		tile_id tile_id(scm.map_data[i]);
		if (tile_id.group_index() >= info.tileset_data.cv5.size())
		{
			tile_id = {};
		}

		auto tileGroup = info.tileset_data.cv5.at(tile_id.group_index());
		size_t megatile_index = tileGroup.mega_tile_index[tile_id.subtile_index()];
		auto flagsToClear = tile_t::flag_walkable
			| tile_t::flag_unwalkable
			| tile_t::flag_very_high
			| tile_t::flag_middle
			| tile_t::flag_high
			| tile_t::flag_partially_walkable;
		int cv5_flags = tileGroup.flags & ~flagsToClear;
		info.tiles_mega_tile_index[i] = (uint16_t)megatile_index;
		info.tiles[i].flags = info.mega_tile_flags.at(megatile_index) | cv5_flags;
		if (tile_id.has_creep()) {
			info.tiles_mega_tile_index[i] |= 0x8000;
			info.tiles[i].flags |= tile_t::flag_has_creep;
		}
	}

	tiles_flags_and(scm, info, 0, scm.dimensions.height - 2, 5, 1, ~(tile_t::flag_walkable | tile_t::flag_has_creep | tile_t::flag_partially_walkable));
	tiles_flags_or(scm, info, 0, scm.dimensions.height - 2, 5, 1, tile_t::flag_unbuildable);
	tiles_flags_and(scm, info, scm.dimensions.width - 5, scm.dimensions.height - 2, 5, 1, ~(tile_t::flag_walkable | tile_t::flag_has_creep | tile_t::flag_partially_walkable));
	tiles_flags_or(scm, info, scm.dimensions.width - 5, scm.dimensions.height - 2, 5, 1, tile_t::flag_unbuildable);

	tiles_flags_and(scm, info, 0, scm.dimensions.height - 1, scm.dimensions.width, 1, ~(tile_t::flag_walkable | tile_t::flag_has_creep | tile_t::flag_partially_walkable));
	tiles_flags_or(scm, info, 0, scm.dimensions.height - 1, scm.dimensions.width, 1, tile_t::flag_unbuildable);

	// regions_create();
}

void parse_placed_units(mpq_file_stream& map, chunkheader header, std::vector<unit_data>& units)
{
	int unitsCount = header.size / 36;
	for (auto i = 0; i < unitsCount; i++)
	{
		unit_data unit;
		read_data(map, unit);
		units.push_back(unit);
	}
}

void parse_fogofwar(mpq_file_stream& map, map_dimensions dimensions, uint8_t* map_data)
{
	map.read(reinterpret_cast<char*>(map_data), dimensions.width * dimensions.height);
}

void set_mega_tile_flags(starcraft_map_info& info, tileset_data& tileset)
{
	info.mega_tile_flags.resize(tileset.vf4.size());
	for (size_t i = 0; i != info.mega_tile_flags.size(); ++i) {
		int flags = 0;
		auto& mt = tileset.vf4[i];
		int walkable_count = 0;
		int middle_count = 0;
		int high_count = 0;
		int very_high_count = 0;
		for (size_t y = 0; y < 4; ++y) {
			for (size_t x = 0; x < 4; ++x) {
				if (mt.flags[y * 4 + x] & vf4_entry::flag_walkable)
				{
					++walkable_count;
				}

				if (mt.flags[y * 4 + x] & vf4_entry::flag_middle)
				{
					++middle_count;
				}

				if (mt.flags[y * 4 + x] & vf4_entry::flag_high)
				{
					++high_count;
				}

				if (mt.flags[y * 4 + x] & vf4_entry::flag_very_high)
				{
					++very_high_count;
				}
			}
		}

		if (walkable_count > 12)
		{
			flags |= tile_t::flag_walkable;
		}
		else
		{
			flags |= tile_t::flag_unwalkable;
		}

		if (walkable_count && walkable_count != 0x10)
		{
			flags |= tile_t::flag_partially_walkable;
		}

		if (high_count < 12 && middle_count + high_count >= 12)
		{
			flags |= tile_t::flag_middle;
		}

		if (high_count >= 12)
		{
			flags |= tile_t::flag_high;
		}

		if (very_high_count)
		{
			flags |= tile_t::flag_very_high;
		}

		info.mega_tile_flags[i] = flags;
	}
}

void parse_map(mpq_file_stream& map, tileset_provider provider, starcraft_map_file& scm, starcraft_parse_status& status)
{
	ZeroMemory(&scm.map, sizeof(scm.map));
	tileset_data tileset;
	starcraft_tileset_parse_status tileset_status;
	while (!map.eof() && map.good())
	{
		chunkheader header;
		map.read(reinterpret_cast<char*>(&header), sizeof(chunkheader));
		switch (header.szName)
		{
		case to_code("TYPE"):
			parse_map_type(map, scm.map);
			break;
		case to_code("VER "):
			parse_map_version(map, scm.map);
			break;
		case to_code("IVER"):
		case to_code("IVE2"):
			// Just read the data, since it has no meaning to us.
			skip_header(map, header);
			break;
		case to_code("VCOD"):
			// Skip verification code.
			skip_header(map, header);
			break;
		case to_code("OWNR"):
			parse_player_types(map, scm.map);
			break;
		case to_code("IOWN"):
			// parse_player_types("IOWNER", map);
			skip_header(map, header);
			break;
		case to_code("ERA "):
			parse_tileset(map, scm.map);
			provider(scm.map.tileset, tileset, tileset_status);
			if (tileset_status.error_code)
			{
				status.error_code = StarcraftMapParse_TilesetParseError;
				return;
			}

			set_mega_tile_flags(scm.info, tileset);
			scm.info.tileset_data = tileset;
			break;
		case to_code("DIM "):
			read_data(map, scm.map.dimensions);
			break;
		case to_code("SIDE"):
			parse_race(map, scm.map);
			break;
		case to_code("MTXM"):
			parse_map_data(map, scm.map, scm.info);
			// Now we could use tileset data during parsing phase
			break;
		case to_code("PUNI"):
		case to_code("UPGR"):
		case to_code("PTEC"):
			skip_header(map, header);
			break;
		case to_code("UNIT"):
			parse_placed_units(map, header, scm.units);
			break;
		case to_code("ISOM"):
		case to_code("TILE"):
		case to_code("DD2 "):
		case to_code("THG2"):
			skip_header(map, header);
			break;
		case to_code("MASK"):
			parse_fogofwar(map, scm.map.dimensions, scm.map.map_visibility);
			break;
		case to_code("STR "):
			skip_header(map, header);
			break;
		case to_code("UPRP"):
		case to_code("UPUS"):
			skip_header(map, header);
			break;
		case to_code("MRGN"):
			skip_header(map, header);
			break;
		case to_code("TRIG"):
		case to_code("MBRF"):
		case to_code("SPRP"):
		case to_code("FORC"):
		case to_code("WAV "):
		case to_code("UNIS"):
		case to_code("UPGS"):
		case to_code("TECS"):
		case to_code("SWNM"):
		case to_code("COLR"):
		case to_code("PUPx"):
		case to_code("PTEx"):
		case to_code("UNIx"):
		case to_code("UPGx"):
		case to_code("TECx"):
			skip_header(map, header);
			break;
		default:
			status.error_code = StarcraftMapParse_UnexpectedFileHeader;
			status.failedHeader = header.szName;
			return;
		}
	}

	status.error_code = StarcraftMapParse_Success;
}
void parse_starcraft_map(const char* mapFile, tileset_provider provider, starcraft_map_file& scm, starcraft_parse_status& status)
{
	HANDLE hArchive;
	if (!SFileOpenArchive(mapFile, 0, MPQ_OPEN_READ_ONLY, &hArchive))
	{
		status.error_code = StarcraftMapParse_FileOpenError;
		return;
	}

	if (!SFileHasFile(hArchive, SCM_INTERNAL_FILE))
	{
		status.error_code = StarcraftMapParse_InvalidMapFormat;
		SFileCloseArchive(hArchive);
		return;
	}

	{
		mpq_file_stream map(hArchive, SCM_INTERNAL_FILE);
		if (!map.component()->isValid())
		{
			status.error_code = StarcraftMapParse_ExtractMapFailed;
			SFileCloseArchive(hArchive);
			return;
		}

		parse_map(map, provider, scm, status);
	}
	
	SFileCloseArchive(hArchive);
}

bool starcraft_map_file::is_walkable(int walk_x, int walk_y)
{
	if (walk_y >= map.dimensions.height * 4 - 4)
	{
		return false;
	}

	if (walk_y >= map.dimensions.height * 4 - 8 && (walk_x < 20 || walk_x >= map.dimensions.width * 4 - 20))
	{
		return false;
	}

	// coordinates of the tile on the map.
	int tx = walk_x / 4;
	int ty = walk_y / 4;

	// coordinates of walk position inside tile
	int mx = walk_x % 4;
	int my = walk_y % 4;

	auto index = tx + ty * map.dimensions.width;
	auto& tile = info.tiles[index];
	if (tile.flags & tile_t::flag_has_creep)
	{
		return true;
	}

	if (tile.flags & tile_t::flag_partially_walkable) {
		tile_id tile_id(map.map_data[index]);
		auto tile_cv5 = info.tileset_data.cv5.at(tile_id.group_index());;
		size_t megatile_index = tile_cv5.mega_tile_index[tile_id.subtile_index()];

		int flags = info.tileset_data.vf4.at(megatile_index).flags[mx + my * 4];
		if (flags & vf4_entry::flag_walkable)
		{
			return true;
		}
	}

	if (tile.flags & tile_t::flag_walkable)
	{
		return true;
	}

	return false;
}
