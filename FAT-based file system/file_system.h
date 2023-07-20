#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// Volume must be maximum 11 characters long and cannot contain the following characters: * ? / \ | , ; : + = < > [ ] ""
#define DISC_NAME_SIZE		11
#define NUMBER_OF_CLUSTERS	30
#define CLUSTER_SIZE		100
#define FILE_NAME_SIZE		10

// Only one disc can be mounted 
static bool mounted = false;

// Reserved sector contains basic information about disc ------------------	first cluster
struct reserved_sector {
	char name[DISC_NAME_SIZE];
	uint8_t number_of_clusters;
	uint8_t cluster_size;
	uint8_t number_of_files;
}rs;

/* FAT region contains array of bytes that represent cluster state --------	second cluster
* 0x00: cluster is empty
* 0x01 - 0x1D: next clsuter in overflow
* 0xFF: file dose not overflow to next cluster
*/
uint8_t file_alocation_table[NUMBER_OF_CLUSTERS];


// TODO on mora rasti 
// Root directory region contains files metadata --------------------------	third cluster
struct file {
	char name[FILE_NAME_SIZE];
	uint16_t size;
	int8_t first_cluster;
};

struct file* directory_table;


void mount(const char*);					// Open or create new file system
void unmount();								// Save changes and close disk file
struct file* open(const char*);				// Return file object
void write(struct file*, const char*);		// Vrite data to clusters
void close(struct file*);					// Close file

int8_t get_empty_cluster();					// Return first free cluster, -1 if there are no available clusters
void disc_info();							// Print disc info
void set_default_fat();						// Set default state for file alocation table

// void create_file(); // Mora dinamički alocirati memeoriju za file names i size
