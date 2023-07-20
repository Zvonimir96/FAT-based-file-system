#include "file_system.h"

int main() {
	mount("test2");
	disc_info();
	struct file* f = open("f1");
	printf("%s %d %d", f->name, f->size, f->first_cluster);
	close(f);

	return 0;
}