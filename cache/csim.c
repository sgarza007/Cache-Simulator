/*
 * Saul Garza & Erick Perez
 * COMP 280:01
 * Proj 5
 *
 * Project Bucks Used: 0
 * Hours worked on: 17
 *
 * csim.c
 *
 * Out program simulates how a cache works. When given a file containing the
 * traces of data, our program will output the total amount of misses, hits,
 * and evictions given the commands from the trace file. Additionally, the
 * user can define the cache by giving us the number of sets, the block
 * offset, and the number of lines per set. From there, we create a
 * respectice cache with those properties.
 *
 */
#include "cachelab.h"
#include <getopt.h> 
#include <stdlib.h> 
#include <unistd.h>
#include <ctype.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	char c;
	int set = 0;
	int eLine = 0;
	int bOffset = 0;
	char *fileTrace;
	//reads in arguments from command line
	while ((c = getopt (argc, argv, "s:E:b:t:")) != -1)
	{
		switch (c)
		{
			case 's':
			{
				set = atoi(optarg);
				break;
			}
			case 'E':
			{
				eLine =atoi(optarg);
				break;
			}
			case 'b':
			{
				bOffset = atoi(optarg);
				break;
			}
			case 't':
			{
				fileTrace = optarg;
				break;
			}
// need to add case for verbose option
			default:
			{
				break;
			}
		}
	}
//use struct of structs that acts similar to a multidimensional array
//except easier to use
	struct Line{
		int valid;
		int tag;
		int lru;
	};
	typedef struct Line Line;

	struct Set{
		Line *lines;
	};
	typedef struct Set Set;

	struct Cache{
		Set *sets;
	};
	typedef struct Cache Cache;

	//added to hold memory address that is an 8 byte data type
	//used ot have an int hold this but no more!
	typedef unsigned long long int mem_addr;
	int numSet= 0;
//finds S and B from the command line arguments
	numSet = (1 << set);

//initializes the counters that will be printed out at the end
	int hits = 0;
	int misses = 0;
	int evictions = 0;
	char command;
	int size = 0;
	int lruTemp = 0;
	mem_addr address;
	int isEmpty = -1;
	int hitFlag = 0;
	int shouldEvict = 0;
	int vHolder = 0; 
//	int evictionFlag = 0;
//	only needed for verbose section if we decide to implement
//	raises error because unused at the moment
	
	Cache cacheSet;
	// initializes and allocates memory for total number of sets needed
	cacheSet.sets = calloc (numSet, sizeof(Set));
//initializes and allocates memry for total number of lines for each set
	for (int i = 0; i < numSet; i++)
	{
		cacheSet.sets[i].lines = calloc(eLine, sizeof(Line));
	}

	// Opening and reading from file below
	FILE *readFile = fopen(fileTrace, "r");
	
	if(readFile != NULL)
	{
		// reads lines and gets command, the address being cached, and the
		// size
		while (fscanf(readFile, " %c %llx,%d", &command, &address, &size) ==3)
		{
			if (command != 'I')
				{
					mem_addr tag = address >> (set + bOffset);
					int tagSize = (64 - (set + bOffset));
					mem_addr newAddr = address << (tagSize);
					mem_addr setter = newAddr >> (tagSize + bOffset);
					// this is just a temp for low so that the first
					// comparison will automatically overwrite it and be able
					// to use it in comparisons
					int min = 2147483647;
				//traverses every line of the set the item should be in	
					for (int i = 0; i < eLine; i++)
					{
						vHolder = cacheSet.sets[setter].lines[i].valid; 
						//if the valid bit is set to one it checks its
						//contents to see if its in there
						if (vHolder == 1)
						{
							if (cacheSet.sets[setter].lines[i].tag == tag)
							{
								//hit is incremented and lru updated if found
								hits++;
								hitFlag = 1;
								cacheSet.sets[setter].lines[i].lru = lruTemp;
								lruTemp++;
							}
							else if (cacheSet.sets[setter].lines[i].lru < min)
							{
								min = cacheSet.sets[setter].lines[i].lru;
								shouldEvict = i;
							}
						}
						else if (isEmpty == -1)
						{
							isEmpty = i;
						}
					}
					// if theres no match then it will make a miss and update
					// the set to contain the new object
					if (hitFlag != 1)
					{
						misses++;
						if (isEmpty > -1)
						{
							// oprtion if empty. Will put it in cache
							cacheSet.sets[setter].lines[isEmpty].valid = 1;
							cacheSet.sets[setter].lines[isEmpty].tag = tag;
							cacheSet.sets[setter].lines[isEmpty].lru = lruTemp;
							lruTemp++;
						}
						else if (isEmpty < 0)
						{
							//no hit and also all lines taken will result in
							//eviction of a line with that is the least recent
							//evictionFlag = 1;
							cacheSet.sets[setter].lines[shouldEvict].tag = tag;
							cacheSet.sets[setter].lines[shouldEvict].lru = lruTemp;
							lruTemp++;
							evictions++;
						}
					}
					if (command == 'M')
					{
						hits++;
					}
					isEmpty = -1;
					hitFlag = 0;
					//evictionFlag = 0;
				}
		}
	}
	//closing the file being read
	fclose(readFile);
	printSummary(hits, misses, evictions);
	//frees line in each set of the cache
	for (int i = 0; i < numSet; i++)
	{
		free(cacheSet.sets[i].lines);
	}
	//free sets of cache
	free(cacheSet.sets);
	return 0;
}

