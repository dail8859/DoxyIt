# DoxyIt
Notepad++ plugin for [Doxygen](http://www.doxygen.org) commenting. This also provides helpful features to support creating and editing comment blocks, even if Doxygen isn't desired. 

Currently there is support for C/C++, Python, and Java.

DoxyIt can be downloaded [here](http://goo.gl/Qjlyw).

## Usage
### Doxygen Function Commenting
Placing the cursor on the line directly above the function definition and pressing `Ctrl+Alt+Shift+D` (or through the menu command *Plugins > DoxyIt > DoxyIt - Function*) will insert a comment block. For example:

```c
/**
 *  \brief [Brief]
 *  
 *  \param [in] p [Param p Description]
 *  \param [in] index [Param index Description]
 *  \return [Return Description]
 *  
 *  \details [Details]
 */
int function(const char *p, int index)
```

### Doxygen File Commenting
Using the menu command *Plugins > DoxyIt > DoxyIt - File* will insert a Doxygen comment block for the file at the current cursor position. For example:
```c
/**
 *  \file DoxyIt.cpp
 *  \brief [Brief]
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

<b>Note:</b> If DoxyIt is configured to use long strings to indicate the start of a comment block, only the first 3 characters need to be typed. For example:
```
/**|
```
and then `Enter` will turn into:
```
/********************************************//**
 *  |
 ***********************************************/
```

### Settings
Each language can be configured to have any string to indicate the start, line, and end of a document block. The settings dialog also provides a live preview of what a document block would look like:
![Settings Dialog](http://goo.gl/E9S66)


##Todo
- ~~[FingerText](http://sourceforge.net/projects/fingertext/) integration for hotspot navigation (optional).~~ [*Pending pull request*](https://github.com/erinata/FingerText/pull/38)
- Line wrapping inside comment blocks.
- Autocomplete for Doxygen commands.
- Generate documentation for other code elements:
    - Classes
    - Enums
    - Structs
    - ...
- Possible support for:
    - PHP
    - C#
- Configuration of:
    - Comment block contents
    - ~~Strings for comment start, line, and end~~
    - ~~Per language settings~~
    - ~~Doxygen command prefixes (i.e. '\' or '@')~~

## Development
The code has been developed using MSVC 2010 Express. Building the "Unicode Release" will generate the DLL which can be used by Notepad++. For convenience, MSVC copies the built DLL into the Notepad++ plugin directory. 

## License
This code is released under the [GNU General Public License version 2](http://www.gnu.org/licenses/gpl-2.0.txt).

The [T-Rex Regular Expression library](http://tiny-rex.sourceforge.net/) used in this project has been released under the [zlib/libpng License](http://opensource.org/licenses/zlib-license.php). (C) 2003-2006 Alberto Demichelis
