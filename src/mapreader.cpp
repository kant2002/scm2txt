#include "mapreader.h"
#include <iosfwd>
#include <iostream>
#include <StormLib.h>
#if USE_BOOST_STREAMS
#include <boost/iostreams/categories.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/iostreams/device/file.hpp>
namespace io = boost::iostreams;
#endif
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

#if USE_BOOST_STREAMS
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

typedef io::stream<mpq_file_source> mpq_file_stream;
#else
class mpq_file_streambuf : public std::streambuf {
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
		if (!SFileReadFile(hFile, &ch, 1, &dwRead, NULL))
		{
			if (dwRead == 0)
			{
				return traits_type::eof();
			}
		}

		setg(&ch, &ch, &ch + 1);
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

typedef mpq_file_istream mpq_file_stream;
#endif

template <typename T>
void read_data(mpq_file_stream& map, T& data)
{
	map.read(reinterpret_cast<char*>(&data), sizeof(T));
}

void skip_header(mpq_file_stream& map, chunkheader& header)
{
#if USE_BOOST_STREAMS
	io::seek(map, header.size, std::ios_base::cur);
#else
	map.seekg(header.size, std::ios_base::cur);
#endif
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

void parse_map_data(mpq_file_stream& map, map_dimensions dimensions, uint16_t* map_data)
{
	map.read(reinterpret_cast<char*>(map_data), sizeof(map_data[0]) * dimensions.width * dimensions.height);
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

void parse_map(mpq_file_stream& map, starcraft_map_file& scm, starcraft_parse_status& status)
{
	ZeroMemory(&scm.map, sizeof(scm.map));
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
			break;
		case to_code("DIM "):
			read_data(map, scm.map.dimensions);
			break;
		case to_code("SIDE"):
			parse_race(map, scm.map);
			break;
		case to_code("MTXM"):
			parse_map_data(map, scm.map.dimensions, scm.map.map_data);
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
void parse_starcraft_map(const char* mapFile, starcraft_map_file& scm, starcraft_parse_status& status)
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

	mpq_file_stream map(hArchive, SCM_INTERNAL_FILE);
	if (!map.component()->isValid())
	{
		status.error_code = StarcraftMapParse_ExtractMapFailed;
		SFileCloseArchive(hArchive);
		return;
	}

	parse_map(map, scm, status);
	SFileCloseArchive(hArchive);
}
