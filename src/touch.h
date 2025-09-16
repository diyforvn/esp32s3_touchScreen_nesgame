#include <Arduino.h>

  #include <Wire.h>


#define TOUCH_ADDR 0x3B
#define TOUCH_SDA 4
#define TOUCH_SCL 8
#define TOUCH_I2C_CLOCK 400000
#define TOUCH_RST_PIN 12
#define TOUCH_INT_PIN 11
#define AXS_MAX_TOUCH_NUMBER 3

void init_touch_AXS15231B()
{
  // Initialize touch
    Wire.begin(TOUCH_SDA, TOUCH_SCL);
    Wire.setClock(TOUCH_I2C_CLOCK);
    
    // Configure touch pins
    pinMode(TOUCH_INT_PIN, INPUT_PULLUP);
    pinMode(TOUCH_RST_PIN, OUTPUT);
    digitalWrite(TOUCH_RST_PIN, LOW);
    delay(200);
    digitalWrite(TOUCH_RST_PIN, HIGH);
}

bool getTouch(uint16_t &x, uint16_t &y) {
    uint8_t data[AXS_MAX_TOUCH_NUMBER * 6 + 2] = {0};
    
    // Define the read command array properly
    const uint8_t read_cmd[11] = {
        0xb5, 0xab, 0xa5, 0x5a, 0x00, 0x00,
        (uint8_t)((AXS_MAX_TOUCH_NUMBER * 6 + 2) >> 8),
        (uint8_t)((AXS_MAX_TOUCH_NUMBER * 6 + 2) & 0xff),
        0x00, 0x00, 0x00
    };
    
    Wire.beginTransmission(TOUCH_ADDR);
    Wire.write(read_cmd, 11);
    if (Wire.endTransmission() != 0) return false;
    
    if (Wire.requestFrom(TOUCH_ADDR, sizeof(data)) != sizeof(data)) return false;
    
    for (int i = 0; i < sizeof(data); i++) {
        data[i] = Wire.read();
    }
    
    if (data[1] > 0 && data[1] <= AXS_MAX_TOUCH_NUMBER) {
        uint16_t rawX = ((data[2] & 0x0F) << 8) | data[3];
        uint16_t rawY = ((data[4] & 0x0F) << 8) | data[5];
        
        if (rawX == 273 && rawY == 273) return false;
        if (rawX > 4000 || rawY > 4000) return false;
        
        y = map(rawX, 0, 320, 320, 0);
        x = rawY;
        
        return true;
    }    
    return false;
}

struct TouchPoint {
    uint16_t x;
    uint16_t y;
    uint8_t id;
};


bool getMultiTouch(TouchPoint *points, uint8_t &count)
{
   // const uint8_t totalBytes = AXS_MAX_TOUCH_NUMBER * 6 + 2;
   const uint8_t totalBytes = static_cast<uint8_t>(AXS_MAX_TOUCH_NUMBER * 6 + 2);

    uint8_t data[totalBytes] = {0};

    const uint8_t read_cmd[11] = {
        0xb5, 0xab, 0xa5, 0x5a, 0x00, 0x00,
        (uint8_t)(totalBytes >> 8),
        (uint8_t)(totalBytes & 0xFF),
        0x00, 0x00, 0x00
    };

    Wire.beginTransmission(TOUCH_ADDR);
    Wire.write(read_cmd, 11);
    if (Wire.endTransmission() != 0) return false;

    if (Wire.requestFrom(TOUCH_ADDR, totalBytes) != totalBytes) return false;

    for (int i = 0; i < totalBytes; i++) {
        data[i] = Wire.read();
    }

    count = data[1]; // số điểm chạm
    if (count == 0 || count > AXS_MAX_TOUCH_NUMBER) return false;

    for (uint8_t i = 0; i < count; i++) {
        uint8_t *d = &data[2 + i * 6];
        uint16_t rawX = ((d[0] & 0x0F) << 8) | d[1];
        uint16_t rawY = ((d[2] & 0x0F) << 8) | d[3];
        uint8_t id = d[4];

        if (rawX > 4000 || rawY > 4000) continue;

        points[i].x = rawY;
        points[i].y = map(rawX, 0, 320, 320, 0); // nếu cần đảo trục
        points[i].id = id;
    }

    return true;
}
