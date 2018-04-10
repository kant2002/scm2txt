#include "tilesetreader.h"
#include <string>
#include <filesystem>
#include <StormLib.h>
#include "mpq.h"
using namespace std;
using namespace std::experimental::filesystem;

// Name of tileset prefixes as they appear in MPQ file
std::array<const char*, 8> tileset_names = {
	"badlands",
	"platform",
	"install",
	"AshWorld",
	"Jungle",
	"Desert",
	"Ice",
	"Twilight"
};

struct vf4_data_file_item
{
	uint16_t flags[16];
};

struct cv5_data_file_item
{
	uint16_t unk1;
	uint16_t flags;
	uint16_t unk2[8];
	uint16_t mega_tile_index[16];
};

extern std::vector<vf4_entry> badlands_vf4;
extern std::vector<cv5_entry> badlands_cv5;

extern std::vector<vf4_entry> platform_vf4;
extern std::vector<cv5_entry> platform_cv5;

extern std::vector<vf4_entry> install_vf4;
extern std::vector<cv5_entry> install_cv5;

extern std::vector<vf4_entry> ashworld_vf4;
extern std::vector<cv5_entry> ashworld_cv5;

extern std::vector<vf4_entry> jungle_vf4;
extern std::vector<cv5_entry> jungle_cv5;

extern std::vector<vf4_entry> desert_vf4;
extern std::vector<cv5_entry> desert_cv5;

extern std::vector<vf4_entry> ice_vf4;
extern std::vector<cv5_entry> ice_cv5;

extern std::vector<vf4_entry> twilight_vf4;
extern std::vector<cv5_entry> twilight_cv5;

template< typename... Args >
std::string format(const char* format, Args... args) {
	int length = std::snprintf(nullptr, 0, format, args...);
	assert(length >= 0);

	char* buf = new char[length + 1];
	std::snprintf(buf, length + 1, format, args...);

	std::string str(buf);
	delete[] buf;
	return std::move(str);
}

std::string getTileSetCV5File(int tilesetIndex)
{
	return format("tileset\\%s.cv5", tileset_names[tilesetIndex]);
}

std::string getTileSetVF4File(int tilesetIndex)
{
	return format("tileset\\%s.vf4", tileset_names[tilesetIndex]);
}

void load_standard_starcraft_tileset(int tilesetIndex, tileset_data& tileset, starcraft_tileset_parse_status& status)
{
	if (tilesetIndex < 0 || tilesetIndex > 7)
	{
		status.error_code = StarcraftTilesetParse_TilesetIndexOutOfRange;
		return;
	}

	tileset.cv5.clear();
	tileset.vf4.clear();
	switch (tilesetIndex)
	{
	case 0:
		copy(begin(badlands_cv5), end(badlands_cv5), back_inserter(tileset.cv5));
		copy(begin(badlands_vf4), end(badlands_vf4), back_inserter(tileset.vf4));
		break;
	case 1:
		copy(begin(platform_cv5), end(platform_cv5), back_inserter(tileset.cv5));
		copy(begin(platform_vf4), end(platform_vf4), back_inserter(tileset.vf4));
		break;
	case 2:
		copy(begin(install_cv5), end(install_cv5), back_inserter(tileset.cv5));
		copy(begin(install_vf4), end(install_vf4), back_inserter(tileset.vf4));
		break;
	case 3:
		copy(begin(ashworld_cv5), end(ashworld_cv5), back_inserter(tileset.cv5));
		copy(begin(ashworld_vf4), end(ashworld_vf4), back_inserter(tileset.vf4));
		break;
	case 4:
		copy(begin(jungle_cv5), end(jungle_cv5), back_inserter(tileset.cv5));
		copy(begin(jungle_vf4), end(jungle_vf4), back_inserter(tileset.vf4));
		break;
	case 5:
		copy(begin(desert_cv5), end(desert_cv5), back_inserter(tileset.cv5));
		copy(begin(desert_vf4), end(desert_vf4), back_inserter(tileset.vf4));
		break;
	case 6:
		copy(begin(ice_cv5), end(ice_cv5), back_inserter(tileset.cv5));
		copy(begin(ice_vf4), end(ice_vf4), back_inserter(tileset.vf4));
		break;
	case 7:
		copy(begin(twilight_cv5), end(twilight_cv5), back_inserter(tileset.cv5));
		copy(begin(twilight_vf4), end(twilight_vf4), back_inserter(tileset.vf4));
		break;
	default:
		status.error_code = StarcraftTilesetParse_TilesetIndexOutOfRange;
		return;
	}

	status.error_code = StarcraftTilesetParse_Success;
}

void parse_starcraft_tileset(const char* starcraftDir, int tilesetIndex, tileset_data& tileset, starcraft_tileset_parse_status& status)
{
	string normalizedStarcraftDir(starcraftDir);
	if (!normalizedStarcraftDir.empty())
	{
		if (normalizedStarcraftDir.back() != '/' && normalizedStarcraftDir.back() != '\\')
		{
			normalizedStarcraftDir += '/';
		}
	}

	HANDLE hArchive;
	string dataFile = normalizedStarcraftDir + (tilesetIndex < 5 ? "StarDat.mpq" : "BrooDat.mpq");
	if (!SFileOpenArchive(dataFile.c_str(), 1000, MPQ_OPEN_FORCE_MPQ_V1 | MPQ_OPEN_READ_ONLY, &hArchive))
	{
		status.error_code = StarcraftTilesetParse_FileOpenError;
		return;
	}

	auto cv5File = getTileSetCV5File(tilesetIndex);
	if (!SFileHasFile(hArchive, cv5File.c_str()))
	{
		status.error_code = StarcraftTilesetParse_CV5Missing;
		SFileCloseArchive(hArchive);
		return;
	}

	auto vf4File = getTileSetVF4File(tilesetIndex);
	if (!SFileHasFile(hArchive, vf4File.c_str()))
	{
		status.error_code = StarcraftTilesetParse_VF4Missing;
		SFileCloseArchive(hArchive);
		return;
	}

	mpq_file_stream cv5stream(hArchive, cv5File.c_str());
	if (!cv5stream.component()->isValid())
	{
		status.error_code = StarcraftTilesetParse_ExtractCV5Failed;
		SFileCloseArchive(hArchive);
		return;
	}

	mpq_file_stream vf4stream(hArchive, vf4File.c_str());
	if (!vf4stream.component()->isValid())
	{
		status.error_code = StarcraftTilesetParse_ExtractVF4Failed;
		SFileCloseArchive(hArchive);
		return;
	}

	std::vector<vf4_entry> vf4Items;
	while (vf4stream.good())
	{
		vf4_data_file_item data_item;
		vf4stream.read(reinterpret_cast<char*>(&data_item), sizeof(data_item));
		
		vf4_entry item;
		std::copy(std::begin(data_item.flags), std::end(data_item.flags), std::begin(item.flags));
		vf4Items.push_back(item);
	}

	std::vector<cv5_entry> cv5Items;
	while (cv5stream.good())
	{
		cv5_data_file_item data_item;
		cv5stream.read(reinterpret_cast<char*>(&data_item), sizeof(data_item));

		cv5_entry item;
		item.flags = data_item.flags;
		std::copy(std::begin(data_item.mega_tile_index), std::end(data_item.mega_tile_index), std::begin(item.mega_tile_index));

		cv5Items.push_back(item);
	}

	tileset.cv5 = cv5Items;
	tileset.vf4 = vf4Items;
	SFileCloseArchive(hArchive);
	status.error_code = StarcraftTilesetParse_Success;
}