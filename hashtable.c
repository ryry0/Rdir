#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <hashtable.h>

long hash (char *key) {
  long hashAddress = 5381;
  for (size_t counter = 0; key[counter]!='\0'; counter++){
        hashAddress = ((hashAddress << 5) + hashAddress) + key[counter];
  }
  return hashAddress;
}

void initTable(hash_table * h_table, size_t size) {
  h_table->table = (hash_node **) calloc (size , sizeof (hash_node*));
  h_table->size = size;
}

void printTable(hash_table * h_table) {
  for(size_t i = 0; i < h_table->size; i++) {
    printNode(h_table->table[i]);
  }
}

void printNode(hash_node * h_node) {
  if (h_node == NULL)
    return;

  printf("key: %s, data: %d\n", h_node->key, h_node->data);
  printNode(h_node->next);
}

void insert(hash_table * h_table, char * key, int data) {
  long index = hash(key) % h_table->size;

  if (retrieve(h_table, key) != NULL)
      return;

  hash_node *temp = (hash_node *) malloc(sizeof(hash_node));
  temp->key = (char * ) malloc(strlen(key) +1);
  strcpy(temp->key,key);
  temp->data = data;

  temp->next = h_table->table[index];
  h_table->table[index]  = temp;
}

//recursive function to traverse the list to remove the node.
hash_node * removeListNode(hash_node **root, char *key)
{
  if(*root == NULL)
    return NULL;
  hash_node *temp = (*root)->next;
  if(strcmp((*root)->key, key)==0) {
    free((*root)->key);
    free(*root);
    (*root) = removeListNode(&temp, key);
  } else {
    (*root)->next = removeListNode(&temp, key);
  }
  return *root;
}

//call to remove a node from the list
void removeNode(hash_table * h_table, char * key) {
  hash_node *found = retrieve(h_table, key);
  if(found == NULL)
    return;
  long index = hash(key) % h_table->size;
  removeListNode(&(h_table->table[index]), key);
}

hash_node * retrieve(hash_table *h_table, char *key) {
  long index = hash(key) % h_table->size;
  hash_node *iterator = h_table->table[index];

  while (iterator) {
    if (strcmp(key, iterator->key) == 0) {
      return iterator;
    }
    iterator = iterator->next;
  } //end while
  return NULL;
} //end retrieve

void destroyTable(hash_table *h_table) {
  for(size_t i = 0; i < h_table->size; i++) {
    destroyNode(h_table->table[i]);
  }
  free(h_table->table);
}

void destroyNode(hash_node *h_node) {
  if (h_node == NULL)
    return;
  destroyNode(h_node->next);

  if (h_node->key != NULL)
    free(h_node->key);

  free(h_node);
}
