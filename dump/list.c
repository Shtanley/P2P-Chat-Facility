#include "list.h"
#include <stdio.h>

// List variables
static List lists[LIST_MAX_NUM_HEADS + 1];
static List * freeLists[LIST_MAX_NUM_HEADS]; // Store list pointers
static int freeListIndex = 0;

static Node nodes[LIST_MAX_NUM_NODES];
static List * freeNodes;

// ---- Private methods ---- //
bool isListEmpty(List * pList) {
    if(pList == NULL || (pList->head == NULL && pList->tail == NULL && pList->curr == NULL && pList->num_nodes == 0)){
        return true;
    }
    else {
        return false;
    }
}

bool isListOOB(List * pList) {
    return (pList->status == LIST_OOB_END || pList->status == LIST_OOB_START);
}

Node * getFreeNode(void * item) {
    if(freeNodes->tail != NULL){
        // Popping off new node from freeNodes list
        Node * nodeToReturn = freeNodes->tail;
        freeNodes->tail = freeNodes->tail->prev;
        nodeToReturn->prev = NULL;
        nodeToReturn->next = NULL;
        nodeToReturn->item = item;
        freeNodes->num_nodes--;
        return nodeToReturn;
    }
    return NULL;
}

void returnFreeNode(Node * node) {
    // Clearing returning node
    node->item = NULL;
    node->next = NULL;
    node->prev = NULL;

    // Changing tail
    if(freeNodes->tail != NULL) {
        Node * prevTail = freeNodes->tail;
        freeNodes->tail = node;
        prevTail->next = node;
        node->prev = prevTail;
    }
    // Creating new tail when freeNodes is empty
    else{
        freeNodes->tail = node;
    }
    freeNodes->num_nodes++;
}

List * getFreeList(){
    if(freeListIndex >= 0){
     
        List * listToReturn = freeLists[freeListIndex--];
        return listToReturn;
    }
    return NULL;
}

bool returnFreeList(List *pList){
    if(pList != NULL){
        pList->num_nodes = 0;
        pList->curr = NULL;
        pList->head = NULL;
        pList->tail = NULL;
        pList->status = LIST_OOB_START;

        if(freeListIndex < LIST_MAX_NUM_HEADS - 1){
            freeLists[++freeListIndex] = pList;
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

void Unlink_node(Node * nodeToRemove, Node * prev, Node * next){
    // Remove node only if it is not null
    if(nodeToRemove != NULL){
        // Remove links from previous and next nodes of
        // the removing node, if they exist
        nodeToRemove->next = NULL;
        nodeToRemove->prev = NULL;
        if(next != NULL && prev != NULL){
            prev->next = next;
            next->prev = prev;
        }
        else if(prev != NULL){
            prev->next = NULL;
        }
        else if(next != NULL) {
            next->prev = NULL;
        }

    }
}

void Link_2_nodes(Node * first, Node * second) {
    // Links first and second node if both are not null
    if(first != NULL && second != NULL) {
        first->next = second;
        second->prev = first;
    }
}

void Link_3_nodes(Node * first, Node * second, Node * third){
    // Call 2 separate links for the three nodes
    Link_2_nodes(first, second);
    Link_2_nodes(second, third);
}

void List_add_on_empty(List *pList, Node * newNode){
    // Point head and tail to new node when list empty
    pList->head = newNode;
    pList->tail = newNode;
    pList->status = LIST_NOT_OOB;
}
// ---- List methods ---- //
List* List_create() {
    // Allocating Lists
    for(int i = 0; i < LIST_MAX_NUM_HEADS; i++) {
        lists[i] = (List){
            NULL, NULL, NULL, LIST_OOB_START, 0
        };
        freeLists[i] = &lists[i];
    }
    freeListIndex = LIST_MAX_NUM_HEADS - 1;


    // Allocating Nodes
    lists[LIST_MAX_NUM_HEADS] = (List) {
        NULL, NULL, NULL, LIST_NOT_OOB, 0
    };
    freeNodes = &lists[LIST_MAX_NUM_HEADS];
    Node * prev = NULL;
    for(int i = 0; i < LIST_MAX_NUM_NODES; i++){
        nodes[i] = (Node) {
            NULL, NULL, NULL
        };
        if(prev != NULL) {
            prev->next = &nodes[i];
            nodes[i].prev = prev;
        }

        prev = &nodes[i];
    }

    freeNodes->head = &nodes[0];
    freeNodes->tail = &nodes[LIST_MAX_NUM_NODES - 1];
    freeNodes->num_nodes = LIST_MAX_NUM_NODES;

    return getFreeList();
}

int List_count(List* pList) {
    if(isListEmpty(pList)){
        return 0;
    }
    else {
        return pList->num_nodes;
    }
}

void* List_first(List* pList) {
    if(isListEmpty(pList)){
        pList->curr = NULL;
        return NULL;
    }
    pList->curr = pList->head;
    if(pList->status != LIST_NOT_OOB){
        pList->status = LIST_NOT_OOB;
    }
    return pList->head->item;
}

void* List_last(List* pList){
    if(isListEmpty(pList)) {
        pList->curr = NULL;
        return NULL;
    }
    pList->curr = pList->tail;
    if(pList->status != LIST_NOT_OOB){
        pList->status = LIST_NOT_OOB;
    }
    return pList->tail->item;
}

void* List_next(List* pList){
    if(!isListOOB(pList)){
        pList->curr = pList->curr->next;
        if(pList->curr == NULL){
            pList->status = LIST_OOB_END;
            return NULL;
        }
        return pList->curr->item;
    } 
    else {
        if(pList->status == LIST_OOB_START) {
            pList->curr = pList->head;
            return List_curr(pList);
        }
        else {
            return List_prev(pList);
        }
    }
    return NULL;
}

void* List_prev(List* pList){
    if(!isListOOB(pList)){
        pList->curr = pList->curr->prev;
        if(pList->curr == NULL){
            pList->status = LIST_OOB_START;
            return NULL;
        }
        return pList->curr->item;
    } 
    else {
        if(pList->status == LIST_OOB_END) {
            pList->curr = pList->tail;
            return List_curr(pList);
        }
        else {
            return List_next(pList);
        }
    } 
    return NULL;
}

void* List_curr(List* pList){
    if(pList->curr == NULL){
        return NULL;
    }
    return pList->curr->item;
}

int List_add(List* pList, void* pItem){
    if(pList->status == LIST_OOB_START) {
        return List_insert(pList, pItem);
    }
    Node * newNode = getFreeNode(pItem);
    if(newNode != NULL){
        if(isListEmpty(pList)){
            List_add_on_empty(pList, newNode);
        }
        else {
            Node * prevCurrent = (pList->curr != NULL) // If curr is null, then we are at the end of the list
                                    ? pList->curr : pList->tail;
            Node * prevCurrentNext = prevCurrent->next;
            Link_3_nodes(prevCurrent, newNode, prevCurrentNext);

            if(prevCurrent == pList->tail || pList->status == LIST_OOB_END) {
                pList->status = LIST_NOT_OOB;
                pList->tail = newNode;
            }
        }
        pList->curr = newNode;
        pList->num_nodes++;
        return 0;
    }
    else {
        return -1;
    }
}

int List_insert(List* pList, void* pItem){
    if(pList->status == LIST_OOB_END){
        return List_add(pList, pItem);
    }
    Node * newNode = getFreeNode(pItem);
    if(newNode != NULL){
        if(isListEmpty(pList)){
            List_add_on_empty(pList, newNode);
        }
        else {
            Node * prevCurrent = (pList->curr != NULL) 
                                    ? pList->curr : pList->head;
            Node * prevCurrentPrev = prevCurrent->prev;
            Link_3_nodes(prevCurrentPrev, newNode, prevCurrent);

            if(prevCurrent == pList->head || pList->status == LIST_OOB_START) {
                pList->status = LIST_NOT_OOB;
                pList->head = newNode;
            }
        }
        pList->curr = newNode;
        pList->num_nodes++;
        return 0;
    }
    else {
        return -1;
    }
}

int List_append(List* pList, void* pItem){
    List_last(pList);
    return List_add(pList, pItem);
}

int List_prepend(List* pList, void* pItem){
    List_first(pList);
    return List_insert(pList, pItem);
}

void* List_remove(List* pList){
    if(pList->status == LIST_OOB_END || pList->status == LIST_OOB_START){
        return NULL;
    }
    Node * curr = pList->curr;
    if(curr != NULL) {
        
        List_next(pList);
        if(curr == pList->head){
            pList->head = pList->curr;
        }
        
        Node * currentPrev = curr->prev;
        Node * currentNext = curr->next;
        Unlink_node(curr, currentPrev, currentNext);

        pList->num_nodes--;
        void * item = curr->item;
        returnFreeNode(curr);
        return item;
    }
    return NULL;
}

void List_concat(List* pList1, List* pList2){
    if(!isListEmpty(pList2)){
        if(isListEmpty(pList1)){
            pList1->head = pList2->head;
            pList1->tail = pList2->tail;
            pList1->curr = pList2->curr;
            pList1->num_nodes = pList2->num_nodes;
            pList1->status = pList2->status;
        }
        else {
            Link_2_nodes(pList1->tail, pList2->head);
            pList1->num_nodes += pList2->num_nodes;
            pList1->tail = pList2->tail;
            if(pList1->status == LIST_OOB_END){
                pList1->curr = pList2->head;
            }
        }
    }
    returnFreeList(pList2);    
}

typedef void (*FREE_FN)(void* pItem);
void List_free(List* pList, FREE_FN pItemFreeFn){
    if(pItemFreeFn == NULL) {
        return;
    }
    if(!isListEmpty(pList)){
        pList->curr = pList->head;
        while(pList->curr != NULL){
            Node * node = pList->curr;
            void * item = node->item;
            List_remove(pList);
            pItemFreeFn(item);
        }
    }
    returnFreeList(pList);
}

void* List_trim(List* pList){
    if(isListEmpty(pList)){
        return NULL;
    }
    List_last(pList);
    Node * curr = pList->curr;
    if(curr != NULL) {
        
        List_prev(pList);
        
        if(curr == pList->tail){
            pList->tail = pList->curr;
        }

        if(curr == pList->head){
            pList->head = pList->curr;
        }

        Node * currentPrev = curr->prev;
        Node * currentNext = curr->next;
        Unlink_node(curr, currentPrev, currentNext);

        pList->num_nodes--;
        void * item = curr->item;
        returnFreeNode(curr);
        return item;
    }
    return NULL;
}

typedef bool (*COMPARATOR_FN)(void* pItem, void* pComparisonArg);
void* List_search(List* pList, COMPARATOR_FN pComparator, void* pComparisonArg){
    if(pComparator == pComparisonArg && pComparator == NULL) {
        return NULL;
    }
    if(!isListEmpty(pList)){

        if(pList->curr == NULL){
            List_first(pList);
        }
      
        void * item = NULL;
        while(pList->curr != NULL){
            item = List_curr(pList);
            if(pComparator(item, pComparisonArg)){
                return item;
            }
            else{
                List_next(pList);
            }
        }
        return NULL;
    }
    return NULL;
}