#include <direct.h>
#define BOOST_TEST_MODULE MapLoadingTest
#include <boost/test/included/unit_test.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/test/data/test_case.hpp>
#include <string>
#include <mapreader.h>
#include <filesystem>
#include <bwgame.h>
using namespace std;
namespace utf = boost::unit_test;

bool load_map_bool(istream& stream, std::vector<bool>& map_flags)
{
	string line;
	if (!getline(stream, line))
	{
		return false;
	}
	
	stringstream dimensions_stream(line);
	uint16_t width;
	uint16_t height;
	dimensions_stream >> width >> height;
	map_flags.reserve(width * height);
	while (stream.good())
	{
		if (!getline(stream, line))
		{
			break;
		}

		std::istringstream row_stream(line);
		std::string token;
		while (std::getline(row_stream, token, ',')) {
			map_flags.push_back(token == "1" ? true : false);
		}
	}

	return map_flags.size() == (width * height);
}

void verify_walkable(starcraft_map_file& scm, std::vector<bool>& expectedWalkableValues)
{
	size_t i = 0;
	for (auto y = 0; y < 4 * scm.map.dimensions.height; y++)
	{
		for (auto x = 0; x < 4 * scm.map.dimensions.width; x++)
		{
			auto expectedWalkable = expectedWalkableValues[i];
			auto currentWalkable = scm.is_walkable(x, y);
			if (expectedWalkable)
			{
				int tx = x / 4;
				int ty = y / 4;
				auto index = tx + ty * scm.map.dimensions.width;
				BOOST_TEST(currentWalkable == true, "Item with position W(" << x << "," << y << ") T(" << tx << "," << ty << ") should be walkable (" << scm.map.map_data[index] << ")");
			}
			else
			{
				int tx = x / 4;
				int ty = y / 4;
				auto index = tx + ty * scm.map.dimensions.width;
				BOOST_TEST(currentWalkable == false, "Item with index W(" << x << "," << y << ") T(" << tx << "," << ty << ") should be non walkable (" << scm.map.map_data[index] << ")");
			}

			i++;
		}
	}
}

void verify_walkable(bwgame::game_load_functions& scm, std::vector<bool>& expectedWalkableValues)
{
	size_t i = 0;
	for (auto y = 0; y < scm.st.game->map_walk_height; y++)
	{
		for (auto x = 0; x < scm.st.game->map_walk_width; x++)
		{
			auto expectedWalkable = expectedWalkableValues[i];
			auto currentWalkable = scm.is_walkable(bwgame::xy(8 * x, 8 * y));
			if (expectedWalkable)
			{
				int tx = x / 4;
				int ty = y / 4;
				auto index = tx + ty * scm.st.game->map_tile_width;
				BOOST_TEST(currentWalkable == true, "Item with position W(" << x << "," << y << ") T(" << tx << "," << ty << ") should be walkable");
			}
			else
			{
				int tx = x / 4;
				int ty = y / 4;
				auto index = tx + ty * scm.st.game->map_tile_width;
				BOOST_TEST(currentWalkable == false, "Item with index W(" << x << "," << y << ") T(" << tx << "," << ty << ") should be non walkable");
			}

			i++;
		}
	}
}

void verify_buildable(starcraft_map_file& scm, std::vector<bool>& expectedBuildableValues)
{
	auto is_buildable = [&](size_t index) {
		auto& tile = scm.info.tiles[index];

		if ((tile.flags & tile_t::flag_unbuildable) != 0)
		{
			return false;
		}

		if ((tile.flags & tile_t::flag_partially_walkable) != 0)
		{
			return false;
		}

		if ((tile.flags & tile_t::flag_unwalkable) != 0)
		{
			return false;
		}

		return true;
	};

	size_t i = 0;
	for (auto tile : scm.info.tiles)
	{
		auto expectedBuildable = expectedBuildableValues[i];
		auto currentBuildable = is_buildable(i); // (tile.flags & tile_t::flag_unbuildable) == 0;
		if (expectedBuildable)
		{
			BOOST_TEST(currentBuildable == true, "Item with index " << i << " should be buildable");
		}
		else
		{
			BOOST_TEST(currentBuildable == false, "Item with index " << i << " should be non buildable");
		}

		i++;
	}
}

BOOST_AUTO_TEST_SUITE(suite1, *utf::enabled())

namespace {
	constexpr int MapsCount = 4;

	std::array<char const *, MapsCount> const mapNames = {
		"simplemap",
		"BloodBath",
		"faceoff",
		"waterworld",
	};
	std::array<int, MapsCount> mapWidths = {
		64,
		64,
		128,
		64,
	};
	std::array<int, MapsCount> mapHeights = {
		64,
		64,
		96,
		64,
	};
}

BOOST_DATA_TEST_CASE(WalkableFlagCalculatedCorrectly,
	mapNames ^ mapWidths ^ mapHeights,
	map, expectedWidth, expectedHeight)
{
	starcraft_map_file scm;
	ZeroMemory(&scm.map, sizeof(scm.map));
	starcraft_parse_status status;
	string mapTestFile = string("data/") + map + "/map.scm";
	parse_starcraft_map(mapTestFile.c_str(), load_standard_starcraft_tileset, scm, status);
	BOOST_TEST(status.error_code == StarcraftMapParse_Success, "Map file loading return status " << status.error_code);

	std::vector<bool> expectedWalkableValues;
	string expectedWalkableFileName = string("data/") + map + "/walkable.txt";
	ifstream expectedWalkableFile(expectedWalkableFileName);
	BOOST_TEST(load_map_bool(expectedWalkableFile, expectedWalkableValues), "Loading of walkable data failed");
		
	BOOST_TEST(scm.map.dimensions.width == expectedWidth, "Width of the map should be " << expectedWidth << ", but get " << scm.map.dimensions.width);
	BOOST_TEST(scm.map.dimensions.height == expectedHeight, "Height of the map should be " << expectedHeight << ", but get " << scm.map.dimensions.height);
	
	verify_walkable(scm, expectedWalkableValues);
}

BOOST_DATA_TEST_CASE(WalkableFlagCalculatedCorrectly2,
	mapNames ^ mapWidths ^ mapHeights,
	map, expectedWidth, expectedHeight)
{
const char* data_path = "D:\\Games\\Starcraft\\sc1.6.1\\Brood War";
bwgame::global_state gs;
bwgame::global_init(gs, bwgame::data_loading::data_files_directory(data_path));

bwgame::game_state game_st;
bwgame::state s;
s.global = &gs;
s.game = &game_st;
bwgame::game_load_functions loader(s);
string mapTestFile = string("data/") + map + "/map.scm";
//loader.load_map_file(mapTestFile.c_str());
loader.load_map_file("data/waterworld/map.scm");

	std::vector<bool> expectedWalkableValues;
	string expectedWalkableFileName = string("data/") + map + "/walkable.txt";
	ifstream expectedWalkableFile(expectedWalkableFileName);
	BOOST_TEST(load_map_bool(expectedWalkableFile, expectedWalkableValues), "Loading of walkable data failed");

	BOOST_TEST(s.game->map_tile_width, "Width of the map should be " << expectedWidth << ", but get " << s.game->map_tile_width);
	BOOST_TEST(s.game->map_tile_height == expectedHeight, "Height of the map should be " << expectedHeight << ", but get " << s.game->map_tile_height);

	verify_walkable(loader, expectedWalkableValues);
}

BOOST_AUTO_TEST_CASE(BuildableFlagCalculatedCorrectly, *utf::disabled())
{
	starcraft_map_file scm;
	ZeroMemory(&scm.map, sizeof(scm.map));
	starcraft_parse_status status;
	parse_starcraft_map("data/simplemap/map.scm", load_standard_starcraft_tileset, scm, status);

	std::vector<bool> expectedBuildableValues;
	ifstream expectedBuildableFile("data/simplemap/buildable.txt");
	BOOST_TEST(load_map_bool(expectedBuildableFile, expectedBuildableValues), "Loading of buildable data failed");

	BOOST_TEST(status.error_code == StarcraftMapParse_Success, "Map file loading return status " << status.error_code);
	BOOST_TEST(scm.map.dimensions.width == 64, "Width of the map should be 64");
	BOOST_TEST(scm.map.dimensions.height == 64, "Height of the map should be 64");

	verify_buildable(scm, expectedBuildableValues);
}
BOOST_AUTO_TEST_SUITE_END()