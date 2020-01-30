#pragma once
#include "Engine/EngineCommon.hpp"

struct Rgba;
struct Vec2;

//GameCommon is holding mainly global values that will be used thought the game
//constexpr float CLIENT_ASPECT = 2.0f; // We are requesting a 1:1 aspect (square) window area

class App;
class RenderContext;
class InputSystem;
class AudioSystem;

extern App* g_theApp;
extern AudioSystem* g_theAudio;

//Window config
constexpr float WORLD_ASPECT = 1.7777777777777778f;

//Debug global variables
constexpr int NUM_DISC_VERTICES = 16;

//Camera global variables
constexpr float WORLD_WIDTH = 71.1111111110f;
constexpr float WORLD_HEIGHT = 40.0f;
constexpr float WORLD_CENTER_X = WORLD_WIDTH / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_HEIGHT / 2.f;

// key codes
constexpr int SHIFT_KEY = 16;
constexpr int ESC_KEY = 27;
constexpr int SPACE_BAR = 32;
constexpr int LEFT_ARROW = 37;
constexpr int UP_ARROW = 38;
constexpr int RIGHT_ARROW = 39;
constexpr int DOWN_ARROW = 40;
constexpr int NUM_0_KEY = 48;
constexpr int NUM_1_KEY = 49;
constexpr int NUM_2_KEY = 50;
constexpr int NUM_3_KEY = 51;
constexpr int NUM_4_KEY = 52;
constexpr int NUM_5_KEY = 53;
constexpr int NUM_6_KEY = 54;
constexpr int NUM_7_KEY = 55;
constexpr int NUM_8_KEY = 56;
constexpr int NUM_9_KEY = 57;
constexpr int A_KEY = 65;
constexpr int B_KEY = 66;
constexpr int C_KEY = 67;
constexpr int D_KEY = 68;
constexpr int E_KEY = 69;
constexpr int F_KEY = 70;
constexpr int G_KEY = 71;
constexpr int H_KEY = 72;
constexpr int I_KEY = 73;
constexpr int J_KEY = 74;
constexpr int K_KEY = 75;
constexpr int L_KEY = 76;
constexpr int M_KEY = 77;
constexpr int N_KEY = 78;
constexpr int O_KEY = 79;
constexpr int P_KEY = 80;
constexpr int Q_KEY = 81;
constexpr int R_KEY = 82;
constexpr int S_KEY = 83;
constexpr int T_KEY = 84;
constexpr int U_KEY = 85;
constexpr int V_KEY = 86;
constexpr int W_KEY = 87;
constexpr int X_KEY = 88;
constexpr int Y_KEY = 89;
constexpr int Z_KEY = 90;
constexpr int F1_KEY = 112;
constexpr int F2_KEY = 113;
constexpr int F8_KEY = 119;
constexpr int TILDE_KEY = 192;

// camera global variables
constexpr float MAX_SCREEN_SHAKE = 2.0f;
constexpr float SCREEN_SHAKE_REDUCTION = 1.0f;

//One-off drawing functions
void DrawLine(const Vec2& start, const Vec2& end, float thickness, const Rgba& tint);
