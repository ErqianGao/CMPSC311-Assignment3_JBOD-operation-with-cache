#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "cache.h"

static cache_entry_t *cache = NULL;
static int cache_size = 0;
static int clock = 0;
static int num_queries = 0;
static int num_hits = 0;

static bool cacheRunning = false;

int cache_create(int num_entries) {
   if( cacheRunning == true || num_entries < 2 || num_entries > 4096 ){
    return -1;
  }
  
  cacheRunning = true;
  cache = malloc(num_entries*sizeof(cache_entry_t));
  cache_size = num_entries;
  for( int i = 0; i < cache_size; i++ ){
    cache[i].valid = false;
    cache[i].access_time = 0;
  }
  
  return 1;
}

int cache_destroy(void) {
  if( cacheRunning == false ){
    return -1;
  }
  cache_size = 0;
  cacheRunning = false;
  free(cache);
  return 1;
}

int cache_lookup(int disk_num, int block_num, uint8_t *buf) {
  if( cacheRunning == false || disk_num < 0 || disk_num >= JBOD_NUM_DISKS || block_num < 0 || block_num >= JBOD_NUM_BLOCKS_PER_DISK || buf == NULL ){
    return -1;
  }

  clock++;
  num_queries++;
  
  for( int i = 0; i < cache_size; i++ ){
    if( cache[i].disk_num == disk_num && cache[i].block_num == block_num ){
      memcpy( buf, cache[i].block, JBOD_BLOCK_SIZE );
      cache[i].access_time = clock;
      num_hits++;
      return 1;
    }
  }
  
  return -1;
}


int cache_insert(int disk_num, int block_num, const uint8_t *buf) {
  if( cacheRunning == false || disk_num < 0 || disk_num >= JBOD_NUM_DISKS || block_num < 0 || block_num >= JBOD_NUM_BLOCKS_PER_DISK || buf == NULL ){
    return -1;
  }

  int target = 0;
  int least_recent = clock;
  for( int i = 0; i < cache_size; i++ ){
    if( cache[i].disk_num == disk_num && cache[i].block_num == block_num ){
      return -1;
    }
    
    if( cache[i].valid == false ){
      cache[i].valid = true;
      target = i;
      break;
    }

    if( cache[i].access_time < least_recent ){
      least_recent = cache[i].access_time;
      target = i;
    }
  }


  clock++;
  cache[target].disk_num = disk_num;
  cache[target].block_num = block_num;
  cache[target].access_time = clock;
  memcpy( cache[target].block, buf, JBOD_BLOCK_SIZE );
  return 1;
}

void cache_update(int disk_num, int block_num, const uint8_t *buf) {
  if( cacheRunning == false || disk_num < 0 || disk_num >= JBOD_NUM_DISKS || block_num < 0 || block_num >= JBOD_NUM_BLOCKS_PER_DISK || buf == NULL ){
    return;
  }
  
  for( int i = 0; i < cache_size; i++ ){
    if( cache[i].disk_num == disk_num && cache[i].block_num == block_num ){
      memcpy( cache[i].block, buf, JBOD_BLOCK_SIZE );
      cache[i].access_time = clock;
      return;
    }
  }
}

bool cache_enabled(void) {
  return cacheRunning;
}

void cache_print_hit_rate(void) {
  fprintf(stderr, "Hit rate: %5.1f%%\n", 100 * (float) num_hits / num_queries);
}
