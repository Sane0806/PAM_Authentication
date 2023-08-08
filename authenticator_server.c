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
    unsigned char id[HASH_LENGTH / 2];
    unsigned char passwordHash[HASH_LENGTH / 2];
} UserData;

void sha256_hash(const char *input, unsigned char *output) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, strlen(input));
    SHA256_Final(output, &sha256);
}
void hashID(const char* input, unsigned char *output) {
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, input, strlen(input));
    SHA256_Final(output, &sha256);
}

int readUserFile(const char* filename, UserData** users, int* numUsers) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Fehler beim Öffnen der Benutzerdatendatei");
        return 0;
    }

    int capacity = 5;
    *users = (UserData*)malloc(capacity * sizeof(UserData));
    *numUsers = 0;

    char line[MAX_ID_LENGTH + HASH_LENGTH + 10];
    while (fgets(line, sizeof(line), file)) {
        if (*numUsers >= capacity) {
            capacity *= 2;
            *users = (UserData*)realloc(*users, capacity * sizeof(UserData));
        }

        char* newline = strchr(line, '\n');
        if (newline)
            *newline = '\0';

        char id[MAX_ID_LENGTH + 1];
        char passwordHashHex[HASH_LENGTH + 1];

        int result = sscanf(line, "%[^:]:%s", id, passwordHashHex);
        if (result != 2) {
            fprintf(stderr, "Ungültiges Format in Benutzerdatendatei: %s\n", line);
            continue;
        }

        for (int i = 0; i < HASH_LENGTH / 2; i++) {
            sscanf(&id[2 * i], "%02hhx", &(*users)[*numUsers].id[i]);
            sscanf(&passwordHashHex[2 * i], "%02hhx", &(*users)[*numUsers].passwordHash[i]);
        }

        (*numUsers)++;
    }

    fclose(file);
    return 1;
}


int authenticateUser(const char* id, const char* inputPassword, UserData* users, int numUsers) {
    unsigned char inputIDHash[HASH_LENGTH / 2];
    hashID(id, inputIDHash);

    unsigned char inputPasswordHash[HASH_LENGTH / 2];
    sha256_hash(inputPassword, inputPasswordHash);

    for (int i = 0; i < numUsers; i++) {
        if (memcmp(inputIDHash, users[i].id, HASH_LENGTH / 2) == 0) {
            printf("Eingegebener ID-Hash: ");
            for (int j = 0; j < HASH_LENGTH / 2; j++) {
                printf("%02x", inputIDHash[j]);
            }
            printf("\n");

            printf("Gespeicherter ID-Hash: ");
            for (int j = 0; j < HASH_LENGTH / 2; j++) {
                printf("%02x", users[i].id[j]);
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


int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    UserData* users;
    int numUsers;

    // Erstelle einen Server-Socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket-Fehler");
        exit(EXIT_FAILURE);
    }

    // Setze den Socket-Option zum Wiederverwenden des Ports
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Binde den Server-Socket an den Port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("Bind-Fehler");
        exit(EXIT_FAILURE);
    }

    // Warte auf eingehende Verbindungen
    if (listen(server_fd, 3) < 0) {
        perror("Listen-Fehler");
        exit(EXIT_FAILURE);
    }

    while (1) {
        printf("Warte auf Verbindung...\n");

        // Akzeptiere eine eingehende Verbindung
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0) {
            perror("Accept-Fehler");
            exit(EXIT_FAILURE);
        }

        printf("Verbindung hergestellt!\n");

        // Empfange die Benutzerdaten vom Client
        UserData inputUser;
        recv(new_socket, &inputUser, sizeof(inputUser), 0);

        // Lese Benutzerdaten aus der Datei
        if (readUserFile("gehashte_benutzerdaten.txt", &users, &numUsers)) {
            // Benutzer authentifizieren
            int authResult = authenticateUser(inputUser.id, (const char *)inputUser.passwordHash, users, numUsers);
            free(users);

            // Sende das Authentifizierungsergebnis zurück an den Client
            send(new_socket, &authResult, sizeof(authResult), 0);
        }

        // Schließe die Verbindung zum Client
        close(new_socket);
    }

    return 0;
}





