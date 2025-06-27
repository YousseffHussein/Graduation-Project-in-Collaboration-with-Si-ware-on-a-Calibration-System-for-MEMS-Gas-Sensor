#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#define MINPRESSURE 100
#define MAXPRESSURE 1000

MCUFRIEND_kbv tft;

// Peltier & L298N Setup
#define IN1 41 // L298N Input 1 (TEC Cooling)
#define IN2 43 // L298N Input 2 (TEC Heating)
#define ENA 45 // PWM Control for TEC
#define vcc1 31 // Power supply control for TEC
#define laser 24 // Laser control

// Pump Motor Setup
#define ENA2 53   // PWM control for pump motor speed
#define IN3 51    // Pump Motor direction control
#define IN4 49    // Pump Motor direction control

int mode = 0; // 0 = Off, 1 = Heating, 2 = Cooling
int pumpState = 0; // 1 = Pump On, 0 = Pump Off
const int maxPower = 170; // Maximum power for heating and cooling

const int XP = 8, XM = A2, YP = A3, YM = 9; // Touchscreen pins
const int TS_LEFT = 122, TS_RT = 905, TS_TOP = 945, TS_BOT = 92; // Calibration

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

bool Touch_getXY(int &pixel_x, int &pixel_y) {
    TSPoint p = ts.getPoint();
    pinMode(YP, OUTPUT);
    pinMode(XM, OUTPUT);
    digitalWrite(YP, HIGH);
    digitalWrite(XM, HIGH);

    if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
        pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());
        pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
        pixel_y = tft.height() - pixel_y; // Adjust y-axis for proper touch alignment
        return true;
    }
    return false;
}

void drawInterface() {
    tft.fillScreen(0x0000);
    tft.fillRect(0, 0, tft.width(), 40, 0x007A);
    tft.setTextColor(0xFFFF);
    tft.setTextSize(2);
    tft.setCursor(20, 10);
    tft.print("Peltier Control");

    tft.fillRoundRect(10, 50, 90, 40, 5, 0xF800);
    tft.drawRoundRect(10, 50, 90, 40, 5, 0xFFFF);
    tft.fillRoundRect(120, 50, 90, 40, 5, 0x001F);
    tft.drawRoundRect(120, 50, 90, 40, 5, 0xFFFF);
    tft.fillRoundRect(10, 100, 90, 40, 5, 0x07E0);
    tft.drawRoundRect(10, 100, 90, 40, 5, 0xFFFF);
    tft.fillRoundRect(120, 100, 90, 40, 5, 0xFD20);
    tft.drawRoundRect(120, 100, 90, 40, 5, 0xFFFF);

    tft.setTextColor(0xFFFF);
    tft.setTextSize(2);
    tft.setCursor(30, 65);
    tft.print("HEAT");
    tft.setCursor(140, 65);
    tft.print("COOL");
    tft.setCursor(30, 115);
    tft.print("OFF");
    tft.setCursor(140, 115);
    tft.print("PUMP");

    tft.setCursor(10, 160);
    tft.print("Mode: OFF");
}

void updateStatus(const char *statusText) {
    tft.fillRect(60, 160, 100, 20, 0x0000);
    tft.setCursor(60, 160);
    tft.setTextColor(0xFFFF);
    tft.print(statusText);
}

void updatePumpStatus() {
    tft.fillRect(10, 180, 150, 20, 0x0000);
    tft.setCursor(10, 180);
    tft.setTextColor(0xFFFF);
    tft.print("Pump: ");
    tft.print(pumpState == 1 ? "ON" : "OFF");
}

void handleTouch(int x, int y) {
    if (x > 10 && x < 100 && y > 50 && y < 90) { 
        // HEAT Mode
        mode = 1;
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, HIGH);
        analogWrite(ENA, maxPower);
        updateStatus("HEAT");
    } else if (x > 120 && x < 210 && y > 50 && y < 90) { 
        // COOL Mode
        mode = 2;
        digitalWrite(IN1, HIGH);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, maxPower);
        updateStatus("COOL");
    } else if (x > 10 && x < 100 && y > 100 && y < 140) { 
        // OFF Mode
        mode = 0;
        digitalWrite(IN1, LOW);
        digitalWrite(IN2, LOW);
        analogWrite(ENA, 0);
        updateStatus("OFF");
    } else if (x > 120 && x < 210 && y > 100 && y < 140) { 
        // PUMP Toggle
        pumpState = !pumpState;
        if (pumpState == 1) {
            digitalWrite(IN3, HIGH);
            digitalWrite(IN4, LOW);
            analogWrite(ENA2, 255); // Full speed
        } else {
            digitalWrite(IN3, LOW);
            digitalWrite(IN4, LOW);
            analogWrite(ENA2, 0);
        }
        updatePumpStatus();
    }
}

void setup() {
    tft.begin(0x9341);
    tft.setRotation(0);
    drawInterface();
    updatePumpStatus();

    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(ENA, OUTPUT);
    pinMode(vcc1, OUTPUT);
    pinMode(laser, OUTPUT);
    
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ENA2, OUTPUT);
    
    digitalWrite(vcc1, HIGH);
    digitalWrite(laser, HIGH);
}

void loop() {
    int x, y;
    if (Touch_getXY(x, y)) {
        handleTouch(x, y);
    }
}
