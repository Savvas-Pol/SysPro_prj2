#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashtable_monitor.h"
#include "hashtable_virus.h"

HashtableMonitor* hash_monitor_create(int hashNodes) {

    int i;

    HashtableMonitor* ht = malloc(sizeof (HashtableMonitor));
    ht->hash_nodes = hashNodes;

    ht->nodes = (HashtableMonitorNode**) malloc(hashNodes * sizeof (HashtableMonitorNode*)); //create hashtable for countries
    for (i = 0; i < hashNodes; i++) {
        ht->nodes[i] = NULL;
    }

    return ht;
}

void hash_monitor_destroy(HashtableMonitor* ht) {

    int i;

    for (i = 0; i < ht->hash_nodes; i++) {
        HashtableMonitorNode* temp = ht->nodes[i];

        while (temp != NULL) {
            ht->nodes[i] = temp->next;

            free(temp->monitorName);
            free(temp);

            temp = ht->nodes[i];
        }
    }

    free(ht->nodes);
    free(ht);
}

HashtableMonitorNode* hash_monitor_search(HashtableMonitor* ht, char* monitorName) {

    int pos = hash_function((unsigned char*) monitorName, ht->hash_nodes);

    HashtableMonitorNode* temp = ht->nodes[pos];

    while (temp != NULL) {
        if (!strcmp(temp->monitorName, monitorName))
            return temp;

        temp = temp->next;
    }
    return temp;
}

HashtableMonitorNode* hash_monitor_search_with_int(HashtableMonitor* ht, int monitorName) {
    char key[100];
    sprintf(key, "%d",monitorName );
    return hash_monitor_search(ht, key);
}

HashtableMonitorNode* hash_monitor_insert(HashtableMonitor* ht, char* monitorName) {

    int pos = hash_function((unsigned char*) monitorName, ht->hash_nodes);

    HashtableMonitorNode* new;

    new = (HashtableMonitorNode*) malloc(sizeof (HashtableMonitorNode));

    new->monitorName = (char*) malloc(strlen(monitorName) + 1);
    strcpy(new->monitorName, monitorName);
    new->next = ht->nodes[pos];
    ht->nodes[pos] = new;

    return new;
}

void hash_monitor_delete(HashtableMonitor* ht, char* monitorName) {

    int pos = hash_function((unsigned char*) monitorName, ht->hash_nodes);

    HashtableMonitorNode* temp = ht->nodes[pos], *temp2;
    int first = 1; // flag to check if we are in first node

    while (temp != NULL) {
        if (!strcmp(temp->monitorName, monitorName)) {
            if (first)
                ht->nodes[pos] = temp->next;
            else
                temp2->next = temp->next;

            free(temp->monitorName);
            free(temp);
            return;
        }
        temp2 = temp;
        temp = temp->next;
        first = 0;
    }
}