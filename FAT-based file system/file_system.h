#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define DISC_NAME_SIZE		11
#define NUMBER_OF_CLUSTERS	30
#define CLUSTER_SIZE		100
#define FILE_NAME_SIZE		20

// Only one disc can be mounted 
static bool mounted = false;

// Reserved sector contains basic information about disc ------------------	first cluster
struct reserved_sector {
	char name[DISC_NAME_SIZE];
	int8_t number_of_clusters;
	int8_t cluster_size;
	int8_t number_of_files;
}rs;

/* FAT region contains array of bytes that represent cluster state --------	second cluster
* 0x00: cluster is empty
* 0x01 - 0x1D: next clsuter in overflow
* 0xFF: file dose not overflow to next cluster
*/
uint8_t file_alocation_table[NUMBER_OF_CLUSTERS];


// Root directory region contains files metadata --------------------------	third cluster
struct file {
	char name[FILE_NAME_SIZE];
	int16_t size;
	int8_t first_cluster;
};

struct file* directory_table;


void mount(const char*);										// Open or create new file system
void unmount();													// Save changes and close disk file
struct file* open(const char*);									// Return file object
void write(struct file*, const char*);							// Write data to clusters, returns 1 if cluster allocatian was successful
char* read(struct file*);										// Read data from cluster
void delete(const char*);										// Delete file from directory region
void close(struct file*);										// Close file

uint8_t get_empty_cluster();									// Return first free cluster, -1 if there are no available clusters
void disc_info();												// Print disc info
void file_info(struct file*);									// Print file info
void set_default_fat();											// Set default state for file alocation table
int8_t find_file(const char*);									// Find file in directory region, return -1 if file cannot be found
int8_t free_file(struct file*);									// Free clusters that file occupies
void print_clusters();											// Print data from cluster
