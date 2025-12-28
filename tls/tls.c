#include <stdint.h>
struct recordHeader {
    uint8_t type;
    uint16_t protocol;
    uint16_t messageLength;
};
struct handshakeHeader {
    uint8_t type;
    uint8_t padding[1];
    uint16_t handshakeLength;
};
struct clientVersion {
    uint8_t minor;
    uint8_t major;
};
struct clientRandom {
    uint8_t random[32];
};
struct sessionID {
    uint8_t IDLength;
    uint8_t IDBytes[];
};
struct cipherSuites {
    uint16_t cipherLength;
    uint16_t ciphers[];
};
struct compressionMethods {
    uint8_t compressionMethodLength;
    uint8_t compressionMethods[];
};
struct extension {
    uint16_t extensionID;
    uint16_t extensionLength;
    void * extensionBody;
};
struct extensions {
    uint16_t extenstionsLength;
    struct extension ext[];
};
#pragma pack(push, 1)
struct extServerNameItem {
    uint16_t entryLength;
    uint8_t entryType;
    uint16_t entryValueLength;
    uint8_t * entry;
};
struct extServerName {
    uint16_t serverNameExtLength;
    struct extServerNameItem items[];
};

struct extECPointFormats {
    uint16_t formatsLength;
    uint8_t formats[];
};

struct extSupportedGroups {
    uint16_t supportedGroupsLength;
    uint16_t supportedCurvesLength;
    uint16_t supportedCurves[];
};
struct extSessionTicket {
    uint16_t sessionTicketLength;
    uint8_t sessionTicket[];
    //Not sure if this is it
};
struct extEncThenMAC {
    uint16_t encThenMACLength;
    uint8_t encThenMAC[];
    //Not sure if this is it
};
struct extExtendedMasterSecret {
    uint16_t extendedMasterSecretLength;
    uint8_t extendedMasterSecret[];
    //Not sure if this is it
};
struct extSignatureAlgorithmsList {
    uint16_t listLength;
    uint16_t listItem[];
};
struct extSignatureAlgorithms {
    uint16_t signatureAlgorithmsLength;
    struct extSignatureAlgorithmsList signatureAlgorithms[]; 
};
struct extTLSVersions {
    uint8_t TLSVersionLength;
    uint8_t TLSMajor;
    uint8_t TLSMinor;
};
struct extSupportedVersions {
    uint16_t supportedVersionsLength;
    struct extTLSVersions supportedVersions[];
};
struct extExchangeModes {
    uint8_t exchangeModeLength;
    uint8_t exchangeMode[];
};
struct extPSKKeyExchangeModes {
    uint16_t PSKKeyExchangeModesLength;
    struct extExchangeModes exchangeModes[];
};
struct keyShareData {
    uint16_t keyShareDataLength;
    uint16_t keyShare;
    uint16_t publicKeyLength;
    uint8_t publicKey[];
};
struct extKeyShare {
    uint16_t keyShareLength;
    struct keyShareData keyShare[];
};
struct TLSHello {
   struct recordHeader recordHeader;
   struct handshakeHeader handshakeHeader;
   struct clientVersion clientVersion;
   struct clientRandom clientRandom;
   struct sessionID *sessionID;
   struct cipherSuites *cipherSuites;
   struct compressionMethods* compressionMethods;
   struct extensions extensions;
};
#pragma pack(pop)
