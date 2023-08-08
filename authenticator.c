#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>

#define MAX_ID_LENGTH 100
#define MAX_PASSWORD_LENGTH 100
#define HASH_LENGTH 64 // SHA-256 gibt einen 64-Byte-Hash zurück

typedef struct {
    unsigned char idHash[HASH_LENGTH / 2]; // Die Hälfte der Länge, da wir binäre Daten speichern
    unsigned char passwordHash[HASH_LENGTH / 2];
} User;

void sha256_hash(const char *input, unsigned char *output) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, strlen(input));
    SHA256_Final(output, &sha256);
}

int readUserFile(const char* filename, User** users, int* numUsers) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Fehler beim Öffnen der Benutzerdatendatei");
        return 0;
    }

    int capacity = 5;
    *users = (User*)malloc(capacity * sizeof(User));
    *numUsers = 0;

    char line[MAX_ID_LENGTH + HASH_LENGTH + 10]; // Erhöht die Puffergröße zum Einlesen des kompletten Zeile
    while (fgets(line, sizeof(line), file)) {
        if (*numUsers >= capacity) {
            capacity *= 2;
            *users = (User*)realloc(*users, capacity * sizeof(User));
        }

        char* newline = strchr(line, '\n');
        if (newline)
            *newline = '\0';

        char id[MAX_ID_LENGTH + 1];
        char passwordHashHex[HASH_LENGTH + 1];

        // Format des Eintrags in der Datei: ID:gesamter_hash:salz
        int result = sscanf(line, "%[^:]:%s", id, passwordHashHex);
        if (result != 2) {
            fprintf(stderr, "Ungültiges Format in Benutzerdatendatei: %s\n", line);
            continue;
        }

        // Den Hexadezimal-String in binären Hash umwandeln
        for (int i = 0; i < HASH_LENGTH / 2; i++) {
            sscanf(&id[2 * i], "%2hhx", &(*users)[*numUsers].idHash[i]);
            sscanf(&passwordHashHex[2 * i], "%2hhx", &(*users)[*numUsers].passwordHash[i]);
        }

        (*numUsers)++;
    }

    fclose(file);
    return 1;
}

void hashID(const char* input, unsigned char *output) {
    sha256_hash(input, output);
}

int authenticateUser(const char* id, const char* inputPassword, User* users, int numUsers) {
    unsigned char inputIDHash[HASH_LENGTH / 2];
    hashID(id, inputIDHash);

    unsigned char inputPasswordHash[HASH_LENGTH / 2];
    sha256_hash(inputPassword, inputPasswordHash);

    for (int i = 0; i < numUsers; i++) {
        if (memcmp(inputIDHash, users[i].idHash, HASH_LENGTH / 2) == 0) {

            printf("Eingegebener ID-Hash: ");
            for (int j = 0; j < HASH_LENGTH / 2; j++) {
                printf("%02x", inputIDHash[j]);
            }
            printf("\n");

            printf("Gespeicherter ID-Hash: ");
            for (int j = 0; j < HASH_LENGTH / 2; j++) {
                printf("%02x", users[i].idHash[j]);
            }
            printf("\n");

            printf("Eingegebener Passwort-Hash: ");
            for (int j = 0; j < HASH_LENGTH / 2; j++) {
                printf("%02x", inputPasswordHash[j]);
            }
            printf("\n");

            printf("Gespeicherter Passwort-Hash: ");
            for (int j = 0; j < HASH_LENGTH / 2; j++) {
                printf("%02x", users[i].passwordHash[j]);
            }
            printf("\n");

            if (memcmp(inputPasswordHash, users[i].passwordHash, HASH_LENGTH / 2) == 0) {
                return 1; // Authentifizierung erfolgreich
            } else {
                return 0; // Falsches Passwort
            }
        }
    }
    return -1; // Benutzer nicht gefunden
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Verwendung: %s gehashte_benutzerdaten.txt\n", argv[0]);
        return 1;
    }

    const char* userFile = argv[1];
    User* users;
    int numUsers;

    if (!readUserFile(userFile, &users, &numUsers)) {
        return 1;
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

        int authResult = authenticateUser(id, inputPassword, users, numUsers);
        if (authResult == 1) {
            printf("Erfolgreich authentifiziert.\n");
        } else if (authResult == 0) {
            printf("Falsches Passwort.\n");
        } else {
            printf("Benutzer nicht gefunden.\n");
        }
    } else {
        fprintf(stderr, "Fehler beim Lesen des Passworts.\n");
    }

    free(users);
    return 0;
}
