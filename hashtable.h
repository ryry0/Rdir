#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_


typedef struct hash_node_t {
  struct hash_node_t * next ;
  char * key;
  int  data;
} hash_node;

typedef struct hash_table_t {
  size_t size;
  struct hash_node_t ** table;
} hash_table;

void initTable(hash_table * h_table, size_t size);
void insert(hash_table * h_table, char * key, int data);
void removeNode(hash_table * h_table, char * key);
void destroyTable(hash_table *h_table);
hash_node * retrieve(hash_table * h_table, char * key);
void printTable(hash_table * h_table);
long hash (char *key);
void printNode(hash_node *);
void destroyNode(hash_node *h_node);

#endif
