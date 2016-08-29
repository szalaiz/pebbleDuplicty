// Duplicity by szala - 2014
// based on Simplicity watchface design
//
// configurable home time added to the screen

#include <pebble.h>

#define OFFSET_HOUR 15
#define OFFSET_MINS 25

Window *window;
TextLayer *localTime_layer, *homeText_layer, *homeTime_layer, *date_layer;
BitmapLayer *line_layer, *topLine_layer;

//Test layer
//TextLayer *testText_layer;

enum TimeDifference {
  TIME_DIFF = 0x0
};

int offset_hour, offset_mins;
int offset_time;


static void in_recv_handler(DictionaryIterator *iterator, void *context) {
	//Get Tuple
	Tuple *t = dict_read_first(iterator);

	if(t) {
		switch (t->key) {
			case TIME_DIFF:
			{
				//Convert time difference to hours/minutes
				offset_time = atoi(t->value->cstring);
				offset_mins = offset_time % 60;
				offset_hour = (offset_time - offset_mins) / 60;

				//Save time difference into persistent memory
				persist_write_int(OFFSET_HOUR, offset_hour);
				persist_write_int(OFFSET_MINS, offset_mins);

				break;
			}
		}
	}
}


char localTime_text[] = "00:00", homeTime_text[] = "00:00", day_text[] = "Xxxxxxxxxxxx", date_text[] = "Xxxxxxxxxxxxxxxxxxxxxxx 00";

//Hard coded time difference
//static const int offset_hour = +3, offset_mins = +30;

void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
	//Test layer
	//text_layer_set_text(testText_layer, "test");

	//Get time difference from persistent memory
	if (persist_exists(OFFSET_HOUR)) { offset_hour = persist_read_int(OFFSET_HOUR);	}
	if (persist_exists(OFFSET_MINS)) { offset_mins = persist_read_int(OFFSET_MINS); }

	//Format date
	strftime(date_text, sizeof(date_text), "%B %e", tick_time);

	//Show date
	text_layer_set_text(date_layer, date_text);

	//Format the buffer string using tick_time as the time source
	strftime(localTime_text, sizeof(localTime_text), "%H:%M", tick_time);

	//Show local time
	text_layer_set_text(localTime_layer, localTime_text);

	//Get a time structure for home time calculations
	time_t t = time(NULL);
	struct tm *now = localtime(&t);

	//Calculate home time minutes
	now->tm_min -= offset_mins;
	if (now->tm_min > 60) {
		now->tm_hour++;
		now->tm_min -= 60;
	} else {
		if (now->tm_min < 0) {
			now->tm_hour--;
			now->tm_min += 60;
		}
	}

	//Calculate home time hours
	now->tm_hour -= offset_hour;
	if (now->tm_hour >= 24)
		now->tm_hour -= 24;
	if (now->tm_hour < 0)
		now->tm_hour += 24;

	//Displays home time or weekday name if it is same as local time
	if ((offset_hour == 0) && (offset_mins == 0)) {
		strftime(day_text, sizeof(day_text), "%A", now);
		text_layer_set_font(homeTime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
		text_layer_set_text(homeTime_layer, day_text);
		text_layer_set_text(homeText_layer, "");
		layer_remove_from_parent(bitmap_layer_get_layer(topLine_layer));
	} else {
		strftime(homeTime_text, sizeof(homeTime_text), "%H:%M", now);
		text_layer_set_font(homeTime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
		text_layer_set_text(homeTime_layer, homeTime_text);
		text_layer_set_text(homeText_layer, "Home");
		layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(topLine_layer));
	}
}

void window_load(Window *window) {
	//Main window
	window_set_background_color(window, GColorBlack);

	//Date layer
	date_layer = text_layer_create(GRect(8, 68, 144 - 8, 168 - 68));
	text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
	text_layer_set_background_color(date_layer, GColorClear);
	text_layer_set_text_color(date_layer, GColorWhite);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(date_layer));

	//Line layer
	line_layer = bitmap_layer_create(GRect(8, 97, 124, 2));
	bitmap_layer_set_background_color(line_layer, GColorWhite);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(line_layer));

	//Local time layer
	localTime_layer = text_layer_create(GRect(7, 92, 144 - 7, 168 - 92));
	text_layer_set_font(localTime_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	text_layer_set_background_color(localTime_layer, GColorClear);
	text_layer_set_text_color(localTime_layer, GColorWhite);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(localTime_layer));

	//Home time layer
	homeTime_layer = text_layer_create(GRect(0, 0, 144, 36));
	text_layer_set_font(homeTime_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
	text_layer_set_text_alignment(homeTime_layer, GTextAlignmentRight);
	text_layer_set_background_color(homeTime_layer, GColorClear);
	text_layer_set_text_color(homeTime_layer, GColorWhite);
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(homeTime_layer));

	//Top line layer
	topLine_layer = bitmap_layer_create(GRect(95, 31, 144 - 95, 1));
	bitmap_layer_set_background_color(topLine_layer, GColorWhite);
	layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(topLine_layer));

	//Home text layer
	homeText_layer = text_layer_create(GRect(0, 28, 144, 20));
	text_layer_set_font(homeText_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(homeText_layer, GTextAlignmentRight);
	text_layer_set_background_color(homeText_layer, GColorClear);
	text_layer_set_text_color(homeText_layer, GColorWhite);
	text_layer_set_text(homeText_layer, "Home");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(homeText_layer));

	//Test layer
	//testText_layer = text_layer_create(GRect(0, 0, 72, 20));
	//text_layer_set_font(testText_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	//text_layer_set_text_alignment(testText_layer, GTextAlignmentLeft);
	//text_layer_set_background_color(testText_layer, GColorClear);
	//text_layer_set_text_color(testText_layer, GColorWhite);
	//text_layer_set_text(testText_layer, "");
	//layer_add_child(window_get_root_layer(window), text_layer_get_layer(testText_layer));

	//Get a time structure so that the face doesn't start blank
	time_t temp = time(NULL);
	struct tm *t = localtime(&temp);

	//Manually call the tick handler when the window is loading
	tick_handler(t, MINUTE_UNIT);
}
 
void window_unload(Window *window) {
	//We will safely destroy the window's elements here
	text_layer_destroy(date_layer);
	bitmap_layer_destroy(line_layer);
	text_layer_destroy(localTime_layer);
	text_layer_destroy(homeTime_layer);
	bitmap_layer_destroy(topLine_layer);
	text_layer_destroy(homeText_layer);

	//Test layer
	//text_layer_destroy(testText_layer);
}
 
void init() {
	//Initialize the app elements here
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers) {
		.load = window_load,
		.unload = window_unload,
	});

    app_message_register_inbox_received((AppMessageInboxReceived) in_recv_handler);
    app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

//	tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
	tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);

	window_stack_push(window, true);
}
 
void deinit() {
	//De-initialize elements here to save memory
	tick_timer_service_unsubscribe();

	window_destroy(window);
}
 
int main(void) {
	init();
	app_event_loop();
	deinit();
}