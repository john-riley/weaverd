#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#if defined(__FreeBSD__)
#  include <sys/mman.h>
#endif
#include <fcntl.h>
#include <getopt.h>
#include <gmime/gmime.h>
#include <dirent.h>
#include <errno.h>
#include <time.h>

#include "weaver.h"
#include "config.h"
#include "hash.h"
#include "input.h"
#include "../mdb/util.h"

struct option long_options[] = {
  {"spool", 1, 0, 's'},
  {"index", 1, 0, 'i'},
  {"help", 0, 0, 'h'},
  {"user", 1, 0, 'u'},
  {"recursive", 0, 0, 'r'},
  {0, 0, 0, 0}
};

static int output_thread = 0;
static int input_spool = 0;

int parse_args(int argc, char **argv) {
  int option_index = 0, c;
  while (1) {
    c = getopt_long(argc, argv, "htrs:i:u:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 's':
      news_spool = optarg;
      break;
      
    case 'i':
      index_dir = optarg;
      break;
      
    case 'u':
      lock_and_uid(optarg);
      break;
      
    case 't':
      output_thread = 1;
      break;
      
    case 'r':
      input_spool = 1;
      break;
      
    case 'h':
      printf ("Usage: we:index [--spool <directory>] <directories ...>\n");
      break;

    default:
      break;
    }
  }

  return optind;
}

#define MAX_FILE_NAME 10000

static int commands = 0, last_total_commands = 0;

void input_directory(const char* dir_name) {
  time_t start_time, now;
  int elapsed;
  DIR *dirp;
  struct dirent *dp;
  char file_name[MAX_FILE_NAME];
  struct stat stat_buf;

  printf("%s\n", dir_name); 

  inhibit_thread_flattening = 1;
    
  dirp = opendir(dir_name);
  time(&start_time);
    
  while ((dp = readdir(dirp)) != NULL) {

    snprintf(file_name, sizeof(file_name), "%s/%s", dir_name,
	     dp->d_name);

    if (strcmp(dp->d_name, ".") &&
	strcmp(dp->d_name, "..")) {
    
      if (stat(file_name, &stat_buf) == -1) {
	perror("tokenizer");
	break;
      }
    
      if (S_ISDIR(stat_buf.st_mode)) 
	input_directory(file_name);
      else if (is_number(dp->d_name)) {
	thread_file(file_name);

	if (!(commands++ % 10000)) {
	  time(&now);
	  elapsed = now-start_time;
	  printf("    %d files (%d/s last %d seconds)\n",
		 commands - 1, 
		 (elapsed?
		  (int)((commands-last_total_commands) / elapsed):
		  0),
		 (int)elapsed);
	  last_total_commands = commands;
	  start_time = now;
	}
      }
    }
  }
}

int main(int argc, char **argv)
{
  int dirn;
  struct stat stat_buf;
  char *spool;

  dirn = parse_args(argc, argv);
  
  /* Initialize key/data structures. */
  init();

  if (output_thread) {
    output_threads("gmane.discuss");
    exit(0);
  } 

  if (input_spool) {
    spool = cmalloc(strlen(news_spool) + 1);
    memcpy(spool, news_spool, strlen(news_spool));
    *(spool+strlen(news_spool)-1) = 0;
    //input_directory("/mirror/var/spool/news/articles/gmane/os/cygwin/patches");
    input_directory(spool);
    flush_hash();
    exit(0);
  } 

  if (stat(argv[dirn], &stat_buf) == -1) {
    perror("interactive");
    exit(0);
  }
      
  if (S_ISREG(stat_buf.st_mode)) 
    thread_file(argv[dirn]);

  flush_hash();

  exit(0);
}
