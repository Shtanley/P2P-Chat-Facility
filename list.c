#include "list.h"

// --------------------------------------- Global variables ---------------------------------------
static List lists[LIST_MAX_NUM_HEADS + 1];
static List *freeLists[LIST_MAX_NUM_HEADS]; // Store free list pointers
static int flIndex = 0; // free list index

static Node nodes[LIST_MAX_NUM_NODES];
static List *freeNodes; // list of free nodes

// --------------------------------------- Helper functions ---------------------------------------
bool emptyList(List *pList) {
    if(pList == NULL || (pList->head == NULL && pList->tail == NULL && pList->curr == NULL && pList->num_nodes == 0))
        return true;
    return false;
}

bool isListOOB(List * pList) {
    return (pList->status == LIST_OOB_END || pList->status == LIST_OOB_START);
}

Node *consumeFreeNode(void *item) { // If there are free nodes, return the tail node
    if(freeNodes->tail != NULL) {
        Node * temp = freeNodes->tail;
        freeNodes->tail = freeNodes->tail->prev;
        temp->item = item;
        temp->next = NULL;
        temp->prev = NULL;
        freeNodes->num_nodes--;
        return temp;
    }
    return NULL;
}

void produceFreeNode(Node *node) {  // Produces a free node
    // Resetting node values
    node->item = NULL;
    node->next = NULL;
    node->prev = NULL;

    if(freeNodes->tail != NULL) {   // Adding new tail to freeNodes list
        Node * temp = freeNodes->tail;
        freeNodes->tail = node;
        temp->next = node;
        node->prev = temp;
    }
    else    // Adding first node to freeNodes list
        freeNodes->tail = node;

    freeNodes->num_nodes++;
}

List *consumeFreeList(){   // Returns a free list if there are any
    if(flIndex >= 0) {
        List * temp = freeLists[flIndex--];
        return temp;
    }
    return NULL;
}

bool produceFreeList(List *pList){  // Produces a free list
    if(pList != NULL){  // If list is not null, reset values and add to free list
        pList->head = NULL;
        pList->tail = NULL;
        pList->curr = NULL;
        pList->num_nodes = 0;
        pList->status = LIST_OOB_START;

        if(flIndex < LIST_MAX_NUM_HEADS - 1) {   // If there is space in free list, add list
            freeLists[++flIndex] = pList;
            return true;
        }
        else    // If there is no space in free list, return false
            return false;
    }
    return false;
}

void unlinkNodes(Node *prev, Node *node, Node *next){    // Unlinks a node from the list
    if(node != NULL){
        node->next = NULL;
        node->prev = NULL;
        if(next != NULL && prev != NULL) {  // If there are nodes before and after, link them
            prev->next = next;
            next->prev = prev;
        }
        // If there is only a node before, set it to null
        else if(prev != NULL)
            prev->next = NULL;
        else if(next != NULL)
            next->prev = NULL;
    }
}

void linkNodes(Node * first, Node * second) {   // Linking two nodes
    if(first != NULL && second != NULL) {
        first->next = second;
        second->prev = first;
    }
}

// ---------------------------------------- List functions ----------------------------------------

// Makes a new, empty list, and returns its reference on success. 
// Returns a NULL pointer on failure.
List* List_create() {
    for(int i = 0; i < LIST_MAX_NUM_HEADS; i++) {   // Allocate lists
        lists[i].head = NULL;
        lists[i].tail = NULL;
        lists[i].curr = NULL;
        lists[i].num_nodes = 0;
        lists[i].status = LIST_OOB_START;
        freeLists[i] = &lists[i];
    }
    flIndex = LIST_MAX_NUM_HEADS - 1;

    lists[LIST_MAX_NUM_HEADS].head = NULL;
    lists[LIST_MAX_NUM_HEADS].tail = NULL;
    lists[LIST_MAX_NUM_HEADS].curr = NULL;
    lists[LIST_MAX_NUM_HEADS].status = LIST_OOB_NONE;
    lists[LIST_MAX_NUM_HEADS].num_nodes = 0;
    freeNodes = &lists[LIST_MAX_NUM_HEADS];
    Node *prev = NULL;
    for(int i = 0; i < LIST_MAX_NUM_NODES; i++) {   // Allocate nodes
        nodes[i].item = NULL;
        nodes[i].next = NULL;
        nodes[i].prev = NULL;
        if(prev != NULL) {
            prev->next = &nodes[i];
            nodes[i].prev = prev;
        }

        prev = &nodes[i];
    }

    freeNodes->head = &nodes[0];
    freeNodes->tail = &nodes[LIST_MAX_NUM_NODES - 1];
    freeNodes->num_nodes = LIST_MAX_NUM_NODES;

    if(flIndex < 0) // Check if there are any free lists
        return NULL;
    return freeLists[flIndex--];
}

// Returns the number of items in pList.
int List_count(List* pList) {
    if (!pList) // check if list/node is uninitialized
        return LIST_FAIL;

    return pList->num_nodes;
}

// Returns a pointer to the first item in pList and makes the first item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_first(List* pList) {
    if (emptyList(pList)) { // Check if list is empty/active
        pList->curr = NULL;
        return NULL;
    }

    // Set current item to first item
    pList->curr = pList->head;
    if(pList->status != LIST_OOB_NONE)   // Check if current item is out of bounds
        pList->status = LIST_OOB_NONE;   // If so, set status to NULL

    return pList->curr->item;
}

// Returns a pointer to the last item in pList and makes the last item the current item.
// Returns NULL and sets current item to NULL if list is empty.
void* List_last(List* pList) {
    // Check if list is empty
    if (emptyList(pList)) {
        pList->curr = NULL;
        return NULL;
    }

    // Set current item to last item
    pList->curr = pList->tail;
    if(pList->status != LIST_OOB_NONE)   // Check if current item is out of bounds
        pList->status = LIST_OOB_NONE;   // If so, set status to NULL

    return pList->curr->item;
}

// Advances pList's current item by one, and returns a pointer to the new current item.
// If this operation advances the current item beyond the end of the pList, a NULL pointer 
// is returned and the current item is set to be beyond end of pList.
void* List_next(List* pList) {
    // Check if current item is out of bounds
    if (!(pList->status == LIST_OOB_END || pList->status == LIST_OOB_START)) {
        pList->curr = pList->curr->next;
        if(pList->curr == NULL) {
            pList->status = LIST_OOB_END;
            return NULL;
        }
        return pList->curr->item;
    }
    else {  // If current item is out of bounds, move to next item
        if(pList->status == LIST_OOB_START) {
            pList->curr = pList->head;
            return List_curr(pList);
        }
        else
            return List_prev(pList);
    }

    return NULL;
}

// Backs up pList's current item by one, and returns a pointer to the new current item. 
// If this operation backs up the current item beyond the start of the pList, a NULL pointer 
// is returned and the current item is set to be before the start of pList.
void* List_prev(List* pList) {
    // Check if current item is out of bounds
    if (!(pList->status == LIST_OOB_END || pList->status == LIST_OOB_START)) {
        pList->curr = pList->curr->prev;
        if(pList->curr == NULL) {
            pList->status = LIST_OOB_START;
            return NULL;
        }
        return pList->curr->item;
    }
    else {  // If current item is out of bounds, move to previous item
        if(pList->status == LIST_OOB_END) {
            pList->curr = pList->tail;
            return List_curr(pList);
        }
        else
            return List_next(pList);
    }

    return NULL;
}

// Returns a pointer to the current item in pList.
void* List_curr(List* pList) {
    if (pList->curr == NULL || !pList || !pList->curr)  // Check if current item is NULL
        return NULL;

    return pList->curr->item;
}

// Adds the new item to pList directly after the current item, and makes item the current item. 
// If the current pointer is before the start of the pList, the item is added at the start. If 
// the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_after(List* pList, void* pItem) {
    Node *newNode = consumeFreeNode(pItem); // get a tail node from the free nodes list
    if(newNode != NULL) {
        if(emptyList(pList)) {  // add node to empty list
            pList->head = newNode;
            pList->tail = newNode;
            pList->status = LIST_OOB_NONE;
        }
        else {  // add node to non-empty list
            Node *temp;
            if(pList->status == LIST_OOB_END)
                temp = pList->tail;
            else
                temp = pList->curr;
            // link new node to current node
            Node *tempNext = temp->next;
            linkNodes(temp, newNode);
            linkNodes(newNode, tempNext);

            if(temp == pList->tail || pList->status == LIST_OOB_END) {
                pList->status = LIST_OOB_NONE;
                pList->tail = newNode;
            }
        }
        pList->curr = newNode;
        pList->num_nodes++;
        return LIST_SUCCESS;
    }
    else
        return LIST_FAIL;
}

// Adds item to pList directly before the current item, and makes the new item the current one. 
// If the current pointer is before the start of the pList, the item is added at the start. 
// If the current pointer is beyond the end of the pList, the item is added at the end. 
// Returns 0 on success, -1 on failure.
int List_insert_before(List* pList, void* pItem) {
    Node *newNode = consumeFreeNode(pItem);
    if(newNode != NULL){
        if(emptyList(pList)){
            pList->head = newNode;
            pList->tail = newNode;
            pList->status = LIST_OOB_NONE;
        }
        else {
            Node *temp;
            if(pList->curr != NULL) // If current is null, then we are at the start of the list
                temp = pList->curr;
            else
                temp = pList->head;

            Node *tempPrev = temp->prev;
            linkNodes(tempPrev, newNode);
            linkNodes(newNode, temp);

            if(temp == pList->head || pList->status == LIST_OOB_START) {
                pList->status = LIST_OOB_NONE;
                pList->head = newNode;
            }
        }
        pList->curr = newNode;
        pList->num_nodes++;
        return LIST_SUCCESS;
    }
    else
        return LIST_FAIL;
}

// Adds item to the end of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_append(List* pList, void* pItem) {
    List_last(pList);
    return List_insert_after(pList, pItem);
}

// Adds item to the front of pList, and makes the new item the current one. 
// Returns 0 on success, -1 on failure.
int List_prepend(List* pList, void* pItem) {
    List_first(pList);
    return List_insert_before(pList, pItem);
}

// Return current item and take it out of pList. Make the next item the current one.
// If the current pointer is before the start of the pList, or beyond the end of the pList,
// then do not change the pList and return NULL.
void* List_remove(List* pList) {
    if(pList->status == LIST_OOB_END || pList->status == LIST_OOB_START)    // Check if current item is out of bounds
        return NULL;

    Node * current = pList->curr;
    if(current != NULL) {
        List_next(pList);
        // If current is the head, then move the head to the next node
        if(current == pList->head)
            pList->head = pList->curr;

        // If current is the tail, then move the tail to the previous node
        Node * currentPrev = current->prev;
        Node * currentNext = current->next;
        unlinkNodes(current, currentPrev, currentNext); // Unlink current node from list
        pList->curr--;
        void * item = current->item;
        produceFreeNode(current);   // Add current node to free nodes list
        return item;
    }
    return NULL;
}

// Return last item and take it out of pList. Make the new last item the current one.
// Return NULL if pList is initially empty.
void* List_trim(List* pList) {
    if(emptyList(pList))
        return NULL;

    List_last(pList);   // Move current to the end of the list
    Node * current = pList->curr;
    if(current != NULL) {
        List_prev(pList);   // Move current to the previous node
        // If current is the head, then move the head to the next node
        if(current == pList->tail){
            pList->tail = pList->curr;
        }
        // If current is the tail, then move the tail to the previous node
        if(current == pList->head){
            pList->head = pList->curr;
        }

        Node * currentPrev = current->prev;
        Node * currentNext = current->next;
        unlinkNodes(current, currentPrev, currentNext); // Unlink current node from list

        pList->num_nodes--;
        void * item = current->item;
        produceFreeNode(current);   // Add current node to free nodes list
        return item;
    }
    return NULL;
}

// Adds pList2 to the end of pList1. The current pointer is set to the current pointer of pList1. 
// pList2 no longer exists after the operation; its head is available
// for future operations.
void List_concat(List* pList1, List* pList2) {
    if(!emptyList(pList2)) {    // If list is not empty, link the two lists
        if(!emptyList(pList1)) {
            pList1->head = pList2->tail;
            pList1->tail = pList2->tail;
            pList1->curr = pList2->curr;
            pList1->num_nodes = pList2->num_nodes;
            pList1->status = pList2->status;
        }
        else {
            linkNodes(pList1->tail, pList2->head);
            pList1->tail = pList2->tail;
            pList1->num_nodes += pList2->num_nodes;
            if(pList1->status == LIST_OOB_END)
                pList1->curr = pList2->head;
        }
    }

    if(pList2 != NULL){  // If list is not null, reset values and add to free list
        pList2->head = NULL;
        pList2->tail = NULL;
        pList2->curr = NULL;
        pList2->num_nodes = 0;
        pList2->status = LIST_OOB_START;

        if(flIndex < LIST_MAX_NUM_HEADS - 1)   // If there is space in free list, add list
            freeLists[++flIndex] = pList2;
    }
    
    return;
}

// Delete pList. pItemFreeFn is a pointer to a routine that frees an item. 
// It should be invoked (within List_free) as: (*pItemFreeFn)(itemToBeFreedFromNode);
// pList and all its nodes no longer exists after the operation; its head and nodes are 
// available for future operations.
void List_free(List* pList, FREE_FN pItemFreeFn) {
    if(pItemFreeFn == NULL)
        return;

    if(!emptyList(pList)){  // If list is not empty, free all nodes
        pList->curr = pList->head;
        while(pList->curr != NULL){
            Node * node = pList->curr;
            void * item = node->item;
            List_remove(pList);
            pItemFreeFn(item);
        }
    }
    if(pList != NULL){  // If list is not null, reset values and add to free list
        pList->head = NULL;
        pList->tail = NULL;
        pList->curr = NULL;
        pList->num_nodes = 0;
        pList->status = LIST_OOB_START;

        if(flIndex < LIST_MAX_NUM_HEADS - 1)  // If there is space in free list, add list
            freeLists[++flIndex] = pList;
    }
    return;
}

// Search pList, starting at the current item, until the end is reached or a match is found. 
// In this context, a match is determined by the comparator parameter. This parameter is a
// pointer to a routine that takes as its first argument an item pointer, and as its second 
// argument pComparisonArg. Comparator returns 0 if the item and comparisonArg don't match, 
// or 1 if they do. Exactly what constitutes a match is up to the implementor of comparator. 
// 
// If a match is found, the current pointer is left at the matched item and the pointer to 
// that item is returned. If no match is found, the current pointer is left beyond the end of 
// the list and a NULL pointer is returned.
// 
// If the current pointer is before the start of the pList, then start searching from
// the first node in the list (if any).
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg) {
    if(pComparator == pComparisonArg && pComparator == NULL)
        return NULL;
    
    if(!emptyList(pList)) { // If list is not empty, search for item
        if(pList->status == LIST_OOB_START) // If current is before start, move to start
            List_first(pList);
        else if(pList->status == LIST_OOB_END)  // If current is after end, move to end
            List_last(pList);

        void *item = NULL;
        while(pList->curr != NULL) {    // While current is not null, check if item matches
            item = List_curr(pList);
            if(pComparator(item, pComparisonArg) == 1)  // If item matches, return item
                return item;
            else    // If item does not match, move to next item
                List_next(pList);
        }

        return NULL;
    }

    return NULL;
}

// Functions to help debug the list

// Print the contents of the list
// void *printList(List *pList) {
//     if(!emptyList(pList)) { // If list is not empty, print list
//         pList->curr = pList->head;
//         while(pList->curr != NULL) {    // While current is not null, print item
//             void *item = List_curr(pList);
//             printf("%d ", *(int *)item);
//             List_next(pList);
//         }
//         printf("\n");
//     }
//     return NULL;
// }