# DoxyIt

[![Build status](https://ci.appveyor.com/api/projects/status/mma3iba7vlvjk6jf?svg=true)](https://ci.appveyor.com/project/dail8859/doxyit)

Notepad++ plugin for [Doxygen](http://www.doxygen.org) commenting. This also provides helpful features for creating and editing comment blocks, even if Doxygen isn't desired.

The release versions of DoxyIt can be downloaded [here](https://github.com/dail8859/DoxyIt/releases).

Currently there is support for:
- C
- C++
- Python
- Java
- PHP
- JavaScript
- C#
- User Defined Languages

**Note:** DoxyIt uses very simplistic parsing mechanisms and does not enforce valid language syntax, meaning it can easily be fooled. Thus, it is up to the user to ensure the function/method is syntactically valid when attempting to generate Doxygen function commenting as described below.

## Usage
### Doxygen Function Commenting
Just start by opening a function block above the function. 

![Demo](/img/Demo.gif)

You can also manually trigger the function documentation (this is sometimes required for specific settings). Place the cursor on the line directly above the function definition and press `Ctrl+Alt+Shift+D` (or through the menu command *Plugins > DoxyIt > DoxyIt - Function*).

**Note:** Function commenting for User Defined Languages inserts a comment block but does not parse any text.

### Doxygen File Commenting
Using the menu command *Plugins > DoxyIt > DoxyIt - File* will insert a Doxygen comment block for the file at the current cursor position.

### Active Commenting
Even if Doxygen commands aren't desired, you can still take advantage of the active commenting feature. A new comment block can be started by typing the opening string and pressing `Enter`. Adding a new line within the block will extend it.

![Demo2](/img/Demo2.gif)

**Note:** If DoxyIt is configured to use long strings to indicate the start of a comment block, only the first 3 characters need to be typed.

## Settings
Each language can be configured to have any string to indicate the start, middle, and end of a document block. The settings dialog also provides a live preview of what a documentation block would look like:

![Settings Dialog](/img/Settings.png)

### Doxygen Commenting Format
The format string is used to customize the Doxygen Function Commenting block generated. There are currently a few keywords that are used.
- `$FILENAME` - The current file name.
- `$FUNCTION` - The name of the function/method.
- `$PARAM` - Expands to a single function/method parameter. Any line containing this will get repeated for each parameter.
- `$COMPUTER` - Current computer name.
- `$USER` - User account name of current user (for example, smith).
- `$FULLUSER` - Full user name of current user (for example, Jeff Smith).
- `$DATE` - Current date and time, format ISO 8601 (for example, 2009-06-30T18:30:00).
- `$DATE_a` - Abbreviated weekday name
- `$DATE_A` - Full weekday name
- `$DATE_b` - Abbreviated month name
- `$DATE_B` - Full month name
- `$DATE_c` - Date and time representation appropriate for locale
- `$DATE_d` - Day of month as decimal number (01 - 31)
- `$DATE_H` - Hour in 24-hour format (00 - 23)
- `$DATE_I` - Hour in 12-hour format (01 - 12)
- `$DATE_j` - Day of year as decimal number (001 - 366)
- `$DATE_m` - Month as decimal number (01 - 12)
- `$DATE_M` - Minute as decimal number (00 - 59)
- `$DATE_p` - Current locale's A.M./P.M. indicator for 12-hour clock
- `$DATE_S` - Second as decimal number (00 - 59)
- `$DATE_U` - Week of year as decimal number, with Sunday as first day of week (00 - 53)
- `$DATE_w` - Weekday as decimal number (0 - 6; Sunday is 0)
- `$DATE_W` - Week of year as decimal number, with Monday as first day of week (00 - 53)
- `$DATE_x` - Date representation for current locale
- `$DATE_X` - Time representation for current locale
- `$DATE_y` - Year without century, as decimal number (00 - 99)
- `$DATE_Y` - Year with century, as decimal number
- `$DATE_z`, `$DATE_Z` - Either the time-zone name or time zone abbreviation, depending on registry settings; no characters if time zone is unknown
- `$@` - Expands to the prefix character for Doxygen commands.
- `$|` - Marks the alignment position. This flag is only valid for lines containing $PARAM.
- `$(...)` - Defines a location to jump to when pressing the Tab key. Text within the parenthesis is used as a default value.

Not all keywords are valid for User Defined Languages. 

### Jump Locations
The function and file formats can specify jump locations. These are areas of text that can be jumped to by pressing the `Tab` key. Using `$(...)` will use the text within the parenthesis as a default value for the location. Jump locations with the same default text will be edited simultaneously. Pressing the `Esc` key will remove all marked jump locations.

### UDL Support
Custom UDLs can be added by pressing the `+` button next to the list of supported languages. The name entered for the UDL must match the name of the desired UDL. Pressing the `-` button will remove the selected UDL. Function commenting is partially supported for UDLs. It will insert a comment block at the current location but will not attempt to parse any text.

## Installation
Use the built in Plugin Admin to install the plugin.

## Development
The code is developed using Visual Studio 2017. Building the code will generate a DLL which can be used by Notepad++. For convenience, Visual Studio copies the DLL into the Notepad++ plugin directory.

## License
This code is released under the [GNU General Public License version 2](http://www.gnu.org/licenses/gpl-2.0.txt).

The [T-Rex Regular Expression library](http://tiny-rex.sourceforge.net/) used in this project has been released under the [zlib/libpng License](http://opensource.org/licenses/zlib-license.php). (C) 2003-2006 Alberto Demichelis
