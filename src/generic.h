#ifndef _GENERIC_H
#define _GENERIC_H

typedef struct st_blob {
	int length;
	char *bytes;
} Blob;

Blob *create_blob(int length);
void resize_blob(Blob *blob, int length);
void shift_blob(Blob *blob, int shift);

/**
 * StringArray is used as a higher-level representation of an array of strings.
 *
 * Goals:
 * - easily serializable and deserializable
 *
 * Notes:
 * - StringArray manages the memory for each string it contains. This means
 *   that when setting an item, the string will be copied to a new memory
 *   allocation.
 * - Resizing this type is also more efficient than using a builtin array
 */
typedef struct st_string_array {
	int length;
	char **strings;
} StringArray;

StringArray *string_array_create(int length);
void string_array_destroy(StringArray *arr);

/**
 * If the length is shorter than the previous length, then strings will be
 * automatically cleaned up and freed.
 *
 * @param new_length
 */
void string_array_resize(StringArray *arr, int new_length);

void string_array_set_item(StringArray *arr, int index, char *value);
char *string_array_get_item(StringArray *arr, int index);

Blob *string_array_serialize(StringArray *arr);
StringArray *string_array_deserialize(Blob *blob);

#endif

// vi:noet:sts=0:sw=0:ts=8
