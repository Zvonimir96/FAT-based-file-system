#include "file_system.h"
#define T_SIZE 350
#define N_FILES 7

int main() {
	// Mount disc and print information
	mount("disc1");
	disc_info();

	struct file** files[N_FILES];

	for (uint8_t i = 0; i < N_FILES; i++) {
		// Create text file to be saved
		char text[T_SIZE + 1];

		// Fill text with simbols
		char c = 48 + i;
		for (uint16_t j = 0; j < T_SIZE; j++)
			text[j] = c;
		text[T_SIZE] = '\0';

		char buf[3];
		snprintf(buf, 3, "f%d", i);
	
		files[i] = open(buf);
		// Check if file was successfully opened
		if (files[i]) {
			write(files[i], text);
			close(files[i]);
		}
	}

	// Print information
	printf("--------------------------- All files written --------------------------------\n");
	disc_info();
	print_clusters();
	
	// Delete second and fourth file
	delete("f1");
	delete("f3");

	printf("---------------------- Second and fourth file deleted --------------------------\n");
	disc_info();
	
	// Create text file to be saved
	char text[1000 + 1];

	// Fill text with simbols
	char c = 48 + N_FILES;
	for (uint16_t j = 0; j < 1000; j++)
		text[j] = c;
	text[1000] = '\0';

	char buf[3];
	snprintf(buf, 3, "f%d", N_FILES);

	struct file* file = open(buf);
	// Check if file was successfully opened
	if (file) {
		write(file, text);
		close(file);
	}

	printf("----------------------------- Last file written ---------------------------------\n");
	disc_info();
	print_clusters();

	unmount();

	return 0;
}