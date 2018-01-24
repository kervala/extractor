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
struct Entry1
{
	Entry1():size(0), offset(0)
	{
	}

	std::string filename;
	uint32_t size;
	uint32_t offset;

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
		printf("- %s (%u bytes)\n", filename.c_str(), size);
	}
};

bool extractVersion1(FILE *file)
{
	// read the offset where we can find files content
	uint32_t contentOffset;
	if (!readType(file, contentOffset)) return false;

	// read the number of files
	uint32_t entriesCount;
	if (!readType(file, entriesCount)) return false;

	if (contentOffset < 8 || entriesCount < 1)
	{
		fprintf(stderr, "Wrong WAD file header\n");
		return false;
	}

	printf("Hotline Miama 1 WAD with %u files and content at offset %u\n", entriesCount, contentOffset);

	std::vector<Entry1> entries;

	// optimize files list since we know the exact size
	entries.reserve(entriesCount);

	for (uint32_t i = 0; i < entriesCount; ++i)
	{
		Entry1 entry;

		if (entry.read(file))
		{
			entry.print();

			// add entry to array
			entries.push_back(entry);
		}
	}

	size_t pos = ftell(file);

	if (pos != contentOffset)
	{
		fprintf(stderr, "Position %zu differs from header end %u\n", pos, contentOffset);
		return false;
	}

	// extract all entries
	for (uint32_t i = 0; i < entriesCount; ++i)
	{
		// for faster access
		const Entry1 &entry = entries[i];

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
		fseek(file, contentOffset + entry.offset, SEEK_SET);

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

		printf("Extracting %s (%u bytes)... ", filename.c_str(), entry.size);

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

			fprintf(stderr, "Unable to write %s (only %zu bytes written on %u)\n", filename.c_str(), written, entry.size);

			return false;
		}
	}

	return true;
}
