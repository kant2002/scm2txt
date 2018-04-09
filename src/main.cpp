#include <iostream>
#include <string>
#include <iosfwd>
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>
#include <StormLib.h>
using namespace std;
namespace io = boost::iostreams;

#define CHARLIST(x) (char)(x & 0xFF), (char)((x & 0xFF00) >> 8), (char)((x & 0xFF0000) >> 16), (char)((x & 0xFF000000) >> 24)

constexpr uint32_t to_code(char section_name[4])
{
	return (section_name[3] << 24) +
		(section_name[2] << 16) +
		(section_name[1] << 8) +
		(section_name[0] << 0);
}

string to_name(uint32_t code)
{
	return string({ CHARLIST(code) });
}

const char* SCM_INTERNAL_FILE = "staredit\\scenario.chk";

class mpq_file_source {
public:
	typedef char        char_type;
	typedef io::seekable_device_tag  category;
	
	mpq_file_source(HANDLE hArchive, const char* fileName)
		: hFile(NULL)
	{
		if (!SFileOpenFileEx(hArchive, fileName, SFILE_OPEN_FROM_MPQ, &hFile))
		{
			hFile = NULL;
		}
	}

	void close()
	{
		if (hFile != NULL)
		{
			SFileCloseFile(hFile);
			hFile = NULL;
		}
	}

	bool isValid() const { return hFile != NULL; }

	std::streamsize read(char* s, std::streamsize n)
	{
		// Read up to n characters from the underlying data source
		// into the buffer s, returning the number of characters
		// read; return -1 to indicate EOF
		DWORD dwRead = 0;
		if (!SFileReadFile(hFile, s, n, &dwRead, NULL))
		{
			if (dwRead == 0)
			{
				return -1;
			}

			return dwRead;
		}

		return dwRead;
	}
	std::streamsize write(const char* s, std::streamsize n)
	{
		// Write up to n characters to the underlying 
		// data sink into the buffer s, returning the 
		// number of characters written
		return 0;
	}
	
	io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way)
	{
		// Seek to position off and return the new stream 
		// position. The argument way indicates how off is
		// interpretted:
		//    - std::ios_base::beg indicates an offset from the 
		//      sequence beginning 
		//    - std::ios_base::cur indicates an offset from the 
		//      current character position 
		//    - std::ios_base::end indicates an offset from the 
		//      sequence end 
		DWORD seekMethod = 0;
		if (way == std::ios_base::beg)
		{
			seekMethod = 0;
		}
		else if (way == std::ios_base::cur)
		{
			seekMethod = 1;
		}
		else if (way == std::ios_base::end)
		{
			seekMethod = 2;
		}

		int64_t position = static_cast<int64_t>(off);
		LONG hPosition = 0;
		return SFileSetFilePointer(hFile, static_cast<LONG>(position), &hPosition, seekMethod);
	}


	/* Other members */
private:
	HANDLE hFile;
};

class mpq_file_streambuf: public std::streambuf {
public:
	mpq_file_streambuf(HANDLE hArchive, const char* fileName)
		: hFile(NULL)
	{
		if (!SFileOpenFileEx(hArchive, fileName, SFILE_OPEN_FROM_MPQ, &hFile))
		{
			hFile = NULL;
		}
	}

	bool isValid() const { return hFile != NULL; }

protected:
	virtual int underflow()
	{
		DWORD dwRead = 0;
		setg(&ch, &ch, &ch + 1);
		if (!SFileReadFile(hFile, &ch, 1, &dwRead, NULL))
		{
			if (dwRead == 0)
			{
				return traits_type::eof();
			}

			return dwRead;
		}

		return dwRead;
	}
	pos_type seekoff(off_type off,
		std::ios_base::seekdir dir,
		std::ios_base::openmode which = std::ios_base::in)
	{
		DWORD seekMethod = 0;
		if (dir == std::ios_base::beg)
		{
			seekMethod = 0;
		}
		else if (dir == std::ios_base::cur)
		{
			seekMethod = 1;
		}
		else if (dir == std::ios_base::end)
		{
			seekMethod = 2;
		}

		int64_t position = static_cast<int64_t>(off);
		LONG hPosition = 0;
		return SFileSetFilePointer(hFile, static_cast<LONG>(position), &hPosition, seekMethod);
	}

private:
	char ch;
	HANDLE hFile;
};

class mpq_file_istream : public std::istream {
public:
	mpq_file_istream(HANDLE hArchive, const char* fileName)
		: istream(&_buf), _buf(hArchive, fileName)
	{
	}

	mpq_file_streambuf* component()
	{
		return &_buf;
	}
private:
	mpq_file_streambuf _buf;
};

// typedef io::stream<mpq_file_source> mpq_file_stream;
typedef mpq_file_istream mpq_file_stream;

void printUsage()
{
	cout << "scm2txt <mapfile>" << endl;
}

struct chunkheader
{
	uint32_t szName;
	uint32_t size;
};

struct map_type
{
	uint32_t scenario_type;
};

constexpr uint16_t MapVersionStarcraft = 59;
constexpr uint16_t MapVersionHybrid = 63;
constexpr uint16_t MapVersionBroodWar = 205;

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

struct map_version
{
	uint16_t code;
};
struct verification_code
{
	uint32_t seed[265];
	uint8_t opcodes[16];
};

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

template <typename T>
void read_data(mpq_file_stream& map, T& data)
{
	map.read(reinterpret_cast<char*>(&data), sizeof(T));
}

void skip_header(mpq_file_stream& map, chunkheader& header)
{
	io::seek(map, header.size, std::ios_base::cur);
}

void parse_map_type(mpq_file_stream& map)
{
	map_type map_type;
	read_data(map, map_type);
	cout << "TYPE" << endl;
	cout << to_name(map_type.scenario_type) << endl;
}

void parse_map_version(mpq_file_stream& map)
{
	map_version version;
	read_data(map, version);
	cout << "VERSION" << endl;
	cout << map_version_name(version.code) << endl;
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

void parse_player_types(const char* section_header, mpq_file_stream& map)
{
	uint8_t player_data[12];
	read_data(map, player_data);
	cout << section_header << endl;
	for (auto i = 0; i < sizeof(player_data); i++)
	{
		if (i != 0)
		{
			cout << ",";
		}

		cout << decode_player_type(player_data[i]);
	}

	cout << endl;
}

void parse_tileset(mpq_file_stream& map)
{
	uint16_t tileset;
	read_data(map, tileset);
	cout << "TILESET" << endl;
	cout << decode_tileset(tileset) << endl;
}

void parse_race(mpq_file_stream& map)
{
	uint8_t races[12];
	read_data(map, races);
	cout << "RACE" << endl;
	for (auto i = 0; i < sizeof(races); i++)
	{
		if (i != 0)
		{
			cout << ",";
		}

		cout << decode_race(races[i]);
	}

	cout << endl;
}

void parse_map_data(mpq_file_stream& map, map_dimensions dimensions, uint16_t* map_data)
{
	map.read(reinterpret_cast<char*>(map_data), 2 * dimensions.width * dimensions.height);
	int counter = 0;
	cout << "MAP" << endl;
	cout << dimensions.width << "," << dimensions.height << endl;
	for (auto y = 0; y < dimensions.height; y++)
	{
		for (auto x = 0; x < dimensions.width; x++)
		{
			if (x != 0)
			{
				cout << ",";
			}

			cout << map_data[counter];
			counter++;
		}

		cout << endl;
	}
}

void parse_placed_unit(mpq_file_stream& map, chunkheader header)
{
	cout << "UNITS" << endl;
	int unitsCount = header.size / 36;
	cout << unitsCount << endl;
	for (auto i = 0; i < unitsCount; i++)
	{
		unit_data unit;
		read_data(map, unit);

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

void parse_fogofwar(mpq_file_stream& map, map_dimensions dimensions, uint8_t* map_data)
{
	map.read(reinterpret_cast<char*>(map_data), dimensions.width * dimensions.height);
	int counter = 0;
	cout << "FOGOFWAR" << endl;
	cout << dimensions.width << "," << dimensions.height << endl;
	for (auto y = 0; y < dimensions.height; y++)
	{
		for (auto x = 0; x < dimensions.width; x++)
		{
			if (x != 0)
			{
				cout << ",";
			}

			cout << (uint16_t)map_data[counter];
			counter++;
		}

		cout << endl;
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printUsage();
		return -1;
	}

	string mapFile(argv[1]);
	HANDLE hArchive;

	cout << "Opening '" << mapFile << "'..." << endl;
	if (!SFileOpenArchive(mapFile.c_str(), 0, MPQ_OPEN_READ_ONLY, &hArchive))
	{
		cerr << "Failed to open the file '" << mapFile << "'" << endl;
		return -1;
	}

	if (!SFileHasFile(hArchive, SCM_INTERNAL_FILE))
	{
		cerr << "File '" << mapFile << "' appearts to be not map file. staredit\\scenario.chk not present in the archive" << endl;
		SFileCloseArchive(hArchive);
		return -1;
	}

	mpq_file_stream map(hArchive, SCM_INTERNAL_FILE);
	if (!map.component()->isValid())
	{
		cerr << "Failed to extract the list of files" << endl;
		SFileCloseArchive(hArchive);
		return -1;
	}

	map_dimensions dimensions = { 0, 0 };
	uint16_t map_data[256 * 256];
	uint8_t map_visibility[256 * 256];
	ZeroMemory(map_data, sizeof(map_data));
	while (!map.eof())
	{
		chunkheader header;
		map.read(reinterpret_cast<char*>(&header), sizeof(chunkheader));
		switch (header.szName)
		{
		case to_code("TYPE"):
			parse_map_type(map);
			break;
		case to_code("VER "):
			parse_map_version(map);
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
			parse_player_types("OWNER", map);
			break;
		case to_code("IOWN"):
			// parse_player_types("IOWNER", map);
			skip_header(map, header);
			break;
		case to_code("ERA "):
			parse_tileset(map);
			break;
		case to_code("DIM "):
			read_data(map, dimensions);
			break;
		case to_code("SIDE"):
			parse_race(map);
			break;
		case to_code("MTXM"):
			parse_map_data(map, dimensions, map_data);
			break;
		case to_code("PUNI"):
		case to_code("UPGR"):
		case to_code("PTEC"):
			skip_header(map, header);
			break;
		case to_code("UNIT"):
			parse_placed_unit(map, header);
			break;
		case to_code("ISOM"):
		case to_code("TILE"):
		case to_code("DD2 "):
		case to_code("THG2"):
			skip_header(map, header);
			break;
		case to_code("MASK"):
			parse_fogofwar(map, dimensions, map_visibility);
			break;
		case to_code("STR "):
			skip_header(map, header);
			break;
		case to_code("UPRP"):
		case to_code("UPUS"):
			skip_header(map, header);
			break;
		case to_code("MRGN"):
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
			cout << "Unknown header name: " << to_name(header.szName) << endl;
			SFileCloseArchive(hArchive);
			return -1;
		}
	}

	SFileCloseArchive(hArchive);
	return 0;
}
