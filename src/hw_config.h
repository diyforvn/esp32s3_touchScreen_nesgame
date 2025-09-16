#ifndef _HW_CONFIG_H_
    #define _HW_CONFIG_H_

        // SD card
    #define SD_SUPPORTED
    #define SD_SCK 12
    #define SD_MOSI 11 // CMD
    #define SD_MISO 13 // D0
    #define SD_CS 10 // D3

    #define FSROOT "/fs"

    /* M5Stack */
    #if defined(ARDUINO_M5Stack_Core_ESP32) || defined(ARDUINO_M5STACK_FIRE)

        // Uncomment one of below, M5Stack support SPIFFS and SD
        // #define FILESYSTEM_BEGIN SPIFFS.begin(false, FSROOT); FS filesystem = SPIFFS;
        #define FILESYSTEM_BEGIN SD.begin(4, SPI, 40000000, FSROOT); FS filesystem = SD;

        /* enable audio */
        #define HW_AUDIO
        #define HW_AUDIO_SAMPLERATE 22050

        /* controller is I2C M5Stack CardKB */
        #define HW_CONTROLLER_I2C_M5CARDKB

    /* Odroid-Go */
    #elif defined(ARDUINO_ODROID_ESP32)

        // Uncomment one of below, ODROID support SPIFFS and SD
        // #define FILESYSTEM_BEGIN SPIFFS.begin(false, FSROOT); FS filesystem = SPIFFS;
        #define FILESYSTEM_BEGIN SD.begin(SS, SPI, 40000000, FSROOT); FS filesystem = SD;
        /* enable audio */
        #define HW_AUDIO
        #define HW_AUDIO_SAMPLERATE 22050

        /* controller is GPIO */
        #define HW_CONTROLLER_GPIO
        #define HW_CONTROLLER_GPIO_ANALOG_JOYSTICK
        #define HW_CONTROLLER_GPIO_UP_DOWN 35
        #define HW_CONTROLLER_GPIO_LEFT_RIGHT 34
        #define HW_CONTROLLER_GPIO_SELECT 27
        #define HW_CONTROLLER_GPIO_START 39
        #define HW_CONTROLLER_GPIO_A 32
        #define HW_CONTROLLER_GPIO_B 33
        #define HW_CONTROLLER_GPIO_X 13
        #define HW_CONTROLLER_GPIO_Y 0

    /* TTGO T-Watch */
    #elif defined(ARDUINO_T) || defined(ARDUINO_TWATCH_BASE) || defined(ARDUINO_TWATCH_2020_V1) || defined(ARDUINO_TWATCH_2020_V2) // TTGO T-Watch

        // TTGO T-watch with game module only support SPIFFS
        #define FILESYSTEM_BEGIN SPIFFS.begin(false, FSROOT); FS filesystem = SPIFFS;

        /* buzzer audio */
        #define HW_AUDIO_BUZZER
        #define HW_AUDIO_BUZZER_PIN 4
        #define HW_AUDIO_SAMPLERATE 22050 // nofrendo minimum sample rate

        /* controller is GPIO */
        #define HW_CONTROLLER_GPIO
        #define HW_CONTROLLER_GPIO_ANALOG_JOYSTICK
        #define HW_CONTROLLER_GPIO_UP_DOWN 34
        #define HW_CONTROLLER_GPIO_LEFT_RIGHT 33
        #define HW_CONTROLLER_GPIO_SELECT 15
        #define HW_CONTROLLER_GPIO_START 36
        #define HW_CONTROLLER_GPIO_A 13
        #define HW_CONTROLLER_GPIO_B 25
        #define HW_CONTROLLER_GPIO_X 14
        #define HW_CONTROLLER_GPIO_Y 26

    /* custom hardware */
    #else

        // Uncomment one of below, ESP32 support SPIFFS SD_MMC and SD
        /* FatFS */
        // #define FILESYSTEM_BEGIN FFat.begin(false, FSROOT); FS filesystem = FFat;
        /* SPIFFS */
         #define FILESYSTEM_BEGIN SPIFFS.begin(false, FSROOT); FS filesystem = SPIFFS;
        /* 1-bit SD mode SD_MMC, always retry once for begin() failed */
        // #define FILESYSTEM_BEGIN (!SD_MMC.begin(FSROOT, true)) && (!SD_MMC.begin(FSROOT, true)); FS filesystem = SD_MMC;
        /* 4-bit SD mode SD_MMC, always retry once for begin() failed */
        // #define FILESYSTEM_BEGIN (!SD_MMC.begin(FSROOT, false)) && (!SD_MMC.begin(FSROOT, false)); FS filesystem = SD_MMC;
        /* SD using default SPI settings */
        // #define FILESYSTEM_BEGIN SD.begin(22 /* SS */, SPI, 8000000, FSROOT); FS filesystem = SD;
        /* SD using custom SPI settings */
        //#define FILESYSTEM_BEGIN SPIClass spi = SPIClass(HSPI); spi.begin(14, 2, 15, 13); SD.begin(13, spi, 8000000, FSROOT); FS filesystem = SD;
        //#define FILESYSTEM_BEGIN SPIClass spi = SPIClass(HSPI); spi.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS); SD.begin(SD_CS, spi, 8000000, FSROOT); FS filesystem = SD;
       
        // enable audio
        /*#define HW_AUDIO
        #define HW_AUDIO_EXTDAC
        #define HW_AUDIO_EXTDAC_WCLK 21
        #define HW_AUDIO_EXTDAC_BCLK 22
        #define HW_AUDIO_EXTDAC_DOUT 19
        #define HW_AUDIO_SAMPLERATE 22050*/

        #define HW_AUDIO
        #define HW_AUDIO_EXTDAC
        #define HW_AUDIO_EXTDAC_WCLK -1
        #define HW_AUDIO_EXTDAC_BCLK 42
        #define HW_AUDIO_EXTDAC_DOUT 41
        #define HW_AUDIO_SAMPLERATE 22050

        /*        // I2S output
#define I2S_OUTPUT
#define I2S_DEFAULT_GAIN_LEVEL 0.8
#define I2S_OUTPUT_NUM I2S_NUM_0
#define I2S_MCLK -1
#define I2S_BCLK 42
#define I2S_LRCK 2
#define I2S_DOUT 41
#define I2S_DIN -1*/

        /* controller is GPIO */
       /* #define HW_CONTROLLER_GPIO
        #define HW_CONTROLLER_GPIO_ANALOG_JOYSTICK
        // #define HW_CONTROLLER_GPIO_REVERSE_UD
        #define HW_CONTROLLER_GPIO_UP_DOWN 34
        #define HW_CONTROLLER_GPIO_REVERSE_LF
        #define HW_CONTROLLER_GPIO_LEFT_RIGHT 35
        #define HW_CONTROLLER_GPIO_SELECT 27
        #define HW_CONTROLLER_GPIO_START 26
        #define HW_CONTROLLER_GPIO_A 5
        #define HW_CONTROLLER_GPIO_B 4
        #define HW_CONTROLLER_GPIO_X 23
        #define HW_CONTROLLER_GPIO_Y 18*/

        /* controller is I2C M5Stack CardKB */
        // #define HW_CONTROLLER_I2C_M5CARDKB

        /* controller is I2C BBQ10Keyboard */
        // #define HW_CONTROLLER_I2C_BBQ10KB

        #define TOUCH_CONTROLLER

    #endif /* custom hardware */


#endif /* _HW_CONFIG_H_ */
