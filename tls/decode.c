#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define ROTL8(x,shift) ((uint8_t) ((x) << (shift)) | ((x) >> (8 - (shift))))

#define ROTR32(x,shift) ((uint32_t) ((x) >> (shift * 8)) | ((x) << (32 - (shift * 8))))
#define ROTL32(x,shift) ((uint32_t) ((x) << (shift * 8)) | ((x) >> (32 - (shift * 8))))
static uint8_t subbox[256];
static uint8_t revbox[256];
static uint32_t rcon[11] = {0, 1 , 2 , 4 , 8 , 0x10 , 0x20 , 0x40 , 0x80 , 0x1B , 0x36 };
void initAESSBox(uint8_t sbox[256]) {
    uint8_t p = 1, q = 1;

    /* loop invariant: p * q == 1 in the Galois field */
    do {
        /* multiply p by 3 */
        p = p ^ (p << 1) ^ (p & 0x80 ? 0x1B : 0);

        /* divide q by 3 (equals multiplication by 0xf6) */
        q ^= q << 1;
        q ^= q << 2;
        q ^= q << 4;
        q ^= q & 0x80 ? 0x09 : 0;

        /* compute the affine transformation */
        uint8_t xformed = q ^ ROTL8(q, 1) ^ ROTL8(q, 2) ^ ROTL8(q, 3) ^ ROTL8(q, 4);

        sbox[p] = xformed ^ 0x63;
    } while (p != 1);

    /* 0 is a special case since it has no inverse */
    sbox[0] = 0x63;
}
void initAESRBox( uint8_t rbox[256], const uint8_t sbox[256]){
    for(int i = 0; i < 256; i++){
        rbox[sbox[i]] = i;
    }
}
uint32_t subWord(uint32_t number){
    return subbox[number & 0xFF] | (subbox[(number >> 8) & 0xFF] << 8) | (subbox[(number >> 16) & 0xFF] << 16) | (subbox[(number >> 24) & 0xFF] << 24);
}
void genKeys(int N, const uint32_t* key, uint32_t* newKeys){
    int R = N + 7;
    for(int i = 0; i < 4*R; i++){
        if(i < N){
            newKeys[i] = key[i];
        }else if(i >= N && !(i%N)){
            uint32_t temp = ROTR32(newKeys[i-1], 1);
            temp = subWord(temp);
            temp ^= rcon[i/N];
            newKeys[i] = newKeys[i-N]^temp;
        }else if(i >= N && N > 6 && (i%N ==4)){
            newKeys[i] = newKeys[i-N]^subWord(newKeys[i-1]);
        }else{
            newKeys[i] = newKeys[i-N]^newKeys[i-1];
        }
    }
}	
char hexToChar(char* hex){
    if((((hex[0]|0x20) >= 'a' && (hex[0]|0x20) <= 'f') || (hex[0] >= '0' && hex[0] <= '9')) &&
            (((hex[1]|0x20) >= 'a' && (hex[1]|0x20) <= 'f') || (hex[1] >= '0' && hex[1] <= '9'))){
        return ((hex[0] >= 'a') ? 0x10*(hex[0] - 'a' + 10) : 0x10*(hex[0] - '0')) + ((hex[1] >= 'a') ? (hex[1] - 'a' + 10) : (hex[1] - '0'));
    }else{
        return 0;
    }
}
void addRoundKey(uint32_t roundKey[4], uint32_t* state){
    state[0] ^= roundKey[0];
    state[1] ^= roundKey[1];
    state[2] ^= roundKey[2];
    state[3] ^= roundKey[3];
}
void shiftRows(uint8_t* state){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j<(i%4); j++){
            uint8_t temp = state[i%4];
            state[i%4] = state[i%4 + 4];
            state[i%4 + 4] = state[i%4 + 8];
            state[i%4 + 8] = state[i%4 + 12];
            state[i%4 + 12] = temp;
        }
    }
}
void revShiftRows(uint8_t* state){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j<(i%4); j++){
            uint8_t temp = state[i%4 + 12];
            state[i%4 + 12] = state[i%4 + 8];
            state[i%4 + 8] = state[i%4 + 4];
            state[i%4 + 4] = state[i%4];
            state[i%4] = temp;
        }
    }

}
void mixColumns(uint8_t* state){
    for(int i = 0; i < 4; i++){
        uint8_t tempcol[4] = {state[i*4],state[i*4 +1],state[i*4 + 2],state[i*4 + 3]};
        uint8_t doubletemp[4] = {(tempcol[0] >> 7) ? (tempcol[0]<< 1)^0x1B : (tempcol[0] << 1),
            (tempcol[1] >> 7) ? (tempcol[1] << 1)^0x1B : (tempcol[1] << 1), 
            (tempcol[2] >> 7) ? (tempcol[2] << 1)^0x1B : (tempcol[2] << 1), 
            (tempcol[3] >> 7) ? (tempcol[3] << 1)^0x1B : (tempcol[3] << 1)}; 

        state[i*4] = doubletemp[0] ^ doubletemp[1] ^ tempcol[1] ^ tempcol[2] ^ tempcol[3];
        state[i*4 + 1] = tempcol[0] ^ doubletemp[1] ^ tempcol[2] ^ doubletemp[2] ^ tempcol[3];
        state[i*4 + 2] = tempcol[0] ^ tempcol[1] ^ doubletemp[2] ^ doubletemp[3] ^ tempcol[3];
        state[i*4 + 3] = doubletemp[0] ^ tempcol[0] ^ tempcol[1] ^ tempcol[2] ^ doubletemp[3];
    }
}
uint8_t gf256mul(uint8_t a, uint8_t b){
    uint8_t p = 0;
    for(int i = 0; i < 8 && b && a; i++){
        p ^= (b & 1) ? a: 0;	
        b = b >> 1;
        int c = a >> 7;
        a = a << 1;
        a ^= c * 0x1B;
    }
    return p;
}
void revMixColumns(uint8_t* state){
    for(int i = 0; i < 4; i++){
        uint8_t temp[4] = {state[i*4],state[i*4 +1],state[i*4 + 2],state[i*4 + 3]};
        state[i*4] = gf256mul(14, temp[0]) ^ gf256mul(11, temp[1]) ^ gf256mul(13, temp[2]) ^ gf256mul(9, temp[3]);
        state[i*4 + 1] = gf256mul(9, temp[0]) ^ gf256mul(14, temp[1]) ^ gf256mul(11, temp[2]) ^ gf256mul(13, temp[3]);
        state[i*4 + 2] = gf256mul(13, temp[0]) ^ gf256mul(9, temp[1]) ^ gf256mul(14, temp[2]) ^ gf256mul(11, temp[3]);
        state[i*4 + 3] = gf256mul(11, temp[0]) ^ gf256mul(13, temp[1]) ^ gf256mul(9, temp[2]) ^ gf256mul(14, temp[3]);
    }	
}
void RSAenc(uint8_t* key, uint8_t* state, uint8_t* iV){
    initAESSBox(subbox);
    initAESRBox(revbox, subbox);
    uint32_t newKeys[44];
    genKeys(4, (uint32_t *)key, &newKeys[0]);
    addRoundKey(&newKeys[0], (uint32_t *) state);
    for(int i = 1; i <= 9; i++){
        for(int j = 0; j < 16; j++){
            state[j] = subWord(state[j]);
        }
        shiftRows(state);
        mixColumns(state);
        addRoundKey(&newKeys[4*i], (uint32_t *)state);
    }
    for(int j = 0; j < 16; j++){
        state[j] = subWord(state[j]);
    }
    shiftRows(state);
    addRoundKey(&newKeys[40], (uint32_t *)state);
}
void RSAdec(uint8_t* key, uint8_t* state){
    initAESSBox(subbox);
    initAESRBox(revbox, subbox);
    uint32_t newKeys[44];
    genKeys(4, (uint32_t *)key, &newKeys[0]);
    addRoundKey(&newKeys[40], (uint32_t *) state);
    for(int i = 1; i <= 9; i++){
        revShiftRows(state);
        for(int j = 0; j < 16; j++){
            state[j] = revbox[state[j]];
        }		
        addRoundKey(&newKeys[40-4*i], (uint32_t *)state);
        revMixColumns(state);
    }
    revShiftRows(state);
    for(int j = 0; j < 16; j++){
        state[j] = revbox[state[j]];
    }
    addRoundKey(&newKeys[0], (uint32_t *)state);
}
int main(int argc, char* argv[]){
    if(argc < 4){
        printf("requires three arguments!\n");
        printf("%x", gf256mul(0xed, 0x70));
        return 1;
    }
    int hex = (argc > 4 && argv[4][0] == 'h');
    int left = (argv[3][0] == 'b' || argv[3][0] == 'l');
    int right = (argv[3][0] == 'b' || argv[3][0] == 'r');
    int method = (argc > 5) ? argv[5][0] : 'x';
    uint8_t iV[16] = {0};
    uint8_t key[16] = {0};
    uint8_t state[strlen(argv[2])];
    switch(method){
        case 'x':
            for(int i = 0; argv[1][i*(1+left)]; i++){
                if(!hex){
                    putchar(((left) ? hexToChar(&argv[1][i*2]) : argv[1][i]) ^ ((right) ? hexToChar(&argv[2][i*2]) : argv[2][i]));
                }else{
                    printf("%02hhx", ((left) ? hexToChar(&argv[1][i*2]) : argv[1][i]) ^ ((right) ? hexToChar(&argv[2][i*2]) : argv[2][i]));
                }
            }
            break;
        case 'a':
            for(int i = 0; i < 16; i++){
                key[i] = ((left) ? hexToChar(&argv[1][i*2]) : argv[1][i]);
            }
            for(int i = 0; i < strlen(argv[2])/(1+right); i++){
                state[i] = ((right) ? hexToChar(&argv[2][i*2]) : argv[2][i]);
            }
            if(argc > 6){
                for(int i = 0; i < 16; i++){
                    iV[i] = hexToChar(&argv[6][i*2]);
                }
            }
            for(int i = 0; i < 16; i++){
                state[i] ^= iV[i];
            }
            for(int i = 0; i < strlen(argv[2])/(1+right); i+= 16){
                RSAenc(key, &state[i], iV);
                for(int j = 0; j < 16; j++){
                    if(iV[j]){
                        addRoundKey((uint32_t*)&state[i], (uint32_t*)&state[i+16]);
                        break;
                    }
                }
            }
            printf("\n");
            if(hex){
                for(int i =0; i < strlen(argv[2])/2; i++){
                    printf("%02hhx", state[i]);
                }
            }else{
                state[strlen(argv[2])/(2-right)] = 0;
                printf("%s", state);	
            }
            break;
        case 'd':
            for(int i = 0; i < 16; i++){
                key[i] = ((left) ? hexToChar(&argv[1][i*2]) : argv[1][i]);
            }
            for(int i = 0; i < strlen(argv[2])/(1+right); i++){
                state[i] = ((right) ? hexToChar(&argv[2][i*2]) : argv[2][i]);
            }
            if(argc > 6){
                for(int i = 0; i < 16; i++){
                    iV[i] = hexToChar(&argv[6][i*2]);
                }
            }
            for(int i = strlen(argv[2])/(1+right) - 16; i >= 0; i-= 16){
                RSAdec(key, &state[i]);
                for(int j = 0; j < 16; j++){
                    if(iV[j]){
                        if(i == 0){
                            addRoundKey((uint32_t*)iV, (uint32_t*)state);
                        }
                        else{
                            addRoundKey((uint32_t*)&state[i-16], (uint32_t*)&state[i]);
                        }
                        break;
                    }
                }
            }
            printf("\n");
            if(hex){
                for(int i = 0; i < strlen(argv[2])/(1+right); i++){
                    printf("%02hhx", state[i]);
                }
            }else{
                state[strlen(argv[2])/(2-right)] = 0;
                printf("%s", state);	
            }
    }
    printf("\n");
    return 0;
}
