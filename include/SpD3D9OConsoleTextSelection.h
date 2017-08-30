// Author: Sean Pesce

#pragma once


#ifndef _SP_D3D9O_CONSOLE_TEXT_SELECTION_H_
	#define _SP_D3D9O_CONSOLE_TEXT_SELECTION_H_

#include "SpD3D9OConsole.h"



// Clears text selection (if some text was selected)
void SpD3D9OConsole::clear_selection()
{
	// Reset text selection structure
	selection.focus = SP_D3D9O_SELECT_NONE;
	selection.line1 = SP_D3D9O_C_NO_SELECTION_LINE;		// Line number of the character that was first selected
	selection.i1 = SP_D3D9O_C_NO_SELECTION_INDEX;		// Index in the line of the character that was first selected
	selection.line2 = SP_D3D9O_C_NO_SELECTION_LINE;		// Line number of the character that was last selected
	selection.i2 = SP_D3D9O_C_NO_SELECTION_INDEX;		// Index in the line of the character that was last selected
	selection.start_line = &selection.line1;
	selection.start_index = &selection.i1;
	selection.end_line = &selection.line2;
	selection.end_index = &selection.i2;
	selection.autocomplete_selection = 0;
}


// Determines the start/end points of selected text
void SpD3D9OConsole::cursor_pos_to_selection(long row, long column, long max_chars, SP_D3D9O_CONSOLE_SELECT_FOCUS_ENUM *focus, int *line, int *index, int *line2, int *index2)
{
	if (line == NULL || index == NULL || focus == NULL)
	{
		SetLastError(ERROR_INVALID_ADDRESS);
		return;
	}

	std::string console_line;
	if (row < (output_log_displayed_lines + 1) && row > -1
			&& column < max_chars && column > -1)
	{
		// Selecting text

		if (row < output_log_displayed_lines)
		{
			//*focus = SP_D3D9O_SELECT_OUTPUT;
			*line = (output_log.size() - (output_log_displayed_lines - row));
			console_line = output_log.at(*line);
		}
		else
		{
			//*focus = SP_D3D9O_SELECT_INPUT;
			*line = SP_D3D9O_C_INPUT_LINE;

			add_prompt_elements(&console_line);

			// Concatenate extended prompt if it's too long
			if (console_line.length() >= max_chars)
			{
				console_line = console_line.substr(0, max_chars);
			}
			else
			{
				console_line.append(command.substr(input_display_start, ((input_display_end + 1) - input_display_start)));
			}
		}

		if (column >= console_line.length())
		{
			*index = console_line.length();
		}
		else if (column <= 0)
		{
			*index = 0;
		}
		else
		{
			*index = column;
		}

		if (*focus == SP_D3D9O_SELECT_NONE)
		{
			*focus = SP_D3D9O_SELECT_TEXT;
		}
	}
	else if(*focus == SP_D3D9O_SELECT_TEXT)
	{
		// Extend existing text selection

		if (row <= 0)
		{
			// Extend selection to top output line
			*line = output_log.size() - output_log_displayed_lines;
			console_line = output_log.at(*line);
		}
		else if (row >= output_log_displayed_lines)
		{
			// Extend selection to input line
			*line = SP_D3D9O_C_INPUT_LINE;

			add_prompt_elements(&console_line);

			// Concatenate extended prompt if it's too long
			if (console_line.length() >= max_chars)
			{
				console_line = console_line.substr(0, max_chars);
			}
			else
			{
				console_line.append(command.substr(input_display_start, ((input_display_end+1) - input_display_start)));
			}
		}
		else // if (row < output_log_displayed_lines)
		{
			// Extend selection into output lines
			*line = (output_log.size() - (output_log_displayed_lines - row));
			console_line = output_log.at(*line);
		}

		if (column >= console_line.length())
		{
			*index = console_line.length();
		}
		else if (column <= 0)
		{
			*index = 0;
		}
		else
		{
			*index = column;
		}
	}
	else
	{
		// Don't begin selecting text
		*line = SP_D3D9O_C_NO_SELECTION_LINE;
		*index = SP_D3D9O_C_NO_SELECTION_INDEX;
	}

	// Fill in optional arguments, if necessary
	if (line2 != NULL)
	{
		*line2 = *line;
	}
	if (index2 != NULL)
	{
		*index2 = *index;
	}
}


// Obtains various screenspace measurements related to selecting on-screen text
#define _GET_SS_S_AND_L_ARG_COUNT_ 12
int SpD3D9OConsole::get_screenspace_values(RECT *window, SIZE *char_size, RECT *console_lims, long *max_chars,
													long *row, long *column, std::string *full_prompt, long *max_input_chars,
													std::vector<std::string> *autocomplete_opts, int *longest_autocomplete,
													RECT *autocomplete_lims,  int *autocomplete_hover, int return_after_obtaining)
{
	int ret_val = 0; // Number of values obtained
	int ret_after; // Return after obtaining this many values (rather than doing additional calculations that won't be used)
	if (return_after_obtaining < 0)
	{
		ret_after = _GET_SS_S_AND_L_ARG_COUNT_;
	}
	else
	{
		ret_after = return_after_obtaining;
	}
	if (ret_val >= ret_after) return ret_val;


	// Get window dimensions
	RECT window_tmp;
	if (!GetClientRect(*overlay->game_window, &window_tmp))
	{
		// Handle error
		// Call GetLastError() on return for error code
		return ret_val;
	}
	if (window != NULL)
	{
		window->left = window_tmp.left;
		window->top = window_tmp.top;
		window->right = window_tmp.right;
		window->bottom = window_tmp.bottom;
		if (++ret_val >= ret_after) return ret_val;
	}


	// Get char dimensions
	SIZE char_size_tmp;
	font->GetTextExtent("|", &char_size_tmp);
	if (char_size != NULL)
	{
		char_size->cx = char_size_tmp.cx;
		char_size->cy = char_size_tmp.cy;
		if (++ret_val >= ret_after) return ret_val;
	}


	// Get console text area limits
	RECT console_lims_tmp;
	console_lims_tmp = { (long)border_width, (long)border_width, window_tmp.right - (long)border_width,  (char_size_tmp.cy  * ((long)output_log_displayed_lines + 1)) + (long)border_width };
	if (console_lims != NULL)
	{
		console_lims->left = console_lims_tmp.left;
		console_lims->top = console_lims_tmp.top;
		console_lims->right = console_lims_tmp.right;
		console_lims->bottom = console_lims_tmp.bottom;
		if (++ret_val >= ret_after) return ret_val;
	}


	// Get max displayable chars
	long max_chars_tmp;
	max_chars_tmp = (console_lims_tmp.right - console_lims_tmp.left) / char_size_tmp.cx; // Maximum characters per line
	if (max_chars != NULL)
	{
		*max_chars = max_chars_tmp;
		if (++ret_val >= ret_after) return ret_val;
	}
	

	// Output screen text row position of mouse cursor (vertical line coordinate)
	long row_tmp;
	if (SpD3D9OInputHandler::get()->cursor_position.y < (long)border_width)
	{
		row_tmp = -1;
	}
	else
	{
		row_tmp = (SpD3D9OInputHandler::get()->cursor_position.y - (long)border_width) / char_size_tmp.cy;
	}
	if (row != NULL)
	{
		*row = row_tmp;
		if (++ret_val >= ret_after) return ret_val;
	}


	// Output screen column index position of mouse cursor (horizontal char coordinate)
	long column_tmp;
	if (SpD3D9OInputHandler::get()->cursor_position.x < (long)border_width)
	{
		column_tmp = -1;
	}
	else
	{
		column_tmp = (SpD3D9OInputHandler::get()->cursor_position.x - (long)border_width) / char_size_tmp.cx;
	}
	if (column != NULL)
	{
		*column = column_tmp;
		if (++ret_val >= ret_after) return ret_val;
	}


	// Get full prompt string
	std::string full_prompt_tmp;
	add_prompt_elements(&full_prompt_tmp);
	if (full_prompt != NULL)
	{
		*full_prompt = full_prompt_tmp;
		if (++ret_val >= ret_after) return ret_val;
	}


	// Get max displayable input chars
	long max_input_chars_tmp = max_chars_tmp - full_prompt_tmp.length();
	if (caret_position == command.length())
	{
		max_input_chars_tmp--;
	}
	if (max_input_chars != NULL)
	{
		*max_input_chars = max_input_chars_tmp;
		if (++ret_val >= ret_after) return ret_val;
	}


	// Get autocomplete options
	std::vector<std::string> autocomplete_opts_tmp;
	int longest_autocomplete_tmp;
	get_autocomplete_options(command.c_str(), autocomplete_limit, &autocomplete_opts_tmp, &longest_autocomplete_tmp);
	if (autocomplete_opts != NULL)
	{
		*autocomplete_opts = autocomplete_opts_tmp;
		if (++ret_val >= ret_after) return ret_val;
	}
	if (longest_autocomplete != NULL)
	{
		*longest_autocomplete = longest_autocomplete_tmp;
		if (++ret_val >= ret_after) return ret_val;
	}

	// Get autocomplete options screenspace limits
	RECT autocomplete_lims_tmp;
	if (autocomplete_opts_tmp.size() == 0)
	{
		// No autocomplete options available
		autocomplete_lims_tmp = { -1, -1, -1, -1 };
	}
	else
	{
		autocomplete_lims_tmp.left = (long)border_width + (char_size_tmp.cx * full_prompt_tmp.length());
		autocomplete_lims_tmp.top = (long)console_lims_tmp.bottom + 1;
		autocomplete_lims_tmp.right = autocomplete_lims_tmp.left + (char_size_tmp.cx * longest_autocomplete_tmp);
		autocomplete_lims_tmp.bottom = autocomplete_lims_tmp.top + (char_size_tmp.cy  * autocomplete_opts_tmp.size());
	}
	if (autocomplete_lims != NULL)
	{
		autocomplete_lims->left = autocomplete_lims_tmp.left;
		autocomplete_lims->top = autocomplete_lims_tmp.top;
		autocomplete_lims->right = autocomplete_lims_tmp.right;
		autocomplete_lims->bottom = autocomplete_lims_tmp.bottom;
		if (++ret_val >= ret_after) return ret_val;
	}
	
	// Check if cursor is hovering over an autocomplete suggestion
	int autocomplete_hover_tmp = -1;
	for (int i = 0; i < autocomplete_opts_tmp.size(); i++)
	{
		if (SpD3D9OInputHandler::get()->cursor_position.x >= autocomplete_lims_tmp.left
			&& SpD3D9OInputHandler::get()->cursor_position.x <= autocomplete_lims_tmp.right
			&& SpD3D9OInputHandler::get()->cursor_position.y >= (autocomplete_lims_tmp.top + (i * char_size_tmp.cy))
			&& SpD3D9OInputHandler::get()->cursor_position.y <= (autocomplete_lims_tmp.top + ((i + 1) * char_size_tmp.cy)))
		{
			autocomplete_hover_tmp = i;
			break;
		}
	}
	if (autocomplete_hover != NULL)
	{
		*autocomplete_hover = autocomplete_hover_tmp;
		if (++ret_val >= ret_after) return ret_val;
	}

	return ret_val;
}


// Called when user first clicks to select text or an autocomplete option
void SpD3D9OConsole::start_selection()
{
	clear_selection();

	// Get screenspace limits
	RECT console_lims;
	long max_chars, row, column;
	int autocomplete_hover;
	get_screenspace_values(NULL, NULL, &console_lims,  &max_chars, &row, &column, NULL, NULL, NULL, NULL, NULL, &autocomplete_hover);


	// Stop the game from reading the click message if the user clicks in the bounds of the console
	if (SpD3D9OInputHandler::get()->cursor_position.y <= (console_lims.bottom + border_width) || autocomplete_hover > -1)
	{
		SpD3D9OInputHandler::get()->handled = true;
	}

	if (autocomplete_hover == -1)
	{
		// Move caret, if necessary
		if (row == output_log_displayed_lines)
		{
			std::string console_line;
			add_prompt_elements(&console_line);

			if (command.length() == 0 || ((console_line.length() + command.length()) < max_chars && column >= (console_line.length() + command.length())))
			{
				caret_position = command.length();
			}
			else if (column >= console_line.length())
			{
				caret_position = (column - console_line.length()) + input_display_start;
			}
		}

		// Check if mouse cursor is in an area with selectable text
		cursor_pos_to_selection(row, column, max_chars, &selection.focus, &selection.line1, &selection.i1, &selection.line2, &selection.i2);
	}
	else
	{
		// Mouse is hovering over an autocomplete option
		selection.focus = SP_D3D9O_SELECT_AUTOCOMPLETE;
		selection.autocomplete_selection = autocomplete_hover;
	}
}


// Called when the user has already begun selecting from the autocomplete suggestions and moves the cursor or releases the mouse button
void SpD3D9OConsole::continue_autocomplete_selection()
{
	int autocomplete_hover;
	get_screenspace_values(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &autocomplete_hover);

	if (autocomplete_hover > -1)
	{
		selection.autocomplete_selection = autocomplete_hover;
		if (selection.focus == SP_D3D9O_SELECT_AUTOCOMPLETE && !SpD3D9OInputHandler::get()->mouse_button_down[0])
		{
			std::vector<std::string> matches;
			get_autocomplete_options(command.c_str(), selection.autocomplete_selection + 1, &matches);
			if (matches.size() > selection.autocomplete_selection)
			{
				command = matches.at(selection.autocomplete_selection);
				caret_position = command.length();
			}
			clear_selection();
		}
	}
	else if(selection.focus == SP_D3D9O_SELECT_AUTOCOMPLETE && !SpD3D9OInputHandler::get()->mouse_button_down[0])
	{
		selection.focus = SP_D3D9O_SELECT_NONE;
	}
}


// Called when the user has already begun selecting text and moves the cursor or releases the mouse button;
//	Extends current text selection based on cursor position.
void SpD3D9OConsole::continue_text_selection()
{
	if (selection.focus != SP_D3D9O_SELECT_TEXT)
	{
		return;
	}

	// Get screenspace limits
	long max_chars, row, column;
	std::string console_line;
	get_screenspace_values(NULL, NULL, NULL, &max_chars, &row, &column, &console_line, NULL, NULL, NULL, NULL, NULL, 4);

	// Check if mouse cursor is in an area with selectable text
	cursor_pos_to_selection(row, column, max_chars, &selection.focus, &selection.line2, &selection.i2);

	// Determine which end of the selection appears first in the output string
	if ((selection.line1 < selection.line2 && selection.line1 != SP_D3D9O_C_INPUT_LINE)
		|| (selection.line1 == selection.line2 && selection.i1 <= selection.i2)
		|| (selection.line1 != selection.line2 && selection.line2 == SP_D3D9O_C_INPUT_LINE))
	{
		// When drawing, start from from (line1, index1)
		selection.start_line = &selection.line1;
		selection.start_index = &selection.i1;
		selection.end_line = &selection.line2;
		selection.end_index = &selection.i2;
	}
	else
	{
		// When drawing, start from from (line2, index2)
		selection.start_line = &selection.line2;
		selection.start_index = &selection.i2;
		selection.end_line = &selection.line1;
		selection.end_index = &selection.i1;
	}

	// Move caret, if necessary
	//add_prompt_elements(&console_line);
	if (selection.line2 == SP_D3D9O_C_INPUT_LINE && selection.i2 >= console_line.length() && !(selection.line1 == selection.line2 && selection.i1 == selection.i2))
	{
		if (command.length() == 0 || ((console_line.length() + command.length()) < max_chars && column >= (console_line.length() + command.length())))
		{
			caret_position = command.length();
		}
		else if (column > console_line.length())
		{
			if (selection.line1 == SP_D3D9O_C_INPUT_LINE && selection.i1 < selection.i2)
			{
				caret_position = (column - console_line.length()) + input_display_start - 1;
			}
			else
			{
				caret_position = (column - console_line.length()) + input_display_start;
			}
		}
	}
}


// Gets the starting/ending indexes of selected input substring
void SpD3D9OConsole::get_input_selection(int *start, int *end)
{
	if (start == NULL || end == NULL)
	{
		SetLastError(ERROR_INVALID_ADDRESS);
		return;
	}

	if (selection.focus == SP_D3D9O_SELECT_NONE)
	{
		(*start) = -1;
		(*end) = -1;
		return;
	}

	std::string full_prompt;
	add_prompt_elements(&full_prompt);

	if (*selection.start_line == SP_D3D9O_C_INPUT_LINE && (*selection.start_index) >= full_prompt.length())
	{
		(*start) = ((*selection.start_index) - full_prompt.length()) + input_display_start;
	}
	else
	{
		(*start) = -1;
	}

	if (*selection.end_line == SP_D3D9O_C_INPUT_LINE && (*selection.end_index) >= full_prompt.length())
	{
		(*end) = ((*selection.end_index) - full_prompt.length()) + input_display_start;
	}
	else
	{
		(*end) = -1;
	}
}


// Formats a line of text by shortening it to fit in the console window
void SpD3D9OConsole::format_output_line(std::string *str, int line, int max_chars)
{
	if (str == NULL)
	{
		SetLastError(ERROR_INVALID_ADDRESS);
		return;
	}
	else if (line < 0 || line >= output_log.size())
	{
		SetLastError(ERROR_INVALID_INDEX);
		return;
	}
	else if (max_chars < 0)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return;
	}

	str->clear();
	str->append(output_log.at(line));
	if (str->length() >= max_chars)
	{
		(*str) = str->substr(0, max_chars);
	}  
}


// Renders highlighted text to the console window
void SpD3D9OConsole::draw_highlighted_text(CONSOLE_TEXT_SELECTION p_selection, std::string *input_line)
{
	// Get screenspace limits
	SIZE char_size;
	long max_chars;
	get_screenspace_values(NULL, &char_size, NULL, &max_chars, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 2);

	std::string line;

	if (*p_selection.start_line != *p_selection.end_line)
	{
		// Draw multiple lines

		// Draw start_line
		format_output_line(&line, *p_selection.start_line, max_chars);
		font->DrawText((float)border_width + (char_size.cx * (*p_selection.start_index)), (float)border_width + (char_size.cy * (output_log_displayed_lines - (output_log.size() - *p_selection.start_line))), font_highlight_color, line.substr(*p_selection.start_index).c_str(), D3DFONT_BACKGROUND, background_highlight_color);

		if (*p_selection.end_line != SP_D3D9O_C_INPUT_LINE)
		{
			// Draw until end_line
			
			// Draw middle output lines
			for (int i = *p_selection.start_line+1; i < *p_selection.end_line && i < output_log.size(); i++)
			{
				format_output_line(&line, i, max_chars);
				font->DrawText((float)border_width, (float)border_width + (char_size.cy * (output_log_displayed_lines - (output_log.size() - i))), font_highlight_color, line.c_str(), D3DFONT_BACKGROUND, background_highlight_color);
			}

			// end_line
			format_output_line(&line, *p_selection.end_line, max_chars);
			font->DrawText((float)border_width, (float)border_width + (char_size.cy * (output_log_displayed_lines - (output_log.size() - *p_selection.end_line))), font_highlight_color, line.substr(0, *p_selection.end_index).c_str(), D3DFONT_BACKGROUND, background_highlight_color);
		
		}
		else
		{
			// Draw until end of output, then draw input line

			// Draw middle output lines
			for (int i = *p_selection.start_line + 1; i < output_log.size(); i++)
			{
				format_output_line(&line, i, max_chars);
				font->DrawText((float)border_width, (float)border_width + (char_size.cy * (output_log_displayed_lines - (output_log.size() - i))), font_highlight_color, line.c_str(), D3DFONT_BACKGROUND, background_highlight_color);
			}

			// Draw end_line (Input line)
			font->DrawText((float)border_width, (float)border_width + (char_size.cy * output_log_displayed_lines), font_highlight_color, input_line->substr(0, *p_selection.end_index).c_str(), D3DFONT_BACKGROUND, background_highlight_color);
		}
	}
	else if(*p_selection.start_line == SP_D3D9O_C_INPUT_LINE)
	{
		// Draw from start_index to end_index in input line
		font->DrawText((float)border_width + (char_size.cx * (*p_selection.start_index)), (float)border_width + (char_size.cy * output_log_displayed_lines), font_highlight_color, input_line->substr(*p_selection.start_index, (*p_selection.end_index - *p_selection.start_index)).c_str(), D3DFONT_BACKGROUND, background_highlight_color);
	}
	else
	{
		// Draw from start_index to end_index in start_line
		format_output_line(&line, *p_selection.start_line, max_chars);
		font->DrawText((float)border_width + (char_size.cx * (*p_selection.start_index)), (float)border_width + (char_size.cy * (output_log_displayed_lines - (output_log.size() - *p_selection.start_line))), font_highlight_color, line.substr(*p_selection.start_index, (*p_selection.end_index - *p_selection.start_index)).c_str(), D3DFONT_BACKGROUND, background_highlight_color);
	}
}


// Builds the string of currently-highlighted text
void SpD3D9OConsole::build_highlighted_text(CONSOLE_TEXT_SELECTION p_selection, std::string *highlighted_str)
{
	highlighted_str->clear();

	if (selection.focus != SP_D3D9O_SELECT_TEXT)
	{
		// No text is highlighted
		return;
	}

	// Get screenspace limits
	long max_chars;
	get_screenspace_values(NULL, NULL, NULL, &max_chars, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, 1);

	std::string input_line;
	add_prompt_elements(&input_line);
	input_line.append(command.substr(input_display_start, (input_display_end - input_display_start)+1));
	input_line = input_line.substr(0, max_chars);


	if (*p_selection.start_line != *p_selection.end_line)
	{
		// Selection is multiple output lines

		// Add start_line
		highlighted_str->append(output_log.at(*p_selection.start_line).substr(*p_selection.start_index));

		if (*p_selection.end_line != SP_D3D9O_C_INPUT_LINE)
		{
			// Draw until end_line

			// Draw middle output lines
			for (int i = *p_selection.start_line + 1; i < *p_selection.end_line && i < output_log.size(); i++)
			{
				highlighted_str->append("\n").append(output_log.at(i));
			}

			// end_line
			highlighted_str->append("\n").append(output_log.at(*p_selection.end_line).substr(0, *p_selection.end_index));

		}
		else
		{
			// Selection is 1 or more output lines and input line

			// Draw middle output lines
			for (int i = *p_selection.start_line + 1; i < output_log.size(); i++)
			{
				highlighted_str->append("\n").append(output_log.at(i));
			}

			// Draw end_line (Input line)
			highlighted_str->append("\n").append(input_line.substr(0, *p_selection.end_index));
		}
	}
	else if (*p_selection.start_line == SP_D3D9O_C_INPUT_LINE)
	{
		// Selection from start_index to end_index in input line
		highlighted_str->append(input_line.substr(*p_selection.start_index, (*p_selection.end_index - *p_selection.start_index)));
	}
	else
	{
		// Selection from start_index to end_index in start_line a single output line
		highlighted_str->append(output_log.at(*p_selection.start_line).substr(*p_selection.start_index, (*p_selection.end_index - *p_selection.start_index)));
	}
}


#endif // _SP_D3D9O_CONSOLE_TEXT_SELECTION_H_