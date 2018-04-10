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