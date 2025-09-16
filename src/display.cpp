extern "C"
{
#include <nes/nes.h>
}

#include <Arduino.h>
#include "hw_config.h"
#include "touch.h"
#include <Arduino_GFX_Library.h>


        #define CANVAS 
        #define TFT_BRIGHTNESS 128 /* 0 - 255 */
        #define TFT_BL 1
        Arduino_DataBus *bus = new Arduino_ESP32QSPI(
            45 /* cs */, 47 /* sck */, 21 /* d0 */, 48 /* d1 */, 40 /* d2 */, 39 /* d3 */);
        Arduino_AXS15231B *gfx = new Arduino_AXS15231B(bus, GFX_NOT_DEFINED /* RST */, 0/* rotation */, false /* IPS */, 320 /* width */, 480 /* height */);
        #ifdef CANVAS
            Arduino_Canvas *canvas = new Arduino_Canvas(320, 480, gfx, 0, 0, 0);
        #endif
            
        
        //#define CANVAS_PREFERRED
        #define GFX_SPEED 40000000UL
        #ifdef CANVAS_PREFERRED
            #define DIRECT_RENDER_MODE // full frame buffer
        #endif

extern int16_t w, h, frame_x, frame_y, frame_x_offset, frame_width, frame_height, frame_line_pixels;
extern int16_t bg_color;
extern uint16_t myPalette[];
uint16_t* lineBuffer = nullptr;
uint16_t* frameBuffer = nullptr;
#define NES_LINE_START    10
#define NES_LINE_COUNT    214
#define NES_OFFSET_Y 10   // nếu data[] bắt đầu từ dòng 10

// ========== VIRTUAL BUTTONS ==========
struct TouchButton {
    const char* label;
    int x, y, w, h;
    bool isPressed;
    uint16_t color; // <-- màu riêng cho từng nút
};

TouchPoint touches[AXS_MAX_TOUCH_NUMBER];
uint8_t count = 0;

/*
TouchButton buttons[] = { // dọc
    // D-PAD (trái)
    {"UP",    20, 160, 40, 40},
    {"DOWN",  20, 240, 40, 40},
    {"LEFT",   0, 200, 40, 40},
    {"RIGHT", 40, 200, 40, 40},

    // A-B-C-D (phải)
    {"A", 260, 160, 40, 40},
    {"B", 300, 200, 40, 40},
    {"C", 260, 240, 40, 40},
    {"D", 220, 200, 40, 40},
};
*/

TouchButton buttons[] = {   // ngang x,y
    // D-PAD bên trái
    {"UP",    35, 100, 40, 40, false, 0x841}, // button,x,y,X,Y
    {"DOWN",  35, 180, 40, 40, false, 0x841},
    {"LEFT",   0, 140, 40, 40, false, 0x841},
    {"RIGHT", 70, 140, 40, 40, false, 0x841},

    // Nút A–D bên phải
    {"B", 370, 130, 40, 60, false, 0x841},    
    {"A", 440, 130, 40, 60, false, 0x841},   

    {"SELECT", 110, 280, 60, 40, false, 0x841},
    {"OK"   , 330, 280, 60, 40, false, 0x841},

    {"X", 440, 40, 40, 40, false, 0x841}, // tím hoặc xanh lợt
    {"Y", 370, 40, 40, 40, false, 0x841}, // vàng

};

const int BUTTON_COUNT = sizeof(buttons) / sizeof(buttons[0]);

enum NESButton {
  NES_UP,
  NES_DOWN,
  NES_LEFT,
  NES_RIGHT,
  NES_SELECT,
  NES_START,
  NES_A,
  NES_B,
  NES_X,
  NES_Y
};
uint32_t nes_buttons_state = 0xFFFFFFFF;  // Mặc định tất cả nút thả ra (bit = 1)


void nes_button_set(NESButton btn, bool pressed)
{
    uint32_t bit = (1 << btn);
    if (pressed)
        nes_buttons_state &= ~bit;  // nhấn = clear bit
    else
        nes_buttons_state |= bit;   // thả = set bit
}




void nes_button_set_state(const char* label, bool pressed)
{
    if (strcmp(label, "A") == 0)        nes_button_set(NES_A, pressed);
    else if (strcmp(label, "B") == 0)   nes_button_set(NES_B, pressed);
    else if (strcmp(label, "SELECT") == 0)   nes_button_set(NES_SELECT, pressed);
    else if (strcmp(label, "OK") == 0)   nes_button_set(NES_START, pressed);
    else if (strcmp(label, "UP") == 0)    nes_button_set(NES_UP, pressed);
    else if (strcmp(label, "DOWN") == 0)  nes_button_set(NES_DOWN, pressed);
    else if (strcmp(label, "LEFT") == 0)  nes_button_set(NES_LEFT, pressed);
    else if (strcmp(label, "RIGHT") == 0) nes_button_set(NES_RIGHT, pressed);
    else if (strcmp(label, "X") == 0)     nes_button_set(NES_X, pressed);
    else if (strcmp(label, "Y") == 0)     nes_button_set(NES_Y, pressed);
}



extern "C" uint32_t controller_read_input()
{
    return nes_buttons_state;
}


// ========== DRAW BUTTONS ==========
void drawButtons()
{
    for (int i = 0; i < BUTTON_COUNT; i++) {
        auto &btn = buttons[i];
        uint16_t fill = btn.isPressed
                        ? canvas->color565(255, 80, 80)   // màu sáng khi nhấn
                        : btn.color;                     // màu riêng khi không nhấn
        canvas->fillRoundRect(btn.x, btn.y, btn.w, btn.h, 6, fill);
        canvas->drawRoundRect(btn.x, btn.y, btn.w, btn.h, 6, canvas->color565(255, 255, 255));
        canvas->setCursor(btn.x + 10, btn.y + 14);
        canvas->setTextSize(1);
        canvas->setTextColor(canvas->color565(255, 255, 255));
        canvas->print(btn.label);
    }
    //canvas->flush();  // flush chung với screen
}


bool handleTouch()  // multi touchs
{
    TouchPoint touches[AXS_MAX_TOUCH_NUMBER];
    uint8_t count = 0;
    bool isPressed[BUTTON_COUNT] = {false};
    bool changed = false;

    if (getMultiTouch(touches, count)) {
        for (int i = 0; i < count; i++) {
            uint16_t tx = touches[i].x;
            uint16_t ty = touches[i].y;

            for (int b = 0; b < BUTTON_COUNT; b++) {
                auto &btn = buttons[b];
                bool hit = (tx >= btn.x && tx <= btn.x + btn.w &&
                            ty >= btn.y && ty <= btn.y + btn.h);
                if (hit) {
                    isPressed[b] = true;
                }
            }
        }
    }

    for (int b = 0; b < BUTTON_COUNT; b++) {
        auto &btn = buttons[b];
        if (isPressed[b] && !btn.isPressed) {
            btn.isPressed = true;
            nes_button_set_state(btn.label, true);
            changed = true;
        } else if (!isPressed[b] && btn.isPressed) {
            btn.isPressed = false;
            nes_button_set_state(btn.label, false);
            changed = true;
        }
    }
    //if (changed) drawButtons();  // ✅ Vẽ lại các nút nếu có thay đổi
    if (changed) return true;  // ✅ Vẽ lại các nút nếu có thay đổi
    else return false;
}


extern void display_begin()
{   
    #ifdef CANVAS
    canvas->begin();
    bg_color = canvas->color565(24, 28, 24);
    canvas->fillScreen(bg_color);
    canvas->setRotation(1);
#else
    gfx->begin();
    bg_color = gfx->color565(24, 28, 24);
    gfx->fillScreen(bg_color);
    gfx->setRotation(1);
#endif

#ifdef TFT_BL
    ledcSetup(1, 12000, 8);
    ledcAttachPin(TFT_BL, 1);
    ledcWrite(1, TFT_BRIGHTNESS);
#endif

    init_touch_AXS15231B();
    drawButtons();  // ✅ vẽ nút ngay sau màn hình khởi tạo
#ifdef CANVAS
    canvas->flush();
#endif
}

//extern "C" void display_init()
extern "C" void display_init()
{
     #ifdef CANVAS
        w = canvas->width();
        h = canvas->height();
     #else
        w = gfx->width();
        h = gfx->height();
     #endif
 

    frame_x = (w - NES_SCREEN_WIDTH) / 2;
    frame_y = (h - NES_SCREEN_HEIGHT) / 2;
    frame_line_pixels = NES_SCREEN_WIDTH;


    if (lineBuffer != nullptr) {
        delete[] lineBuffer;  // tránh rò rỉ RAM nếu gọi lại nhiều lần
        lineBuffer = nullptr;
    }

    if (frameBuffer) {
        free(frameBuffer);  // nếu dùng malloc
        // hoặc: delete[] frameBuffer; // nếu dùng new[]
        frameBuffer = nullptr;
        }
    frameBuffer = (uint16_t*) heap_caps_malloc(
        NES_SCREEN_WIDTH * NES_SCREEN_HEIGHT * sizeof(uint16_t),
        MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT
    );

    drawButtons(); // vẽ nút ngay khi khởi tạo
    #ifdef CANVAS
        canvas->flush();  // Đẩy ra LCD
    #endif

    Serial.printf("gfx->width: %d, height: %d\n", w, h);
    Serial.printf("frame_x: %d, frame_y: %d\n", frame_x, frame_y);

}

extern "C" void display_write_frame(const uint8_t *data[])  // load 1 frame
{
    // Convert toàn bộ frame NES sang 1 buffer 16-bit duy nhất
    for (int y = 0; y < NES_SCREEN_HEIGHT; y++) {
        const uint8_t *src = data[y];
        for (int x = 0; x < NES_SCREEN_WIDTH; x++) {
            frameBuffer[y * NES_SCREEN_WIDTH + x] = myPalette[src[x]];
        }
    }

    // Vẽ nguyên khối 1 lần duy nhất
    #ifdef CANVAS
        canvas->draw16bitRGBBitmap(frame_x, frame_y, frameBuffer, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT);
        if (handleTouch()) drawButtons();   // đẩy dữ liệu chung với screen
        canvas->flush();  // Đẩy ra LCD
    #else
        gfx->startWrite();
        gfx->draw16bitRGBBitmap(frame_x, frame_y, frameBuffer, NES_SCREEN_WIDTH, NES_SCREEN_HEIGHT);
        if (handleTouch()) drawButtons();
        gfx->endWrite();
    #endif

    //handleTouch(); // kiểm tra cảm ứng mỗi frame
}

extern "C" void display_clear()
{
    gfx->fillScreen(bg_color);
}


bool getButtonPressed(const char* label) {
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (strcmp(buttons[i].label, label) == 0) {
            return buttons[i].isPressed;
        }
    }
    return false;
}









