#pragma once

#include <stdio.h>
#include "./string.hpp"


static str
read_entire_file(const char *filename, allocator_t *allocator)
{
	FILE *f = fopen(filename, "rb");
	if (f == nullptr) {
		fprintf(stderr, "Could not open '%s'", filename);
		return { nullptr, 0 };
	}
	defer(fclose(f));

	fseek(f, 0, SEEK_END);

	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

	char *string = (char*)allocator->alloc(fsize + 1);
	fread(string, fsize, 1, f);

	string[fsize] = 0;

	return {string, (u64)fsize};
}
