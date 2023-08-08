import hashlib
import os

def sha256_hash(input_string):
    input_bytes = input_string.encode('utf-8')
    sha256 = hashlib.sha256()
    sha256.update(input_bytes)
    return sha256.hexdigest()

username = input("Benutzername eingeben: ")
password = input("Passwort eingeben: ")

# Hashen von Benutzername und Passwort
hashed_username = sha256_hash(username)
hashed_password = sha256_hash(password)

# Überprüfen, ob die Datei bereits existiert
file_exists = os.path.exists("gehashte_benutzerdaten.txt")

if file_exists:
    # Wenn die Datei existiert, die vorhandenen Benutzerdaten lesen
    with open("gehashte_benutzerdaten.txt", "r") as file:
        existing_users = file.readlines()
else:
    # Wenn die Datei nicht existiert, initialisiere die Liste mit leeren Benutzerdaten
    existing_users = []

# Den neuen oder aktualisierten Benutzer in die Liste aufnehmen
updated_users = []
user_found = False
for line in existing_users:
    parts = line.strip().split(":")
    if len(parts) == 2 and parts[0] == hashed_username:
        # Wenn der Benutzer bereits existiert, aktualisiere den Hash-Wert
        updated_users.append(f"{hashed_username}:{hashed_password}\n")
        user_found = True
    else:
        updated_users.append(line)

# Wenn der Benutzername nicht gefunden wurde, füge ihn der Liste hinzu
if not user_found:
    updated_users.append(f"{hashed_username}:{hashed_password}\n")

# Speichern der aktualisierten Benutzerdaten zurück in die Datei
with open("gehashte_benutzerdaten.txt", "w") as file:
    file.writelines(updated_users)

print("Benutzername und Passwort wurden erfolgreich gehasht und in gehashte_benutzerdaten.txt aktualisiert.")
