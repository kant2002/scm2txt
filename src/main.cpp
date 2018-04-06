#include <iostream>
#include <string>
#include <StormLib.h>

using namespace std;

const char* SCM_INTERNAL_FILE = "staredit\\scenario.chk";

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

	if (!SFileExtractFile(hArchive, SCM_INTERNAL_FILE, chkFile.c_str(), SFILE_OPEN_FROM_MPQ))
	{
		cerr << "Failed to extract the list of files" << endl;
		SFileCloseArchive(hArchive);
		return -1;
	}

	SFileCloseArchive(hArchive);
	return 0;
}
