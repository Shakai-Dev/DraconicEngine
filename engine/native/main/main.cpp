import std; 

#include <SDL3/SDL.h> 
#include <bgfx/bgfx.h> 
#include <bgfx/platform.h> 

int main() { 
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        const char* err = SDL_GetError();
        std::println("SDL init failed: '{}'", err && *err ? err : "NO ERROR STRING");
        return -1;
    }

    // Creating the window
    SDL_Window* window = SDL_CreateWindow("Draconic Engine", 1280, 720, SDL_WINDOW_RESIZABLE);
    
    if (!window) { 
        std::println("Failed to create window: {}", SDL_GetError()); 
        return -1; 
    } 

    const char *driver = SDL_GetCurrentVideoDriver();
    std::println("Current video driver: {}", driver ? driver : "Unknown");

    // Fetch platform data for bgfx
    bgfx::PlatformData pd{}; 

#if defined(__linux__)
    SDL_PropertiesID props = SDL_GetWindowProperties(window);

    // X11 Handshake
    if (driver && std::string_view(driver) == "x11") {
        // ndt = Native Display Type (Display*)
        pd.ndt = SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr);

        // nwh = Native Window Handle (Window ID)
        // Note: bgfx expects a void*, so we cast the X11 Window ID (uintptr_t)
        pd.nwh = (void*)(uintptr_t)SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0);

        if (!pd.ndt || !pd.nwh) {
            std::println("X11 handles are null! Make sure you are actually running in an X11 session.");
            return -1;
        }
    } else {
        std::println("Error: Driver is {}, but we're only rocking X11 right now.", driver ? driver : "None");
        return -1;
    }
#endif

    std::println("Native display type: {}", pd.ndt); 
    std::println("Native window handle: {}", pd.nwh);

    bgfx::Init init{}; 
    init.type = bgfx::RendererType::Count; // Auto select
    init.platformData = pd; 
    init.resolution.width = 1280; 
    init.resolution.height = 720; 
    init.resolution.reset = BGFX_RESET_VSYNC; 
    
    if (!bgfx::init(init)) { 
        std::println("bgfx init failed. It might be that your platform doesn't support the selected renderer or there was an issue with the provided platform data."); 
        return -1; 
    } 
    
    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0 ); 
    
    bool running = true; 
    while (running) { 
        SDL_Event event; 
        while (SDL_PollEvent(&event)) { 
            if (event.type == SDL_EVENT_QUIT) running = false; 
        } 

        int w, h; 
        SDL_GetWindowSize(window, &w, &h); 
        
        bgfx::reset(w, h, BGFX_RESET_VSYNC); 
        bgfx::setViewRect(0, 0, 0, uint16_t(w), uint16_t(h)); 
        bgfx::touch(0); 
        bgfx::frame(); 
    } 
    
    bgfx::shutdown(); 
    SDL_DestroyWindow(window); 
    SDL_Quit(); 
    
    return 0; 
}