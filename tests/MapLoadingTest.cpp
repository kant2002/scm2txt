#include <direct.h>
#define BOOST_TEST_MODULE MapLoadingTest
#include <boost/test/included/unit_test.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <mapreader.h>
#include <filesystem>
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
	auto is_walkable = [&](int x, int y) {
		if (y >= scm.map.dimensions.height * 4 - 4)
		{
			return false;
		}

		if (y >= scm.map.dimensions.height * 4 - 8 && (x < 20 || x >= scm.map.dimensions.width * 4 - 20))
		{
			return false;
		}

		// coordinates of the tile on the map.
		int tx = x / 4;
		int ty = y / 4;

		// coordinates of walk position inside tile
		int mx = x % 4;
		int my = y % 4;

		auto index = tx + ty * scm.map.dimensions.width;
		auto& tile = scm.info.tiles[index];
		if (tile.flags & tile_t::flag_has_creep)
		{
			return true;
		}

		if (tile.flags & tile_t::flag_partially_walkable) {
			tile_id tile_id(scm.map.map_data[index]);
			auto tile_cv5 = scm.info.tileset_data.cv5.at(tile_id.group_index());;
			size_t megatile_index = tile_cv5.mega_tile_index[tile_id.subtile_index()];

			int flags = scm.info.tileset_data.vf4.at(megatile_index).flags[mx + my * 4];
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
	};

	size_t i = 0;
	for (auto y = 0; y < 4 * scm.map.dimensions.height; y++)
	{
		for (auto x = 0; x < 4 * scm.map.dimensions.width; x++)
		{
			auto expectedWalkable = expectedWalkableValues[i];
			auto currentWalkable = is_walkable(x, y); // (tile.flags & tile_t::flag_walkable) > 0;
			if (expectedWalkable)
			{
				// sort(begin(scm.map.map_data), end(scm.map.map_data));
				// auto it = std::unique(begin(scm.map.map_data), end(scm.map.map_data));
				int tx = x / 4;
				int ty = y / 4;
				auto index = tx + ty * scm.map.dimensions.width;
				BOOST_TEST(currentWalkable == true, "Item with position (" << tx << "," << ty << ") should be walkable" << scm.map.map_data[index]);
			}
			else
			{
				BOOST_TEST(currentWalkable == false, "Item with index (" << x << "," << y << ") should be non walkable");
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
BOOST_AUTO_TEST_CASE(WalkableFlagCalculatedCorrectly)
{
	starcraft_map_file scm;
	ZeroMemory(&scm.map, sizeof(scm.map));
	//starcraft_parse_status status;
	//parse_starcraft_map("data/simplemap/map.scm", load_standard_starcraft_tileset, scm, status);

	/*std::vector<bool> expectedWalkableValues;
	ifstream expectedWalkableFile("data/simplemap/walkable.txt");
	BOOST_TEST(load_map_bool(expectedWalkableFile, expectedWalkableValues), "Loading of walkable data failed");
		
	BOOST_TEST(status.error_code == StarcraftMapParse_Success, "Map file loading return status " << status.error_code);
	BOOST_TEST(scm.map.dimensions.width == 64, "Width of the map should be 64, but get " << scm.map.dimensions.width);
	BOOST_TEST(scm.map.dimensions.height == 64, "Height of the map should be 64, but get " << scm.map.dimensions.height);
	*/
	// verify_walkable(scm, expectedWalkableValues);
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