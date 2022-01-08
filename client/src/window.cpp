#include "window.hpp"

#include <cmath>

window::window(gmb::ppu& ppu) : ppu(ppu)
{
    win.create(sf::VideoMode(window_width, window_height), "gameboy");
    bool created = lcd.create(lcd_width, lcd_height);

    lcd_sprite = sf::Sprite(lcd);

    pixels = new u32[lcd_width * lcd_height]();

    win.setFramerateLimit(60);
}

window::~window()
{
    delete[] pixels;
}

bool window::get_key(sf::Keyboard::Key key)
{
    return sf::Keyboard::isKeyPressed(key);
}

void window::set_frame_rate(int limit)
{

}

void window::process()
{
    sf::Event event;
    while (win.pollEvent(event))
    {
        switch (event.type)
        {
            case sf::Event::Closed:
                win.close();
                break;
        }
    }
}

void window::update()
{
    /* reposition lcd sprite */
    sf::Vector2u window_size = win.getSize();
    float window_ratio = window_size.x / (float)window_size.y;
    float lcd_ratio = (float)lcd_width / (float)lcd_height;

    float pos_x = 0.0f;
    float pos_y = 0.0f;
    float size_x = 1.0f;
    float size_y = 1.0f;

    if (window_ratio > lcd_ratio)
    {
        size_x = lcd_ratio / window_ratio;
        pos_x = (1.0f - size_x) / 2.0f;
    }
    else
    {
        size_y = window_ratio / lcd_ratio;
        pos_y = (1.0f - size_y) / 2.0f;
    }

    float scale_x = (float)window_width / (float)lcd_width;
    float scale_y = (float)window_height / (float)lcd_height;

    lcd_sprite.setScale(size_x * scale_x, size_y * scale_y);
    lcd_sprite.setPosition(pos_x * window_width, pos_y * window_height);

    for (usize i = 0; i < lcd_width * lcd_height; i++)
    {
        pixels[i] = shader(ppu.core_ppu.lcd[i]);
    }

    win.clear();

    lcd.update(reinterpret_cast<sf::Uint8*>(pixels));

    win.draw(lcd_sprite);
    win.display();
}

bool window::open()
{
    return win.isOpen();
}

float window::lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

u32 window::shader(u32 pixel)
{
    const static float saturation = 0.95f;
    const static float brightness = 0.05f;
    const static float gamma = 1.6f;

    uint8_t a = 0xFF;
    uint8_t r = (pixel >> 16) & 0xFF;
    uint8_t g = (pixel >> 8) & 0xFF;
    uint8_t b = (pixel >> 0) & 0xFF;

    float r_f = ((float)r) / 0xFF;
    float g_f = ((float)g) / 0xFF;
    float b_f = ((float)b) / 0xFF;

    float luma = (r_f + g_f + b_f) / 3.0f;

    r_f = lerp(luma, r_f, saturation) + (luma ? brightness : 0.0f);
    g_f = lerp(luma, g_f, saturation) + (luma ? brightness : 0.0f);
    b_f = lerp(luma, b_f, saturation) + (luma ? brightness : 0.0f);

    r_f = std::pow(r_f, 1.0f / gamma);
    g_f = std::pow(g_f, 1.0f / gamma);
    b_f = std::pow(b_f, 1.0f / gamma);

    r = std::min(std::max(r_f, 0.0f), 1.0f) * 0xFF;
    g = std::min(std::max(g_f, 0.0f), 1.0f) * 0xFF;
    b = std::min(std::max(b_f, 0.0f), 1.0f) * 0xFF;

    return (a << 24) | (r << 16) | (g << 8) | b;
}
