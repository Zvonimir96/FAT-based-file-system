#pragma once
#include "file_system.h"

void mount(const char* disc_name) {
	if (mounted) {
		printf("Disc is already mounted!\n");
	}
	else {
		FILE* disc = NULL;
		disc = fopen(disc_name, "r");

		// If file does not exist, create new file
		if (disc == NULL) {
			// Set reserved sector data
			strcpy(rs.name, disc_name);
			rs.cluster_size = CLUSTER_SIZE;
			rs.number_of_clusters = NUMBER_OF_CLUSTERS;
			rs.number_of_files = 0;

			disc = fopen(disc_name, "w");

			// Write reserved sector
			fwrite(&rs, sizeof(struct reserved_sector), 1, disc);

			// Write FAT region
			set_default_fat();
			for (uint8_t i = 0; i < rs.number_of_clusters; i++) 
				fwrite(&file_alocation_table[i], sizeof(uint8_t), 1, disc);

			// Set file size to 3kb
			long file_size = NUMBER_OF_CLUSTERS * CLUSTER_SIZE;
			fseek(disc, file_size, SEEK_SET);
			fputc('\0', disc);
		}
		else {
			// Read reserved sector
			fread(&rs, sizeof(struct reserved_sector), 1, disc);

			// Read FAT region
			for (uint8_t i = 0; i < rs.number_of_clusters; i++)
				fread(&file_alocation_table[i], sizeof(uint8_t), 1, disc);

			// Read Root directory
			directory_table = malloc(sizeof(struct file) * rs.number_of_files);

			if (directory_table == NULL) {
				printf("Problem in memory allocation for directory table!\n");
				exit(1);
			}

			for (int i = 0; i < rs.number_of_files; i++) 
				fread(&directory_table[i], sizeof(struct file), 1, disc);
		}

		fclose(disc);

		mounted = true;
	}
}

void disc_info() {
	if (mounted) {
		printf("Disc name %s, cluster size %d, number of clusters %d, number of files %d.\n", rs.name, 
			rs.cluster_size, rs.number_of_clusters, rs.number_of_files);
		
		for (uint8_t i = 0; i < rs.number_of_clusters; i++)
			printf("Cluster %d has value %x.\n", i+1, file_alocation_table[i]);

		for (uint8_t i = 0; i < rs.number_of_files; i++)
			printf("File name %s, size %d, first cluster %d.\n", directory_table[i].name, 
				directory_table[i].size, directory_table[i].first_cluster);
	}
	else {
		printf("Disc is not mounted!\n");
	}
}

void set_default_fat() {
	// First three clusters are assigned and do not overflow
	file_alocation_table[0] = 0xFF;
	file_alocation_table[1] = 0xFF;
	file_alocation_table[2] = 0xFF;

	// Other clusters are empty
	for (uint8_t i = 3; i < rs.number_of_clusters; i++)
		file_alocation_table[i] = 0x00;
}