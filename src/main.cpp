#include <string>
#include <vector>
#include <iostream>
#include "mapreader.h"
#include "tilesetreader.h"
using namespace std;

#define CHARLIST(x) (char)(x & 0xFF), (char)((x & 0xFF00) >> 8), (char)((x & 0xFF0000) >> 16), (char)((x & 0xFF000000) >> 24)


string to_name(uint32_t code)
{
	return string({ CHARLIST(code) });
}

void printUsage()
{
	cout << "scm2txt <mapfile> [<sc_dir>]" << endl;
}

struct map_type
{
	uint32_t scenario_type;
};

constexpr const char* map_version_name(uint16_t map_version)
{
	if (map_version == MapVersionStarcraft) {
		return "StarCraft";
	}

	if (map_version == MapVersionHybrid) {
		return "Hybrid";
	}

	if (map_version == MapVersionBroodWar) {
		return "BroodWar";
	}

	return "Unknown";
}

constexpr const char * decode_player_type(uint8_t player_type)
{
	switch (player_type)
	{
	case 0:
		return "Inactive";
	case 1:
		return "AI";
	case 2:
		return "Human";
	case 3:
		return "Rescue";
	case 4:
		return "Unused";
	case 5:
		return "Computer";
	case 6:
		return "Open";
	case 7:
		return "Neutral";
	case 8:
		return "Closed";
	default:
		return "Unknown";
	}
}

constexpr const char* decode_tileset(uint16_t tileset)
{
	switch (tileset & 7)
	{
	case 0:
		return "Badlands";
	case 1:
		return "Space";
	case 2:
		return "Installation";
	case 3:
		return "Ashworld";
	case 4:
		return "Jungle";
	case 5:
		return "Desert";
	case 6:
		return "Arctic";
	case 7:
		return "Twilight";
	}

	return "Unknown";
}

constexpr const char* decode_race(uint16_t race)
{
	switch (race & 7)
	{
	case 0:
		return "Zerg";
	case 1:
		return "Terran";
	case 2:
		return "Protoss";
	case 3:
		return "Independent";
	case 4:
		return "Neutral";
	case 5:
		return "Selectable";
	case 6:
		return "Random";
	case 7:
		return "Inactive";
	}

	return "Unknown";
}

void print_player_types(starcraft_map& scm)
{
	cout << "OWNER" << endl;
	for (auto i = 0; i < sizeof(scm.player_data); i++)
	{
		if (i != 0)
		{
			cout << ",";
		}

		cout << decode_player_type(scm.player_data[i]);
	}

	cout << endl;
}

void print_tileset(starcraft_map& scm)
{
	cout << "TILESET" << endl;
	cout << decode_tileset(scm.tileset) << endl;
}

void print_race(starcraft_map& scm)
{
	cout << "RACE" << endl;
	for (auto i = 0; i < sizeof(scm.races); i++)
	{
		if (i != 0)
		{
			cout << ",";
		}

		cout << decode_race(scm.races[i]);
	}

	cout << endl;
}

std::string print_tile(tile_t tile)
{
	return to_string(tile.visible) + "_" + to_string(tile.explored) + "_" + to_string(tile.flags);
}

void print_map_data(starcraft_map_file& scm)
{
	int counter = 0;
	cout << "MAP" << endl;
	cout << scm.map.dimensions.width << "," << scm.map.dimensions.height << endl;
	for (auto y = 0; y < scm.map.dimensions.height; y++)
	{
		for (auto x = 0; x < scm.map.dimensions.width; x++)
		{
			if (x != 0)
			{
				cout << ",";
			}

			//cout << scm.map_data[counter];
			cout << print_tile(scm.info.tiles[counter]);
			counter++;
		}

		cout << endl;
	}
}

void print_placed_units(const std::vector<unit_data>& units)
{
	cout << "UNITS" << endl;
	int unitsCount = units.size();
	cout << unitsCount << endl;
	for (const auto& unit : units)
	{
		// Let's skip this index, since it is no use in the portability map results
		// cout << unit.dwIndex << ",";
		cout << unit.x << ",";
		cout << unit.y << ",";
		cout << unit.wType << ",";
		cout << unit.wRelation << ",";
		cout << unit.wValidFlags << ",";
		cout << unit.wValidProperties << ",";
		cout << (uint16_t)unit.bOwner << ",";
		cout << (uint16_t)unit.bHitPoints << ",";
		cout << (uint16_t)unit.bShields << ",";
		cout << (uint16_t)unit.bEnergy << ",";
		cout << unit.wResources << ",";
		cout << unit.wHanger << ",";
		cout << unit.wFlags << ",";
		cout << unit.dwRelatedTo;
		cout << endl;
	}
}

void print_fogofwar(starcraft_map& scm)
{
	int counter = 0;
	cout << "FOGOFWAR" << endl;
	cout << scm.dimensions.width << "," << scm.dimensions.height << endl;
	for (auto y = 0; y < scm.dimensions.height; y++)
	{
		for (auto x = 0; x < scm.dimensions.width; x++)
		{
			if (x != 0)
			{
				cout << ",";
			}

			cout << (uint16_t)scm.map_visibility[counter];
			counter++;
		}

		cout << endl;
	}
}

void print_map_type(const starcraft_map& map)
{
	cout << "TYPE" << endl;
	cout << to_name(map.scenario_type) << endl;
}

void print_map_version(starcraft_map& map)
{
	cout << "VERSION" << endl;
	cout << map_version_name(map.version_code) << endl;
}

void print(starcraft_map_file& scm)
{
	print_map_type(scm.map);
	print_map_version(scm.map);
	print_player_types(scm.map);
	print_tileset(scm.map);
	print_race(scm.map);
	print_map_data(scm);
	print_placed_units(scm.units);
	print_fogofwar(scm.map);
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printUsage();
		return -1;
	}

	string mapFile(argv[1]);
	starcraft_map_file scm;
	starcraft_parse_status status;

	tileset_provider tilesetProviderFromBroodWarData = [&](int tilesetIndex, tileset_data& tileset, starcraft_tileset_parse_status& tileset_status)
	{
		parse_starcraft_tileset(argv[2], scm.map.tileset, tileset, tileset_status);
		switch (tileset_status.error_code)
		{
		case StarcraftTilesetParse_FileOpenError:
			cerr << "Failed to open the tileset file" << endl;
			break;
		case StarcraftTilesetParse_CV5Missing:
			cerr << "CV5 file for specified tileset is missing" << endl;
			break;
		case StarcraftTilesetParse_VF4Missing:
			cerr << "VF4 file for specified tileset is missing" << endl;
			break;
		}
	};
	auto tilesetProvider = (argc < 3)
		? load_standard_starcraft_tileset
		: tilesetProviderFromBroodWarData;

	cerr << "Opening '" << mapFile << "'..." << endl;
	parse_starcraft_map(mapFile.c_str(), tilesetProvider, scm, status);
	switch (status.error_code)
	{
	case StarcraftMapParse_FileOpenError:
		cerr << "Failed to open the file '" << mapFile << "'" << endl;
		break;
	case StarcraftMapParse_InvalidMapFormat:
		cerr << "File '" << mapFile << "' appearts to be not map file. staredit\\scenario.chk not present in the archive" << endl;
		break;
	case StarcraftMapParse_ExtractMapFailed:
		cerr << "Failed to extract the list of files" << endl;
		break;
	case StarcraftMapParse_UnexpectedFileHeader:
		cerr << "Unknown header name: " << to_name(status.failedHeader) << endl;
		break;
	case StarcraftMapParse_TilesetParseError:
		cerr << "Tileset parse error" << endl;
		break;
	default:
		print(scm);
		break;
	}

	return 0;
}
