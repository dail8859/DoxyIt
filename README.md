# DoxyIt
Notepad++ plugin for [Doxygen](http://www.doxygen.org) commenting. This also provides helpful features to support creating and editing comment blocks, even if Doxygen isn't desired.

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
Placing the cursor on the line directly above the function definition and pressing `Ctrl+Alt+Shift+D` (or through the menu command *Plugins > DoxyIt > DoxyIt - Function*) will insert a comment block. For example:

```c
/**
 *  \brief Brief
 *  
 *  \param [in] ptr Parameter_Description
 *  \param [in] index Parameter_Description
 *  \return Return_Description
 *  
 *  \details Details
 */
int function(const char *ptr, int index)
```
**Note:** Function commenting for User Defined Languages inserts a comment block and does not parse any text.

### Doxygen File Commenting
Using the menu command *Plugins > DoxyIt > DoxyIt - File* will insert a Doxygen comment block for the file at the current cursor position. For example:
```c
/**
 *  \file DoxyIt.cpp
 *  \brief Brief
 */
```

### Active Commenting
Even if Doxygen commands aren't desired, you can still take advantage of the active commenting feature. This allows document blocks to be created and modified. For example, (where the '|' character represents the cursor) typing:
```
/**|
```
then pressing `Enter`, will close the comment block:
```
/**
 *  |
 */
```
Pressing `Enter` inside a comment block will add a new line prepended with the desired string. Pressing `Enter` again in the above example will create:
```
/**
 *  
 *  |
 */
```

**Note:** If DoxyIt is configured to use long strings to indicate the start of a comment block, only the first 3 characters need to be typed. For example:
```
/**|
```
and then `Enter` will turn into:
```
/********************************************//**
 *  |
 ***********************************************/
```

## Settings
Each language can be configured to have any string to indicate the start, line, and end of a document block. The settings dialog also provides a live preview of what a documentation block would look like:
![Settings Dialog](https://dl.dropboxusercontent.com/u/13788271/DoxyIt/SettingsWithAddRemove.png)

### Doxygen Commenting Format
The format string is used to customize the Doxygen Function Commenting block generated. There are currently a few keywords that are used.
- $FILENAME - The current file name.
- $FUNCTION - The name of the function/method.
- $PARAM - Expands to a single function/method parameter. Any line containing this will get repeated for each parameter.
- $@ - Expands to the prefix character for Doxygen commands.
- $| - Marks the alignment position. This flag is only valid for lines containing $PARAM.

Not all keywords are valid for User Defined Languages. 

### UDL Support
Custom UDLs can be added by pressing the `+` button next to the list of supported languages. The name entered for the UDL must match the name of the desired UDL. Pressing the `-` button will remove the selected UDL. Function commenting is partially supported for UDLs. It will insert a comment block at the current location but will not attempt to parse any text.

##Todo
- Line wrapping inside comment blocks.
- Autocomplete for Doxygen commands.
- Support for other languages
    - External Lexers
    - Built in languages

## Development
The code has been developed using MSVC 2013 Express. Building the "Unicode Release" will generate the DLL which can be used by Notepad++. For convenience, MSVC copies the built DLL into the Notepad++ plugin directory. 

## License
This code is released under the [GNU General Public License version 2](http://www.gnu.org/licenses/gpl-2.0.txt).

The [T-Rex Regular Expression library](http://tiny-rex.sourceforge.net/) used in this project has been released under the [zlib/libpng License](http://opensource.org/licenses/zlib-license.php). (C) 2003-2006 Alberto Demichelis
