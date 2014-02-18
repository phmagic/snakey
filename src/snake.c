#include <pebble.h>
#include "game.h"
#include "debrief.h"

static void init(void) {
    game_init();
    debrief_init();
}

static void deinit(void) {
    game_deinit();
    debrief_deinit();
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}
