/* Arduino Nofrendo
 * Please check hw_config.h and display.cpp for configuration details
 */
#include <esp_wifi.h>
#include <esp_task_wdt.h>
#include "esp_heap_caps.h"
#include <FFat.h>
#include <SPIFFS.h>
#include <SD.h>
#include <SD_MMC.h>

#include <Arduino_GFX_Library.h>

#include "hw_config.h"

extern "C"
{
#include <nofrendo.h>
#include <nes/nes.h>
}



int16_t bg_color;
int16_t w, h, frame_x, frame_y, frame_x_offset, frame_width, frame_height, frame_line_pixels;
extern Arduino_TFT *gfx;
extern void display_begin();
extern "C" void display_init();

/*############ game list ###########*/
extern void drawButtons();
extern bool handleTouch();
extern Arduino_Canvas *canvas;

// ================= GAME LIST ===================
#define MAX_GAMES 50
String gameList[MAX_GAMES];
int gameCount = 0;
int selectedGame = 0;

// ======== LẤY TRẠNG THÁI NÚT =========
extern bool getButtonPressed(const char* label);
extern void nes_button_set_state(const char* label, bool pressed);

// ========== LOAD DANH SÁCH GAME ==========
void loadGameList() {
     // filesystem defined in hw_config.h
    FILESYSTEM_BEGIN
    File root = filesystem.open("/");
    if (!root) {
        Serial.println("Filesystem mount failed!");
        return;
    }

    File file = root.openNextFile();
    while (file && gameCount < MAX_GAMES) {
        if (!file.isDirectory()) {
            String name = file.name();
            if (name.endsWith(".nes") || name.endsWith(".NES")) {
                gameList[gameCount++] = name;
                Serial.println("Found: " + name);
            }
        }
        file = root.openNextFile();
    }

    if (gameCount == 0) {
        Serial.println("No .nes files found!");
    }
}

// ========== HIỂN THỊ DANH SÁCH GAME ==========
void drawGameList(int selected) {
    // Chỉ vẽ bên trong vùng NES
    canvas->fillRect(frame_x, frame_y, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, bg_color);

    canvas->setTextSize(1);  // vì vùng NES nhỏ, dùng chữ nhỏ
    canvas->setTextColor(canvas->color565(255, 255, 255));

    int lineHeight = 16;
    int maxLines = NES_SCREEN_HEIGHT / lineHeight;
    int startY = frame_y + 10;

    for (int i = 0; i < gameCount && i < maxLines; i++) {
        int y = startY + i * lineHeight;
        if (i == selected) {
            // vẽ dòng được chọn
            canvas->fillRect(frame_x + 5, y - 2, NES_SCREEN_WIDTH - 10, lineHeight, canvas->color565(60, 60, 60));
            canvas->setTextColor(canvas->color565(255, 255, 0)); // vàng
        } else {
            canvas->setTextColor(canvas->color565(255, 255, 255)); // trắng
        }

        canvas->setCursor(frame_x + 10, y);
        canvas->print(gameList[i]);
    }

    canvas->flush();
}


// ========== VÒNG LẶP CHỌN GAME ==========
void gameSelectLoop() {
    drawGameList(selectedGame);
    while (true) {
        handleTouch();  // cập nhật trạng thái nút

        static unsigned long lastPress = 0;
        unsigned long now = millis();

        if (now - lastPress > 200) {
            if (getButtonPressed("UP")) {
                if (selectedGame > 0) {
                    selectedGame--;
                    drawGameList(selectedGame);
                }
                lastPress = now;
            } else if (getButtonPressed("DOWN")) {
                if (selectedGame < gameCount - 1) {
                    selectedGame++;
                    drawGameList(selectedGame);
                }
                lastPress = now;
            } else if (getButtonPressed("OK")) {
                canvas->fillRect(frame_x, frame_y, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT, bg_color);  // xoá list rồi play game
                break; // chọn game
            }
        }

        delay(10);
    }
}

// ========== CHẠY GAME ==========
void runSelectedGame() {
    char fullFilename[256];
    sprintf(fullFilename, "%s/%s", FSROOT, gameList[selectedGame].c_str());
    char* argv[1] = { fullFilename };

    Serial.printf("Running game: %s\n", fullFilename);
    nofrendo_main(1, argv);
    Serial.println("Game ended");
}

void setup()
{ 
  delay(1000);
    Serial.begin(115200);
    Serial.println("Starting NES Game Selector..");

    // start display
    display_begin();
  
    loadGameList();
    display_init();
    if (gameCount == 0) {
        canvas->fillScreen(canvas->color565(0, 0, 0));
        canvas->setCursor(30, 120);
        canvas->setTextSize(2);
        canvas->setTextColor(canvas->color565(255, 0, 0));
        canvas->print("No .nes game found!");
        canvas->flush();
        return;
    }

    drawButtons();  // vẽ nút điều khiển
    gameSelectLoop(); // chọn game
    runSelectedGame(); // chạy game
}

void loop()
{
}
