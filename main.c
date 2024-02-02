#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <fontconfig/fontconfig.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

// https://stackoverflow.com/a/69543995/22470070

unsigned short* get_screen_size(void) {
    static unsigned short size[2];
    char* array[8];
    char screen_size[64];
    char* token = NULL;

    FILE* cmd = popen("xdpyinfo | awk '/dimensions/ {print $2}'", "r");

    if (!cmd)
        return 0;

    while (fgets(screen_size, sizeof(screen_size), cmd) != NULL);
    pclose(cmd);

    token = strtok(screen_size, "x\n");

    if (!token)
        return 0;

    for (unsigned short i = 0; token != NULL; ++i) {
        array[i] = token;
        token = strtok(NULL, "x\n");
    }
    size[0] = atoi(array[0]);
    size[1] = atoi(array[1]);
    size[2] = -1;

    return size;
}

static SDL_Renderer* renderer;
static SDL_Window* window;
static TTF_Font* LiberationMono;
static SDL_Event evt;
static SDL_Color textcolor = {0, 0, 0};

static void render_text(char* msg, int x, int y) {
    SDL_Surface* surface = TTF_RenderText_Solid(LiberationMono, msg, textcolor);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect message_rect = { x, y, 0, 0 };
    TTF_SizeText(LiberationMono, msg, &message_rect.w, &message_rect.h);
    SDL_RenderCopy(renderer, texture, NULL, &message_rect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int main() {
start:

    TTF_Init();
    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(512, 64, 0, &window, &renderer);
    SDL_SetWindowTitle(window, "Hydra");
    short* dims = get_screen_size();
    SDL_SetWindowPosition(window, rand() % dims[0], rand() % dims[1]);

    FcConfig* config = FcInitLoadConfigAndFonts();
    FcPattern* pat = FcNameParse((const FcChar8*)"Liberation Mono");
    FcConfigSubstitute(config, pat, FcMatchPattern);
	FcDefaultSubstitute(pat);
    char* file;
    FcResult result;
    FcPattern* font = FcFontMatch(config, pat, &result);

    if(font) {
        FcChar8* file = NULL;
        if(FcPatternGetString(font, FC_FILE, 0, &file) == FcResultMatch)
            LiberationMono = TTF_OpenFont((char*)file, 16);
    } else return 0;

    SDL_SetRenderDrawColor(renderer, 0xff, 0xff, 0xff, 0xff);
    SDL_RenderClear(renderer);
    render_text("Cut off a head, two more will take its place.", 40, 24);
    SDL_RenderPresent(renderer);

    while(1) {
        SDL_WaitEvent(&evt);
        if(evt.type == SDL_QUIT) {
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            SDL_Quit();
            for(uint8_t i = 0; i < 2; i++) {
                pid_t p = fork();
                if(p == 0) {
                    srand(time(NULL) + i);
                    goto start;
                }
            }
            break;
        }
    }
    
    return 0;
}