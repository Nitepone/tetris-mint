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

StringArray *create_string_array(int length) {
	StringArray *arr = malloc(sizeof(StringArray));
	arr->length = length;
	arr->strings = malloc(sizeof(char *) * length);
	return arr;
}

void set_string_array(StringArray *arr, int index, char *value) {
	int len = strlen(value);
	arr->strings[index] = malloc(len + 1);
	memcpy(arr->strings[index], value, len);
}

char *get_string_array(StringArray *arr, int index, char *value) {
	return arr->strings[index];
}

Blob *serialize_string_array(StringArray *arr) {
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

StringArray *deserialize_string_array(Blob *blob) {
	int length = *((int *)blob->bytes);
	StringArray *arr = create_string_array(length);

	int message_index = 4;

	for (int i = 0; i < length; i++) {
		int string_length = strlen(blob->bytes + message_index);
		set_string_array(arr, i, blob->bytes + message_index);
		message_index += string_length + 1;
	}
	return arr;
}

void destroy_string_array(StringArray *arr) {
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
