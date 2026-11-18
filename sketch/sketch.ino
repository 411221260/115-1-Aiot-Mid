
#include <Arduino.h>
#include <Arduino_LED_Matrix.h>
#include <Arduino_RouterBridge.h>
#include <string.h>

// =====================================================
// 1. 硬體設定
// =====================================================

Arduino_LED_Matrix matrix;

const int RED_PIN = LED3_R;
const int GREEN_PIN = LED3_G;
const int BLUE_PIN = LED3_B;

// =====================================================
// 2. 全域變數
// =====================================================

volatile int currentWeatherCode = 0;
volatile int currentRainProbability = 0;
volatile bool hasWeatherData = false;

bool showRainProbability = false;
bool firstWeatherDisplay = false;

unsigned long lastDisplayTime = 0;
const unsigned long DISPLAY_INTERVAL = 2000;

uint8_t frame[104];

int lastWeatherCode = -999;
int lastRainProbability = -999;

// =====================================================
// 3. Bridge 接收 Python 天氣資料
// =====================================================

void receiveWeatherDisplay(int weatherCode, int rainProbability) {

    currentWeatherCode = weatherCode;

    currentRainProbability = constrain(
        rainProbability,
        0,
        100
    );

    hasWeatherData = true;

    Serial.println("收到 Python 天氣資料");
    Serial.print("Weather Code: ");
    Serial.println(weatherCode);

    Serial.print("Rain Probability: ");
    Serial.println(rainProbability);
}

// =====================================================
// 4. RGB LED 控制
// =====================================================

// 沿用原本低電位點亮方式
void setRGBColor(int red, int green, int blue) {

    red = constrain(red, 0, 255);
    green = constrain(green, 0, 255);
    blue = constrain(blue, 0, 255);

    analogWrite(RED_PIN, 255 - red);
    analogWrite(GREEN_PIN, 255 - green);
    analogWrite(BLUE_PIN, 255 - blue);
}

// =====================================================
// 5. 根據天氣代碼設定 LED 顏色
// =====================================================

void setWeatherColor(int weatherCode) {

    // 0：晴朗
    if (weatherCode == 0) {

        setRGBColor(255, 180, 0);
    }

    // 1：大致晴朗
    else if (weatherCode == 1) {

        setRGBColor(255, 210, 50);
    }

    // 2：局部多雲
    else if (weatherCode == 2) {

        setRGBColor(100, 180, 255);
    }

    // 3：陰天
    else if (weatherCode == 3) {

        setRGBColor(180, 180, 190);
    }

    // 45、48：霧
    else if (
        weatherCode == 45 ||
        weatherCode == 48
    ) {

        setRGBColor(220, 220, 220);
    }

    // 51～57：毛毛雨
    else if (
        weatherCode >= 51 &&
        weatherCode <= 57
    ) {

        setRGBColor(0, 180, 180);
    }

    // 61～67：雨天
    else if (
        weatherCode >= 61 &&
        weatherCode <= 67
    ) {

        setRGBColor(0, 80, 255);
    }

    // 71～77：雪天
    else if (
        weatherCode >= 71 &&
        weatherCode <= 77
    ) {

        setRGBColor(0, 255, 220);
    }

    // 80～82：陣雨
    else if (
        weatherCode >= 80 &&
        weatherCode <= 82
    ) {

        setRGBColor(0, 100, 255);
    }

    // 85、86：陣雪
    else if (
        weatherCode == 85 ||
        weatherCode == 86
    ) {

        setRGBColor(0, 255, 220);
    }

    // 95、96、99：雷雨
    else if (
        weatherCode == 95 ||
        weatherCode == 96 ||
        weatherCode == 99
    ) {

        setRGBColor(180, 0, 255);
    }

    // 未知天氣
    else {

        setRGBColor(255, 255, 255);
    }
}

// =====================================================
// 6. LED 點陣基本繪圖
// =====================================================

void drawPattern(const char *pattern[8]) {

    for (int row = 0; row < 8; row++) {

        for (int col = 0; col < 13; col++) {

            int index = row * 13 + col;

            frame[index] =
                (pattern[row][col] == '1') ? 1 : 0;
        }
    }

    matrix.draw(frame);
}

// =====================================================
// 7. 天氣圖案
// =====================================================

// 晴天：太陽
void drawSun() {

    const char *pattern[8] = {

        "0000010000000",
        "0010010010000",
        "0000111000000",
        "0001111100000",
        "0011111110000",
        "0001111100000",
        "0000111000000",
        "0000010000000"
    };

    drawPattern(pattern);
}

// 大致晴朗：較小的太陽
void drawMostlySunny() {

    const char *pattern[8] = {

        "0000000000000",
        "0000010000000",
        "0001010100000",
        "0000111000000",
        "0001111100000",
        "0000111000000",
        "0000010000000",
        "0000000000000"
    };

    drawPattern(pattern);
}

// 局部多雲：太陽加雲朵
void drawPartlyCloudy() {

    const char *pattern[8] = {

        "0000010000000",
        "0001010100000",
        "0000111000000",
        "0011111100000",
        "0111111110000",
        "1111111111000",
        "0111111110000",
        "0000000000000"
    };

    drawPattern(pattern);
}

// 陰天：雲朵
void drawCloud() {

    const char *pattern[8] = {

        "0000000000000",
        "0000110000000",
        "0001111000000",
        "0011111100000",
        "0111111110000",
        "0111111110000",
        "0011111100000",
        "0000000000000"
    };

    drawPattern(pattern);
}

// 雨天：雲朵和雨滴
void drawRain() {

    const char *pattern[8] = {

        "0000111000000",
        "0011111100000",
        "0111111110000",
        "0111111110000",
        "0000000000000",
        "0010001000100",
        "0001000100010",
        "0010001000100"
    };

    drawPattern(pattern);
}

// 雪天：雪花
void drawSnow() {

    const char *pattern[8] = {

        "0000010000000",
        "0010010010000",
        "0001010100000",
        "0000111000000",
        "0111111110000",
        "0000111000000",
        "0001010100000",
        "0010010010000"
    };

    drawPattern(pattern);
}

// 雷雨：雲朵加閃電
void drawThunder() {

    const char *pattern[8] = {

        "0000111000000",
        "0011111100000",
        "0111111110000",
        "0111111110000",
        "0000011000000",
        "0000110000000",
        "0001100000000",
        "0001000000000"
    };

    drawPattern(pattern);
}

// 霧：水平線
void drawFog() {

    const char *pattern[8] = {

        "0000000000000",
        "0000000000000",
        "0111111111000",
        "0000000000000",
        "0011111111100",
        "0000000000000",
        "0111111111000",
        "0000000000000"
    };

    drawPattern(pattern);
}

// =====================================================
// 8. 根據天氣代碼顯示圖案
// =====================================================

void drawWeatherIcon(int weatherCode) {

    // 0：晴朗
    if (weatherCode == 0) {

        drawSun();
    }

    // 1：大致晴朗
    else if (weatherCode == 1) {

        drawMostlySunny();
    }

    // 2：局部多雲
    else if (weatherCode == 2) {

        drawPartlyCloudy();
    }

    // 3：陰天
    else if (weatherCode == 3) {

        drawCloud();
    }

    // 45、48：霧
    else if (
        weatherCode == 45 ||
        weatherCode == 48
    ) {

        drawFog();
    }

    // 51～67：毛毛雨、雨
    else if (
        weatherCode >= 51 &&
        weatherCode <= 67
    ) {

        drawRain();
    }

    // 71～77：雪
    else if (
        weatherCode >= 71 &&
        weatherCode <= 77
    ) {

        drawSnow();
    }

    // 80～82：陣雨
    else if (
        weatherCode >= 80 &&
        weatherCode <= 82
    ) {

        drawRain();
    }

    // 85、86：陣雪
    else if (
        weatherCode == 85 ||
        weatherCode == 86
    ) {

        drawSnow();
    }

    // 95、96、99：雷雨
    else if (
        weatherCode == 95 ||
        weatherCode == 96 ||
        weatherCode == 99
    ) {

        drawThunder();
    }

    // 未知天氣
    else {

        drawCloud();
    }
}

// =====================================================
// 9. 降雨機率數字圖案
// =====================================================

const byte digits[10][5][3] = {

    // 0
    {
        {1,1,1},
        {1,0,1},
        {1,0,1},
        {1,0,1},
        {1,1,1}
    },

    // 1
    {
        {0,1,0},
        {1,1,0},
        {0,1,0},
        {0,1,0},
        {1,1,1}
    },

    // 2
    {
        {1,1,1},
        {0,0,1},
        {1,1,1},
        {1,0,0},
        {1,1,1}
    },

    // 3
    {
        {1,1,1},
        {0,0,1},
        {0,1,1},
        {0,0,1},
        {1,1,1}
    },

    // 4
    {
        {1,0,1},
        {1,0,1},
        {1,1,1},
        {0,0,1},
        {0,0,1}
    },

    // 5
    {
        {1,1,1},
        {1,0,0},
        {1,1,1},
        {0,0,1},
        {1,1,1}
    },

    // 6
    {
        {1,1,1},
        {1,0,0},
        {1,1,1},
        {1,0,1},
        {1,1,1}
    },

    // 7
    {
        {1,1,1},
        {0,0,1},
        {0,1,0},
        {0,1,0},
        {0,1,0}
    },

    // 8
    {
        {1,1,1},
        {1,0,1},
        {1,1,1},
        {1,0,1},
        {1,1,1}
    },

    // 9
    {
        {1,1,1},
        {1,0,1},
        {1,1,1},
        {0,0,1},
        {1,1,1}
    }
};

// =====================================================
// 10. 繪製指定數字
// =====================================================

void drawDigit(int number, int startX, int startY) {

    if (number < 0 || number > 9) {
        return;
    }

    for (int y = 0; y < 5; y++) {

        for (int x = 0; x < 3; x++) {

            if (digits[number][y][x] == 1) {

                int index =
                    (startY + y) * 13 + startX + x;

                if (index >= 0 && index < 104) {

                    frame[index] = 1;
                }
            }
        }
    }
}

// =====================================================
// 11. 繪製百分比符號
// =====================================================

void drawPercentSymbol(int startX, int startY) {

    const byte percent[5][3] = {

        {1,0,1},
        {0,0,1},
        {0,1,0},
        {1,0,0},
        {1,0,1}
    };

    for (int y = 0; y < 5; y++) {

        for (int x = 0; x < 3; x++) {

            if (percent[y][x] == 1) {

                int index =
                    (startY + y) * 13 + startX + x;

                if (index >= 0 && index < 104) {

                    frame[index] = 1;
                }
            }
        }
    }
}

// =====================================================
// 12. 顯示降雨機率
// =====================================================

void drawRainProbability() {

    memset(frame, 0, sizeof(frame));

    int value = constrain(
        (int)currentRainProbability,
        0,
        100
    );

    // 0～9
    if (value < 10) {

        drawDigit(value, 4, 1);

        drawPercentSymbol(9, 1);
    }

    // 10～99
    else if (value < 100) {

        int tens = value / 10;
        int ones = value % 10;

        drawDigit(tens, 0, 1);

        drawDigit(ones, 4, 1);

        drawPercentSymbol(9, 1);
    }

    // 100
    else {

        drawDigit(1, 0, 1);

        drawDigit(0, 4, 1);

        drawDigit(0, 8, 1);
    }

    matrix.draw(frame);
}

// =====================================================
// 13. Setup
// =====================================================

void setup() {

    // RGB LED 初始化
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);

    // 尚未取得天氣資料時，RGB LED 熄滅
    setRGBColor(0, 0, 0);

    // 點陣初始化
    matrix.begin();

    matrix.setGrayscaleBits(1);

    // Bridge 初始化
    Bridge.begin();

    // 註冊 Python 呼叫的函式
    Bridge.provide(
        "set_weather_display",
        receiveWeatherDisplay
    );

    // 尚未收到資料，固定顯示雲朵
    drawCloud();

    showRainProbability = false;

    firstWeatherDisplay = false;

    lastDisplayTime = millis();

    Serial.println("UNO Q 天氣顯示系統啟動完成");
}

// =====================================================
// 14. Loop
// =====================================================

void loop() {

    // 尚未收到 Python 天氣資料
    if (!hasWeatherData) {

        return;
    }

    // 第一次收到天氣資料
    if (!firstWeatherDisplay) {

        firstWeatherDisplay = true;

        showRainProbability = false;

        lastDisplayTime = millis();

        // 顯示天氣圖案
        drawWeatherIcon(currentWeatherCode);

        // 設定 RGB LED 顏色
        setWeatherColor(currentWeatherCode);

        lastWeatherCode = currentWeatherCode;

        lastRainProbability = currentRainProbability;

        return;
    }

    // 天氣代碼改變
    if (lastWeatherCode != currentWeatherCode) {

        lastWeatherCode = currentWeatherCode;

        // 更新 RGB LED
        setWeatherColor(currentWeatherCode);

        // 如果目前顯示天氣圖案，就立即更新
        if (!showRainProbability) {

            drawWeatherIcon(currentWeatherCode);
        }
    }

    // 降雨機率改變時記錄新資料
    if (lastRainProbability != currentRainProbability) {

        lastRainProbability = currentRainProbability;

        if (showRainProbability) {

            drawRainProbability();
        }
    }

    unsigned long currentTime = millis();

    // 每 2 秒切換一次
    if (
        currentTime - lastDisplayTime >=
        DISPLAY_INTERVAL
    ) {

        lastDisplayTime = currentTime;

        showRainProbability = !showRainProbability;

        if (showRainProbability) {

            // 顯示降雨機率
            drawRainProbability();
        }

        else {

            // 顯示天氣圖案
            drawWeatherIcon(currentWeatherCode);
        }
    }
}
