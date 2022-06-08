#include "window.hpp"

#include <cmath>

window::window(gmb::ppu& ppu) : ppu(ppu)
{
    SDL_Init(SDL_INIT_VIDEO);
    window_ = SDL_CreateWindow("gameboy", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    renderer = SDL_CreateRenderer(window_, -1, SDL_RENDERER_ACCELERATED);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, lcd_width, lcd_height);

    SDL_RenderSetLogicalSize(renderer, lcd_width, lcd_height);

    pixels = std::make_unique<u32[]>(lcd_width * lcd_height);

    running = true;
}

window::~window()
{
    SDL_DestroyWindow(window_);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
}

bool window::get_key(SDL_Scancode key)
{
    return keys[key];
}

void window::process()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
            case SDL_QUIT:
                running = false;
                break;
        }
    }

    keys = reinterpret_cast<const u8*>(SDL_GetKeyboardState(NULL));
}

void window::update()
{
    for (usize i = 0; i < lcd_width * lcd_height; i++)
    {
        pixels.get()[i] = shader(ppu.core_ppu.lcd[i]);
    }

    SDL_UpdateTexture(texture, NULL, pixels.get(), lcd_width * sizeof(u32));

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);

    while (true) {
        u32 delta = SDL_GetTicks() - last_time;

        if (delta > 1000 / 60.0) {
            last_time = SDL_GetTicks();
            break;
        }
    }
}

bool window::open()
{
    return running;
}

float window::lerp(float a, float b, float t)
{
    return a + t * (b - a);
}

u32 window::shader(u32 pixel)
{
    const static float saturation = 0.95f;
    const static float brightness = 0.03f;
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

bool window::focused()
{
    return SDL_GetWindowFlags(window_) & SDL_WINDOW_INPUT_FOCUS;
}
