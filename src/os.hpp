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
	rewind(f);

	char *s = (char*)allocator->alloc(fsize + 1);
	fread(s, fsize, 1, f);

	s[fsize] = 0;

	return {s, (u64)fsize};
}
