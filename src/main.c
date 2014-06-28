#include <pebble.h>

GBitmap *ball_bitmap;
BitmapLayer *ball_layer;


Window* window;
TextLayer *text_layer;
TextLayer *text_layer_goal;
InverterLayer *inv_layer;
char buffer[] = "00:00";



void on_animation_stopped(Animation *anim, bool finished, void *context)
{
    //Free the memory used by the Animation
    property_animation_destroy((PropertyAnimation*) anim);
}
 
void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay)
{
    //Declare animation
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);
 
    //Set characteristics
    animation_set_duration((Animation*) anim, duration);
    animation_set_delay((Animation*) anim, delay);
 
    //Set stopped handler to free memory
    AnimationHandlers handlers = {
        //The reference to the stopped handler is the only one in the array
        .stopped = (AnimationStoppedHandler) on_animation_stopped
    };
    animation_set_handlers((Animation*) anim, handlers, NULL);
 
    //Start animation!
    animation_schedule((Animation*) anim);
}

void accel_tap_handler(AccelAxisType axis, int32_t direction) {
  // Process tap on ACCEL_AXIS_X, ACCEL_AXIS_Y or ACCEL_AXIS_Z
  // Direction is 1 or -1
  GRect ball_start = GRect(0, 25, 24, 24);
  GRect ball_finish = GRect(144, 25, 24, 24);
  animate_layer(bitmap_layer_get_layer(ball_layer), &ball_start, &ball_finish, 750, 500);
  psleep(650);
  GRect ball_start_end = GRect(0, -25, 24, 24);
  GRect ball_finish_end = GRect(0, 25, 24, 24);
  animate_layer(bitmap_layer_get_layer(ball_layer), &ball_start_end, &ball_finish_end, 750, 500);
}


void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    //Format the buffer string using tick_time as the time source
    if(clock_is_24h_style()){
      strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
    } else {
      strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
    }
 
    int seconds = tick_time->tm_sec;
 
    if(seconds == 59)
    {
        BatteryChargeState charge_state = battery_state_service_peek();
        if (charge_state.charge_percent < 5){
          vibes_long_pulse();
        }
        //Slide offscreen to the right
        GRect text_start = GRect(0, 53, 144, 168);
        GRect text_finish = GRect(144, 53, 144, 168);
        GRect ball_start = GRect(0, 25, 24, 24);
        GRect ball_finish = GRect(144, 25, 24, 24);
        animate_layer(text_layer_get_layer(text_layer), &text_start, &text_finish, 300, 500);
        animate_layer(bitmap_layer_get_layer(ball_layer), &ball_start, &ball_finish, 750, 500);
    }
 
    else if(seconds == 0)
    {
        //Change the TextLayer text to show the new time!
        text_layer_set_text(text_layer, buffer);
        text_layer_set_text(text_layer,"GOAL");
        //Slide onscreen from the left
        GRect start = GRect(-144, 53, 144, 168);
        GRect finish = GRect(0, 53, 144, 168);
        GRect ball_start = GRect(0, -25, 24, 24);
        GRect ball_finish = GRect(0, 25, 24, 24);
        animate_layer(text_layer_get_layer(text_layer), &start, &finish, 300, 500);
        animate_layer(bitmap_layer_get_layer(ball_layer), &ball_start, &ball_finish, 750, 500);
    }
 
    else
    {
        //Change the TextLayer text to show the new time!
        text_layer_set_text(text_layer, buffer);
    }
}
void window_load(Window *window)
{
  //Load bitmaps into GBitmap structures
  ball_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BALL);
  ball_layer = bitmap_layer_create(GRect(0, 25, 24, 24));
  bitmap_layer_set_bitmap(ball_layer, ball_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(ball_layer));
  //fonts
  ResHandle font_main = resource_get_handle(RESOURCE_ID_MAIN_45);
  ResHandle font_goal = resource_get_handle(RESOURCE_ID_GOAL17);


  

	//Time layer
	text_layer = text_layer_create(GRect(0, 53, 132, 168));
	text_layer_set_background_color(text_layer, GColorClear);
	text_layer_set_text_color(text_layer, GColorBlack);
	text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  text_layer_set_font(text_layer, fonts_load_custom_font(font_main));
	
	layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
  //Goal layer
  text_layer_goal = text_layer_create(GRect(5,113,132,168));
  text_layer_set_background_color(text_layer_goal, GColorClear);
  text_layer_set_text_color(text_layer_goal, GColorBlack);
	text_layer_set_text_alignment(text_layer_goal, GTextAlignmentCenter);
  text_layer_set_font(text_layer_goal, fonts_load_custom_font(font_goal));

  layer_add_child(window_get_root_layer(window), (Layer*) text_layer_goal);
  text_layer_set_text(text_layer_goal,"i belive that we are going to win");

	//Inverter layer
	inv_layer = inverter_layer_create(GRect(0, 50, 144, 62));
	layer_add_child(window_get_root_layer(window), (Layer*) inv_layer);
	
	//Get a time structure so that the face doesn't start blank
	struct tm *t;
	time_t temp;	
	temp = time(NULL);	
	t = localtime(&temp);	
	//Manually call the tick handler when the window is loading
	tick_handler(t, MINUTE_UNIT);
}
 
void window_unload(Window *window)
{
	//We will safely destroy the Window's elements here!
	text_layer_destroy(text_layer);
  text_layer_destroy(text_layer_goal);
	
	inverter_layer_destroy(inv_layer);
}
 
void init(){
	//Initialize the app elements here!
  accel_tap_service_subscribe(&accel_tap_handler);

	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});
	
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);

	
	window_stack_push(window, true);
}
 
void deinit()
{
  accel_tap_service_unsubscribe();

	//De-initialize elements here to save memory!
	tick_timer_service_unsubscribe();
	window_destroy(window);
  //Destroy GBitmaps
  gbitmap_destroy(ball_bitmap);
  //Destroy BitmapLayers
  bitmap_layer_destroy(ball_layer);
}
 
int main(void)
{
	init();
	app_event_loop();
	deinit();
}