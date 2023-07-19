#include <stdio.h>
#include <stdlib.h>

int main() {
	FILE* output_file = NULL;

	output_file = fopen("test.txt", "r");

	if (output_file == NULL) {
		printf("Neuspjesno otvaranje datoteke!");
		return 1;
	}

	char text[1000];
	fscanf(output_file, "%s", text);

	fclose(output_file);

	printf("%s", text);

	return 0;
}