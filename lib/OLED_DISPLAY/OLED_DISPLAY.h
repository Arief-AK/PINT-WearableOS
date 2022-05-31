#include <SPI.h>
#include <Wire.h>
#include <string>

#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_SSD1306.h>

#define i2c_Address 0x3C
#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET -1

class OLED_DISPLAY
{
    public:

        OLED_DISPLAY();
        ~OLED_DISPLAY();

        void display_setup();

        void print(std::string text, int cursor_x, int cursor_y, bool clear_display);
        void clear();

    private:

        Adafruit_SH1106G m_display = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Uncomment for other OLED LCD
        //Adafruit_SSD1306 m_display = Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET); // Uncomment for Adafruit OLED LCD

        int m_screen_width;
        int m_screen_height;
};