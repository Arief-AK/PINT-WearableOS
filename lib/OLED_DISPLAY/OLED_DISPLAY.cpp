#include "OLED_DISPLAY.h"

OLED_DISPLAY::OLED_DISPLAY()
{
    // Initialise screen configurations
    m_screen_height = SCREEN_HEIGHT;
    m_screen_width = SCREEN_WIDTH;

    // Setup text properties
    m_display.setTextSize(1);
    m_display.setTextColor(SH110X_WHITE);
}

OLED_DISPLAY::~OLED_DISPLAY()
{
    delete this;
}

void OLED_DISPLAY::display_setup()
{
    // Setup I2C connection
    m_display.begin(i2c_Address, true);
    m_display.clearDisplay();

    // Set cursor to default position
    m_display.setCursor(0,0);
}

void OLED_DISPLAY::print(std::string text, int cursor_x, int cursor_y)
{
    // Set user-defined cursor
    m_display.setCursor(cursor_x,cursor_y);

    // Print to display buffer
    m_display.println(text.c_str());
    
    // Display on OLED screen
    m_display.display();
}

void OLED_DISPLAY::clear()
{
    // Clear the display
    m_display.clearDisplay();
}