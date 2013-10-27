/*
 * config_file.c - Configuration file reader/writer
 *
 * Copyright (c) 2013   A. Dilly
 *
 * AirCat is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation.
 *
 * AirCat is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AirCat.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config_file.h"

enum Type {
	string,
	number,
	boolean
};

struct config_field {
	const char *name;
	enum Type type;
	void *value;
	const char *comment;
} static field_table[] = {
	{"name", string, &config.name, "# Name of AirCat device (AirCat by default).\n"},
	{"password", string, &config.password, "# Password for remote access (none by default).\n"},
	{"port", number, &config.port, "# Listen port for remote access.\n"},
	{"radio.enabled", boolean, &config.radio_enabled, "# Enable radio module.\n"},
	{"raop.enabled", boolean, &config.raop_enabled, "# Enable RAOP module.\n"},
	{"raop.name", string, &config.raop_name, "# Name of RAOP device (same as general name by default).\n"},
	{"raop.password", string, &config.raop_password, "# Password for using RAOP module.\n"},
	{NULL, 0, NULL}
};

int config_load(const char *file)
{
	FILE *fp;
	char line[255];
	char *name;
	char *value;
	int i;

	/* Open configuration file */
	fp = fopen(file, "r");
	if(fp == NULL)
	{
		return -1;
	}

	/* Read the file */
	while(!feof(fp))
	{
		/* Read a line */
		fgets(line, 255, fp);
		if(line[0] == '#' || line[0] == '\n')
			continue;

		/* Split the line */
		name = line;
		value = strchr(line, '=');
		if(value == NULL)
			continue;
		*value++ = '\0';
		value[strlen(value)-1] = '\0';
		if(value[0] == '\0' || value[0] == '\n')
			continue;

		/* Parse the line */
		for(i = 0; field_table[i].name != NULL; i++)
		{
			if(strcmp(name, field_table[i].name) == 0)
			{
				switch(field_table[i].type)
				{
					case string:
						if(*((char**)field_table[i].value) != NULL)
							free(*((char**)field_table[i].value));
						*((char**)field_table[i].value) = strdup(value);
						break;
					case number:
						*((long*)field_table[i].value) = atol(value);
						break;
					case boolean:
						*((int*)field_table[i].value) = strcmp(value, "true") == 0 ? 1 : 0;
				}
			}
		}
	}

	/* Close configuration file */
	fclose(fp);

	return 0;
}

int config_save(const char *file)
{
	char *tmp_file;
	FILE *fp;
	int i;

	/* Create a temporary file name */
	tmp_file = malloc(strlen(file)+2);
	sprintf(tmp_file, ".%s", file);

	/* Create temporary file */
	fp = fopen(tmp_file, "w");
	if(fp == NULL)
	{
		free(tmp_file);
		return -1;
	}

	/* Put head comments */
	fputs("# This file is automatically generated by AirCat\n# Be careful if you edit!\n\n", fp);

	/* Print all settings */
	for(i = 0; field_table[i].name != NULL; i++)
	{
		fputs(field_table[i].comment, fp);
		switch(field_table[i].type)
		{
			case string:
				if(*((char**)field_table[i].value) != NULL && (*((char**)field_table[i].value))[0] != '\0')
					fprintf(fp, "%s=%s\n\n", field_table[i].name, *((char**)field_table[i].value));
				else
					fprintf(fp, "#%s=\n\n", field_table[i].name);
				break;
			case number:
				fprintf(fp, "%s=%ld\n\n", field_table[i].name, *((long*)field_table[i].value));
				break;
			case boolean:
				fprintf(fp, "%s=%s\n\n", field_table[i].name, *((int*)field_table[i].value) == 1 ? "true" : "false");
		}
	}

	/* Close temporary file */
	fclose(fp);

	/* Rename temporary file to real file.
	 * This operation is atomic which guarantees file always exist.
	 */
	rename(tmp_file, file);

	/* Free temporary file name */
	free(tmp_file);

	return 0;
}
