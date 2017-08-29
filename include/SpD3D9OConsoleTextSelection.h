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
	else if(*focus != SP_D3D9O_SELECT_NONE)
	{
		// Extend existing selection

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
int SpD3D9OConsole::get_screenspace_limits(RECT *window, SIZE *char_size, RECT *console_lims, long *max_chars, long *row, long *column)
{
	if (window == NULL)
	{
		SetLastError(ERROR_INVALID_ADDRESS);
		return -1;
	}

	// Get window dimensions
	if (!GetClientRect(*overlay->game_window, window))
	{
		// Handle error
		// Call GetLastError() on return for error code
		return 0;
	}

	// Get char dimensions
	if (char_size == NULL)
	{
		return 1;
	}
	font->GetTextExtent("|", char_size);

	// Get console text area limits
	if (console_lims == NULL)
	{
		return 2;
	}
	*console_lims = { (long)border_width, (long)border_width, window->right - (long)border_width,  (char_size->cy  * ((long)output_log_displayed_lines + 1)) + (long)border_width };

	// Get max displayable chars
	if (max_chars == NULL)
	{
		return 3;
	}
	*max_chars = (console_lims->right - console_lims->left) / char_size->cx; // Maximum characters per line

	// Output screen text row position of mouse cursor (vertical line coordinate)
	if (row == NULL)
	{
		return 4;
	}
	if (SpD3D9OInputHandler::get()->cursor_position.y < (long)border_width)
	{
		*row = -1;
	}
	else
	{
		*row = (SpD3D9OInputHandler::get()->cursor_position.y - (long)border_width) / char_size->cy;
	}

	// Output screen column index position of mouse cursor (horizontal char coordinate)
	if (column == NULL)
	{
		return 5;
	}
	if (SpD3D9OInputHandler::get()->cursor_position.x < (long)border_width)
	{
		*column = -1;
	}
	else
	{
		*column = (SpD3D9OInputHandler::get()->cursor_position.x - (long)border_width) / char_size->cx;
	}

	return 6;
}


// Called when user first clicks to select text
void SpD3D9OConsole::start_text_selection()
{
	clear_selection();

	// Get screenspace limits
	RECT window, console_lims;
	SIZE char_size;
	long max_chars, row, column;
	get_screenspace_limits(&window, &char_size, &console_lims,  &max_chars, &row, &column);


	// Stop the game from reading the click message if the user clicks in the bounds of the console
	if (SpD3D9OInputHandler::get()->cursor_position.y <= (console_lims.bottom + border_width))
	{
		SpD3D9OInputHandler::get()->handled = true;
	}


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


// Called when the user has already begun selecting text and moves the cursor or releases the mouse button;
//	Extends current text selection based on cursor position.
void SpD3D9OConsole::continue_text_selection()
{
	if (selection.focus == SP_D3D9O_SELECT_NONE)
	{
		return;
	}

	// Get screenspace limits
	RECT window, console_lims;
	SIZE char_size;
	long max_chars, row, column;
	get_screenspace_limits(&window, &char_size, &console_lims, &max_chars, &row, &column);

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
	std::string console_line;
	add_prompt_elements(&console_line);
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
	RECT window, console_lims;
	SIZE char_size;
	long max_chars;
	get_screenspace_limits(&window, &char_size, &console_lims, &max_chars);

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
	// Get screenspace limits
	RECT window, console_lims;
	SIZE char_size;
	long max_chars;
	get_screenspace_limits(&window, &char_size, &console_lims, &max_chars);

	std::string input_line;
	add_prompt_elements(&input_line);
	input_line.append(command.substr(input_display_start, (input_display_end - input_display_start)));
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