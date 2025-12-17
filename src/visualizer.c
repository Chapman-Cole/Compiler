#include "Strings.h"
#include "raylib.h"
#include <stdbool.h>

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
    Font font;
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

int text_box_set_font(text_box* tbox, string path);

int text_box_destroy(text_box* tbox);

int text_box_render(text_box* tbox, Vector2 transform, float zoom);

float lerp(float a, float b, float t) {
    return a + t * (b - a);
}

int width = 800;
int height = 450;
float aspect_ratio = (float)450 / (float)800;

bool leftClickHold = false;
Vector2 prevMouse = { 0.0f, 0.0f };
Vector2 currMouse = { 0.0f, 0.0f };
Vector2 prevView = { 0.0f, 0.0f };
float scroll_factor = 0.075f;

// Used for translation later on
Vector2 viewPos = { 0.0f, 0.0f };

float zoom_factor = 1.0f;

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(width, height, "raylib [core] example - basic window");

    text_box text;
    text_box_init(&text, STRING("Sample Text\nNewline"));
    text_box_set_font(&text, STRING("/usr/share/fonts/TTF/JetBrainsMonoNerdFont-Bold.ttf"));
    text.pos = (Vector2){25, 50};

    text_box text2;
    text_box_init(&text2, STRING("Some Sample\nText!"));
    text_box_set_font(&text2, STRING("/usr/share/fonts/Adwaita/AdwaitaMono-Bold.ttf"));
    text2.pos = (Vector2){75, 50};

    text_box rect;
    text_box_init(&rect, STRING(""));
    rect.background = (Color){255, 128, 128, 255};

    while (!WindowShouldClose()) {
        BeginDrawing();

        if (IsWindowResized()) {
            width = GetScreenWidth();
            height = GetScreenHeight();
            aspect_ratio = (float)height / (float)width;
        }

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            if (leftClickHold == false) {
                prevMouse = GetMousePosition();
                prevView = viewPos;
                leftClickHold = true;
            }

            currMouse = GetMousePosition();

            viewPos = (Vector2){ 
                .x = prevView.x + scroll_factor * (prevMouse.x - currMouse.x),
                .y = prevView.y + scroll_factor * (prevMouse.y - currMouse.y)
            };

        } else {
            leftClickHold = false;
        }

        float mouseScroll = GetMouseWheelMove();
        if (mouseScroll != 0) {
            zoom_factor += 0.05f * mouseScroll;
            if (zoom_factor < 0.0f) {
                zoom_factor = 0.0f;
            }
        }

        ClearBackground((Color){203, 195, 227, 150});
        text_box_render(&rect, (Vector2){0.0f, 0.0f}, 1.0f);
        text_box_render(&text, viewPos, zoom_factor);
        text_box_render(&text2, viewPos, zoom_factor);

        EndDrawing();
    }

    CloseWindow();
    text_box_destroy(&text);
    text_box_destroy(&text2);
    text_box_destroy(&rect);
    return 0;
}

int text_box_init(text_box* tbox, string text) {
    string_init(&tbox->text);
    string_copy(&tbox->text, &text);

    // Initialize the rest to some default values that can be edited directly in the struct
    tbox->background = WHITE;
    tbox->font = GetFontDefault();
    tbox->font_size = 12.0f;
    tbox->pos = (Vector2){ .x = 50, .y = 50 };
    tbox->dimensions = (Vector2){ .x = 30, .y = 30};
    tbox->padding = 0.0f;
    tbox->text_color = BLACK;
    tbox->text_pos = TBOX_CENTER;
    tbox->spacing = 1.0f; // 1 is normal character spacing
    return 0;
}

int text_box_set_font(text_box* tbox, string path) {
    tbox->font = LoadFontEx(path.str, 32, NULL, 0);
    return 0;
}

int text_box_destroy(text_box* tbox) {
    string_free(&tbox->text);
    return 0;
}

int text_box_render(text_box* tbox, Vector2 transform, float zoom) {
    // DrawRectangle refers to the top left corner in raylib, so transformations
    // must be done to make it refer to the center of the rectangle (text box)
    float width_transl = (int)((tbox->dimensions.x / 100.0f * zoom) * (float)width * aspect_ratio);
    float height_transl = (int)((tbox->dimensions.y / 100.0f * zoom) * (float)height);
    float x_transl = (int)(((tbox->pos.x - transform.x)) / 100.0f * (float)width) - (width_transl / 2.0f);
    float y_transl = (int)(((tbox->pos.y - transform.y)) / 100.0f * (float)height) - (height_transl / 2.0f);

    // Make font_size relative for better scaling
    float font_scaling_constant = 0.002f;
    int font_size_transl = (int)((float)tbox->font_size * (float)height * font_scaling_constant * zoom);

    DrawRectangle(x_transl, y_transl, width_transl, height_transl, tbox->background);
    switch (tbox->text_pos) {
        case TBOX_CENTER:
            DrawTextEx(tbox->font, tbox->text.str, (Vector2){x_transl, y_transl}, font_size_transl, tbox->spacing, tbox->text_color);
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
