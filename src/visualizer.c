#include "DynamicArray.h"
#include "Strings.h"
#include "math.h"
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

// Example emscripten compilation: emcc -o visualizer.html src/visualizer.c src/DynamicArray.c src/Strings.c -I/src -I/home/Cole/Programs/raylib/src -L/home/Cole/Programs/raylib/src -l:libraylib.web.a -lm -s USE_GLFW=3 -s WASM=1 -s ASYNCIFY -s GL_ENABLE_GET_PROC_ADDRESS=1
// Note: in order for this to work, a localhost will have to be performed, such as by doing python -m http.server

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

// x_pos and y_pos refer to the center of the text_box
// everything is based on percentages, except for font size, to make scaling work properly
typedef struct text_box {
    Color background;
    Color text_color;
    // x = width, y = height
    Vector2 dimensions;
    Vector2 pos;
    string text;
    float font_size;
    // The positioning of the text in the box based on
    // one of the values in the text_box_positions enum
    int text_pos;
    // Padding, like position and dimensions, is expressed a percentage, in this case a percentage of the height of
    // the total window. This means padding should be a very small percent. Also, it only applies to text that brushes
    // against the borders of the box
    float padding;
    Font* font;
    float spacing;
    float roundness;
    Color border_color;
    float border_thickness;
} text_box;

enum text_box_positions {
    TBOX_CENTER,
    TBOX_CENTER_LEFT,
    TBOX_CENTER_RIGHT,
    TBOX_TOP_CENTER,
    TBOX_TOP_LEFT,
    TBOX_TOP_RIGHT,
    TBOX_BOTTOM_CENTER,
    TBOX_BOTTOM_LEFT,
    TBOX_BOTTOM_RIGHT
};

// Note: the string is copied into the text_box, not stored as a reference to
// the string that is passed in
int text_box_init(text_box* tbox, string* text);

int text_box_destroy(text_box* tbox);

int text_box_render(text_box* tbox);

// For use with the dynamic_array_registry_type_append function
int text_box_deallocator(void* tbox) {
    text_box_destroy((text_box*)tbox);
    return 0;
}

int width = 800;
int height = 450;
float aspect_ratio = (float)450 / (float)800;

bool leftClickHold = false;
Vector2 prevMouse = {0.0f, 0.0f};
Vector2 currMouse = {0.0f, 0.0f};
Vector2 prevView = {0.0f, 0.0f};
float drag_factor = 0.8f;
float scroll_factor = 0.2f;
Camera2D camera = {0};
DynamicArray text_boxes;

DynamicArray fonts;

void UpdateDrawFrame(void);

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT | FLAG_WINDOW_HIGHDPI | FLAG_WINDOW_TRANSPARENT | FLAG_WINDOW_ALWAYS_RUN);
    InitWindow(width, height, "raylib [core] example - basic window");

    dynamic_array_registry_type_append(&STRING("Font"), NULL, sizeof(Font));
    dynamic_array_registry_type_append(&STRING("text_box"), text_box_deallocator, sizeof(text_box));
    dynamic_array_init(&fonts, &STRING("Font"));

    Font default_font, font1;
    default_font = GetFontDefault();
    font1 = LoadFontEx("../Merriweather-VariableFont_opsz,wdth,wght.ttf", 128, NULL, 0);
    dynamic_array_append(&fonts, &default_font);
    dynamic_array_append(&fonts, &font1);

    dynamic_array_init(&text_boxes, &STRING("text_box"));

    for (int i = 0; i < 9; i++) {
        text_box temp_box;
        text_box_init(&temp_box, &STRING("Sample Text\nMultiline"));
        temp_box.pos = (Vector2){(float)(20 + 50 * (i % 2)), (float)(40 + 40 * (int)(i / 2))};
        temp_box.font = (Font*)dynamic_array_get(&fonts, &INDEX(1));
        temp_box.text_pos = i;
        temp_box.padding = 1.5f;
        temp_box.roundness = 0.3f;
        temp_box.background = (Color){62, 180, 137, 255};
        temp_box.border_thickness = 0.3f;
        temp_box.border_color = BLACK;
        dynamic_array_append(&text_boxes, &temp_box);
    }

    camera.target = (Vector2){0.0f, 0.0f};
    camera.offset = (Vector2){0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    while (!WindowShouldClose()) {
        UpdateDrawFrame();
    }
#endif

    CloseWindow();
    dynamic_array_free(&text_boxes);
    dynamic_array_free(&fonts);
    return 0;
}

int text_box_init(text_box* tbox, string* text) {
    string_init(&tbox->text);
    string_copy(&tbox->text, text);

    // Initialize the rest to some default values that can be edited directly in the struct
    tbox->background = WHITE;
    tbox->font = (Font*)dynamic_array_get(&fonts, &INDEX(0));
    tbox->font_size = 12.0f;
    tbox->pos = (Vector2){.x = 50, .y = 50};
    tbox->dimensions = (Vector2){.x = 30, .y = 30};
    tbox->padding = 0.0f;
    tbox->text_color = BLACK;
    tbox->text_pos = TBOX_CENTER;
    tbox->spacing = 1.0f; // 1 is normal character spacing
    tbox->roundness = 0.0f;
    tbox->border_color = WHITE;
    tbox->border_thickness = 0.0f;
    return 0;
}

int text_box_destroy(text_box* tbox) {
    string_free(&tbox->text);
    return 0;
}

int text_box_render(text_box* tbox) {
    // DrawRectangle refers to the top left corner in raylib, so transformations
    // must be done to make it refer to the center of the rectangle (text box)
    float width_transl = (tbox->dimensions.x / 100.0f) * (float)width * aspect_ratio;
    float height_transl = (tbox->dimensions.y / 100.0f) * (float)height;
    float x_transl = (tbox->pos.x / 100.0f * (float)width * aspect_ratio) - (width_transl / 2.0f);
    float y_transl = (tbox->pos.y / 100.0f * (float)height) - (height_transl / 2.0f);
    float padding_transl = tbox->padding / 100.0f * (float)height;

    // Make font_size relative for better scaling
    float font_scaling_constant = 0.002f;
    float font_size_transl = tbox->font_size * (float)height * font_scaling_constant;
    float border_thickness_transl = tbox->border_thickness / 100.0f * (float)height;

    Vector2 textDimensions = MeasureTextEx(*tbox->font, tbox->text.str, font_size_transl, tbox->spacing);
    // DrawRectangle(x_transl, y_transl, width_transl, height_transl, tbox->background);
    DrawRectangleRounded((Rectangle){x_transl, y_transl, width_transl, height_transl}, tbox->roundness, 16, tbox->background);
    if (tbox->border_thickness != 0.0f) {
        DrawRectangleRoundedLinesEx((Rectangle){x_transl, y_transl, width_transl, height_transl}, tbox->roundness, 16, border_thickness_transl, tbox->border_color);
    }
    switch (tbox->text_pos) {
        case TBOX_CENTER:
            DrawTextEx(
                *tbox->font, tbox->text.str,
                (Vector2){
                    x_transl + width_transl / 2.0f - textDimensions.x / 2.0f,
                    y_transl + height_transl / 2.0f - textDimensions.y / 2.0f},
                font_size_transl, tbox->spacing, tbox->text_color);
            break;

        case TBOX_CENTER_LEFT:
            DrawTextEx(
                *tbox->font, tbox->text.str,
                (Vector2){
                    x_transl + padding_transl,
                    y_transl + height_transl / 2.0f - textDimensions.y / 2.0f},
                font_size_transl, tbox->spacing, tbox->text_color);
            break;

        case TBOX_CENTER_RIGHT:
            DrawTextEx(
                *tbox->font, tbox->text.str,
                (Vector2){
                    x_transl + width_transl - textDimensions.x - padding_transl,
                    y_transl + height_transl / 2.0f - textDimensions.y / 2.0f},
                font_size_transl, tbox->spacing, tbox->text_color);
            break;

        case TBOX_TOP_CENTER:
            DrawTextEx(
                *tbox->font, tbox->text.str,
                (Vector2){
                    x_transl + width_transl / 2.0f - textDimensions.x / 2.0f,
                    y_transl + padding_transl},
                font_size_transl, tbox->spacing, tbox->text_color);
            break;

        case TBOX_TOP_LEFT:
            DrawTextEx(
                *tbox->font, tbox->text.str,
                (Vector2){
                    x_transl + padding_transl,
                    y_transl + padding_transl},
                font_size_transl, tbox->spacing, tbox->text_color);
            break;

        case TBOX_TOP_RIGHT:
            DrawTextEx(
                *tbox->font, tbox->text.str,
                (Vector2){
                    x_transl + width_transl - textDimensions.x - padding_transl,
                    y_transl + padding_transl},
                font_size_transl, tbox->spacing, tbox->text_color);
            break;

        case TBOX_BOTTOM_CENTER:
            DrawTextEx(
                *tbox->font, tbox->text.str,
                (Vector2){
                    x_transl + width_transl / 2.0f - textDimensions.x / 2.0f,
                    y_transl + height_transl - textDimensions.y - padding_transl},
                font_size_transl, tbox->spacing, tbox->text_color);
            break;

        case TBOX_BOTTOM_LEFT:
            DrawTextEx(
                *tbox->font, tbox->text.str,
                (Vector2){
                    x_transl + padding_transl,
                    y_transl + height_transl - textDimensions.y - padding_transl},
                font_size_transl, tbox->spacing, tbox->text_color);
            break;

        case TBOX_BOTTOM_RIGHT:
            DrawTextEx(
                *tbox->font, tbox->text.str,
                (Vector2){
                    x_transl + width_transl - textDimensions.x - padding_transl,
                    y_transl + height_transl - textDimensions.y - padding_transl},
                font_size_transl, tbox->spacing, tbox->text_color);
            break;

        default:
            DrawTextEx(
                *tbox->font, tbox->text.str,
                (Vector2){
                    x_transl + width_transl / 2.0f - textDimensions.x / 2.0f,
                    y_transl + height_transl / 2.0f - textDimensions.y / 2.0f},
                font_size_transl, tbox->spacing, tbox->text_color);
    }

    return 0;
}

void UpdateDrawFrame(void) {
    if (IsWindowResized()) {
        width = GetScreenWidth();
        height = GetScreenHeight();
        aspect_ratio = (float)height / (float)width;
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        Vector2 mouse_delta = GetMouseDelta();

        camera.offset.x += mouse_delta.x * drag_factor;
        camera.offset.y += mouse_delta.y * drag_factor;
    }

    float mouseScroll = GetMouseWheelMove();
    if (mouseScroll != 0) {
        Vector2 mousePos = GetMousePosition();
        Vector2 preMousePos = GetScreenToWorld2D(mousePos, camera);

        camera.target = Vector2Subtract(camera.target, preMousePos);
        camera.zoom = Clamp(expf(logf(camera.zoom) + scroll_factor * mouseScroll), 0.125f, 64.0f);
        Vector2 postMousePos = GetScreenToWorld2D(mousePos, camera);
        camera.target = Vector2Add(camera.target, preMousePos);
        camera.target = Vector2Subtract(camera.target, postMousePos);
    }

    BeginDrawing();

    //ClearBackground((Color){203, 195, 227, 100});
    ClearBackground(BLANK);
    //DrawFPS(10, 10);

    BeginMode2D(camera);

    for (int i = 0; i < text_boxes.len; i++) {
        text_box* tempRender = (text_box*)dynamic_array_get(&text_boxes, &INDEX(i));
        text_box_render(tempRender);
    }

    EndMode2D();

    //EndBlendMode();
    EndDrawing();
}
