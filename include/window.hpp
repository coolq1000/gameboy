
#ifndef WINDOW_H
#define WINDOW_H

#include <memory>
#include <core/dmg.hpp>
#define SDL_MAIN_HANDLED
#include <SDL.h>

constexpr auto lcd_width = 160;
constexpr auto lcd_height = 144;

constexpr auto window_width = lcd_width * 2;
constexpr auto window_height = lcd_height * 2;

struct Window
{
    bool running;

    SDL_Window* handle;
    SDL_Renderer* renderer;
    SDL_Texture* texture;
    std::unique_ptr<u32[]> pixels;

    u32 last_time = 0;

    const u8* keys;

    gmb::PPU ppu;

    Window(gmb::PPU& ppu);
    ~Window();

    bool get_key(SDL_Scancode key);

    void process();
    void update();
    bool open();

    float lerp(float a, float b, float t);
    u32 shader(u32 pixel);

    bool focused();
};

#endif
