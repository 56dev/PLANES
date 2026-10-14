#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include <vector>

float adjust_mouse(int, int);
void draw_to_screen(RenderTexture2D, int, int, float);
void render_plane();
Camera3D init_camera();
void update_camera(Camera3D &camera, Vector3 p);
std::vector<Vector2> map_artist_points_onto_world(std::vector<Vector2>&);
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
    std::vector<Vector2> return_points() {
        return points;
    }


};
class plane {
private:
    float needed_angle{};
public:
    Vector3 position{};
    // all in radians
    float yaw{};
    float pitch{};
    float roll{};
    float forward_velocity{};
    std::vector<Vector2> points{};
    int idx_of_next_point{0};
    Vector3 forward_vec{};
    char yaw_debug_text[100];

    void update() {
        if(idx_of_next_point < points.size() - 1) {
            Vector2 next_point = points[idx_of_next_point];
            if(Vector3DistanceSqr(Vector3{position.x, 0, position.z}, Vector3{next_point.y, 0, next_point.x}) < 10 * 10) {
                ++idx_of_next_point;
            }
            needed_angle = atan2f(next_point.y - position.x, next_point.x - position.z);
            float yaw_speed = 1.0f;
            float epsilon = 2.0f;
            if(abs(yaw - needed_angle) > epsilon) {
                if(needed_angle < yaw) {
                    const char *d = TextFormat("TURNING LEFT, angle needed: %.5f, yaw: %.5f", needed_angle, yaw);
                    strncpy(yaw_debug_text, d, 100);
                    yaw -= yaw_speed * GetFrameTime();
                }
                else if (needed_angle > yaw) {
                    const char *d = TextFormat("TURNING RIGHT, angle needed: %.5f, yaw: %.5f", needed_angle, yaw);
                    strncpy(yaw_debug_text, d, 100);
                    yaw += yaw_speed * GetFrameTime();
                }
            }
//            yaw = needed_angle;
            Vector2 vec = Vector2{sin(yaw), cos(yaw)};
            forward_vec =Vector3{vec.x, 0, vec.y};
            position += forward_vec * 10.0f * GetFrameTime();
        }



    }
    void render_plane() {
        static Vector3 up{0, 1, 0};
        static Vector3 right{1, 0, 0};
        static Vector3 forward{0, 0, 1};
        DrawLine3D(position, position + up*10, GREEN);
        DrawLine3D(position, position + right*10, BLUE);
        DrawLine3D(position, position + forward*10, RED);
        DrawLine3D(position, position + forward_vec*30, PURPLE);
        DrawSphere(position, 1.0f, BLACK);



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
    FlyState fly_state{FlyState::DRAW};
    std::vector<Vector2> points_on_world{};

    while(!WindowShouldClose()) {
        float scale = adjust_mouse(W, H);
        BeginTextureMode(target);
        ClearBackground(RAYWHITE);

        if(fly_state == FlyState::DRAW) {
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

                if(GuiButton(button_1, "OK")) {
                    fly_state = FlyState::FLY;
                    std::vector<Vector2> points = a.return_points();
                    points_on_world = map_artist_points_onto_world(points);
                    p.points = points_on_world;
                }
                if(GuiButton(button_2, "CANCEL")) {
                    a.reset();
                }
        }
        } else if (fly_state == FlyState::FLY) {
            BeginMode3D(camera);
                p.update();
                p.render_plane();
                update_camera(camera, p.position);
                for(std::size_t i = 0; i < points_on_world.size(); ++i) {
                    Color col = p.idx_of_next_point == i ? RED : GREEN;
                    DrawSphere(Vector3{points_on_world[i].y, 0, points_on_world[i].x}, 3.0f, col);
                }
                DrawGrid(1000, 10.0f);
            EndMode3D();
            DrawText(p.yaw_debug_text, 0, 0, 30, BLACK);

        }


        EndTextureMode();
        draw_to_screen(target, W, H, scale);

    }
    return 0;
}

std::vector<Vector2> map_artist_points_onto_world(std::vector<Vector2>& artist_points) {
    /*
        The artist has a rectangular canvas of size 1920 by 1080 pixels, with (0, 0) in the upper left corner.
        The world, for simplicity's sake, shall also have dimensions 1920 by 1080 units, but (0, 0) is right in the middle.
        Therefore, the points will need to be translated left 1920 / 2, and up 1080 / 2
    */
   std::vector<Vector2> result{};
    for(std::size_t i = 0; i < artist_points.size(); ++i) {
        float x = artist_points[i].x;
        float y = artist_points[i].y;
        result.push_back(Vector2{x - 1920 / 2, -y + 1080 / 2});
    }
    return result;

}
void update_camera(Camera3D &camera, Vector3 p) {
    camera.target = p;
    UpdateCamera(&camera, CAMERA_THIRD_PERSON);
    return;
    camera.position = (Vector3){0, 10, -20} + p;
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