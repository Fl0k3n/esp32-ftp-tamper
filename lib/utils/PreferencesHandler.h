#ifndef PREFERENCES_HANDLER
#define PREFERENCES_HANDLER

#include <Preferences.h>
#include <SD.h>
#include "ftpconf.h"

#define MAX_INIT_PIN_RETRIES 3

#define CONFIG_FILENAME  "/.conf"
#define CONFIG_NAMESPACE "config"

#define SECRET_KEY            "secret"
#define SSID_KEY              "ssid"
#define WIFI_PASSWD_KEY       "wifiPasswd"
#define PIN_KEY               "pin"
#define FTP_USERNAME_KEY      "ftpUsername"
#define FTP_PASSWD_KEY        "ftpPasswd"
#define EMAIL_KEY             "email"
#define EMAIL_PASSWD_KEY      "emailPasswd"
#define EMAIL_TO_NOTIFY_KEY   "emailToNotify"
#define PERSIST_SECRET_KEY    "persistSecret"


#define UTILS_NAMESPACE "utils"

#define INIT_PIN_RETRIES_LEFT_KEY "initPinRetrL"

// const int KEYS_COUNT = sizeof(keys) / sizeof(CONFIG_KEYS[0]);
#define KEYS_COUNT 9

class PreferencesHandler {
private:
    Preferences prefs;
    Preferences utils;
    int retriesLeft;

    // config keys for convenience
    const char* CONFIG_KEYS[KEYS_COUNT];

    bool isValid(String key, String val) {
        if (key == String(SECRET_KEY)) {
            return val.length() == KEY_LEN;
        }
        if (key == String(PIN_KEY)) {
            return val.length() >= 4;
        }
        if (key == String(PERSIST_SECRET_KEY)) {
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

        String* vals[KEYS_COUNT];
        getStringConfigParams(vals);

        for (int i = 0; i < KEYS_COUNT; i++) {
            *(vals[i]) = prefs.getString(CONFIG_KEYS[i]);

            if (!isValid(String(CONFIG_KEYS[i]), *vals[i])) {
                Serial.println("Invalid key: " + String(CONFIG_KEYS[i]));
                return false;
            }
        }

        return true;
    }

    bool isKeyPersisted() {
        return persistSecretKey == "y";
    }

    void initUtils() {
        retriesLeft = utils.getInt(INIT_PIN_RETRIES_LEFT_KEY, MAX_INIT_PIN_RETRIES); // not sure about this default value
    }

    void getStringConfigParams(String* buff[]) {
        String* vals[] = { &ssid, &wifiPasswd, &pin, &ftpUsername, &ftpPasswd,
                           &email, &emailPasswd, &emailToNotify, &persistSecretKey };
        memcpy(buff, vals, KEYS_COUNT * sizeof(String*));
    }

public:
    uint8_t secretKey[KEY_LEN];
    String ssid;
    String wifiPasswd;
    String pin;
    String ftpUsername;
    String ftpPasswd;
    String email;
    String emailPasswd;
    String emailToNotify;
    String persistSecretKey;

    PreferencesHandler() : CONFIG_KEYS{ SSID_KEY, WIFI_PASSWD_KEY, PIN_KEY, FTP_USERNAME_KEY, FTP_PASSWD_KEY,
                       EMAIL_KEY, EMAIL_PASSWD_KEY, EMAIL_TO_NOTIFY_KEY, PERSIST_SECRET_KEY } {

    }

    bool begin() {
        prefs.begin(CONFIG_NAMESPACE, false);
        utils.begin(UTILS_NAMESPACE, false);

        initUtils();

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
            String propertyValue = line.substring(sep + 1, line.length() - 1);

            if (!isValid(propertyName, propertyValue)) {
                Serial.println("invalid property: " + propertyName + " with value: " + propertyValue);
                return false;
            }

            if (propertyName == SECRET_KEY) {
                prefs.putBytes(SECRET_KEY, propertyValue.c_str(), KEY_LEN);
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

            setPinRetriesLeft(MAX_INIT_PIN_RETRIES);
        }
        else {
            Serial.println("Failed to read preferences, required keys are:");
            Serial.println(SECRET_KEY);
            for (int i = 0; i < KEYS_COUNT; i++) {
                Serial.println(CONFIG_KEYS[i]);
            }
        }

        return prefsValid;
    }

    bool loadFromSDIfPresent() {
        return loadFromSDCard();
    }

    bool isConfigPresentOnSD() {
        File cfg = SD.open(CONFIG_FILENAME, FILE_READ);
        bool present = cfg;
        cfg.close();
        return present;
    }

    void storePrefs() {
        String* vals[KEYS_COUNT];
        getStringConfigParams(vals);

        prefs.putBytes(SECRET_KEY, secretKey, KEY_LEN);

        for (int i = 0; i < KEYS_COUNT; i++) {
            prefs.putString(CONFIG_KEYS[i], *vals[i]);
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
        String* vals[KEYS_COUNT];
        getStringConfigParams(vals);
        for (int i = 0; i < KEYS_COUNT; i++) {
            Serial.println(String(CONFIG_KEYS[i]) + "=" + *vals[i]);
        }
    }


    void dumpToConfigFile() {
        String persistSecret = isKeyPersisted() ? "y" : "n";
        String* vals[KEYS_COUNT];
        getStringConfigParams(vals);

        File cfg = SD.open(CONFIG_FILENAME, FILE_WRITE);

        if (isKeyPersisted()) {
            cfg.print(String(SECRET_KEY) + "=");
            cfg.write(secretKey, KEY_LEN);
            cfg.print("/n");
        }

        for (int i = 0; i < KEYS_COUNT; i++) {
            cfg.println(String(CONFIG_KEYS[i]) + "=" + *vals[i]);
        }

        cfg.close();
    }

    void createConfigFileExample() {
        File cfg = SD.open(CONFIG_FILENAME, FILE_WRITE);

        cfg.println(String(SECRET_KEY) + "=");

        for (int i = 0; i < KEYS_COUNT; i++) {
            cfg.println(String(CONFIG_KEYS[i]) + "=");
        }

        cfg.close();
    }

    void setPinRetriesLeft(int retries) {
        utils.putInt(INIT_PIN_RETRIES_LEFT_KEY, retries);
        retriesLeft = retries;
    }

    void resetPinRetries() {
        retriesLeft = MAX_INIT_PIN_RETRIES;
        utils.putInt(INIT_PIN_RETRIES_LEFT_KEY, retriesLeft);
    }

    int getPinRetriesLeft() {
        return retriesLeft;
    }

    // for testing only (for now)
    void flushConfig() {
        for (int i = 0; i < KEYS_COUNT; i++) {
            prefs.remove(CONFIG_KEYS[i]);
        }

        prefs.remove(SECRET_KEY);
    }
};

#endif 