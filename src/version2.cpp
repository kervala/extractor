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


// used to store files in WAD
struct Entry2
{
	Entry2():size(0), offset(0)
	{
	}

	std::string filename;
	uint64_t size;
	uint64_t offset; // offset from start of content

	bool read(FILE *file)
	{
		// max length for a filename
		char buffer[BUFFER_SIZE];

		// read filename length
		uint32_t length;
		if (!readType(file, length)) return false;

		// length should never be longer than BUFFER_SIZE
		if (length >= BUFFER_SIZE)
		{
			fprintf(stderr, "Filename longer than %d\n", BUFFER_SIZE);
			return false;
		}

		// read filename into temporary buffer
		if (!readBytes(file, buffer, length)) return false;

		// construct string with filename read from buffer
		filename = std::string(buffer, length);

		// read file size
		if (!readType(file, size)) return false;

		// read file offset
		if (!readType(file, offset)) return false;

		return true;
	}
	
	void print()
	{
		printf("- %s (%llu bytes)\n", filename.c_str(), size);
	}
};

// used to get files and directories hierarchy
struct Index
{
	Index():directory(false), root(false)
	{
	}

	std::string name;
	bool directory; // directory or a file
	bool root; // root directory
	std::vector<Index> files; // if a directory, contains all its files

	bool read(FILE *file, bool isroot)
	{
		// max length for a filename
		char buffer[BUFFER_SIZE];

		// read filename length
		uint32_t length;
		if (!readType(file, length)) return false;

		// length should never be longer than BUFFER_SIZE
		if (length >= BUFFER_SIZE)
		{
			fprintf(stderr, "Filename longer than %d\n", BUFFER_SIZE);
			return false;
		}

		// only read filename if length > 0
		if (length > 0)
		{
			// read filename into temporary buffer
			if (!readBytes(file, buffer, length)) return false;

			// construct string with filename read from buffer
			name = std::string(buffer, length);
		}

		// is a root directory?
		if (isroot)
		{
			// a root directory is always a directory (obvious)
			directory = true;
			root = true;

			// files numnber in directories
			uint32_t filesCount;
			if (!readType(file, filesCount)) return false;

			// optimize files list since we know the exact size
			files.reserve(filesCount);

			Index fileIndex;

			// read all files indices
			for (uint32_t j = 0; j < filesCount; ++j)
			{
				// read file index
				if (fileIndex.read(file, false))
				{
					// add entry to array
					files.push_back(fileIndex);
				}
			}
		}
		else
		{
			// entry is a directory?
			uint8_t dir;
			if (!readType(file, dir)) return false;

			// convert byte to bool
			directory = dir == 1;

			// not root
			root = false;
		}

		return true;
	}

	void print()
	{
		if (root)
		{
			printf("- Directory %s with %zu entries:\n", name.c_str(), files.size());

			// print all files
			for (size_t j = 0, jlen = files.size(); j < jlen; ++j)
			{
				files[j].print();
			}
		}
		else
		{
			printf("  - %s (%s)\n", name.c_str(), directory ? "directory" : "file");
		}
	}
};

bool extractVersion2(FILE *file)
{
	// always 1, don't know what is it
	uint32_t unknown1;
	if (!readType(file, unknown1)) return false;

	// always 1, don't know what is it
	uint64_t unknown2;
	if (!readType(file, unknown2)) return false;

	// read the number of files
	uint32_t entriesCount;
	if (!readType(file, entriesCount)) return false;

	if (unknown1 != 1 || unknown2 != 1 || entriesCount < 1)
	{
		fprintf(stderr, "Wrong WAD file header\n");
		return false;
	}

	printf("Hotline Miami 2 WAD with %u files\n", entriesCount);

	std::vector<Entry2> entries;

	// optimize files list since we know the exact size
	entries.reserve(entriesCount);

	for (uint32_t i = 0; i < entriesCount; ++i)
	{
		Entry2 entry;

		if (entry.read(file))
		{
			entry.print();

			// add entry to array
			entries.push_back(entry);
		}
	}

	uint32_t indicesCount;
	if (!readType(file, indicesCount)) return false;

	std::vector<Index> indices;

	// optimize files list since we know the exact size
	indices.reserve(indicesCount);

	// read all directories
	for (uint32_t i = 0; i < indicesCount; ++i)
	{
		Index dirIndex;

		// read directory
		if (dirIndex.read(file, true))
		{
			// add entry to array
			indices.push_back(dirIndex);
		}
	}

	size_t contentOffset = ftell(file);

	// extract all entries
	for (uint32_t i = 0; i < entriesCount; ++i)
	{
		// for faster access
		const Entry2 &entry = entries[i];

		std::string filename = entry.filename;

		// check filename is valid before creating directories
		if (filename.empty()) continue;

		// create all intermediate directories
		boost::filesystem::path path(filename);
		boost::filesystem::create_directories(path.parent_path());

		// check if size is valid before allocating a buffer
		if (entry.size < 1) continue;

		// allocate a buffer with the correct size (it'll be deleted automatically)
		std::unique_ptr<uint8_t[]> tmp(new uint8_t[entry.size]);

		// seek to the right offset (currently useless, but helpful to extract a specific file)
		fseek(file, (long)(contentOffset + entry.offset), SEEK_SET);

		// read the whole file in memory
		if (!readBytes(file, tmp.get(), entry.size)) return false;

		// try to create the file
		FILE *out = fopen(filename.c_str(), "wb");

		if (!out)
		{
			// probably a wrong filename or out of space
			fprintf(stderr, "Unable to create %s\n", filename.c_str());
			return false;
		}

		printf("Extracting %s (%llu bytes)... ", filename.c_str(), entry.size);

		// write the file
		size_t written = fwrite(tmp.get(), 1, entry.size, out);

		// close the file
		fclose(out);

		// check if the whole file has been successfully written
		if (written == entry.size)
		{
			printf("OK\n");
		}
		else
		{
			printf("Error\n");

			fprintf(stderr, "Unable to write %s (only %zu bytes written on %llu)\n", filename.c_str(), written, entry.size);

			return false;
		}
	}

	return true;
}

