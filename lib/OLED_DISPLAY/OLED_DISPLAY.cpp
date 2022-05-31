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
    m_display.begin(i2c_Address, true); // Uncomment for other OLED LCD

    //m_display.begin(SSD1306_SWITCHCAPVCC,0x3C); // Uncomment for Adafruit OLED LCD
    m_display.clearDisplay();

    // Set cursor to default position
    m_display.setCursor(0,0);

    // Print initialisation message
    m_display.print("Initialising");
    m_display.display();

    // Print fancy dots
    for (int i = 0; i < 3; i++)
    {
        m_display.print(".");
        m_display.display();
        delay(500);
    }

    // Show complete initialisation
    m_display.clearDisplay();
    m_display.setCursor(0,24);
    
    m_display.print("Initialised");
    m_display.display();
}

void OLED_DISPLAY::print(std::string text, int cursor_x, int cursor_y, bool clear_display)
{

    if(clear_display)
    {
        m_display.clearDisplay();
    }

    if(text == "3")
    {
        m_display.setCursor(cursor_x, cursor_y);
        m_display.setTextSize(1);
        m_display.write(3);
        m_display.display();
    }
    else
    {
        // Set user-defined cursor
        m_display.setCursor(cursor_x,cursor_y);

        // Print to display buffer
        m_display.println(text.c_str());
        
        // Display on OLED screen
        m_display.display();
    }
}

void OLED_DISPLAY::clear()
{
    // Clear the display
    m_display.clearDisplay();
}