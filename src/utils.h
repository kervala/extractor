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

#ifndef UTILS_H
#define UTILS_H

#define BUFFER_SIZE 1024

bool extractVersion2(FILE *file);
bool extractVersion1(FILE *file);

template<class T>
bool readType(FILE *file, T &data)
{
	size_t size = sizeof(data);

	if (fread(&data, size, 1, file) != 1)
	{
		fprintf(stderr, "File truncated, unable to read %zu bytes\n", size);
		return false;
	}

	return true;
}

template<class T>
bool readBytes(FILE *file, T *data, size_t size)
{
	size_t read = fread(data, 1, size, file);

	if (read != size)
	{
		fprintf(stderr, "File truncated, unable to read %zu bytes (only %zu read)\n", size, read);
		return false;
	}

	return true;
}

#endif
