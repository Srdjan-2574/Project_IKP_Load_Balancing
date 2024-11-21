﻿// Worker.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <winsock.h>
#include "../Common/hashtable.h"
#include <sstream>
#include <cstring>
using namespace std;
#define PORT 5059
#define MAX_TOKENS 100
#define MAX_TOKEN_LEN 255
#define _CRT_SECURE_NO_WARNINGS

int nWorkerSocket;
struct sockaddr_in srv;

HASH_TABLE_MSG* nClientWorkerMSGTable;



// Funkcija za podelu stringa po delimiteru
int split_string(const char* str, char delimiter, char output[MAX_TOKENS][MAX_TOKEN_LEN]) {
    int count = 0;
    char* token;
    char* str_copy = _strdup(str);  // Napravite kopiju stringa
    char* context = nullptr;  // Kontekst za strtok_s

    token = strtok_s(str_copy, &delimiter, &context);  // Koristimo sigurniju verziju strtok

    while (token != NULL && count < MAX_TOKENS) {
        strncpy_s(output[count], token, MAX_TOKEN_LEN - 1);  // Dodajte -1 da biste ostavili prostor za '\0'
        output[count][MAX_TOKEN_LEN - 1] = '\0';  // Osigurajte da string bude nul-terminiran
        token = strtok_s(NULL, &delimiter, &context);  // Za sledeći token
        count++;
    }

    free(str_copy);  // Oslobađanje memorije
    return count;
}

void ParseFromStringToHashTable(char* data)
{
    cout << endl << "Data to parse: " << data;

    // Podela podataka po ';' da bi se dobili klijenti
    char clients[MAX_TOKENS][MAX_TOKEN_LEN];
    int num_clients = split_string(data, ';', clients);

    for (int i = 0; i < num_clients; i++) {
        if (strlen(clients[i]) == 0) continue;  // Preskočite prazne stringove

        // Definišite kontekst za strtok_s
        char* context = nullptr;

        // Podelite klijenta po ":" da biste dobili ime klijenta i poruke
        char* client_name = strtok_s(clients[i], ":", &context);
        char* messages_str = strtok_s(nullptr, ":", &context);

        // Podela poruka po ','
        char messages[MAX_TOKENS][MAX_TOKEN_LEN];
        int num_messages = split_string(messages_str, ',', messages);

        // Dodavanje klijenta u tabelu
        if (add_list_table_msg(nClientWorkerMSGTable, client_name)) {}

        // Dodavanje poruka za tog klijenta
        for (int j = num_messages - 1; j >= 0; j--) {
            char* msg = messages[j]; // Poruka je već string, nije potrebno konvertovati u broj
            if (add_table_item_msg(nClientWorkerMSGTable, client_name, msg)) {}
        }
    }
}

void ParseAndAddToHashTable(char* msg)
{
    if (msg == NULL || strlen(msg) == 0)
    {
        cout << "Invalid message!" << endl;                                     
        return;
    }

    char* context = NULL;
    char* key = strtok_s(msg, ":", &context); 
    char* value = strtok_s(NULL, ":", &context);

    if (key != NULL && value != NULL)
    {
        cout << "Key: " << key << ", Value: " << value << endl;

        // Dodajte ključ i vrednost u hash tabelu
        if (!get_table_item_msg(nClientWorkerMSGTable, key))
        {
            if (add_list_table_msg(nClientWorkerMSGTable, key)) {}
        }
        if (add_table_item_msg(nClientWorkerMSGTable, key, value)) {}

        print_hash_table_msg(nClientWorkerMSGTable);
    }
    else
    {
        cout << "Message format is invalid!" << endl;
    }
}

void ProcessNewMessgae(SOCKET nWorkerSocket)
{
    cout << endl << "Procesing message from LB: " << nWorkerSocket;
    char buffer[256 + 1] = { 0, };

    // Proveri da li je socket još uvek otvoren pre nego što pozoveš recv
    int nRet = recv(nWorkerSocket, buffer, 256, 0);
    if (nRet <= 0)
    {
        cout << endl << "Something bad happen. Closing LB Socket" << endl;
        closesocket(nWorkerSocket);
    }
    else 
    {
        cout << endl << buffer << endl;
        ParseAndAddToHashTable(buffer);
    }
}

DWORD WINAPI ProcessLBMessage(LPVOID lpParam)
{
    char buffer[1024] = { 0 }; // Bafer za prijem podataka
    fd_set read_fds;           // Skup soketa za praćenje događaja za čitanje
    struct timeval timeout;    // Tajmaut za `select`
    int nRet;
    while (1)
    {
        FD_ZERO(&read_fds);
        FD_SET(nWorkerSocket, &read_fds);

        // Postavljanje tajmauta (1 sekunde)
        timeout.tv_sec = 1;

        // Praćenje događaja na soketu
        nRet = select(0, &read_fds, NULL, NULL, &timeout);
        if (nRet > 0) 
        {
            if (FD_ISSET(nWorkerSocket, &read_fds)) {
                ProcessNewMessgae(nWorkerSocket);
            }
            
        }

    }
}

int main()
{
    int nRet = 0;
    //Init WSA 
    WSADATA ws;
    if (WSAStartup(MAKEWORD(2, 2), &ws) < 0)
    {
        cout << endl << "WSA Failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "WSA Succes";
    }

    //Init Socket
    nWorkerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (nWorkerSocket < 0)
    {
        cout << endl << "Socket not open";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        cout << endl << "Socket open";
    }

    //Init env for sockaddr structure
    srv.sin_family = AF_INET;
    srv.sin_port = htons(PORT);
    srv.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(&(srv.sin_zero), 0, 8);

    //connect
    nRet = connect(nWorkerSocket, (struct sockaddr*)&srv, sizeof(srv));
    if (nRet < 0)
    {
        cout << endl << "Connect Failed";
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    else
    {
        send(nWorkerSocket, "WORKER", 6, 0);

        cout << endl << "WORKER Connect Success";
        //Server accepted
        char buffer[255] = { 0, };

        recv(nWorkerSocket, buffer, 255, 0);
        cout << endl << buffer;
    }

    nClientWorkerMSGTable = init_hash_table_msg();

    char buffer[255] = { 0, };

    recv(nWorkerSocket, buffer, 255, 0);
    
    ParseFromStringToHashTable(buffer);

    cout << endl;
    print_hash_table_msg(nClientWorkerMSGTable);
    
    HANDLE hThread1 = CreateThread(NULL, 0, ProcessLBMessage, NULL, 0, NULL);

    WaitForSingleObject(hThread1, INFINITE);

    cout << endl << "Press any to exit... Worker is Running";
    char exit = getchar();
    WSACleanup();
}

