// This file is part of DoxyIt.
// 
// Copyright (C)2017 Justin Dailey <dail8859@yahoo.com>
// 
// DoxyIt is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "JumpLocations.h"
#include "PluginDefinition.h"
#include "Utils.h"
#include <utility>

typedef std::pair<int, int> Location;

const Location INVALID_LOCATION = std::make_pair(INVALID_POSITION, INVALID_POSITION);
const char *JUMPLOCATION_REGEX = "\\$\\((.*?)\\)";

static bool IsLocationValid(const Location& location)
{
	return location.first != INVALID_LOCATION.first && location.second != INVALID_LOCATION.second;
}

static void PreProcessJumpLocation(Location& location)
{
	if (IsLocationValid(location))
	{
		// For empty locations e.g. "$()" replace it with "()"
		if (location.second - location.first == strlen("$()"))
		{
			editor.SetTargetRange(location.first, location.second);
			editor.ReplaceTarget(-1, "()");
		}
		else
		{
			// Remove the "$(" and ")" to just keep the text
			std::string text = GetTextRange(location.first, location.second);
			editor.SetTargetRange(location.first, location.second);
			editor.ReplaceTarget((int)text.length() - 3, &text[2]);
		}

		// Set the location to the new range
		location.first = editor.GetTargetStart();
		location.second = editor.GetTargetEnd();
	}
}

static Location FindNextJumpLocation(int startPosition, bool wrap)
{
	int curPos = startPosition;
	int length = editor.GetTextLength();

	// Check to see if the position is inside an indicator range already
	if (editor.IndicatorAllOnFor(curPos) & (1 << JUMPLOCATION_INDICATOR))
	{
		return std::make_pair(editor.IndicatorStart(JUMPLOCATION_INDICATOR, curPos), editor.IndicatorEnd(JUMPLOCATION_INDICATOR, curPos));
	}

	int start = editor.IndicatorEnd(JUMPLOCATION_INDICATOR, curPos);
	int end = editor.IndicatorEnd(JUMPLOCATION_INDICATOR, start);

	if (wrap && (start == 0 || start >= length))
	{
		// Start again at the top of the file
		start = editor.IndicatorEnd(JUMPLOCATION_INDICATOR, 0);
		end = editor.IndicatorEnd(JUMPLOCATION_INDICATOR, start);
	}

	if (start != 0 && start < length)
	{
		return std::make_pair(start, end);
	}

	return INVALID_LOCATION;
}

static void SelectMatchingJumpLocations(const Location& location)
{
	if (IsLocationValid(location))
	{
		std::string jumpLocationText = GetTextRange(location.first, location.second);

		// A location of "()" is interpreted as an empty jump
		// location which should not match other emtpy locations
		if (jumpLocationText == "()")
		{
			return;
		}

		int startPos = 0;
		do {
			auto match = FindNextJumpLocation(startPos, false);
			startPos = match.second + 1;

			if (IsLocationValid(match))
			{
				if (match.first != location.first)
				{
					std::string nextLocationText = GetTextRange(match.first, match.second);

					if (jumpLocationText == nextLocationText)
					{
						editor.AddSelection(match.second, match.first);
					}
				}
			}
			else
			{
				break;
			}
		} while (startPos != INVALID_POSITION);
	}
}

static void MarkJumpLocationsInRange(int start, int end)
{
	std::vector<Location> jumpLocations;

	// Find all the jump locations
	int startPos = start;
	auto match = FindInRange(JUMPLOCATION_REGEX, startPos, end, true);
	while (IsLocationValid(match))
	{
		jumpLocations.push_back(match);
		startPos = match.second;
		match = FindInRange(JUMPLOCATION_REGEX, startPos, end, true);
	}

	// Iterate them backwards since the text could be modified by PreProcessJumpLocation
	editor.SetIndicatorCurrent(JUMPLOCATION_INDICATOR);
	for (auto it = jumpLocations.rbegin(); it != jumpLocations.rend(); ++it)
	{
		PreProcessJumpLocation(*it);
		editor.IndicatorFillRange(it->first, it->second - it->first);
	}
}

static bool GoToJumpLocation(const Location& location)
{
	if (IsLocationValid(location))
	{
		editor.SetSel(location.first, location.second);
		SelectMatchingJumpLocations(location);
		editor.RotateSelection(); // Rotate to the selection that was made first

		// Turn these off incase they were being shown
		editor.AutoCCancel();
		editor.CallTipCancel();

		return true;
	}

	return false;
}

static bool IsLocationOnJumpLocation(const Location& location)
{
	return IsLocationValid(location) &&
	       location.first != location.second &&
	       location.first == editor.IndicatorStart(JUMPLOCATION_INDICATOR, location.first) &&
	       location.second == editor.IndicatorEnd(JUMPLOCATION_INDICATOR, location.second - 1);
}

void ProcessTextRangeForNewJumpLocations(int startPosition, int endPosition)
{
	MarkJumpLocationsInRange(startPosition, endPosition);

	// Since this is a new text range, don't wrap because it wouldn't 
	// make sense to jump above the position.
	GoToNextJumpLocation(startPosition, false);
}

bool GoToNextJumpLocation(int startPosition, bool wrap)
{
	auto selection = std::make_pair(editor.GetSelectionStart(), editor.GetSelectionEnd());
	bool selectionWasOnJumpLocation = IsLocationOnJumpLocation(selection);
	if (selectionWasOnJumpLocation)
	{
		editor.SetIndicatorCurrent(JUMPLOCATION_INDICATOR);
		editor.IndicatorClearRange(selection.first, selection.second - selection.first);
	}

	auto location = FindNextJumpLocation(startPosition, wrap);

	if (IsLocationValid(location))
	{
		return GoToJumpLocation(location);
	}
	else if (selectionWasOnJumpLocation)
	{
		// The last jump location has been cleared, so move the cursor to the end of it
		editor.GotoPos(selection.second);
		return true;
	}
	else
	{
		return false;
	}
}

void ClearJumpLocations()
{
	editor.SetIndicatorCurrent(JUMPLOCATION_INDICATOR);
	editor.IndicatorClearRange(0, editor.GetTextLength());
}
