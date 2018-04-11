#define BOOST_TEST_MODULE StandardTilesetTest
#include <boost/test/included/unit_test.hpp>
#include <boost/algorithm/string.hpp>
#include <string>
#include <mapreader.h>
#include <filesystem>
using namespace std;
namespace utf = boost::unit_test;

BOOST_AUTO_TEST_CASE(BadlandsLoaded)
{
	tileset_data tileset_data;
	starcraft_tileset_parse_status tileset_parse_status;
	load_standard_starcraft_tileset(0, tileset_data, tileset_parse_status);

	BOOST_TEST(tileset_parse_status.error_code == StarcraftTilesetParse_Success, "Failed to parse Badlands");
	BOOST_TEST(tileset_data.cv5.size() == 1666);
	BOOST_TEST(tileset_data.vf4.size() == 4845);
}

BOOST_AUTO_TEST_CASE(PlatformsLoaded)
{
	tileset_data tileset_data;
	starcraft_tileset_parse_status tileset_parse_status;
	load_standard_starcraft_tileset(1, tileset_data, tileset_parse_status);

	BOOST_TEST(tileset_parse_status.error_code == StarcraftTilesetParse_Success, "Failed to parse Platforms");
	BOOST_TEST(tileset_data.cv5.size() == 1514);
	BOOST_TEST(tileset_data.vf4.size() == 3056);
}

BOOST_AUTO_TEST_CASE(InstallationLoaded)
{
	tileset_data tileset_data;
	starcraft_tileset_parse_status tileset_parse_status;
	load_standard_starcraft_tileset(2, tileset_data, tileset_parse_status);

	BOOST_TEST(tileset_parse_status.error_code == StarcraftTilesetParse_Success, "Failed to parse Installation");
	BOOST_TEST(tileset_data.cv5.size() == 1266);
	BOOST_TEST(tileset_data.vf4.size() == 1432);
}

BOOST_AUTO_TEST_CASE(AshworldLoaded)
{
	tileset_data tileset_data;
	starcraft_tileset_parse_status tileset_parse_status;
	load_standard_starcraft_tileset(3, tileset_data, tileset_parse_status);

	BOOST_TEST(tileset_parse_status.error_code == StarcraftTilesetParse_Success, "Failed to parse Ashworld");
	BOOST_TEST(tileset_data.cv5.size() == 1263);
	BOOST_TEST(tileset_data.vf4.size() == 3498);
}

BOOST_AUTO_TEST_CASE(JungleLoaded)
{
	tileset_data tileset_data;
	starcraft_tileset_parse_status tileset_parse_status;
	load_standard_starcraft_tileset(4, tileset_data, tileset_parse_status);

	BOOST_TEST(tileset_parse_status.error_code == StarcraftTilesetParse_Success, "Failed to parse Jungle");
	BOOST_TEST(tileset_data.cv5.size() == 1579);
	BOOST_TEST(tileset_data.vf4.size() == 5047);
}

BOOST_AUTO_TEST_CASE(DesertLoaded)
{
	tileset_data tileset_data;
	starcraft_tileset_parse_status tileset_parse_status;
	load_standard_starcraft_tileset(5, tileset_data, tileset_parse_status);

	BOOST_TEST(tileset_parse_status.error_code == StarcraftTilesetParse_Success, "Failed to parse Desert");
	BOOST_TEST(tileset_data.cv5.size() == 1521);
	BOOST_TEST(tileset_data.vf4.size() == 5811);
}

BOOST_AUTO_TEST_CASE(IceLoaded)
{
	tileset_data tileset_data;
	starcraft_tileset_parse_status tileset_parse_status;
	load_standard_starcraft_tileset(6, tileset_data, tileset_parse_status);

	BOOST_TEST(tileset_parse_status.error_code == StarcraftTilesetParse_Success, "Failed to parse Ice");
	BOOST_TEST(tileset_data.cv5.size() == 1416);
	BOOST_TEST(tileset_data.vf4.size() == 4519);
}

BOOST_AUTO_TEST_CASE(TwilightLoaded)
{
	tileset_data tileset_data;
	starcraft_tileset_parse_status tileset_parse_status;
	load_standard_starcraft_tileset(7, tileset_data, tileset_parse_status);

	BOOST_TEST(tileset_parse_status.error_code == StarcraftTilesetParse_Success, "Failed to parse Twilight");
	BOOST_TEST(tileset_data.cv5.size() == 1495);
	BOOST_TEST(tileset_data.vf4.size() == 5233);
}

BOOST_AUTO_TEST_CASE(LargeTilesetDoNotSupported)
{
	tileset_data tileset_data;
	starcraft_tileset_parse_status tileset_parse_status;
	load_standard_starcraft_tileset(8, tileset_data, tileset_parse_status);

	BOOST_TEST(tileset_parse_status.error_code == StarcraftTilesetParse_TilesetIndexOutOfRange, "Out of range should be returned");
}

BOOST_AUTO_TEST_CASE(NegativeTilesetDoNotSupported)
{
	tileset_data tileset_data;
	starcraft_tileset_parse_status tileset_parse_status;
	load_standard_starcraft_tileset(-1, tileset_data, tileset_parse_status);

	BOOST_TEST(tileset_parse_status.error_code == StarcraftTilesetParse_TilesetIndexOutOfRange, "Out of range should be returned");
}