#include <pebble.h>
#include "debrief.h"

#define min(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })

static Window *debrief_window;

static TextLayer *debrief_title_layer;
static TextLayer *debrief_compliments_layer;
static TextLayer *debrief_score_layer;

const char *compliments_strings[50];
const char *title_strings[10];

//--------------------------------------------- 
// User Input Handlers
//---------------------------------------------

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {

}

static void click_config_provider(void *context) {
    window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
}

//--------------------------------------------- 
// Window Events
//---------------------------------------------

static void window_load(Window *window) {

    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    debrief_title_layer = text_layer_create(GRect(0, 0, bounds.size.w, 24));
    text_layer_set_background_color(debrief_title_layer, GColorBlack);
    text_layer_set_text_color(debrief_title_layer, GColorWhite);
    text_layer_set_text_alignment(debrief_title_layer, GTextAlignmentCenter);
    text_layer_set_font(debrief_title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(debrief_title_layer));

    debrief_score_layer = text_layer_create(GRect(0, 24, bounds.size.w, 56));
    text_layer_set_background_color(debrief_score_layer, GColorBlack);
    text_layer_set_text_color(debrief_score_layer, GColorWhite);
    text_layer_set_text_alignment(debrief_score_layer, GTextAlignmentCenter);
    text_layer_set_font(debrief_score_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_LIGHT));
    layer_add_child(window_layer, text_layer_get_layer(debrief_score_layer));

    debrief_compliments_layer = text_layer_create(GRect(0, 72, bounds.size.w, 100));
    text_layer_set_background_color(debrief_compliments_layer, GColorBlack);
    text_layer_set_text_color(debrief_compliments_layer, GColorWhite);
    text_layer_set_text_alignment(debrief_compliments_layer, GTextAlignmentCenter);
    text_layer_set_font(debrief_compliments_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    layer_add_child(window_layer, text_layer_get_layer(debrief_compliments_layer));
}

static void window_unload(Window *window) {
    text_layer_destroy(debrief_title_layer);
    text_layer_destroy(debrief_compliments_layer);
    text_layer_destroy(debrief_score_layer);
}

void debrief_user_with_score(unsigned score)
{
    const bool animated = true;
    window_stack_push(debrief_window, animated);

    static char score_text[8];
    snprintf(score_text, sizeof(score_text), "%d", score);
    text_layer_set_text(debrief_score_layer, score_text);

    unsigned adjusted_score = min((unsigned)189, score);
    unsigned title_index = (unsigned)(adjusted_score / 21);

    text_layer_set_text(debrief_title_layer, title_strings[title_index]);

    unsigned compliments_index = title_index*5 + (rand() % 5);
    text_layer_set_text(debrief_compliments_layer, compliments_strings[compliments_index]);

}

//--------------------------------------------- 
// Program Init
//---------------------------------------------

static void load_compliments()
{
    title_strings[0] = "REALLY?!";
    title_strings[1] = "MEH...";
    title_strings[2] = "COME ON!";
    title_strings[3] = "NOT BAD";
    title_strings[4] = "DECENT";
    title_strings[5] = "COMMENDABLE";
    title_strings[6] = "WATCH OUT!";
    title_strings[7] = "HIGH-FIVE!";
    title_strings[8] = "INCREDIBLE!";
    title_strings[9] = "WOWZA!";

    compliments_strings[0] = "HOW DID YOU MANAGE THAT?";
    compliments_strings[1] = "IT'S IMPOSSIBLE TO BE THIS BAD...";
    compliments_strings[2] = "REMEMBER: HIGHER SCORE IS BETTER";
    compliments_strings[3] = "WERE YOU RAISED IN A BARN?";
    compliments_strings[4] = "I'VE GOT A BAD FEELING ABOUT THIS";
    compliments_strings[5] = "NEED MORE PRACTICE, FO SHO";
    compliments_strings[6] = "GOTTA BRUSH UP ON YOUR SKILLS";
    compliments_strings[7] = "WHERE'D YOU LEARN TO PLAY? A TOUCHSCREEN?";
    compliments_strings[8] = "YOU DISAPPOINT ME";
    compliments_strings[9] = "WHAT'S THE DEAL?";
    compliments_strings[10] = "INDIANA JONES WAS BETTER WITH SNAKES THAN YOU";
    compliments_strings[11] = "DO I HAVE TO PRESS THE BUTTONS MYSELF?";
    compliments_strings[12] = "YOU USED TO BELIEVE. WHAT HAPPENED?";
    compliments_strings[13] = "IT'S LIKE YOU'RE NOT EVEN TRYING";
    compliments_strings[14] = "THERE IS NO EXCUSE FOR BEING THIS BAD";
    compliments_strings[15] = "KEEP PRACTICING, AND YOUR DAY JOB";
    compliments_strings[16] = "SOON, YOU'LL BE READY FOR THE BIG LEAGUES";
    compliments_strings[17] = "YOU CAN ALMOST PASS FOR A GAMER, ALMOST";
    compliments_strings[18] = "ANOTHER! ANOTHER!";
    compliments_strings[19] = "YOU CAN YOU BETTER THAN THAT!";
    compliments_strings[20] = "LOOKS LIKE YOU'RE GETTING THE HANG OF THINGS";
    compliments_strings[21] = "I'M SURPRISED YOU MADE IT THIS FAR";
    compliments_strings[22] = "ARE YOU JUST PLAYING TO READ THIS?";
    compliments_strings[23] = "DON'T GIVE UP ON ME NOW!";
    compliments_strings[24] = "DON'T LET THE MUGGLES GET YOU DOWN";
    compliments_strings[25] = "YOU CANNOT EXPECT VICTORY PLAYING LIKE THAT";
    compliments_strings[26] = "THANKFULLY, PERSISTENCE IS A GREAT SUBSTITUTE FOR TALENT";
    compliments_strings[27] = "ONE MORE GAME...YOLO";
    compliments_strings[28] = "EVERY FAILURE BRINGS YOU CLOSER TO SUCCESS";
    compliments_strings[29] = "BELIEVE YOU CAN AND YOU'RE HALFWAY THERE";
    compliments_strings[30] = "YOU MAKE ME SMILE ;)";
    compliments_strings[30] = "MY MY, AREN'T YOU A (SNAKE) CHARMER?";
    compliments_strings[31] = "WHERE DID YOU LEARN TO PLAY LIKE THAT?";
    compliments_strings[32] = "THERE ARE NO TRAFFIC JAMS ALONG THE EXTRA MILE";
    compliments_strings[33] = "NICE ONE, SPORT!";
    compliments_strings[34] = "GOOD BUT NOT GREAT";
    compliments_strings[35] = "LOOK OUT FOLKS, THIS KID'S GOT TALENT!";
    compliments_strings[36] = "HOT STUFF COMING THROUGH!";
    compliments_strings[37] = "THERE'S A FUTURE FOR YOU!";
    compliments_strings[38] = "HOW LONG YOU BEEN PLAYING?";
    compliments_strings[39] = "TRY NOT. DO, OR DO NOT. THERE IS NO TRY";
    compliments_strings[40] = "I GUESS YOU DIDN'T NEED LUCK";
    compliments_strings[40] = "HOLY SMOKES!";
    compliments_strings[41] = "TAKE A PICTURE! QUICK!";
    compliments_strings[42] = "BETTER CALL YOUR MOM AND SAY 'I TOLD YOU SO'";
    compliments_strings[43] = "DO YOU DO THIS ALL DAY?!";
    compliments_strings[44] = "JUST HOW ARE YOU THIS GOOD?!";
    compliments_strings[45] = "YOU HAVE ARRIVED";
    compliments_strings[46] = "YOU NEED NOT PLAY ANY MORE";
    compliments_strings[47] = "YOU'VE UNLOCKED THE TIME MACHINE. WHEN TO?";
    compliments_strings[48] = "THE BEST REVENGE IS MASSIVE SUCCESS";
    compliments_strings[49] = "FANTASTIC!";

}

void debrief_init(void) {
    debrief_window = window_create();
    window_set_click_config_provider(debrief_window, click_config_provider);
    window_set_window_handlers(debrief_window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload
    });
    
    window_set_fullscreen(debrief_window, false);

    load_compliments();
}

void debrief_deinit(void) {
    window_destroy(debrief_window);
}
