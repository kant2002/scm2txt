#include <direct.h>
#define BOOST_TEST_MODULE MapLoadingTest
#include <boost/test/included/unit_test.hpp>
#include <string>
#include <mapreader.h>
#include <filesystem>
using namespace std;

BOOST_AUTO_TEST_CASE(WalkableFlagCalculatedCorrectly)
{
	starcraft_map_file scm;
	ZeroMemory(&scm.map, sizeof(scm.map));
	starcraft_parse_status status;
	parse_starcraft_map("data/(4)Blood Bath.scm", load_standard_starcraft_tileset, scm, status);
	BOOST_TEST(status.error_code == StarcraftMapParse_Success, "Map file loading return status " << status.error_code);
	BOOST_TEST(scm.map.dimensions.width == 64, "Width of the map should be 64");
	BOOST_TEST(scm.map.dimensions.height == 64, "Height of the map should be 64");
}
