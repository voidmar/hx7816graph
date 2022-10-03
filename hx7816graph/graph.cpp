// OpenGL
#include <GL/glew.h> // must be before freeglut
#include <GL/freeglut.h>

// 2D graphics lib
#include <nanovg.h>
#define NANOVG_GL2_IMPLEMENTATION // Use GL2 implementation.
#include <nanovg_gl.h>

#include "lcos_ctl_data.h"

#include <cstdint>
#include <algorithm>

using std::min;
using std::max;

struct Subframe
{
    uint8_t color = 0;
    uint16_t start_line = 0;
    uint16_t end_line = 0;
};

uint8_t subframe_count = 0;
Subframe frc_frames[10];
Subframe led_frames[10];

uint8_t frc_sel = 0;
uint16_t frc_data = 0;

uint8_t led_sel = 0;
uint16_t led_data = 0;

NVGcontext* nvg_context = nullptr;

void apply_frc_data()
{
    if (frc_sel > 9)
        return;

    if (frc_sel > 0)
        frc_frames[frc_sel].start_line = frc_frames[frc_sel - 1].end_line;

    frc_frames[frc_sel].end_line = frc_frames[frc_sel].start_line + frc_data;
}

void apply_led_data()
{
    int subframe = led_sel / 2;
    if (led_sel & 1)
        led_frames[subframe].end_line = led_data;
    else
        led_frames[subframe].start_line = led_data;
}

void decode_hx7816_register(uint8_t reg, uint8_t value)
{
    switch (reg)
    {
    case 0x77:
        subframe_count = value;
        break;
    case 0x78:
        frc_frames[0].color = value & 0x03;
        frc_frames[1].color = (value & 0x0C) >> 2;
        frc_frames[2].color = (value & 0x30) >> 4;
        frc_frames[3].color = (value & 0xC0) >> 6;
        break;
    case 0x79:
        frc_frames[4].color = value & 0x03;
        frc_frames[5].color = (value & 0x0C) >> 2;
        frc_frames[6].color = (value & 0x30) >> 4;
        frc_frames[7].color = (value & 0xC0) >> 6;
        break;
    case 0x7A:
        frc_frames[8].color = value & 0x03;
        frc_frames[9].color = (value & 0x0C) >> 2;
        break;

    case 0x7B:
        frc_sel = value;
        break;
    case 0x7C:
        frc_data = (frc_data & 0xFF00) | value;
        break;
    case 0x7D:
        frc_data = (frc_data & 0xFF) | value << 8;
        apply_frc_data();
        ++frc_sel;
        break;

    case 0xC0:
        led_frames[0].color = value & 0x03;
        led_frames[1].color = (value & 0x0C) >> 2;
        led_frames[2].color = (value & 0x30) >> 4;
        led_frames[3].color = (value & 0xC0) >> 6;
        break;
    case 0xC1:
        led_frames[4].color = value & 0x03;
        led_frames[5].color = (value & 0x0C) >> 2;
        led_frames[6].color = (value & 0x30) >> 4;
        led_frames[7].color = (value & 0xC0) >> 6;
        break;
    case 0xC2:
        led_frames[8].color = value & 0x03;
        led_frames[9].color = (value & 0x0C) >> 2;
        break;

    case 0xC3:
        led_sel = value;
        break;
    case 0xC4:
        led_data = (led_data & 0xFF00) | value;
        break;
    case 0xC5:
        led_data = (led_data & 0xFF) | value << 8;
        apply_led_data();
        ++led_sel;
        break;
    }
}

NVGcolor convert_color(int color)
{
    switch (color)
    {
    case 0:
        return nvgRGBf(1, 0, 0);
    case 1:
        return nvgRGBf(0, 1, 0);
    case 2:
        return nvgRGBf(0, 0, 1);
    case 3:
        return nvgRGBf(0.25f, 0.25f, 0.25f);
    }
}

void render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    auto vg = nvg_context;

    nvgBeginFrame(vg, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT), 1.0f);

    int v_border_left = 424;
    int v_border_right = 424;

    int v_last = frc_frames[subframe_count].end_line;

    float v_scale = (float)glutGet(GLUT_WINDOW_WIDTH) / (frc_frames[subframe_count].end_line + v_border_left + v_border_right);

    for (int f = 0; f < subframe_count; ++f)
    {
        auto& frc_frame = frc_frames[f];

        int x = v_border_left + frc_frame.start_line;
        int w = frc_frame.end_line - frc_frame.start_line;

        auto base_color = convert_color(frc_frame.color);
        nvgFillColor(vg, nvgTransRGBAf(base_color, 0.25f));
        nvgStrokeColor(vg, base_color);

        nvgBeginPath(vg);
        nvgRect(vg, x * v_scale, 15, w * v_scale, 50);
        nvgFill(vg);
        nvgStroke(vg);

        char label[8];
        sprintf(label, "%d", f);

        nvgFillColor(vg, nvgRGBf(1, 1, 1));
        nvgText(vg, x * v_scale + 2, 29, label, NULL);
    }

    for (int c = 0; c < 3; ++c)
    {
        int y = 15 + 50 + 50 * c;

        auto base_color = convert_color(c);
        nvgStrokeColor(vg, base_color);

        for (int f = 0; f < subframe_count; ++f)
        {
            auto& led_frame = led_frames[f];
            if (led_frame.color != c)
                continue;

            int end_line = led_frame.end_line;
            if (end_line < led_frame.start_line)
                end_line += v_last; // LED timing is mod sum of v_total

            int x = v_border_left + led_frame.start_line;
            int w = end_line - led_frame.start_line;

            nvgBeginPath(vg);
            nvgRect(vg, x * v_scale, y + 1, w * v_scale, 50);
            nvgFillColor(vg, nvgTransRGBAf(base_color, 0.25f));
            nvgFill(vg);
            nvgStroke(vg);

            char label[8];
            sprintf(label, "%d", f);

            nvgFillColor(vg, nvgRGBf(1, 1, 1));
            nvgText(vg, x * v_scale + 2, y + 14, label, NULL);
        }
    }

    nvgEndFrame(vg);

    glutSwapBuffers();
    glutPostRedisplay();
}

int main(int argc, char* argv[])
{
    int reg_count = sizeof(hx7816_init_regs) / sizeof(hx7816_init_regs[0]) / 2;
    for (int i = 0; i < reg_count; ++i)
        decode_hx7816_register(hx7816_init_regs[i * 2], hx7816_init_regs[i * 2 + 1]);

    glutInit(&argc, argv);
    glutInitWindowPosition(-1, -1);
    glutInitWindowSize(1024, 768);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_STENCIL);
    glutCreateWindow("HX7816 Viewer");

    glutDisplayFunc(&render);

    glewExperimental = true; // required to init on Linux, wtf?

    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        printf("Failed to init GLEW: %s\n", glewGetErrorString(err));
        return 1;
    }

    nvg_context = nvgCreateGL2(NVG_ANTIALIAS);

    int font = nvgCreateFont(nvg_context, "label", "assets/VeraMono.ttf");
    if (font == -1)
    {
        printf("Failed to create font", glewGetErrorString(err));
        return 1;
    }

    glutMainLoop();

    nvgDeleteGL2(nvg_context);
}
