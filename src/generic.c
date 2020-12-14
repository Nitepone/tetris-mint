#include <stdlib.h>
#include <string.h>

#include "generic.h"

Blob *create_blob(int length) {
	Blob *blob = malloc(sizeof(Blob));
	blob->bytes = malloc(length);
	blob->length = length;
	return blob;
}

void resize_blob(Blob *blob, int length) {
	blob->bytes = realloc(blob->bytes, length);
	blob->length = length;
}

void shift_blob(Blob *blob, int shift) {
	blob->bytes = realloc(blob->bytes, blob->length + shift);
	memcpy(blob->bytes + shift, blob->bytes, blob->length);
	blob->length = blob->length + shift;
}

StringArray *string_array_create(int length) {
	StringArray *arr = malloc(sizeof(StringArray));
	arr->length = length;
	arr->strings = calloc(sizeof(char *), length);
	return arr;
}

void string_array_resize(StringArray *arr, int new_length) {
	//
	// The beauty of this implementation is that only the pointers need to
	// be copied. The underlying strings allocations remain the same!
	//
	int old_length = arr->length;
	char **old_strings = arr->strings;

	// length_to_copy is the number of strings that can be copied from the
	// old array to the new array. In practice, this is the smaller of the
	// old length and the new length.
	int length_to_copy = new_length < old_length ? new_length : old_length;

	// build the new string array
	arr->length = new_length;
	arr->strings = calloc(sizeof(char *), new_length);
	memcpy(arr->strings, old_strings, sizeof(char *) * length_to_copy);

	// free the old array of pointers
	free(old_strings);
};

void string_array_set_item(StringArray *arr, int index, char *value) {
	int len = strlen(value);
	arr->strings[index] = malloc(len + 1);
	memcpy(arr->strings[index], value, len);
}

char *string_array_get_item(StringArray *arr, int index) {
	return arr->strings[index];
}

Blob *string_array_serialize(StringArray *arr) {
	int message_size = 128;
	int message_index = 0;
	Blob *blob = create_blob(message_size);

	// first four bytes will be the number of strings in the array
	*((int *)blob->bytes) = arr->length;
	message_index += 4;

	// copy the strings into the BLOB
	for (int i = 0; i < arr->length; i++) {
		int len = strlen(arr->strings[i]) + 1;
		// if the blob is not big enough, double it
		while (message_index + len >= message_size) {
			message_size *= 2;
			resize_blob(blob, message_size);
		}
		memcpy(blob->bytes + message_index, arr->strings[i], len);
		message_index += len;
	}

	return blob;
}

StringArray *string_array_deserialize(Blob *blob) {
	int length = *((int *)blob->bytes);
	StringArray *arr = string_array_create(length);

	int message_index = 4;

	for (int i = 0; i < length; i++) {
		int string_length = strlen(blob->bytes + message_index);
		string_array_set_item(arr, i, blob->bytes + message_index);
		message_index += string_length + 1;
	}
	return arr;
}

void string_array_destroy(StringArray *arr) {
	// free all the strings
	for (int i = 0; i < arr->length; i++)
		if (arr->strings[i])
			free(arr->strings[i]);

	// free the string pointer array
	if (arr->strings)
		free(arr->strings);

	// free the metadata struct
	free(arr);
}

// vi:noet:sts=0:sw=0:ts=8
