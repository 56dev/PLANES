#include <raylib.h>
#include <raymath.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <vector>

float adjust_mouse(int, int);
void draw_to_screen(RenderTexture2D, int, int, float);
void render_plane();
Camera3D init_camera();
void update_camera(Camera3D &camera, Vector3 p);
RenderTexture2D target{};
enum class FlyState {
    DRAW,
    FLY
};
struct artist {
    std::vector<Vector2> points{};
    float canvas_W{};
    float canvas_H{};
    bool should_lock_starting_point{true};
    Vector2 locked_starting_point{};
    artist(float W, float H, bool slsp=true, Vector2 lsp=(Vector2){0, 0}) : canvas_W{W}, canvas_H{H}, should_lock_starting_point{slsp}, locked_starting_point{lsp} {

    }
    bool drawing_allowed{true};
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
    bool is_mouse_near_the_locked_starting_point() {
        constexpr float max_distance{50};
        return Vector2DistanceSqr(GetMousePosition(), locked_starting_point) <= max_distance*max_distance;
    }
    void add_points() {
        if(should_lock_starting_point) {
            if(points.size() == 0) {
                if(is_mouse_near_the_locked_starting_point()) {
                    points.push_back(locked_starting_point);
                }
                return;
            }
        }
        if(is_the_mouse_too_near_to_the_previous_point() == false && is_the_line_too_long() == false && is_the_mouse_in_bounds() == true) {
            points.push_back(GetMousePosition());
        }
    }

    void observe_mouse() {
        if(IsMouseButtonDown(MOUSE_BUTTON_LEFT) && drawing_allowed) {
            add_points();
        }
        if(IsMouseButtonReleased(MOUSE_BUTTON_LEFT) && points.size() > 0) {
            drawing_allowed = false;
        }
    }
    void reset() {
        drawing_allowed = true;
        points.clear();
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

    bool is_line_done_drawing() {
        return drawing_allowed == false;
    }


};
struct plane {
    Vector3 position{};
    float yaw{};
    float pitch{};
    float roll{};
    float forward_velocity{};

    void update() {
        position.z += 10.0f * GetFrameTime();
    }
    void render_plane() {
        DrawCube(position, 2, 2, 4, RED);
        DrawCubeWires(position, 2, 2, 4, BLACK);

        DrawCube(position, 5, 0.5, 2, RED);
        DrawCubeWires(position, 5, 0.5, 2, BLACK);

        DrawCube(position + Vector3(0, 1.8, -2), 0.5, 2, 1, RED);
        DrawCubeWires(position + Vector3(0, 1.8, -2), 0.5, 2, 1, BLACK);
    }
};
int main() {
    constexpr int W{1920};
    constexpr int H{1080};
    constexpr Vector2 center{W/2, H/2};
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT | FLAG_WINDOW_MAXIMIZED );
    InitWindow(W, H, "PLANES!!");
    InitAudioDevice();
    SetWindowMinSize(640, 360);
    SetTargetFPS(60);
    target = LoadRenderTexture(W, H);
    SetTextureFilter(target.texture, TEXTURE_FILTER_TRILINEAR);
    Camera3D camera{ init_camera() };
    artist a{W, H, true, center};
    plane p{};
    FlyState fly_state = FlyState::DRAW;
    while(!WindowShouldClose()) {
        float scale = adjust_mouse(W, H);
        UpdateCamera(&camera, CAMERA_THIRD_PERSON);
        BeginTextureMode(target);
        ClearBackground(RAYWHITE);
        if(fly_state == FlyState::DRAW) {

        } else if (fly_state == FlyState::FLY) {

        }
        a.observe_mouse();
        a.draw_points();
        DrawCircleV(center, 2.0f, BLUE);
        if(a.is_line_done_drawing()) {
            DrawText("OK / CANCEL", 0, 0, 100, BLACK);
            Vector2 button_dims{50, 20};
            Rectangle button_1{
                center.x - button_dims.x,
                center.y + button_dims.y,
                button_dims.x, button_dims.y
                };
            Rectangle button_2{
                center.x + button_dims.x,
                center.y + button_dims.y,
                button_dims.x, button_dims.y
            };
            GuiButton(button_1, "OK");
            if(GuiButton(button_2, "CANCEL")) {
                a.reset();
            }
        }
        /*BeginMode3D(camera);
            p.update();
            p.render_plane();
            update_camera(camera, p.position);
            DrawGrid(1000, 10.0f);
        EndMode3D();*/
        EndTextureMode();
        draw_to_screen(target, W, H, scale);

    }
    return 0;
}
void update_camera(Camera3D &camera, Vector3 p) {
    camera.target = p;
}

Camera3D init_camera() {
    Camera3D camera{};
    camera.position = (Vector3){10, 10, 10};
    camera.target = (Vector3){0, 0, 0};
    camera.up = (Vector3){0.0f, 1.0f, 0.0f};
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
    return camera;
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