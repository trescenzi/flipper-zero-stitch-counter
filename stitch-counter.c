#include <furi.h>
#include <gui/gui.h>
#include <gui/elements.h>
#include <input/input.h>
#include <stdlib.h>
#include <storage/storage.h>
#include <toolbox/stream/stream.h>
#include <toolbox/stream/file_stream.h>
#include <flipper_format/flipper_format.h>
// #include <counter_icons.h>

#define MAX_COUNT 99
#define BOXTIME 2
#define BOXWIDTH 32
#define MAX_X 128
#define MAX_Y 64
#define ROW_COUNT_MIDDLE_X MAX_X / 4
#define ROW_COUNT_MIDDLE_Y MAX_Y / 2
#define ROW_COUNT_TOP_X ROW_COUNT_MIDDLE_X - BOXWIDTH / 2
#define ROW_COUNT_TOP_Y ROW_COUNT_MIDDLE_Y - BOXWIDTH / 2
#define OTHER_COUNT_MIDDLE_X MAX_X - BOXWIDTH
#define OTHER_COUNT_MIDDLE_Y MAX_Y / 2
#define OTHER_COUNT_TOP_X OTHER_COUNT_MIDDLE_X - BOXWIDTH / 2
#define OTHER_COUNT_TOP_Y OTHER_COUNT_MIDDLE_Y - BOXWIDTH / 2
#define OFFSET_Y 9

#define TAG "stitch_counter"
#define OTHER_COUNT "other_count"
#define ROW_COUNT "row_count"

typedef struct {
  FuriMessageQueue* input_queue;
  ViewPort* view_port;
  Gui* gui;
  FuriMutex** mutex;

  uint32_t row_count;
  uint32_t other_count;

  bool row_active;
  bool pressed;
  int boxtimer;
} Counter;

const char* DATA_PATH = EXT_PATH("data.txt");

void state_free(Counter* c) {
  gui_remove_view_port(c->gui, c->view_port);
  furi_record_close(RECORD_GUI);
  view_port_free(c->view_port);
  furi_message_queue_free(c->input_queue);
  furi_mutex_free(c->mutex);
  free(c);
}

static void input_callback(InputEvent* input_event, void* ctx) {
  Counter* c = ctx;
  if(input_event->type == InputTypeShort) {
    furi_message_queue_put(c->input_queue, input_event, 0);
  }
}

static void render_callback(Canvas* canvas, void* ctx) {
  Counter* c = ctx;
  furi_check(furi_mutex_acquire(c->mutex, FuriWaitForever) == FuriStatusOk);
  canvas_clear(canvas);
  canvas_set_color(canvas, ColorBlack);
  canvas_set_font(canvas, FontBigNumbers);

  char scount[5];
  if(c->pressed == true || c->boxtimer > 0) {
    int top_x = c->row_active ? ROW_COUNT_TOP_X : OTHER_COUNT_TOP_X;
    int top_y = c->row_active ? ROW_COUNT_TOP_Y : OTHER_COUNT_TOP_Y;
    int unselected_top_x = !c->row_active ? ROW_COUNT_TOP_X : OTHER_COUNT_TOP_X;
    int unselected_top_y = !c->row_active ? ROW_COUNT_TOP_Y : OTHER_COUNT_TOP_Y;
    elements_bold_rounded_frame(canvas, top_x, top_y, BOXWIDTH, BOXWIDTH);
    elements_bold_rounded_frame(canvas, top_x - 1, top_y - 1, BOXWIDTH + 2, BOXWIDTH + 2);
    elements_bold_rounded_frame(canvas, top_x - 2, top_y - 2, BOXWIDTH + 4, BOXWIDTH + 4);
    elements_slightly_rounded_frame(canvas, unselected_top_x, unselected_top_y, BOXWIDTH, BOXWIDTH);
    c->pressed = false;
    c->boxtimer--;
  } else {
    // Draw the boxes at rest, bold the active one
    if (c->row_active) {
      elements_bold_rounded_frame(canvas, ROW_COUNT_TOP_X, ROW_COUNT_TOP_Y, BOXWIDTH, BOXWIDTH);
      elements_slightly_rounded_frame(canvas, OTHER_COUNT_TOP_X, OTHER_COUNT_TOP_Y, BOXWIDTH, BOXWIDTH);
    } else {
      elements_slightly_rounded_frame(canvas, ROW_COUNT_TOP_X, ROW_COUNT_TOP_Y, BOXWIDTH, BOXWIDTH);
      elements_bold_rounded_frame(canvas, OTHER_COUNT_TOP_X, OTHER_COUNT_TOP_Y, BOXWIDTH, BOXWIDTH);
    }
  }
  // Draw the counts
  snprintf(scount, sizeof(scount), "%ld", c->row_count);
  canvas_draw_str_aligned(canvas, ROW_COUNT_MIDDLE_X, ROW_COUNT_MIDDLE_Y, AlignCenter, AlignCenter, scount);
  snprintf(scount, sizeof(scount), "%ld", c->other_count);
  canvas_draw_str_aligned(canvas, OTHER_COUNT_MIDDLE_X, OTHER_COUNT_MIDDLE_Y, AlignCenter, AlignCenter, scount);

  canvas_set_font(canvas, FontPrimary);
  canvas_draw_str_aligned(canvas, ROW_COUNT_MIDDLE_X, ROW_COUNT_TOP_Y - 7, AlignCenter, AlignCenter, "Row:");
  canvas_draw_str_aligned(canvas, OTHER_COUNT_MIDDLE_X, OTHER_COUNT_TOP_Y - 7, AlignCenter, AlignCenter, "Other:");
  furi_mutex_release(c->mutex);
}

int get_initial_count(const char* key) {
  uint32_t starting_count = 0;
  Storage* storage = furi_record_open(RECORD_STORAGE);
  FlipperFormat* file = flipper_format_file_alloc(storage);

  FURI_LOG_I(TAG, "-------------------Opening-----------------");
  do {
    if (!flipper_format_file_open_existing(file, DATA_PATH)) {
      FURI_LOG_E(TAG, "Failed to open initial count: %s", key);
      break;
    }
    if (!flipper_format_read_uint32(file, key, &starting_count, 1)) {
      FURI_LOG_E(TAG, "Failed to read initial count: %s", key);
      break;
    }
  } while(0);
  FURI_LOG_I(TAG, "---------------Done------------------------");

  flipper_format_free(file);
  furi_record_close(RECORD_STORAGE);

  return starting_count;
}

bool save_count(uint32_t row_count, uint32_t other_count) {
  Storage* storage = furi_record_open(RECORD_STORAGE);
  FlipperFormat* format = flipper_format_file_alloc(storage);

  FURI_LOG_I(TAG, "------------------Saving-----------------");
  FURI_LOG_E(TAG, "Saving row: %ld, other: %ld",row_count, other_count);
  if(!flipper_format_file_open_always(format, DATA_PATH)) {
    FURI_LOG_E(TAG, "Failed to open file to save count");
    return false;
  }
  if(!flipper_format_write_uint32(format, ROW_COUNT, &row_count, 1)) {
    FURI_LOG_E(TAG, "Failed to save row");
    return false;
  }
  if (!flipper_format_write_uint32(format, OTHER_COUNT, &other_count, 1)) {
    FURI_LOG_E(TAG, "Failed to save other");
    return false;
  }
  FURI_LOG_I(TAG, "-------------Done Saving-----------------");

  flipper_format_free(format);
  furi_record_close(RECORD_STORAGE);
  return true;
}

Counter* state_init() {
  Counter* c = malloc(sizeof(Counter));
  c->input_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
  c->view_port = view_port_alloc();
  c->gui = furi_record_open(RECORD_GUI);
  c->mutex = furi_mutex_alloc(FuriMutexTypeNormal);
  c->row_count = get_initial_count(ROW_COUNT);
  c->other_count = get_initial_count(OTHER_COUNT);
  c->boxtimer = 0;
  c->row_active = true;
  view_port_input_callback_set(c->view_port, input_callback, c);
  view_port_draw_callback_set(c->view_port, render_callback, c);
  gui_add_view_port(c->gui, c->view_port, GuiLayerFullscreen);
  return c;
}

int32_t stitchcounter(void) {
  Counter* c = state_init();

  while(1) {
    InputEvent input;
    while(furi_message_queue_get(c->input_queue, &input, FuriWaitForever) == FuriStatusOk) {
      furi_check(furi_mutex_acquire(c->mutex, FuriWaitForever) == FuriStatusOk);

      if(input.key == InputKeyBack) {
        furi_mutex_release(c->mutex);
        state_free(c);
        return 0;
      } else if(input.key == InputKeyUp && c->row_count < MAX_COUNT) {
        c->pressed = true;
        c->boxtimer = BOXTIME;
        if (c->row_active) {
          c->row_count++;
        } else {
          c->other_count++;
        }
        save_count(c->row_count, c->other_count);
      } else if(input.key == InputKeyDown && c->row_count != 0) {
        c->pressed = true;
        c->boxtimer = BOXTIME;
        if (c->row_active) {
          c->row_count--;
        } else {
          c->other_count--;
        }
        save_count(c->row_count, c->other_count);
      } else if (input.key == InputKeyRight || input.key == InputKeyLeft) {
        c->pressed = true;
        c->boxtimer = BOXTIME;
        c->row_active = !c->row_active;
      } else if (input.key == InputKeyOk) {
        c->pressed = true;
        c->boxtimer = BOXTIME;
        if (c->row_active) {
          c->row_count = 0;
        } else {
          c->other_count = 0;
        }
        save_count(c->row_count, c->other_count);
      }
      furi_mutex_release(c->mutex);
      view_port_update(c->view_port);
    }
  }
  state_free(c);
  return 0;
}
