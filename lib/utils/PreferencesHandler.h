#ifndef PREFERENCES_HANDLER
#define PREFERENCES_HANDLER

#include <Preferences.h>
#include <SD.h>
#include "ftpconf.h"

#define CONFIG_FILENAME  "/.conf"
#define CONFIG_NAMESPACE "config"

#define SECRET_KEY          "secret"
#define SSID_KEY            "ssid"
#define WIFI_PASSWD_KEY     "wifiPasswd"
#define PIN_KEY             "pin"
#define FTP_USERNAME_KEY    "ftpUsername"
#define FTP_PASSWD_KEY      "ftpPasswd"
#define EMAIL_KEY           "email"
#define EMAIL_TO_NOTIFY_KEY "emailToNotify"
#define PERSIST_SECRET_KEY  "persistSecret"


// string keys for convenience
const char* keys[] = { SSID_KEY, WIFI_PASSWD_KEY, PIN_KEY, FTP_USERNAME_KEY,
                       FTP_PASSWD_KEY, EMAIL_KEY, EMAIL_TO_NOTIFY_KEY, PERSIST_SECRET_KEY };

class PreferencesHandler {
private:
    Preferences prefs;

    bool isValid(String key, String val) {
        if (key == SECRET_KEY) {
            return val.length() == KEY_LEN;
        }
        if (key == PIN_KEY) {
            return val.length() >= 4;
        }
        if (key == PERSIST_SECRET_KEY) {
            val.toLowerCase();
            if (val != "y" && val != "n") {
                Serial.println("Expected y/n for " + String(PERSIST_SECRET_KEY) + " assuming n");
                val = "n";
            }
        }

        return val.length() > 0;
    }

    bool readPrefs() {
        int rd;
        if ((rd = prefs.getBytes(SECRET_KEY, secretKey, KEY_LEN)) < KEY_LEN) {
            Serial.println("Invalid secret, required len is " + String(KEY_LEN));
            return false;
        }

        String* vals[] = { &ssid, &wifiPasswd, &pin, &ftpUsername, &ftpPasswd, &email, &emailToNotify, &persistSecretKey };

        for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
            *(vals[i]) = prefs.getString(keys[i]);

            if (!isValid(String(keys[i]), *vals[i])) {
                Serial.println("Invalid key: " + String(keys[i]));
                return false;
            }
        }

        return true;
    }

    bool isKeyPersisted() {
        return persistSecretKey == "y";
    }

public:
    uint8_t secretKey[KEY_LEN];
    String ssid;
    String wifiPasswd;
    String pin;
    String ftpUsername;
    String ftpPasswd;
    String email;
    String emailToNotify;
    String persistSecretKey;


    bool begin() {
        prefs.begin(CONFIG_NAMESPACE, false);
        return readPrefs();
    }

    bool loadFromSDCard() {
        File cfg = SD.open(CONFIG_FILENAME, FILE_READ);
        if (!cfg) {
            return false;
        }

        int fileLen = 0;
        while (true) {
            String line = cfg.readStringUntil('\n');
            fileLen += line.length();
            Serial.println("Reading line: ->" + line + "<-");

            if (line.length() == 0) {
                break;
            }

            int sep = line.indexOf("=");
            if (sep == -1)
                return false;

            String propertyName = line.substring(0, sep);
            String propertyValue = line.substring(sep + 1);

            if (!isValid(propertyName, propertyValue)) {
                Serial.println("invalid property: " + propertyName + " with value: " + propertyValue);
                return false;
            }

            if (propertyName == SECRET_KEY) {
                prefs.putBytes(SECRET_KEY, propertyName.c_str(), KEY_LEN);
            }
            else {
                prefs.putString(propertyName.c_str(), propertyValue);
            }
        }

        cfg.close();
        bool prefsValid = readPrefs();
        if (prefsValid) {
            // overwrite data with zero bytes
            cfg = SD.open(CONFIG_FILENAME, FILE_WRITE);
            for (int i = 0; i < fileLen; i++) {
                cfg.write(0);
            }
            cfg.close();
            SD.remove(CONFIG_FILENAME);
            Serial.println("Preferences loaded successfully");

            if (isKeyPersisted()) {
                Serial.println("WARNING: Persisting secret key is not recommended");
            }
        }
        else {
            Serial.println("Failed to read preferences, required keys are:");
            Serial.println(SECRET_KEY);
            for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
                Serial.println(keys[i]);
            }
        }

        return prefsValid;
    }


    void storePrefs() {
        String* vals[] = { &ssid, &wifiPasswd, &pin, &ftpUsername, &ftpPasswd, &email, &emailToNotify, &persistSecretKey };

        prefs.putBytes(SECRET_KEY, secretKey, KEY_LEN);

        for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
            prefs.putString(keys[i], *vals[i]);
        }
    }

    void clearSecrets() {
        for (int i = 0; i < KEY_LEN; i++)
            secretKey[i] = 0;

        if (isKeyPersisted()) {
            prefs.putBytes(SECRET_KEY, secretKey, KEY_LEN);
            prefs.remove(SECRET_KEY);
        }
    }

    void printPrefs() {
        String* vals[] = { &ssid, &wifiPasswd, &pin, &ftpUsername, &ftpPasswd, &email, &emailToNotify, &persistSecretKey };
        for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
            Serial.println(String(keys[i]) + "=" + *vals[i]);
        }
    }


    void dumpToConfigFile() {
        String persistSecret = isKeyPersisted() ? "y" : "n";
        String* vals[] = { &ssid, &wifiPasswd, &pin, &ftpUsername, &ftpPasswd, &email, &emailToNotify, &persistSecret };

        File cfg = SD.open(CONFIG_FILENAME, FILE_WRITE);

        if (isKeyPersisted()) {
            cfg.print(String(SECRET_KEY) + "=");
            cfg.write(secretKey, KEY_LEN);
            cfg.print("/n");
        }

        for (int i = 0; i < sizeof(keys) / sizeof(keys[0]); i++) {
            cfg.println(String(keys[i]) + "=" + *vals[i]);
        }

        cfg.close();
    }

};


#endif 