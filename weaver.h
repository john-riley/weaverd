#ifndef WEAVER_H
#define WEAVER_H

#define MAX_SEARCH_ITEMS 1024

typedef struct {
  unsigned int group_id;
  unsigned int number;         /* Article number in group */
  unsigned int id;             /* Unique article id */

  unsigned int parent;
  unsigned int next_instance;  /* Used for cross-posting. */

  /* These two are offsets into the string storage. */
  unsigned int subject;
  unsigned int author;
  time_t date;

  unsigned int first_child;
  unsigned int next_sibling;
} node;

typedef struct {
  unsigned int group_name;
  unsigned int group_description;
  unsigned int group_id;
  
  unsigned int nodes_on_disk;
  unsigned int next_node;

  unsigned int min_article;
  unsigned int max_article;
  unsigned int total_articles;

  unsigned int **numeric_nodes;
  unsigned int **thread_nodes;
  int dirtyp;
} group;

unsigned int get_parent(const char *parent_message_id);
void thread(node *node);
char *index_file_name(char *name);

extern char *index_dir;
extern group groups[];

#endif

