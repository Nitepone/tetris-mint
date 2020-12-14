#ifndef _GENERIC_H
#define _GENERIC_H

typedef struct st_blob {
	int length;
	char *bytes;
} Blob;

Blob *create_blob(int length);
void resize_blob(Blob *blob, int length);
void shift_blob(Blob *blob, int shift);

typedef struct st_string_array {
	int length;
	char **strings;
} StringArray;

StringArray *create_string_array(int length);
void set_string_array(StringArray *arr, int index, char *value);
char *get_string_array(StringArray *arr, int index);
void destroy_string_array(StringArray *arr);
Blob *serialize_string_array(StringArray *arr);
StringArray *deserialize_string_array(Blob *blob);

#endif

// vi:noet:sts=0:sw=0:ts=8
