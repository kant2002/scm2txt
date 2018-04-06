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

const char* SCM_INTERNAL_FILE = "staredit\\scenario.chk";

class mpq_file_source {
public:
	typedef char        char_type;
	typedef io::source_tag  category;
	
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

	/* Other members */
private:
	HANDLE hFile;
};

void printUsage()
{
	cout << "scm2txt <mapfile> <tempfile>" << endl;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		printUsage();
		return -1;
	}

	string mapFile(argv[1]);
	string chkFile(argv[2]);
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

	io::stream<mpq_file_source> map(hArchive, SCM_INTERNAL_FILE);
	if (!map.component()->isValid())
	{
		cerr << "Failed to extract the list of files" << endl;
		SFileCloseArchive(hArchive);
		return -1;
	}

	io::stream<io::file_sink> out(chkFile.c_str(), std::ios_base::binary | std::ios_base::trunc | std::ios_base::out);
	io::copy(map, out);
	//out.close();

	SFileCloseArchive(hArchive);
	return 0;
}
