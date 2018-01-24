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
