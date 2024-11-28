#pragma once
#ifndef NETWORKING_UTILS_H
#define NETWORKING_UTILS_H

#include <winsock.h>
#include <stdio.h>
#include <string.h>
#include "../Common/hashtable.h"
#include "../Common/queueLBtoWorker.h"

// External hash table declaration
extern HASH_TABLE* nClientWorkerSocketTable;
extern QUEUE* nClientMsgsQueue;
extern HASH_TABLE_MSG* nClientMSGTable;

// Function declaration
void process_new_request(SOCKET clientSocket);
void ProcessNewMessageOrDisconnectWorker(int nWorkerSocket);
void ProcessNewMessage(int nClientSocket);
int serialize_hash_table(HASH_TABLE_MSG* table, char* buffer, size_t size);
int send_hash_table(SOCKET socket, HASH_TABLE_MSG* table);
#endif // NETWORKING_UTILS_H