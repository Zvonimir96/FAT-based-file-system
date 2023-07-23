#pragma once
#include "file_system.h"

void mount(const char* disc_name) {
	if (mounted) {
		printf("Disc is already mounted!\n");
		return;
	}

	FILE* disc = NULL;
	disc = fopen(disc_name, "rb");

	// If file does not exist, create new file
	if (disc == NULL) {
		// Set reserved sector data
		strcpy(rs.name, disc_name);
		rs.cluster_size = CLUSTER_SIZE;
		rs.number_of_clusters = NUMBER_OF_CLUSTERS;
		rs.number_of_files = 0;

		disc = fopen(disc_name, "wb");

		// Write reserved sector
		fwrite(&rs, sizeof(struct reserved_sector), 1, disc);

		// Set file position to second cluster
		long cluster_position = CLUSTER_SIZE;
		fseek(disc, cluster_position, SEEK_SET);

		// Write FAT region
		set_default_fat();
		fwrite(file_alocation_table, 1, rs.number_of_clusters, disc);

		// Set file size to 3kb
		long file_size = NUMBER_OF_CLUSTERS * CLUSTER_SIZE -1;
		fseek(disc, file_size, SEEK_SET);
		fputc('\0', disc);
	}
	else {
		// Read reserved sector
		fread(&rs, sizeof(struct reserved_sector), 1, disc);

		// Set file position to second cluster
		long cluster_position = CLUSTER_SIZE;
		fseek(disc, cluster_position, SEEK_SET);

		// Read FAT region
		fread(file_alocation_table, NUMBER_OF_CLUSTERS, 1, disc);

		// Read root directory
		directory_table = (struct file*)malloc(sizeof(struct file) * rs.number_of_files);

		if (directory_table == NULL) {
			printf("Issue in memory allocation for directory table!\n");
			exit(1);
		}

		uint8_t cluster = 2;
		uint16_t numer_of_clusters = rs.number_of_files * sizeof(struct file) / CLUSTER_SIZE;
		uint8_t max_capacity = CLUSTER_SIZE / (float)sizeof(struct file);

		// Read data to each cluster
		for (uint8_t i = 0; i <= numer_of_clusters; i++) {
			// Locate cluster in file
			long cluster_position = cluster * CLUSTER_SIZE;
			fseek(disc, cluster_position, SEEK_SET);

			// Set buffer index from where to write to file
			uint16_t start_index = i == 0 ? 0 : i * max_capacity;
			// Write maximum number of structures for each cluster, exept if it is last cluster
			size_t write_size = i == numer_of_clusters ? rs.number_of_files - max_capacity : max_capacity;

			// Write data to cluster
			fread(&directory_table[start_index], sizeof(struct file), write_size, disc);

			cluster = file_alocation_table[cluster];
		}
	}

	fclose(disc);

	mounted = true;
}

void unmount() {
	mounted = false;

	FILE* disc = NULL;
	disc = fopen(rs.name, "rb+");

	// Write reserved sector
	fwrite(&rs, sizeof(struct reserved_sector), 1, disc);

	// Set file position to second cluster
	long cluster_position = CLUSTER_SIZE;
	fseek(disc, cluster_position, SEEK_SET);

	// Write FAT region
	fwrite(file_alocation_table, 1, NUMBER_OF_CLUSTERS, disc);

	// Write root directory
	uint8_t cluster = 2;
	uint16_t numer_of_clusters = rs.number_of_files * sizeof(struct file) / CLUSTER_SIZE;
	uint8_t max_capacity = CLUSTER_SIZE / (float) sizeof(struct file);

	// Write data to each cluster
	for (uint8_t i = 0; i <= numer_of_clusters; i++) {
		// Locate cluster in file
		long cluster_position = cluster * CLUSTER_SIZE;
		fseek(disc, cluster_position, SEEK_SET);

		// Set buffer index from where to write to file
		uint16_t start_index = i == 0 ? 0 : i * max_capacity;
		// Write maximum number of structures for each cluster, exept if it is last cluster
		size_t write_size = i == numer_of_clusters ? rs.number_of_files - max_capacity : max_capacity;

		// Write data to cluster
		fwrite(&directory_table[start_index], sizeof(struct file), write_size, disc);

		cluster = file_alocation_table[cluster];
	}

	fclose(disc);

	// Set default data
	strcpy(rs.name, "");
	rs.cluster_size = 0;
	rs.number_of_clusters = 0;
	rs.number_of_files = 0;

	for (uint8_t i = 0; i < rs.number_of_clusters; i++)
		file_alocation_table[i] = 0x00;
}

struct file* open(const char* file_name) {
	// Check ih name is shorter than maximum size
	if (strlen(file_name) > FILE_NAME_SIZE) {
		printf("File name is to long!\n");
		return NULL;
	}

	if (!mounted) {
		printf("In order to open the file you disc needs to be mounted!\n");
		return NULL;
	}

	// Look for file in root directory
	int8_t file_id = find_file(file_name);
	if (file_id != -1) 
		return &directory_table[file_id];

	// If file does not exist create new file
	
	// Check if root directory region needs more memory
	uint8_t numger_of_clusters = 0;
	uint8_t start_cluster = 2;

	while (file_alocation_table[start_cluster] != 0xFF) {
		start_cluster = file_alocation_table[start_cluster];
		numger_of_clusters++;
	}

	uint16_t numer_of_required_cluster = rs.number_of_files * sizeof(struct file) / CLUSTER_SIZE;

	// If diretory table requires more memory, allocate new cluster
	if (numer_of_required_cluster > numger_of_clusters) {
		uint8_t temp_cluster = get_empty_cluster();
		if (temp_cluster == 0)
			return NULL;

		file_alocation_table[start_cluster] = temp_cluster;
	}

	// Check if there are available clusters to save file information
	uint8_t new_cluster = get_empty_cluster();
	if (new_cluster == 0)
		return NULL;

	// Increment number of files in reserved sector
	rs.number_of_files++;

	// Append file to root directory
	directory_table = (struct file*)realloc(directory_table, rs.number_of_files*sizeof(struct file));

	if (directory_table == NULL) {
		printf("Issue in memory allocation for directory table!\n");
		exit(1);
	}

	// Update reserved sector
	strcpy(directory_table[rs.number_of_files - 1].name, file_name);
	directory_table[rs.number_of_files - 1].size = 0;
	directory_table[rs.number_of_files - 1].first_cluster = new_cluster;

	return &directory_table[rs.number_of_files-1];
}

void write(struct file* file, const char* buffer) {
	// Free clusters that file occupies
	if (free_file(file->name) == -1) {
		printf("File does not exist!");
		return NULL;
	}

	// Open disk for writing without overriding it
	FILE* disc = NULL;
	disc = fopen(rs.name, "rb+");

	int8_t temp_cluster;
	uint8_t numer_of_clusters = (float) (strlen(buffer) - 1) / CLUSTER_SIZE;
	file->size = 0;

	// Write data to each cluster
	for (uint8_t i = 0; i <= numer_of_clusters; i++) {
		// Allocate new cluster and check if allocaton was successful
		int8_t cluster = get_empty_cluster();
		if (cluster == 0)
			return NULL;

		// Locate cluster in file
		long cluster_position = cluster * CLUSTER_SIZE;
		fseek(disc, cluster_position, SEEK_SET);

		// Write data to cluster
		// Set buffer index from where to write to file
		uint16_t start_index = i==0 ? 0 : i * CLUSTER_SIZE;
		// Write 100 bites for each cluster, exept if it is last cluster
		size_t write_size = i == numer_of_clusters ? strlen(buffer) - file->size : CLUSTER_SIZE;

		fwrite(&buffer[start_index], write_size, 1, disc);

		file->size += write_size;

		// Change cluster state
		if(i != 0)
			file_alocation_table[temp_cluster] = cluster;
		temp_cluster = cluster;
	}

	fclose(disc);
}

char* read(struct file* file) {
	// Create buffer
	char* buffer = (char*)malloc(sizeof(char) * file->size);

	if (buffer == NULL) {
		printf("Issue in memory allocation while reading from file!\n");
		exit(1);
	}

	// If there is nothing tu read return empty buffer
	if (file->size == 0)
		return buffer;

	FILE* disc = NULL;
	disc = fopen(rs.name, "rb");

	int8_t cluster = file->first_cluster;
	uint8_t numer_of_clusters = (float)file->size / CLUSTER_SIZE;
	
	// Read data to each cluster
	for (uint8_t i = 0; i <= numer_of_clusters; i++) {
		// Locate cluster on disc
		long cluster_position = cluster * CLUSTER_SIZE;
		fseek(disc, cluster_position, SEEK_SET);

		// Read data from cluster
		// Set buffer index from where to read to file
		uint16_t start_index = i == 0 ? 0 : i * CLUSTER_SIZE;
		// Read 100 bites for each cluster, exept if it is last cluster
		size_t write_size = i == numer_of_clusters ? strlen(buffer) - file->size : CLUSTER_SIZE;

		fread(&buffer[start_index], write_size, 1, disc);

		cluster = file_alocation_table[cluster];
	}
	
	return buffer;
}

void delete(const char* file_name) {
	// Find file in root directory
	int8_t file_id = find_file(file_name);
	if (file_id == -1) {
		printf("File does not exist!\n");
		return NULL;
	}

	// Decreese number of files in reserved sector
	rs.number_of_files--;

	// Free clusters that file occupies
	free_file(file_name);

	// Delete file from reserved sector
	if (rs.number_of_files == 0)
		free(directory_table);
	else {
		for (uint8_t i = file_id; i < rs.number_of_files; i++) 
			directory_table[i] = directory_table[i + 1];

		directory_table = (struct file*)realloc(directory_table, rs.number_of_files * sizeof(struct file));

		if (directory_table == NULL) {
			printf("Issue in memory allocation for directory table!\n");
			exit(1);
		}
	}
}

void close(struct file* file) {
	file = NULL;
}

int8_t find_file(const char* file_name) {
	for (uint8_t i = 0; i < rs.number_of_files; i++) {
		if (!strcmp(directory_table[i].name, file_name))
			return i;
	}

	return -1;
}

int8_t free_file(const char* file_name) {
	// Check if file exist
	int8_t file_id = find_file(file_name);
	if (file_id == -1)
		return -1;

	uint8_t cluster = directory_table[file_id].first_cluster;

	while (cluster != 0xFF) {
		uint8_t temp_cluster = file_alocation_table[cluster];
		file_alocation_table[cluster] = 0x00;
		cluster = temp_cluster;
	}

	return 0;
}

uint8_t get_empty_cluster() {
	if (mounted) {
		for (uint8_t i = 0; i < rs.number_of_clusters; i++) {
			if (file_alocation_table[i] == 0x00){
				// Set cluster state as occupied
				file_alocation_table[i] = 0xFF;
				return i;
			}			
		}

		printf("There are no empty clusters!\n");
	}
	else
		printf("Disc must first be mounted!\n");

	return 0;
}

void disc_info() {
	if (mounted) {
		printf("Disc name %s, cluster size %d, number of clusters %d, number of files %d.\n", rs.name, 
			rs.cluster_size, rs.number_of_clusters, rs.number_of_files);
		
		for (uint8_t i = 0; i < rs.number_of_clusters; i++)
			printf("Cluster %x has value %x.\n", i, file_alocation_table[i]);

		for (uint8_t i = 0; i < rs.number_of_files; i++)
			printf("File name %s, size %d, first cluster %x.\n", directory_table[i].name, 
				directory_table[i].size, directory_table[i].first_cluster);
	}
	else {
		printf("Disc is not mounted!\n");
	}
}

void file_info(struct file* file) {
	printf("File name %s, size %d and first cluster %d.\n", file->name, file->size, file->first_cluster);
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

void print_clusters() {
	FILE* disc = NULL;
	disc = fopen(rs.name, "rb");

	char* buffer = (char*)malloc(101);

	for (uint8_t i = 0; i < NUMBER_OF_CLUSTERS; i++) {
		// Locate cluster in file
		long cluster_position = i * CLUSTER_SIZE;
		fseek(disc, cluster_position, SEEK_SET);

		if (buffer == NULL) {
			printf("Issue in memory allocation while reading from file!\n");
			exit(1);
		}

		fread(buffer, 100, 1, disc);
		buffer[100] = '\0';

		printf("Cluster %x: %s\n", i, buffer);
	}

	free(buffer);
	fclose(disc);
}