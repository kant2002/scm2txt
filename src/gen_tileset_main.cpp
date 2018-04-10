#include <string>
#include <iostream>
#include <fstream>
#include "mapreader.h"
#include "tilesetreader.h"
using namespace std;

namespace {
	std::array<const char*, 8> tileset_names = {
		"badlands",
		"platform",
		"install",
		"ashworld",
		"jungle",
		"desert",
		"ice",
		"twilight"
	};
}

void printUsage()
{
	cout << "gen_tileset <sc_dir>" << endl;
}

void print_vf4_code(ostream& cout, string expression, std::vector<vf4_entry> vf4)
{
	cout << expression << " = {" << endl;
	for (auto entry : vf4)
	{
		cout << "{"
			<< entry.flags[0] << "," << entry.flags[1] << "," << entry.flags[2] << "," << entry.flags[3] << ","
			<< entry.flags[4] << "," << entry.flags[5] << "," << entry.flags[6] << "," << entry.flags[7] << ","
			<< entry.flags[8] << "," << entry.flags[9] << "," << entry.flags[10] << "," << entry.flags[11] << ","
			<< entry.flags[12] << "," << entry.flags[13] << "," << entry.flags[14] << "," << entry.flags[15] << ","
			<< "}," << endl;
	}

	cout << "};" << endl;
}
void print_cv5_code(ostream& cout, string expression, std::vector<cv5_entry> cv5)
{
	cout << expression << " = {" << endl;
	for (auto entry : cv5)
	{
		cout << "{" << entry.flags << ",{"
			<< entry.mega_tile_index[0] << "," << entry.mega_tile_index[1] << "," << entry.mega_tile_index[2] << "," << entry.mega_tile_index[3] << ","
			<< entry.mega_tile_index[4] << "," << entry.mega_tile_index[5] << "," << entry.mega_tile_index[6] << "," << entry.mega_tile_index[7] << ","
			<< entry.mega_tile_index[8] << "," << entry.mega_tile_index[9] << "," << entry.mega_tile_index[10] << "," << entry.mega_tile_index[11] << ","
			<< entry.mega_tile_index[12] << "," << entry.mega_tile_index[13] << "," << entry.mega_tile_index[14] << "," << entry.mega_tile_index[15] << ","
			<< "}}," << endl;
	}

	cout << "};" << endl;
}

void print_tileset_code_header(ostream& cout)
{
	cout << "#include <vector>" << endl;
	cout << "#include <array>" << endl;
	cout << "#include \"mapreader.h\"" << endl;
	cout << endl;
}

void print_tileset_code(ostream& cout, string prefix, tileset_data& tileset)
{
	print_vf4_code(cout, string("std::vector<vf4_entry> ") + prefix + "_vf4", tileset.vf4);
	print_cv5_code(cout, string("std::vector<cv5_entry> ") + prefix + "_cv5", tileset.cv5);
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printUsage();
		return -1;
	}

	string starcraftDir(argv[1]);

	tileset_data tileset;
	starcraft_tileset_parse_status tileset_status;
	ofstream generated_data("tileset.cpp");
	print_tileset_code_header(generated_data);

	for (auto i = 0; i < 8; i++)
	{
		parse_starcraft_tileset(argv[1], 0, tileset, tileset_status);
		switch (tileset_status.error_code)
		{
		case StarcraftTilesetParse_FileOpenError:
			cerr << "Failed to open the tileset file" << endl;
			break;
		case StarcraftTilesetParse_CV5Missing:
			cerr << "CV5 file for specified tileset is missing" << endl;
			break;
		case StarcraftTilesetParse_VF4Missing:
			cerr << "VF4 file for specified tileset is missing" << endl;
			break;
		default:
			print_tileset_code(generated_data, tileset_names[i], tileset);
			break;
		}
	}

	return 0;
}