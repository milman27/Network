#include <netinet/ip.h> 
#include <unistd.h>
#include <stdio.h>
#include "tls.c"
#include <malloc.h>
#include <string.h>
uint8_t * convertBigEndian(uint8_t* bytes, size_t len){
     for(int i = 0; i < len/2; i++){
        bytes[i] ^= bytes[len - i - 1]; 
        bytes[len - i - 1] ^= bytes[i];
        bytes[i] ^= bytes[len - i -1];
     }
     return bytes;
}
int openSocket(int port){
    in_port_t inport= htons(port);
    const struct in_addr sin_addr = {INADDR_ANY};
    const struct sockaddr_in ip = {AF_INET,inport,sin_addr,{0}};
    int ourSocket = socket(AF_INET, SOCK_STREAM, 0);
    const int optval = 1;
    int opt = setsockopt(ourSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    bind(ourSocket, (const struct sockaddr *)&ip,sizeof(ip));
    listen(ourSocket, 1);
    return ourSocket;
}
void * parseExtension(uint16_t extensionID,uint16_t extensionLength, uint8_t * extensionData){
    void * returnable = 0;
    switch(extensionID) {
       case 0:
            returnable = malloc(extensionLength); 
            struct extServerName * serverName = returnable;
            serverName->serverNameExtLength = *(uint16_t *)convertBigEndian(extensionData, 2);
            for(int i = 2, list = 0; 
                i < serverName->serverNameExtLength + 2; 
                list += 1) 
            { 
                serverName->items[list].entryType = *(uint8_t *)&extensionData[i];
                i += 1;
                serverName->items[list].entryLength = *(uint16_t *)convertBigEndian(&extensionData[i], 2);
                i += 2;
                serverName->items[list].entryValueLength = *(uint16_t *)convertBigEndian(&extensionData[i], 2);
                i += 2;
                serverName->items[list].entry = malloc(serverName->items[list].entryValueLength);
                memcpy(serverName->items[list].entry, &extensionData[i], serverName->items[list].entryValueLength);  
                i += serverName->items[list].entryValueLength;
            }
            break;
       default:
            returnable = 0;
            break;
   }
   return returnable;
}
int main(){
    int ourSocket = openSocket(1028);
    struct TLSHello hello = {0};
    uint8_t buffer[50];
    for(;;){
        int peer = accept(ourSocket, 0, 0);
        {
            read(peer, &buffer, 1);
            hello.recordHeader.type = *buffer;
            read(peer, &buffer, 2);
            convertBigEndian(buffer, 2);
            hello.recordHeader.protocol = *(uint16_t *)buffer;
            read(peer, &buffer, 2);
            convertBigEndian(buffer, 2);
            hello.recordHeader.messageLength = *(uint16_t *)buffer;
            
        }
        uint8_t message[hello.recordHeader.messageLength];
        read(peer, message, hello.recordHeader.messageLength);
        int index = 0;
        hello.handshakeHeader.type = message[index];
        index+=2;
        hello.handshakeHeader.handshakeLength = *(uint16_t *)convertBigEndian(&message[index], 2);
        index += 2;
        hello.clientVersion.minor = message[index];
        hello.clientVersion.major = message[++index];
        index++;
        for(int i = index; index < i + 32; index++){
            hello.clientRandom.random[index - i] = message[index]; 
        }

        hello.sessionID = malloc(message[index] + 1);
        hello.sessionID->IDLength = message[index++];
        for(int i = index; index < i + hello.sessionID->IDLength; index++){
            hello.sessionID->IDBytes[index - i] = message[index];
        }
        hello.cipherSuites = malloc(*convertBigEndian(&message[index], 2) + 2);
        hello.cipherSuites->cipherLength = *(uint16_t *)&message[index];
        index += 2;
        for(int i = index; index < i + hello.cipherSuites->cipherLength;index+=2){
            hello.cipherSuites->ciphers[(index - i)/2] = *(uint16_t *)convertBigEndian(&message[index], 2);
        }
        hello.compressionMethods = malloc(message[index] + 1);
        hello.compressionMethods->compressionMethodLength = message[index++];
        for(int i = index; index < i + hello.compressionMethods->compressionMethodLength;index+=1){
            hello.compressionMethods->compressionMethods[index - i] = message[index];
        }
        hello.extensions.extenstionsLength = *(uint16_t *)convertBigEndian(&message[index], 2);
        int extCount = 0;
        index +=2;
        for(int consumedExtBytes = 0; consumedExtBytes < hello.extensions.extenstionsLength;){
            hello.extensions.ext[extCount].extensionID = *(uint16_t *)convertBigEndian(&message[index], 2);
            consumedExtBytes += 2;
            index +=2;
            hello.extensions.ext[extCount].extensionLength = *(uint16_t *)convertBigEndian(&message[index], 2);
            consumedExtBytes += 2;
            index +=2;
            hello.extensions.ext[extCount].extensionBody = 
                parseExtension(hello.extensions.ext[extCount].extensionID,
                        hello.extensions.ext[extCount].extensionLength,
                        &message[index]);
            consumedExtBytes += hello.extensions.ext[extCount].extensionLength;
            index += hello.extensions.ext[extCount].extensionLength;
            extCount++;
        }

        printf("%d,%d,%d\nExtCount: %d\n", hello.recordHeader.type,
                hello.recordHeader.protocol,
                hello.recordHeader.messageLength,
                extCount);
    }
    
    return 0;
}
