#include "parse.h"
void destroyParsedHTTP(HTTPRequest* request){
    for(int i = request->length; i > 0; i--){
        free(request->headers[i].value);
        free(request->headers[i].key);
    }
    free(request->headers[0].value);
    free(request->headers);
    free(request);
}
HTTPRequest* parseHTTP(token* tokens){
    HTTPRequest* http = malloc(sizeof(HTTPRequest));
    http->headers = calloc(128, sizeof(void*));
    int i = 0;

    for(;tokens[i].type == WHITESPACE || tokens[i].type == NEWLINE;i++){}
    if(stringCmp(tokens[i].start, "GET", tokens[i].length)){
        i++;
        http->rType = GET;
    }else{
        printf("%s",tokens[i].start);
        i++;
        free(http->headers);
        free(http);
        return NULL;
    }
    if(tokens[i].type == WHITESPACE)
        i++;
    int sum = 0;
    int j = 0;
    for(;tokens[i+j].type != WHITESPACE;j++){
        sum += tokens[i+j].length; 
    }
    http->headers[0].key = "Path";
    http->headers[0].value = malloc((sum+1)*sizeof(char));
    memcpy(http->headers[0].value, tokens[i].start, sum);
    http->headers[0].value[sum] = '\0';
    http->length = 1; 

    i += j;
    j = 0;
    sum = 0;
    if(tokens[i].type == WHITESPACE)
        i++;
    for(;tokens[i+j].type != NEWLINE; j++){
        sum += tokens[i+j].length;
    }
    if(stringCmp("HTTP/1.1" , tokens[i].start, sum)){
        http->rProto = v11;   
    }else{
        http->rProto = INVALIDPROTOCOL;
    }
    
    i += j;
    for(;;){
        if(tokens[i+1].type == END)
            break;
        if(http->length >= 126)
            break;
        if(tokens[i].type == NEWLINE)
            i++;
        j = 0;
        sum = 0;
        for(;tokens[i+j].type != COLON;j++){
            sum += tokens[i+j].length;
        }
        http->headers[http->length].key = malloc((sum+1)*sizeof(char));
        memcpy(http->headers[http->length].key, tokens[i].start, sum);
        http->headers[http->length].key[sum] = '\0';
        i += j;
        sum = 0;
        j = 0;
        if(tokens[i].type == COLON)
            i++;
        if(tokens[i].type == WHITESPACE)
            i++;
        for(;tokens[i+j].type != NEWLINE;j++){
            sum += tokens[i+j].length;
        }
        http->headers[http->length].value = malloc((sum+1)*sizeof(char));
        memcpy(http->headers[http->length].value, tokens[i].start, sum);
        http->headers[http->length].value[sum] = '\0';
        i += j;
        http->length++;
    }
    return http;
}
int stringCmp(char* first, char* sec, int len){
    if(first == NULL || sec == NULL)
        return 0;
    
    for(int i = 0; i < len; i++){
        if(first[i] == '\0' || sec[i] == '\0')
            return first[i] == sec[i];
        if(first[i] != sec[i])
            return 0;
    }
    return 1;
}
token* tokenizeString(char* string){
    token* tokens = malloc(strlen(string)*sizeof(token));
    tokens[0].type = INVALID;
    int j = 0;
    enum types type = INVALID;
    for(int i = 0;;i++){
        if((type = evalChar(string[i])) == END || type == SEMICOLON || type == COLON){
           tokens[++j].type = type;
           tokens[j].start = &string[i];
           tokens[j].length = 1;
           if(type == END) break; 
        }else{
            if(tokens[j].type != INVALID && tokens[j].type != type){
                j++;
            }
            if(tokens[j].type == type){
                tokens[j].length++; 
            }else{
                tokens[j].type = type;
                tokens[j].start = &string[i];
                tokens[j].length = 1;
            }
        }
    }
    return tokens;
}
char* valFromKey(kVPair* map, char* key){
    int len = strlen(key);
    for(int i = 0; map[i].key != NULL;i++){
       if(stringCmp(key, map[i].key, len)){
            return map[i].value;
       }
    }
   return NULL;
}
enum types evalChar(char chara){
    if(chara == '\n' || chara == '\r')
        return NEWLINE;
    if(chara == '\t' || chara == ' ')
        return WHITESPACE;
    if(chara <= '9' && chara >= '0') 
        return NUM;
    if(chara <= 'z' && chara >= 'A') 
        return ALPHA;
    if(chara == '\0')
        return END;
    if(chara == ':')
        return COLON;
    if(chara == ';')
        return SEMICOLON;
    return OTHER;
}
