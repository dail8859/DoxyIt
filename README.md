# DoxyIt
Notepad++ plugin for [Doxygen](http://www.doxygen.org) commenting. Support for C/C++, Python, and Java.

<b><i> Note: This still in the pre-alpha stages and should not be considered stable. No DLL is being released at this time unless requested. </i></b>

## Usage
Placing the cursor on the line directly above the function definition and pressing `Ctrl+Alt+Shift+D` will insert a comment block. For example:

```c
/**
 *  \brief [Brief]
 *  
 *  \param [in] p [Param p Description]
 *  \param [in] index [Param pointer Description]
 *  \return [Return Description]
 *  
 *  \details [Details]
 */
int function(const char *p, int index)
```

## Features
- ~~[FingerText](http://sourceforge.net/projects/fingertext/) integration for hotspot navigation (optional).~~ *Pending pull request*
- Active Commenting - automatically closes newly created comment blocks (e.g. type /** then enter). Also, when creating a new line in a comment block, it prefixes the line with " * ".
- Per language settings

![Settings Dialog](http://goo.gl/E9S66)

##Todo
- Line wrapping inside comment blocks.
- Autocomplete for Doxygen commands.
- Generate documentation for other code elements:
    - Classes
    - Enums
    - Structs
    - ...
- Support for:
    - ~~Python~~
    - ~~Java~~
    - PHP(?)
    - C#(?)
- Configuration of:
    - ~~Strings for comment start, line, and end~~
    - Comment block contents
    - ~~Per language settings~~
    - ~~Doxygen command prefixes (i.e. '\' or '@')~~

## Development
The code has been developed using MSVC 2010 Express. Building the "unicode release" will generate the DLL which can be used by Notepad++. For convenience, MSVC copies the built DLL into the Notepad++ plugin directory. 

## License
This code is released under the [GNU General Public License version 2](http://www.gnu.org/licenses/gpl-2.0.txt).

The [T-Rex Regular Expression library](http://tiny-rex.sourceforge.net/) used in this project has been released under the [zlib/libpng License](http://opensource.org/licenses/zlib-license.php). (C) 2003-2006 Alberto Demichelis
