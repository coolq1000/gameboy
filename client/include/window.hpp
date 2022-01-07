
#ifndef WINDOW_H
#define WINDOW_H

#include <core/dmg.hpp>
#include <SDL.h>

constexpr auto lcd_width = 160;
constexpr auto lcd_height = 144;

constexpr auto window_width = lcd_width * 2;
constexpr auto window_height = lcd_height * 2;

class window
{
    bool running;

    SDL_Window* window_;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    u32* pixels;

    const u8* keys;

    gmb::ppu ppu;

public:

    window(gmb::ppu& ppu);
    ~window();

    bool get_key(SDL_Scancode key);
    void set_frame_rate(int limit);

    void process();
    void update();
    bool open();

    float lerp(float a, float b, float t);
    u32 shader(u32 pixel);
};

#endif
