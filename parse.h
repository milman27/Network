#ifndef PARSEH
#define PARSEH
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
enum types {
    INVALID, 
    WHITESPACE, 
    NEWLINE,
    ALPHA,
    NUM,
    OTHER,
    END,
    COLON,
    SEMICOLON,

};
enum requestTypes{
  INVALIDREQUEST,
  GET,
  POST,
  PUT,
};
typedef struct {
    enum types type;
    int length;
    char* start;

}token;
typedef struct {
   char* key;
   char* value;
}kVPair;
enum requestProto { 
  INVALIDPROTOCOL,
  v9,
  v1,
  v11,
  v2,
  v3,

};
typedef struct {
    enum requestTypes rType;
    enum requestProto rProto;
    int length; 
    kVPair* headers;
}HTTPRequest;
enum types evalChar(char);
int stringCmp(char* first, char* sec, int len);
token* tokenizeString(char* string);
HTTPRequest* parseHTTP(token* tokens);
char* valFromKey(kVPair*, char*);
#endif
