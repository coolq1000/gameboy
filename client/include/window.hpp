
#ifndef WINDOW_H
#define WINDOW_H

#include <core/dmg.hpp>
#include <SFML/Graphics.hpp>

constexpr auto lcd_width = 160;
constexpr auto lcd_height = 144;

constexpr auto window_width = lcd_width * 2;
constexpr auto window_height = lcd_height * 2;

class window
{
    bool running;

    sf::RenderWindow win;
    sf::Texture lcd;
    sf::Sprite lcd_sprite;
    u32* pixels;

    gmb::ppu ppu;

public:

    window(gmb::ppu& ppu);
    ~window();

    bool get_key(sf::Keyboard::Key key);
    void set_frame_rate(int limit);

    void process();
    void update();
    bool open();

    float lerp(float a, float b, float t);
    u32 shader(u32 pixel);
};

#endif
