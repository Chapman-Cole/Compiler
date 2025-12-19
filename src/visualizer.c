#include "DynamicArray.h"
#include "Strings.h"
#include "math.h"
#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

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
    int padding;
    Font* font;
    float spacing;
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
int text_box_init(text_box* tbox, string text);

int text_box_destroy(text_box* tbox);

int text_box_render(text_box* tbox);

float lerp(float a, float b, float t) {
    return a + t * (b - a);
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

DynamicArray fonts;

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "raylib [core] example - basic window");

    dynamic_array_registry_type_append(&STRING("Font"), NULL, sizeof(Font));
    dynamic_array_init(&fonts, &STRING("Font"));

    Font default_font, font1, font2;
    default_font = GetFontDefault();
    font1 = LoadFontEx("/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Bold.ttf", 64, NULL, 0);
    font2 = LoadFontEx("/usr/share/fonts/Adwaita/AdwaitaMono-Bold.ttf", 64, NULL, 0);
    dynamic_array_append(&fonts, &default_font);
    dynamic_array_append(&fonts, &font1);
    dynamic_array_append(&fonts, &font2);

    text_box text;
    text_box_init(&text, STRING("Sample Text\nNewline"));
    text.pos = (Vector2){25, 50};
    text.font = (Font*)dynamic_array_get(&fonts, &INDEX(1));

    text_box text2;
    text_box_init(&text2, STRING("Some Sample\nText!"));
    text2.pos = (Vector2){75, 50};
    text2.font = (Font*)dynamic_array_get(&fonts, &INDEX(2));

    text_box rect;
    text_box_init(&rect, STRING(""));
    rect.background = (Color){255, 128, 128, 255};

    Camera2D camera = {0};
    camera.target = (Vector2){0.0f, 0.0f};
    camera.offset = (Vector2){0.0f, 0.0f};
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            width = GetScreenWidth();
            height = GetScreenHeight();
            aspect_ratio = (float)height / (float)width;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (leftClickHold == false) {
                prevMouse = GetScreenToWorld2D(GetMousePosition(), camera);
                prevView = camera.target;
                leftClickHold = true;
            }

            currMouse = GetScreenToWorld2D(GetMousePosition(), camera);

            camera.target = (Vector2){
                .x = prevView.x + drag_factor * (prevMouse.x - currMouse.x),
                .y = prevView.y + drag_factor * (prevMouse.y - currMouse.y)};

        } else {
            leftClickHold = false;
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

        ClearBackground((Color){203, 195, 227, 150});

        BeginMode2D(camera);

        text_box_render(&rect);
        text_box_render(&text);
        text_box_render(&text2);

        EndMode2D();

        EndDrawing();
    }

    CloseWindow();
    text_box_destroy(&text);
    text_box_destroy(&text2);
    text_box_destroy(&rect);
    dynamic_array_free(&fonts);
    return 0;
}

int text_box_init(text_box* tbox, string text) {
    string_init(&tbox->text);
    string_copy(&tbox->text, &text);

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
    return 0;
}

int text_box_destroy(text_box* tbox) {
    string_free(&tbox->text);
    return 0;
}

int text_box_render(text_box* tbox) {
    // DrawRectangle refers to the top left corner in raylib, so transformations
    // must be done to make it refer to the center of the rectangle (text box)
    float width_transl = (int)((tbox->dimensions.x / 100.0f) * (float)width * aspect_ratio);
    float height_transl = (int)((tbox->dimensions.y / 100.0f) * (float)height);
    float x_transl = (int)(((tbox->pos.x)) / 100.0f * (float)width) - (width_transl / 2.0f);
    float y_transl = (int)(((tbox->pos.y)) / 100.0f * (float)height) - (height_transl / 2.0f);

    // Make font_size relative for better scaling
    float font_scaling_constant = 0.002f;
    int font_size_transl = (int)((float)tbox->font_size * (float)height * font_scaling_constant);

    DrawRectangle(x_transl, y_transl, width_transl, height_transl, tbox->background);
    switch (tbox->text_pos) {
        case TBOX_CENTER:
            DrawTextEx(*tbox->font, tbox->text.str, (Vector2){x_transl, y_transl}, font_size_transl, tbox->spacing, tbox->text_color);
            break;

        case TBOX_CENTER_LEFT:
            break;

        case TBOX_CENTER_RIGHT:
            break;

        case TBOX_TOP_CENTER:
            break;

        case TBOX_TOP_LEFT:
            break;

        case TBOX_TOP_RIGHT:
            break;

        case TBOX_BOTTOM_CENTER:
            break;

        case TBOX_BOTTOM_LEFT:
            break;

        case TBOX_BOTTOM_RIGHT:
            break;

        default:
            DrawText(tbox->text.str, x_transl, y_transl, font_size_transl, tbox->text_color);
    }

    return 0;
}
