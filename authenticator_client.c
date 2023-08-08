#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <openssl/sha.h>

#define PORT 8080
#define MAX_ID_LENGTH 100
#define MAX_PASSWORD_LENGTH 100
#define HASH_LENGTH 64

typedef struct {
    char id[MAX_ID_LENGTH];
    unsigned char passwordHash[HASH_LENGTH / 2];
} UserData;

void sha256_hash(const char *input, unsigned char *output) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, strlen(input));
    SHA256_Final(output, &sha256);
}

void hashPassword(const char *inputPassword, unsigned char *outputPasswordHash) {
    // Berechne den Hash des Passworts
    sha256_hash(inputPassword, outputPasswordHash);
}

int main() {
    struct sockaddr_in serv_addr;
    int sockfd;

    // Erstelle den Client-Socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket-Fehler");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Konvertiere die IPv4-Adresse von "localhost" zu einer binären Darstellung
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Adresse-Konvertierungsfehler");
        exit(EXIT_FAILURE);
    }

    // Verbinde mit dem Server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Verbindungsfehler");
        exit(EXIT_FAILURE);
    }

    char id[MAX_ID_LENGTH];
    char inputPassword[MAX_PASSWORD_LENGTH];

    printf("ID: ");
    if (fgets(id, sizeof(id), stdin)) {
        char* newline = strchr(id, '\n');
        if (newline)
            *newline = '\0';
    } else {
        fprintf(stderr, "Fehler beim Lesen der ID.\n");
        return 1;
    }

    printf("Passwort: ");
    if (fgets(inputPassword, sizeof(inputPassword), stdin)) {
        char* newline = strchr(inputPassword, '\n');
        if (newline)
            *newline = '\0';
    } else {
        fprintf(stderr, "Fehler beim Lesen des Passworts.\n");
        return 1;
    }

    // Benutzerdaten in eine Struktur packen
    UserData userData;
    strncpy(userData.id, id, sizeof(userData.id));
    hashPassword(inputPassword, userData.passwordHash);

    // Sende die Benutzerdaten an den Server
    send(sockfd, &userData, sizeof(userData), 0);

    int authResult;

    // Empfange das Authentifizierungsergebnis vom Server
    recv(sockfd, &authResult, sizeof(authResult), 0);

    // Auswertung des Authentifizierungsergebnisses
    if (authResult == 1) {
        printf("Erfolgreich authentifiziert.\n");
    } else if (authResult == 0) {
        printf("Falsches Passwort.\n");
    } else {
        printf("Benutzer nicht gefunden.\n");
    }

    close(sockfd);
    return 0;
}
