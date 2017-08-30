// Author: Sean Pesce

#pragma once


#ifndef _SP_D3D9O_CONSOLE_H_
	#define _SP_D3D9O_CONSOLE_H_

#include "stdafx.h"
#include "SpD3D9.h"

#include "D3DFont.h"
#include "seqan/index.h" // String, StringSet, Index, and Finder classes


#ifndef hotkey_is_down
	#define hotkey_is_down(hotkey) (hotkey != 0 && (key_state[hotkey] & _SP_KEY_DOWN_))
#endif

#define _CLOSE_CONSOLE_KEY_ VK_ESCAPE // Escape key

#define _SP_D3D9O_C_DEFAULT_OUTPUT_LOG_CAPACITY_ 100
#define _SP_D3D9O_C_DEFAULT_COMMAND_LOG_CAPACITY_ 20
#define _SP_D3D9O_C_DEFAULT_ECHO_VALUE_ true
#define _SP_D3D9O_C_DEFAULT_OUTPUT_STREAM_VALUE_ true
#define _SP_D3D9O_C_DEFAULT_PROMPT_ ">"
#define _SP_D3D9O_C_DEFAULT_CARET_ '_'
#define _SP_D3D9O_C_DEFAULT_BLINK_DELAY_ 500
#define _SP_D3D9O_C_DEFAULT_FONT_HEIGHT_ _SP_D3D9O_TF_DEFAULT_FONT_HEIGHT_
#define _SP_D3D9O_C_DEFAULT_FONT_FAMILY_ "Courier New"
#define _SP_D3D9O_C_DEFAULT_FONT_FLAGS_ 0	// D3DFONT_BOLD, etc
#define _SP_D3D9O_C_DEFAULT_FONT_COLOR_ D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);	// White
#define _SP_D3D9O_C_DEFAULT_HIGHLIGHT_FONT_COLOR_ D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f); // Black
#define _SP_D3D9O_C_DEFAULT_HIGHLIGHT_BACKGROUND_COLOR_ D3DXCOLOR(0xFF508CED); // Blue (#508CED)
#define _SP_D3D9O_C_DEFAULT_BACKGROUND_COLOR_ D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f); // Black
#define _SP_D3D9O_C_DEFAULT_BORDER_COLOR_ D3DXCOLOR(0.5f, 0.5f, 0.5f, 0.5f); // Gray
#define _SP_D3D9O_C_DEFAULT_BORDER_WIDTH_ 3
#define _SP_D3D9O_C_DEFAULT_AUTOCOMP_BACKGROUND_COLOR_ D3DXCOLOR(0.0f, 0.0f, 0.0f, 0.5f); // Black
#define _SP_D3D9O_C_DEFAULT_AUTOCOMP_BACKGROUND_HOVER_COLOR_ D3DXCOLOR(0xFF1C1C1C); // Very dark gray
#define _SP_D3D9O_C_DEFAULT_AUTOCOMP_BACKGROUND_SELECT_COLOR_ D3DXCOLOR(0xFF333333); // Dark gray
#define _SP_D3D9O_C_DEFAULT_AUTOCOMP_BORDER_COLOR_ D3DXCOLOR(0.5f, 0.5f, 0.5f, 0.5f); // Gray
#define _SP_D3D9O_C_DEFAULT_AUTOCOMP_BORDER_WIDTH_ 1
#define _SP_D3D9O_C_DEFAULT_OUTPUT_LINES_ 15
#define _SP_D3D9O_C_DEFAULT_AUTOCOMPLETE_LIMIT_ 5
#define _SP_D3D9O_C_DEFAULT_PROMPT_ELEMENTS_ (SP_D3D9O_PROMPT_ELEMENTS_DISABLED)
#define _SP_D3D9O_C_DEFAULT_CURSOR_SHOW_ true
#define _SP_D3D9O_C_DEFAULT_CURSOR_SIZE_ 16
#define _SP_D3D9O_C_DEFAULT_CURSOR_COLOR_ D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);	// White
#define _SP_D3D9O_C_DEFAULT_CURSOR_FONT_FAMILY_ "Consolas"
#define _SP_D3D9O_C_DEFAULT_OLD_OS_CURSOR_FONT_FAMILY_ "Courier New"

#define _SP_D3D9O_C_INVALID_CONSOLE_COMMAND_CHARS_ " \t\n\r"

#define _SP_D3D9O_C_MAX_FONT_SIZE_ 190
#define _SP_D3D9O_C_MIN_FONT_SIZE_ 5

#define CONSOLE_COMMAND_SUCCESS ERROR_SUCCESS  // Value that a console command should return if no errors occurred
#define CONSOLE_COMMAND_NOT_FOUND_ERROR (-1)
#define _SP_D3D9O_C_ERROR_UNKNOWN_COMMAND_ "ERROR: Unrecognized command"

// Denotes whether to display each element of the input prompt
enum SP_D3D9O_CONSOLE_PROMPT_ENUM {
	SP_D3D9O_PROMPT_ELEMENTS_DISABLED = 0,
	SP_D3D9O_PROMPT_USER = 1, // User profile
	SP_D3D9O_PROMPT_HOSTNAME = 2, // Computer name
	SP_D3D9O_PROMPT_CWD = 4 // Current working directory
};


// Used to determine which part of the console is in focus for selecting text
#define SP_D3D9O_C_NO_SELECTION_INDEX -1
#define SP_D3D9O_C_NO_SELECTION_LINE -2
#define SP_D3D9O_C_INPUT_LINE -1
enum SP_D3D9O_CONSOLE_SELECT_FOCUS_ENUM {
	SP_D3D9O_SELECT_NONE = 0,
	SP_D3D9O_SELECT_TEXT = 1,
	//SP_D3D9O_SELECT_INPUT = 1,
	//SP_D3D9O_SELECT_OUTPUT = 2,
	SP_D3D9O_SELECT_AUTOCOMPLETE = 3
};


typedef struct SP_D3D9O_CONSOLE_TEXT_SELECTION {
	SP_D3D9O_CONSOLE_SELECT_FOCUS_ENUM focus = SP_D3D9O_SELECT_NONE;
	int line1 = SP_D3D9O_C_NO_SELECTION_LINE;	// Line number of the first selected character (if selecting text)
	int i1 = SP_D3D9O_C_NO_SELECTION_INDEX;		// Index in the line of the first selected character
	int line2 = SP_D3D9O_C_NO_SELECTION_LINE;		// Line number of the last selected character
	int i2 = SP_D3D9O_C_NO_SELECTION_INDEX;		// Index in the line of the last selected character
	int *start_line = &line1;
	int *start_index = &i1;
	int *end_line = &line2;
	int *end_index = &i2;
	int autocomplete_selection = 0; // Index of the selected autocomplete suggestion
} CONSOLE_TEXT_SELECTION;


typedef struct SP_D3D9O_CONSOLE_COMMAND {
	std::string command = "";
	int(*function)(std::vector<std::string>, std::string *) = NULL;
	std::string help_message = "";
	int id = -1; // ID of command in commands_set
	std::string alias_for = ""; // If not an empty string, the command is an alias or macro
	std::vector<std::string> macro_args; // If alias_for is non-empty and macro_args contains args, command is a macro. If alias_for is empty, macro_args is ignored
} SP_D3D9O_CONSOLE_COMMAND;


class SpD3D9OConsole
{
public:
	SpD3D9Overlay *overlay = NULL; // D3D9 overlay that this console belongs to
	
	bool echo = _SP_D3D9O_C_DEFAULT_ECHO_VALUE_;
	bool output_stream = _SP_D3D9O_C_DEFAULT_OUTPUT_STREAM_VALUE_; // If disabled, printing to console does nothing
	std::string prompt = _SP_D3D9O_C_DEFAULT_PROMPT_;
	int prompt_elements = _SP_D3D9O_C_DEFAULT_PROMPT_ELEMENTS_;
	char caret = _SP_D3D9O_C_DEFAULT_CARET_;
	int caret_blink_delay = _SP_D3D9O_C_DEFAULT_BLINK_DELAY_;  // Speed at which the cursor blinks, in milliseconds
	std::string command = ""; // Current command being typed

	bool show_cursor = _SP_D3D9O_C_DEFAULT_CURSOR_SHOW_;
	// Text cursor data
	std::string cursor_font_family = _SP_D3D9O_C_DEFAULT_CURSOR_FONT_FAMILY_;
	CD3DFont *cursor = NULL;
	int cursor_size = _SP_D3D9O_C_DEFAULT_CURSOR_SIZE_;
	D3DXCOLOR cursor_color = _SP_D3D9O_C_DEFAULT_CURSOR_COLOR_;
	// Windows cursor data
	LPDIRECT3DTEXTURE9 win_cursor_tex = NULL; // Windows cursor texture
	LPD3DXSPRITE win_cursor_sprite = NULL; // Windows cursor sprite

	CD3DFont *font = NULL;
	int font_height = _SP_D3D9O_C_DEFAULT_FONT_HEIGHT_;
	D3DXCOLOR font_color = _SP_D3D9O_C_DEFAULT_FONT_COLOR_;
	D3DXCOLOR font_highlight_color = _SP_D3D9O_C_DEFAULT_HIGHLIGHT_FONT_COLOR_;
	D3DXCOLOR background_highlight_color = _SP_D3D9O_C_DEFAULT_HIGHLIGHT_BACKGROUND_COLOR_;
	std::string font_family = _SP_D3D9O_C_DEFAULT_FONT_FAMILY_;
	D3DXCOLOR background_color = _SP_D3D9O_C_DEFAULT_BACKGROUND_COLOR_;
	D3DXCOLOR border_color = _SP_D3D9O_C_DEFAULT_BORDER_COLOR_;
	unsigned int border_width = _SP_D3D9O_C_DEFAULT_BORDER_WIDTH_;
	D3DXCOLOR autocomplete_background_color = _SP_D3D9O_C_DEFAULT_AUTOCOMP_BACKGROUND_COLOR_;
	D3DXCOLOR autocomplete_background_hover_color = _SP_D3D9O_C_DEFAULT_AUTOCOMP_BACKGROUND_HOVER_COLOR_;
	D3DXCOLOR autocomplete_background_select_color = _SP_D3D9O_C_DEFAULT_AUTOCOMP_BACKGROUND_SELECT_COLOR_;
	D3DXCOLOR autocomplete_border_color = _SP_D3D9O_C_DEFAULT_BORDER_COLOR_;
	unsigned int autocomplete_border_width = _SP_D3D9O_C_DEFAULT_AUTOCOMP_BORDER_WIDTH_;

	std::vector<std::string> command_log; // Log of console commands previously entered
	std::vector<std::string> output_log; // Log of console commands previously entered and their resulting outputs
	unsigned int output_log_displayed_lines = _SP_D3D9O_C_DEFAULT_OUTPUT_LINES_; // Number of lines of previous output to display
	unsigned int output_log_capacity = _SP_D3D9O_C_DEFAULT_OUTPUT_LOG_CAPACITY_; // Number of lines of output to keep in memory (oldest are deleted when max is hit)
	unsigned int command_log_capacity = _SP_D3D9O_C_DEFAULT_COMMAND_LOG_CAPACITY_; // Number of console commands to keep logged (oldest are deleted when max is hit)

	unsigned int caret_position = 0; // Position of cursor in current command
	
	unsigned int input_display_start = 0; // If current command is longer than the screen, these 2 indexes are the start/end of the displayed substring
	unsigned int input_display_end = 0;

	CONSOLE_TEXT_SELECTION selection;
	SP_D3D9O_CONSOLE_SELECT_FOCUS_ENUM selection_focus = SP_D3D9O_SELECT_NONE; // Determines whether input or output is selected (or neither)
	int selection_start_index = -1; // Starting index on starting line of selection
	int selection_start_line = -1; // Starting line of selected chars
	int selection_width = 0; // If single line, the number of chars to select. If multi-line, the index of the last selected char on the lower line
	int selection_vertical_width = 0; // Number of lines that the selection spans

	unsigned int command_log_position = 0; // Used to obtain previous commands with the up/down keys
	unsigned int autocomplete_limit = _SP_D3D9O_C_DEFAULT_AUTOCOMPLETE_LIMIT_; // Maximum number of autocomplete suggestions to show

	// Constructor/destructor
	SpD3D9OConsole(SpD3D9Overlay *new_overlay);
	~SpD3D9OConsole();

	void SpD3D9OConsole::get_input(); // Called from main overlay input loop
	bool SpD3D9OConsole::toggle();
	bool SpD3D9OConsole::is_open();
	void SpD3D9OConsole::draw();
	void SpD3D9OConsole::add_prompt_elements(std::string *full_prompt, int *max_chars = NULL); // Adds extra prompt elements, if enabled (username, hostname, working directory, etc)
	void SpD3D9OConsole::print(const char *new_message); // Prints text to output log
	int SpD3D9OConsole::execute_command(const char *new_command, int *return_code = NULL, std::string *output = NULL);
	void SpD3D9OConsole::clear(); // Clears console by pushing blank messages to output
	DWORD SpD3D9OConsole::copy(std::string *str); // Copies string to clipboard
	DWORD SpD3D9OConsole::paste(); // Paste clipboard data into console input
	HRESULT SpD3D9OConsole::init_win_cursor();
	#ifdef _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
		void SpD3D9OConsole::handle_key_event(DIDEVICEOBJECTDATA *event);
	#else // !_SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
		void SpD3D9OConsole::handle_key_press(WPARAM wParam);
		void SpD3D9OConsole::handle_mouse_input(RAWMOUSE *mouse_input);
	#endif // _SP_USE_DINPUT8_CREATE_DEVICE_INPUT_
	static int register_command(const char *command, int(*function)(std::vector<std::string>, std::string *), const char *help_message, const char *alias_for = "", std::vector<std::string> macro_args = {});
	static int register_alias(const char *new_alias, const char *existing_command, std::vector<std::string> macro_args = {});
	static void get_autocomplete_options(const char *str, unsigned int suggestion_count, std::vector<std::string> *matches, int *longest = NULL);

	static std::vector<SP_D3D9O_CONSOLE_COMMAND> commands;		// Set of available console commands and corresponding functions
	static seqan::StringSet<seqan::String<char>> commands_set;	// Set of available console command strings
	static seqan::Index<seqan::StringSet<seqan::String<char>>> *commands_index;
	static int SpD3D9OConsole::get_console_command_index(const char *command); // Obtains the position (index) of a command, (Note: not the ID)

private:
	bool show_caret = false;
	DWORD next_caret_blink = 0; // Time of next caret toggle

	void SpD3D9OConsole::update_fonts_and_cursor(); // Update text to new font family/size/flags/etc
	void SpD3D9OConsole::set_input_string_display_limits(unsigned int max_chars);

	void SpD3D9OConsole::format_output_line(std::string *str, int line, int max_chars);
	void SpD3D9OConsole::clear_selection();
	int SpD3D9OConsole::get_screenspace_values(RECT *window = NULL, SIZE *char_size = NULL, RECT *console_lims = NULL,
														long *max_chars = NULL, long *row = NULL, long *column = NULL,
														std::string *full_prompt = NULL, long *max_input_chars = NULL,
														std::vector<std::string> *autocomplete_opts = NULL,
														int *longest_autocomplete = NULL, RECT *autocomplete_lims = NULL,
														int *autocomplete_hover = NULL, int return_after_obtaining = -1);
	void SpD3D9OConsole::cursor_pos_to_selection(long row, long column, long max_chars, SP_D3D9O_CONSOLE_SELECT_FOCUS_ENUM *focus, int *line, int *index, int *line2 = NULL, int *index2 = NULL);
	void SpD3D9OConsole::start_selection();
	void SpD3D9OConsole::continue_autocomplete_selection();
	void SpD3D9OConsole::continue_text_selection();
	void SpD3D9OConsole::get_input_selection(int *start, int *end);
	void SpD3D9OConsole::draw_highlighted_text(CONSOLE_TEXT_SELECTION p_selection, std::string *input_line);
	void SpD3D9OConsole::build_highlighted_text(CONSOLE_TEXT_SELECTION p_selection, std::string *highlighted_str);
};


int open_console(); // Opens console

void to_lower(char *string); // Converts a C string to lowercase
void trim(std::string *string, const char *new_mask = " \r\n\t"); // Trims whitespace from ends of string
char parse_args(const char *args_c_str, std::vector<std::string> *args, std::string *output_file); // Breaks an argument string into argument tokens
bool resolve_arg(const char *args_c_str, int *index, std::string *arg);
char check_args_output_redirect(std::vector<std::string> *args, std::string *output_file);

#endif // _SP_D3D9O_CONSOLE_H_