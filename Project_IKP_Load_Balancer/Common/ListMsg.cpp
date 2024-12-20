﻿#pragma once

#include "listmsg.h"

#pragma region function for ..._MSG strukturu

// Inicijalizacija liste
LIST_MSG* init_list_msg()
{
    LIST_MSG* list = (LIST_MSG*)malloc(sizeof(LIST_MSG));
    if (list == NULL)
    {
        cout << "init_list() failed: out of memory" << endl;
        return NULL;
    }

    list->head = NULL;
    list->tail = NULL;
    list->count = 0;
    InitializeCriticalSection(&list->cs);

    return list;
}

// Dodavanje elementa na početak liste
void add_list_front_msg(LIST_MSG* list, LIST_ITEM_MSG data)
{
    if (list == NULL)
    {
        cout << "add_list_front() failed: list is NULL" << endl;
        return;
    }

    EnterCriticalSection(&list->cs);

    // Kreiramo novu stavku liste
    LIST_ITEM_MSG* item = (LIST_ITEM_MSG*)malloc(sizeof(LIST_ITEM_MSG));
    if (item == NULL)
    {
        cout << "add_list_front() failed: out of memory" << endl;
        LeaveCriticalSection(&list->cs);
        return;
    }

    // Prvo kopiramo podatke
    item->data = (char*)malloc(strlen(data.data) + 1);  // Alociramo memoriju za string
    if (item->data == NULL)
    {
        cout << "add_list_front() failed: out of memory for data" << endl;
        free(item);  // Oslobađamo memoriju za item, jer ne možemo da nastavimo
        LeaveCriticalSection(&list->cs);
        return;
    }

    item->data = _strdup(data.data); // Kopiramo podatke

    item->next = NULL;

    // Ako je lista prazna, postavljamo head i tail na ovaj element
    if (list->count == 0)
    {
        list->head = item;
        list->tail = item;
    }
    else
    {
        item->next = list->head;
        list->head = item;
    }

    list->count++;

    LeaveCriticalSection(&list->cs);
}

// Dohvatanje stavke sa određenog indeksa
LIST_ITEM_MSG* get_list_item_msg(LIST_MSG* list, int index)
{
    if (list == NULL)
    {
        cout << "get_list_item() failed: list is NULL" << endl;
        return NULL;
    }

    if (index < 0 || index >= list->count)
    {
        cout << "get_list_item() failed: index out of range" << endl;
        return NULL;
    }

    LIST_ITEM_MSG* item = list->head;
    for (int i = 0; i < index; i++)
    {
        item = item->next;
    }

    return item;
}


bool remove_from_list_msg(LIST_MSG* list, int index)
{
    if (list == NULL)
    {
        cout << "remove_from_list(): list is NULL" << endl;
        return false;
    }

    if (index < 0 || index >= list->count)
    {
        cout << "remove_from_list() failed: index out of range" << endl;
        return false;
    }

    EnterCriticalSection(&list->cs);

    LIST_ITEM_MSG* item = list->head;
    LIST_ITEM_MSG* prev = NULL;

    // Find the element at the specified index
    for (int i = 0; i < index; i++)
    {
        prev = item;
        item = item->next;
    }

    if (item == NULL)
    {
        cout << "remove_from_list() failed: item is NULL" << endl;
        LeaveCriticalSection(&list->cs);
        return false;
    }

    // Link previous element to the next, if applicable
    if (prev == NULL)
    {
        list->head = item->next;
    }
    else
    {
        prev->next = item->next;
    }

    if (item == list->tail)
    {
        list->tail = prev;
    }

    free(item);
    list->count--;
    LeaveCriticalSection(&list->cs);

    return true;
}

// Čišćenje liste
bool clear_list_msg(LIST_MSG* list)
{
    if (list == NULL)
    {
        cout << "clear_list() failed: list is NULL" << endl;
        return false;
    }

    EnterCriticalSection(&list->cs);

    while (list->count > 0)
    {
        if (!remove_from_list_msg(list, 0))
        {
            cout << "[WARN] clear_list() failed to remove an element from the list" << endl;
        }
    }

    LeaveCriticalSection(&list->cs);
    return true;
}

// Oslobađanje resursa liste
bool free_list_msg(LIST_MSG** list)
{
    if (list == NULL || *list == NULL)
    {
        cout << "free_list() failed: list is NULL" << endl;
        return true;
    }

    EnterCriticalSection(&(*list)->cs);
    clear_list_msg(*list);

    DeleteCriticalSection(&(*list)->cs);

    free(*list);
    *list = NULL;  // Set pointer to NULL after freeing to avoid dangling pointer

    return true;
}

// Ispisivanje liste
void print_list_msg(LIST_MSG* list)
{
    if (list == NULL)
    {
        cout << "print_list() failed: list is NULL" << endl;
        return;
    }

    if (list->count <= 0)
    {
        cout << "print_list(): list is empty" << endl;
        return;
    }

    LIST_ITEM_MSG* item = list->head;
    cout << "-------- List --------" << endl;
    cout << "List count: " << list->count << endl << "[";
    while (item != NULL)
    {
        cout << "Data: " << item->data << ", ";
        item = item->next;
    }
    cout << "]" << endl;
    cout << "----------------------" << endl;
}

#pragma endregion