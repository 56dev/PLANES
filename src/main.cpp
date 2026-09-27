#include <raylib.h>
#include <raymath.h>
#include <vector>

float adjust_mouse(int, int);
void draw_to_screen(RenderTexture2D, int, int, float);
RenderTexture2D target{};

struct artist {
    std::vector<Vector2> points{};
    float canvas_W{};
    float canvas_H{};
    artist(float W, float H) : canvas_W{W}, canvas_H{H} {

    }
    bool was_mouse_released{true};
    bool is_the_mouse_too_near_to_the_previous_point() {
        if(points.size() == 0) {
            return false;
        }
        constexpr float threshold{30};
        return Vector2DistanceSqr(GetMousePosition(), points[points.size()-1]) <= threshold * threshold;

    }
    bool is_the_line_too_long() {
        return points.size() > 135;
    }
    bool is_the_mouse_in_bounds() {
        return CheckCollisionPointRec(GetMousePosition(), (Rectangle){0, 0, canvas_W, canvas_H});
    }
    void add_points() {
        if(is_the_mouse_too_near_to_the_previous_point() == false && is_the_line_too_long() == false && is_the_mouse_in_bounds() == true) {
            points.push_back(GetMousePosition());
        }
    }
    void observe_mouse() {
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if(was_mouse_released == true) {
                was_mouse_released = false;
                points.clear();
            }
            add_points();
        }
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            was_mouse_released = true;
        }
    }
    #define px *1
    #define m *10

    void draw_points() {
        if(points.size() == 0) {
            return;
        }
        if(points.size() == 1) {
            DrawCircleV(points[0], 5.0f, RED);
            return;
        }
        for(std::size_t i = 0; i < points.size() - 1; ++i) {
            DrawCircleV(points[i], 5.0f, RED);
            DrawCircleV(points[i+1], 5.0f, RED);
            DrawLineEx(points[i], points[i + 1], 10.0f, RED);
        }
    }


};
int main() {
    constexpr int W{1920};
    constexpr int H{1080};
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_WINDOW_MAXIMIZED );
    InitWindow(W, H, "PLANES!!");
    InitAudioDevice();
    SetWindowMinSize(640, 360);
    SetTargetFPS(60);
    target = LoadRenderTexture(W, H);
    SetTextureFilter(target.texture, TEXTURE_FILTER_TRILINEAR);
    artist ar{W, H};
    while(!WindowShouldClose()) {
        float scale = adjust_mouse(W, H);
        BeginTextureMode(target);
        ClearBackground(RAYWHITE);
        ar.observe_mouse();
        ar.draw_points();
        EndTextureMode();
        draw_to_screen(target, W, H, scale);

    }
    return 0;
}



float adjust_mouse(int game_screen_width, int game_screen_height) {
    #define MIN(a,b) ((a) < (b) ? (a) : (b))
    float scale = MIN((float)GetScreenWidth()/game_screen_width, (float)GetScreenHeight()/game_screen_height);
    Vector2 mouse = GetMousePosition();
    Vector2 virtualMouse = { 0,0 };
    virtualMouse.x = (mouse.x - (GetScreenWidth() - (game_screen_width*scale))*0.5f)/scale;
    virtualMouse.y = (mouse.y - (GetScreenHeight() - (game_screen_height*scale))*0.5f)/scale;
    virtualMouse = Vector2Clamp(
        virtualMouse,
        (Vector2){0, 0},
        (Vector2){
            (float)game_screen_width,
            (float)game_screen_height}
        );
    SetMouseOffset((-(GetScreenWidth() - (game_screen_width*scale))*0.5f),
                (-(GetScreenHeight() - (game_screen_height*scale))*0.5f));
    SetMouseScale(1/scale, 1/scale);
    #undef MIN
    return scale;
}
void draw_to_screen(RenderTexture2D target, int game_screen_width, int game_screen_height, float scale) {
    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(target.texture,
        (Rectangle){
            0.0f, 0.0f,
            (float)target.texture.width,
            (float)-target.texture.height},
        (Rectangle){
            (GetScreenWidth() - ((float)game_screen_width*scale))*0.5f,
            (GetScreenHeight() - ((float)game_screen_height*scale))*0.5f,
            (float)game_screen_width*scale, (float)game_screen_height*scale
        },
        (Vector2){0, 0},
        0,
        WHITE
        );
    EndDrawing();
}