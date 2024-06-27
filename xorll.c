#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

typedef struct Xor_Element Xor_Element;
struct Xor_Element {
    uintptr_t delta_pointer;
};

typedef struct Xor_Linked_List Xor_Linked_List;
struct Xor_Linked_List {
    Xor_Element *first;
    Xor_Element *last;
};

typedef struct Xor_Iterator Xor_Iterator;
struct Xor_Iterator {
    Xor_Element *prev;
    Xor_Element *next;
};

void xor_linked_list_append(Xor_Linked_List *list, Xor_Element *element) {
    if (list->last == NULL) {
        assert(list->first == NULL && "If last is null, first should also be null");
        list->last = list->first = element;
    } else {
        assert(list->first && "If last is not null, first should also be not null");
        element->delta_pointer = (uintptr_t) list->last /* ^ NULL */;
        list->last->delta_pointer = list->last->delta_pointer ^ (uintptr_t) element;
        list->last = element;
    }
}

void xor_linked_list_push(Xor_Linked_List *list, Xor_Element *element) {
    if (list->first == NULL) {
        assert(list->last == NULL && "If first is null, last should also be null");
        list->first = list->last = element /* ^ NULL */;
    } else {
        assert(list->last && "If first is not null, last should also be not null");
        element->delta_pointer = (uintptr_t) list->first;
        list->first->delta_pointer = list->first->delta_pointer ^ (uintptr_t) element;
        list->first = element;
    }
}

void xor_linked_list_reverse(Xor_Linked_List *list) {
    Xor_Element *first = list->first;
    list->first = list->last;
    list->last = first;
}

Xor_Iterator xor_linked_list_iterate(Xor_Linked_List list) {
    return (Xor_Iterator) {
        .prev = NULL,
        .next = list.first,
    };
}

Xor_Element *xor_iterate_next(Xor_Iterator *it) {
    Xor_Element *res = it->next;
    Xor_Element *cur = it->next;
    it->next = (Xor_Element *) (it->next->delta_pointer ^ (uintptr_t) it->prev);
    it->prev = cur;
    return res;
}

void xor_iterate_insert(Xor_Iterator *it, Xor_Element *element) {
    element->delta_pointer = (uintptr_t) it->prev ^ (uintptr_t) it->next;
    it->prev->delta_pointer = it->prev->delta_pointer ^ (uintptr_t) it->next ^ (uintptr_t) element;
    it->next->delta_pointer = it->next->delta_pointer ^ (uintptr_t) it->prev ^ (uintptr_t) element;
    it->next = element;
}

void xor_iterato_remove(Xor_Iterator *it) {
    Xor_Element *prev_prev = (Xor_Element *) (it->prev->delta_pointer ^ (uintptr_t) it->next);
    prev_prev->delta_pointer = prev_prev->delta_pointer ^ (uintptr_t) it->prev ^ (uintptr_t) it->next;
    it->next->delta_pointer = it->next->delta_pointer ^ (uintptr_t) it->prev ^ (uintptr_t) prev_prev;
    it->prev = prev_prev;
}

// Example usage
typedef struct Node Node;
struct Node {
    Xor_Element list;
    int val;
};

int main(void) {
    Xor_Linked_List list = {0};
    for (int i = 0; i < 10; ++i) {
        Node *node = calloc(1, sizeof(Node));
        assert(node != NULL);
        node->val = i;
        if (i % 2 == 0) {
            xor_linked_list_append(&list, (Xor_Element *) node);
        } else {
            xor_linked_list_push(&list, (Xor_Element *) node);
        }
    }

    {
        Xor_Iterator it = xor_linked_list_iterate(list);
        for (int i = 0; i < 5; ++i) {
            xor_iterate_next(&it);
        }
        Node *node = calloc(1, sizeof(Node));
        node->val = 99;
        xor_iterate_insert(&it, (Xor_Element *) node);
        Node *next = (Node *) xor_iterate_next(&it);
        assert(next == node);
        xor_iterato_remove(&it);
        assert(((Node *)it.prev)->val == 1);
    }

    {
        Xor_Iterator it = xor_linked_list_iterate(list);
        while (it.next) {
            Node *node = (Node *) xor_iterate_next(&it);
            printf("%d\n", node->val);
        }
    }

    printf("-----------------------------------------------\n");

    {
        xor_linked_list_reverse(&list); // O(1)
        Xor_Iterator it = xor_linked_list_iterate(list);
        while (it.next) {
            Node *node = (Node *) xor_iterate_next(&it);
            printf("%d\n", node->val);
        }
    }
    return 0;
}
