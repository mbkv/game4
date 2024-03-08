#pragma once

#include <stdio.h>
#include <stdlib.h>

#include "./util.hpp"

#define TINYOBJ_LOADER_C_IMPLEMENTATION
#include "tinyobj_loader_c.h"

struct string_t
{
	char * data;
	size_t length;
};

static string_t
read_entire_file(const char *filename, allocator_t allocator)
{
	FILE *f = fopen(filename, "rb");
	fseek(f, 0, SEEK_END);

	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  /* same as rewind(f); */

	char *string = (char*)allocator(fsize + 1);
	fread(string, fsize, 1, f);
	fclose(f);

	string[fsize] = 0;

	return {string, (size_t)fsize};
}
