/**
* Author: William Liburd
* Assignment: Lunar Lander
* Date due: 2023-11-08, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 1
#define LEVEL1_WIDTH 25
#define LEVEL1_HEIGHT 15

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"

// ————— GAME STATE ————— //
struct GameState
{
    Entity* player;
    Entity* enemies;

    Map* map;

    Mix_Music* bgm;
    Mix_Chunk* jump_sfx;
};

// ————— CONSTANTS ————— //
const int WINDOW_WIDTH = 640,
WINDOW_HEIGHT = 480;

const float BG_RED = 0.0f,
BG_BLUE = 0.0,
BG_GREEN = 0.0f,
BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char GAME_WINDOW_NAME[] = "Hello, Maps!";

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECOND = 1000.0;

const char SPRITESHEET_FILEPATH[] = "assets/images/rocket_ts.png",
MAP_TILESET_FILEPATH[] = "assets/images/tileset.png",
BGM_FILEPATH[] = "assets/audio/dooblydoo.mp3",
JUMP_SFX_FILEPATH[] = "assets/audio/rocket-go-boom.wav",
FONT_SPRITE_FILEPATH[] = "sprites/font1.png";

const int FONTBANK_SIZE = 16;
int secs = 0;

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

float dx = 0.0f;
float dy = 0.0f;

float PI = acos(-1.0f);
float LEFT_RAD = 0.0f;
float RIGHT_RAD = 0.0f;
float MAX_RAD = PI / 2;

unsigned int LEVEL_1_DATA[] =
{
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 0, 0,
    0, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 0, 0,
    0, 3, 3, 3, 3, 3, 3, 3, 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 1, 0,
    3, 3, 3, 3, 3, 3, 3, 3, 1, 3, 0, 0, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 0,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 0, 0, 0, 0, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 0, 0, 3, 0, 3, 3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 1, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3
};

// ————— VARIABLES ————— //
GameState g_state;

SDL_Window* m_display_window;
bool m_game_is_running = true;
GLuint m_text_texture_id;

ShaderProgram m_program;
glm::mat4 m_view_matrix, m_projection_matrix;

float m_previous_ticks = 0.0f,
m_accumulator = 0.0f;

// ————— GENERAL FUNCTIONS ————— //
void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->SetModelMatrix(model_matrix);
    glUseProgram(m_program.programID);

    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->texCoordAttribute);

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return texture_id;
}

void initialise()
{
    // ————— GENERAL ————— //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    m_display_window = SDL_CreateWindow(GAME_WINDOW_NAME,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(m_display_window);
    SDL_GL_MakeCurrent(m_display_window, context);

#ifdef _WINDOWS
    glewInit();
#endif

    // ————— VIDEO SETUP ————— //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    m_program.Load(V_SHADER_PATH, F_SHADER_PATH);

    m_view_matrix = glm::mat4(1.0f);
    m_projection_matrix = glm::ortho(-10.0f, 10.0f, -7.50f, 7.50f, -2.0f, 2.0f);

    m_program.SetProjectionMatrix(m_projection_matrix);
    m_program.SetViewMatrix(m_view_matrix);

    glUseProgram(m_program.programID);

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    //glUseProgram(m_program.programID);
    m_text_texture_id = load_texture(FONT_SPRITE_FILEPATH);

    // ————— MAP SET-UP ————— //
    GLuint map_texture_id = load_texture(MAP_TILESET_FILEPATH);
    g_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 4, 1);

    // ————— GEORGE SET-UP ————— //
    // Existing
    g_state.player = new Entity();
    g_state.player->set_entity_type(PLAYER);
    g_state.player->set_position(glm::vec3(0.0f, 0.0f, 0.0f));
    g_state.player->set_movement(glm::vec3(0.0f));
    g_state.player->set_speed(2.5f);
    g_state.player->set_acceleration(glm::vec3(0.0f, -0.9f, 0.0f));
    g_state.player->m_texture_id = load_texture(SPRITESHEET_FILEPATH);

    // Walking
    g_state.player->m_walking[g_state.player->LEFT] = new int[1] { 1 };
    g_state.player->m_walking[g_state.player->RIGHT] = new int[1] { 2 };
    g_state.player->m_walking[g_state.player->UP] = new int[1] { 0 };
    g_state.player->m_walking[g_state.player->DOWN] = new int[1] { 0 };

    g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->RIGHT];  // start George looking left
    g_state.player->m_animation_frames = 1;
    g_state.player->m_animation_index = 0;
    g_state.player->m_animation_time = 0.0f;
    g_state.player->m_animation_cols = 3;
    g_state.player->m_animation_rows = 1;
    g_state.player->set_height(0.8f);
    g_state.player->set_width(0.8f);

    // Jumping
    g_state.player->m_jumping_power = 0.2f;


    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    g_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    Mix_PlayMusic(g_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 16.0f);

    g_state.jump_sfx = Mix_LoadWAV(JUMP_SFX_FILEPATH);

    // ————— BLENDING ————— //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            m_game_is_running = false;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                m_game_is_running = false;
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_state.player->m_movement.x = -1.0f;
        if (dx > -0.3f) { dx -= 0.05f; }
        g_state.player->set_acceleration(glm::vec3(g_state.player->get_acceleration().x + dx, g_state.player->get_acceleration().y, 0));
        g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_state.player->m_movement.x = 1.0f;
        if (dx < 0.3f) { dx += 0.05f; }
        g_state.player->set_acceleration(glm::vec3(g_state.player->get_acceleration().x + dx, g_state.player->get_acceleration().y, 0));
        g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->RIGHT];
    }
    else if (key_state[SDL_SCANCODE_SPACE])
    {
        g_state.player->m_is_jumping = true;
        g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->UP];
        Mix_PlayChannel(-1, g_state.jump_sfx, 0);
    }
    else {
        g_state.player->m_animation_indices = g_state.player->m_walking[g_state.player->UP];
    }

    // This makes sure that the player can't move faster diagonally
    if (glm::length(g_state.player->m_movement) > 1.0f)
    {
        g_state.player->m_movement = glm::normalize(g_state.player->m_movement);
    }
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - m_previous_ticks;
    m_previous_ticks = ticks;

    delta_time += m_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        m_accumulator = delta_time;
        return;
    }

    if (!g_state.player->collided_bottom()) {
        while (delta_time >= FIXED_TIMESTEP)
        {
            g_state.player->update(FIXED_TIMESTEP, g_state.player, NULL, 0, g_state.map);
            delta_time -= FIXED_TIMESTEP;
        }
    }

    m_accumulator = delta_time;

    m_view_matrix = glm::mat4(1.0f);
    m_view_matrix = glm::translate(m_view_matrix, glm::vec3(-g_state.player->get_position().x, -g_state.player->get_position().y, 0.0f));
}

void render()
{
    m_program.SetViewMatrix(m_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);

    g_state.player->render(&m_program);
    g_state.map->render(&m_program);
    if (g_state.player->collided_bottom()) {
        secs += 1;
        if (g_state.map->get_goal())
        {
            draw_text(&m_program, m_text_texture_id, std::string("Mission Accomplished"), 0.50f, 0.001f, glm::vec3(g_state.player->get_position().x - 4.0f, g_state.player->get_position().y + 2.0f, g_state.player->get_position().z));
        }
        else {
            draw_text(&m_program, m_text_texture_id, std::string("Mission Failed"), 0.50f, 0.01f, glm::vec3(g_state.player->get_position().x - 2.0f, g_state.player->get_position().y + 2.0f, g_state.player->get_position().z));
        }

        if (secs == 3000){ m_game_is_running = false; }
        
    }

    SDL_GL_SwapWindow(m_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete[] g_state.enemies;
    delete    g_state.player;
    delete    g_state.map;
    Mix_FreeChunk(g_state.jump_sfx);
    Mix_FreeMusic(g_state.bgm);
}

// ————— GAME LOOP ————— //
int main(int argc, char* argv[])
{
    initialise();

    while (m_game_is_running)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}