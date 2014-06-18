#include <pebble.h>
#include "game.h"
#include "debrief.h"

#define SNAKE_BODY_WIDTH 5
#define SNAKE_BODY_SPACING 0
#define APPLE_SIZE 5
#define GAME_TICK_INTERVAL 100
#define STATUS_BAR_HEIGHT 16
#define BOUNDS_ADJUSTMENT 2
#define DEFAULT_SNAKE_SIZE 3
#define SNAKE_CONTACT_LENIENCY 1.5
#define BONUS_TIMER_INVTERVAL 500

static float container_width = 100;
static float container_height = 100;

typedef struct snake_section_t {
	float x;
	float y;
	struct snake_section_t *next;
} snake_section_t;

typedef struct snake_t {
    unsigned length;
	unsigned direction; // Directions are 0 - Right, 1 - Down, 2 - Left, 3 - Up
	struct snake_section_t *head;
} snake_t;

typedef struct apple_t {
    float x;
    float y;
} apple_t;

typedef struct game_state_t {
    unsigned alive;
    unsigned is_paused;
    unsigned is_resetting;
    unsigned score;
    unsigned bonus_points; // Bonus points decreases the longer the user takes to eat the apple
    unsigned queued_input; // 0 - Nothing, 1 - Up, 2 - Down, 3 - Pause
} game_state_t;

static Window *game_window;
static Layer *game_layer;

static game_state_t *game;
static snake_t *snake;
static apple_t *apple;

static AppTimer *game_timer;
static AppTimer *bonus_points_timer;

static TextLayer *game_score_label;
static TextLayer *game_bonus_label;

//--------------------------------------------- 
// Convenience Methods
//---------------------------------------------

// Used to place the apple on a grid
// spaced to the width of the apple
int round_to_nearest_multiple(int number, int multiple)
{
    if (multiple == 0) {
        return number;
    }
    int remain = number % multiple;
    return (remain == 0) ? number : number + multiple - remain;
}

//--------------------------------------------- 
// Snake Methods
//---------------------------------------------

static void add_to_head(snake_t *snake)
{
	snake_section_t *new_head = malloc(sizeof(snake_section_t));
    snake_section_t *current_head = snake->head;
    switch (snake->direction) {
        case 1:
            // Down
            new_head->y = current_head->y + 2*SNAKE_BODY_WIDTH + SNAKE_BODY_SPACING;
            new_head->x = current_head->x;
            break;
        case 2:
            // Left
            new_head->x = current_head->x - 2*SNAKE_BODY_WIDTH - SNAKE_BODY_SPACING;
            new_head->y = current_head->y;
            break;
        case 3:
            // Up
            new_head->y = current_head->y - 2*SNAKE_BODY_WIDTH - SNAKE_BODY_SPACING;
            new_head->x = current_head->x;
            break;
        case 0:
        default:
            // Right
            new_head->x = current_head->x + 2*SNAKE_BODY_WIDTH + SNAKE_BODY_SPACING;
            new_head->y = current_head->y;
            break;
    }
	new_head->next = current_head;
	snake->head = new_head;
}

static void move_snake(snake_t *snake)
{
    // Check the bounds
    // If the snake has gone beyond the edge,
    // mkae the head appear on the other side
    snake_section_t *current_head = snake->head;
    if (current_head->x > container_width) {
        current_head->x = -SNAKE_BODY_WIDTH;
        current_head->y = current_head->y;
    } else if (current_head->x < 0) {
        current_head->x = container_width;
        current_head->y = current_head->y;
    }
    
    if (current_head->y > container_height) {
        current_head->y = -SNAKE_BODY_WIDTH;
        current_head->x = current_head->x;
    } else if (current_head->y < 0) {
        current_head->y = container_height;
        current_head->x = current_head->x;
    }
    
    // move snake by adding to the head and removing the last section
    // move the head
    add_to_head(snake);
    snake_section_t *current_section = snake->head;
    snake_section_t *next_section = current_section->next;
    
    // store the new head location to check collision
    unsigned collided = 0;
    int head_x = current_section->x;
    int head_y = current_section->y;
    
    // move the rest of the snake
    while (next_section->next != NULL) {
        current_section = next_section;
        next_section = next_section->next;
        // Check collision
        if (current_section->x == head_x && current_section->y == head_y) {
            collided = 1;
            break;
        }
    }
    // remove last section
    current_section->next = NULL;
    free(next_section);
    
    // Mark game as dead if the snake collided
    if (collided) {
        game->alive = 0;
    }
}

static unsigned snake_has_eaten_apple(snake_t *snake, apple_t *apple) 
{
    snake_section_t *current_head = snake->head;
    return (abs(current_head->x - apple->x) < SNAKE_CONTACT_LENIENCY*APPLE_SIZE && 
            abs(current_head->y - apple->y) < SNAKE_CONTACT_LENIENCY*APPLE_SIZE);
}

static void move_apple()
{
    srand(time(NULL));
    int random = rand();

    apple->x = round_to_nearest_multiple(random % (int)(container_width - 2*SNAKE_BODY_WIDTH), APPLE_SIZE);
    apple->y = round_to_nearest_multiple(random % (int)(container_height - 2*SNAKE_BODY_WIDTH - STATUS_BAR_HEIGHT), APPLE_SIZE);
    
    // Check that the apple has not moved to an area
    // where the snake body currently is
    unsigned is_bad_placement = 0;
    snake_section_t *current_section = snake->head;
    while (current_section) {
        // If the apple is placed near a body section,
        // put up a flag to move it
        if (abs(apple->x - current_section->x) < 2*APPLE_SIZE && abs(apple->y - current_section->y) < 2*APPLE_SIZE ) {
            is_bad_placement = 1;
            break;
        }
        current_section = current_section->next;
    }
    if (is_bad_placement) {
        move_apple();
    }
}

//--------------------------------------------- 
// Game Methods
//---------------------------------------------

static void game_end(unsigned finished)
{
    if (finished) {
        vibes_double_pulse();
        debrief_user_with_score(game->score);
        game->is_resetting = 1;
    }
}

static void game_setup()
{
    if (!game) {
        // Make a NEW game
        game = malloc(sizeof(game_state_t));
        
        // Make a head for a snake
        snake_section_t *head = malloc(sizeof(snake_section_t));
        head->x = SNAKE_BODY_WIDTH;
        head->y = SNAKE_BODY_WIDTH;
        head->next = NULL;
        
        // Make a snake with that head
        snake = malloc(sizeof(snake_t));
        snake->head = head;
        
        // Make an apple
        apple = malloc(sizeof(apple_t));
        
    } else if (game->is_resetting) {
        // Resets the snake
        snake_section_t *current_head = snake->head;
        snake_section_t *current_section = current_head->next;
        snake_section_t *next;
        // free all the snake sections
        while(current_section) {
            next = current_section;
            current_section = current_section->next;
            free(next);
        }
        current_head->x = SNAKE_BODY_WIDTH;
        current_head->y = SNAKE_BODY_WIDTH;
        current_head->next = NULL;
    }
    
    // Resets the game
    game->score = 0;
    game->queued_input = 0;
    game->alive = 1;
    snake->direction = 0;
    game->is_resetting = 0;
    game->bonus_points = 0;
    
    // Make the snake as long as the default size
    for (int i = 1; i < DEFAULT_SNAKE_SIZE; ++i) {
        add_to_head(snake);
    }
    snake->length = DEFAULT_SNAKE_SIZE;

    // Move apple to random location
    move_apple();
}

static void bonus_timer_callback(void *data)
{
    if (game->bonus_points > 0) {
        game->bonus_points -= 1;
        bonus_points_timer = app_timer_register(BONUS_TIMER_INVTERVAL, bonus_timer_callback, NULL);
    }
}

static void game_tick(void *data)
{
    if (game->alive) {
        if (!game->is_resetting) {
             game_timer = app_timer_register(GAME_TICK_INTERVAL, game_tick, NULL);
        }
    } else {
        game_end(1);
    }

    // If game is paused, has the user presed the resume button?
    if (game->is_paused && game->queued_input != 3) {
        return;
    }

    // Check User Input
    if (game->queued_input) {
        switch (game->queued_input) {
            case 1:
                // Up pressed, TURN COUNTERCLOCKWISE
                if (snake->direction > 0) {
                    snake->direction -= 1;
                } else {
                    snake->direction = 3;
                }
                break;
            case 2:
                // Down pressed, TURN CLOCKWISE
                if (snake->direction < 3) {
                    snake->direction  += 1;
                } else {
                    snake->direction = 0;
                }
                break;
            case 3:
                game->is_paused = !game->is_paused;
                break;
            default:
                break;
        }
        game->queued_input = 0; 
    }

    if (game->alive) {
        move_snake(snake);
    }

    // Check if snake ate apple
    if(snake_has_eaten_apple(snake, apple)) {
        add_to_head(snake);
        // Move apple
        move_apple();
        game->score += (1 + game->bonus_points);
        snake->length += 1;
        vibes_short_pulse();

        if (game->bonus_points == 0) {
            bonus_points_timer = app_timer_register(BONUS_TIMER_INVTERVAL, bonus_timer_callback, NULL);
        }

        game->bonus_points = snake->length / 2;
        
    }

    // Update the graphics
    // Note: this could be moved to a separate graphics update interval
    // But since the game tick here is the same as the graphics tick
    // I saw no need to do two timers

    layer_mark_dirty(game_layer);
    
    static char score_text[12];

    // Update score layer
    if (game->is_paused) {
        snprintf(score_text, sizeof(score_text), "Paused");  
    } else {
        snprintf(score_text, sizeof(score_text), "Score: %d ", game->score);   
    }

    text_layer_set_text(game_score_label, score_text);

    static char bonus_text[12];
    snprintf(bonus_text, sizeof(bonus_text), "Bonus: %d", game->bonus_points);
    text_layer_set_text(game_bonus_label, bonus_text);
}

static void game_start()
{
    game_setup();
    game_timer = app_timer_register(GAME_TICK_INTERVAL, game_tick, NULL);
}

//--------------------------------------------- 
// Layer Updating
//---------------------------------------------

static void game_layer_update_proc(Layer *layer, GContext *ctx)
{
    // Draw the apple
    graphics_context_set_stroke_color(ctx, GColorBlack);
    graphics_draw_circle(ctx, GPoint(apple->x,apple->y), APPLE_SIZE);

    // Draw the snake
    snake_section_t *current_section = snake->head;
    while (current_section) {
        graphics_context_set_fill_color(ctx, GColorBlack);
        graphics_fill_circle(ctx, GPoint(current_section->x,current_section->y), SNAKE_BODY_WIDTH);
        current_section = current_section->next;
    }
}

//--------------------------------------------- 
// User Input Handlers
//---------------------------------------------

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
   game->queued_input = 3;
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    game->queued_input = 1;
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    game->queued_input = 2;   
}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
    window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
    window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

//--------------------------------------------- 
// Window Events
//---------------------------------------------

static void window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);
    GRect frame = layer_get_frame(window_layer);
    GRect adjusted_bounds = GRect(0,STATUS_BAR_HEIGHT-BOUNDS_ADJUSTMENT, bounds.size.w, bounds.size.h - STATUS_BAR_HEIGHT + BOUNDS_ADJUSTMENT);

    container_width = frame.size.w;
    container_height = frame.size.h - STATUS_BAR_HEIGHT;
    
    game_layer = layer_create(adjusted_bounds);
    layer_set_update_proc(game_layer, game_layer_update_proc);
    layer_add_child(window_layer, game_layer);

    game_score_label = text_layer_create(GRect(0, -BOUNDS_ADJUSTMENT, bounds.size.w, STATUS_BAR_HEIGHT));
    text_layer_set_background_color(game_score_label, GColorBlack);
    text_layer_set_text_color(game_score_label, GColorWhite);
    text_layer_set_text_alignment(game_score_label, GTextAlignmentRight);
    layer_add_child(window_layer, text_layer_get_layer(game_score_label));
    text_layer_set_font(game_score_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));

    game_bonus_label = text_layer_create(GRect(0, -BOUNDS_ADJUSTMENT, bounds.size.w, STATUS_BAR_HEIGHT));
    text_layer_set_text_color(game_bonus_label, GColorWhite);
    text_layer_set_background_color(game_bonus_label, GColorClear);
    layer_add_child(window_layer, text_layer_get_layer(game_bonus_label));
    text_layer_set_font(game_bonus_label, fonts_get_system_font(FONT_KEY_GOTHIC_14));

}

static void window_appear(Window *window)
{
    game_start();
}

static void window_disappear(Window *window) {
    // If there is a game already, issue a flag to reset the game
    if (game) {
        game->is_resetting = 1;
    }
}

static void window_unload(Window *window) {
    game_end(0);
    text_layer_destroy(game_bonus_label);
    text_layer_destroy(game_score_label);
    layer_destroy(game_layer);
}

//--------------------------------------------- 
// Program Init
//---------------------------------------------

void game_init(void) {
    game_window = window_create();
    window_set_click_config_provider(game_window, click_config_provider);
    window_set_window_handlers(game_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
        .appear = window_appear,
        .disappear = window_disappear
    });
    
    window_set_fullscreen(game_window, true);
    const bool animated = true;
    window_stack_push(game_window, animated);
}

void game_deinit(void) {
    free(game);
    free(snake);
    free(apple);
    window_destroy(game_window);
}
