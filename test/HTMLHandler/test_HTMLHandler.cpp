//
// Created by Nils Bokermann on 29.11.19.
//

#define WiFi_h
#define FS_H

enum wl_enc_type {  /* Values map to 802.11 encryption suites... */
    ENC_TYPE_WEP = 5,
    ENC_TYPE_TKIP = 2,
    ENC_TYPE_CCMP = 4,
    /* ... except these two, 7 and 8 are reserved in 802.11-2007 */
            ENC_TYPE_NONE = 7,
    ENC_TYPE_AUTO = 8
};

namespace fs {
    class SPIFFSConfig {
    public:
        SPIFFSConfig(bool autoFormat = true) {}
    };

    class File {

    };

    class FS {
    public:

        bool setConfig(const SPIFFSConfig &cfg);

        bool begin();

        void end();

        File open(const char *path, const char *mode);

    };
};


fs::FS SPIFFS;

