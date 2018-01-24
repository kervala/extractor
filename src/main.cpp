/*
 *  HotlineMiami WAD Extractor is a tool to extract all files from
 *  WAD file used by Hotline Miami 1 & 2
 *  Copyright (C) 2016-2018  Cedric OCHS
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common.h"
#include "utils.h"

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

int usage(const std::string &filename)
{
	printf("Usage: %s [FILE]\n", filename.c_str());
	return EXIT_FAILURE;    
}

int main(int argc, char *argv[])
{
#if defined(_MSC_VER) && defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	int ret = EXIT_SUCCESS;

	// print usage if no argument specified
	if (argc != 2) return usage(argv[0]);

	std::string filename = argv[1];

	// try to open the WAD file
	FILE *file = fopen(filename.c_str(), "rb");

	if (!file)
	{
		fprintf(stderr, "Unable to open file %s\n", filename.c_str());
		return usage(argv[0]);
	}

	// read header if any
	char header[4];

	if (fread(&header, sizeof(char), 4, file) == 4)
	{
		// Hotline Miama 2 WAD files begin with AGAR
		if (memcmp(&header, "AGAR", 4) == 0)
		{
			// Hotline Miami 2
			if (!extractVersion2(file)) ret = EXIT_SUCCESS;
		}
		else
		{
			// Hotline Miami 1

			// rewind because we need to reread from beginning
			fseek(file, 0, SEEK_SET);

			if (!extractVersion1(file)) ret = EXIT_SUCCESS;
		}
	}
	else
	{
		fprintf(stderr, "Error reading header of file %s\n", filename.c_str());

		ret = EXIT_FAILURE;
	}

	// close the WAD file
	fclose(file);

	return ret;
}

