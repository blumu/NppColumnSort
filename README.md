[![Board Status](https://williamblum.visualstudio.com/a50c4097-c9a3-4262-ba54-b517a7c2683d/6816f3cc-1ab5-4743-a4d4-6b7c8428e1a7/_apis/work/boardbadge/38d41e58-c05f-4a9e-b046-7a412cb606b1)](https://williamblum.visualstudio.com/a50c4097-c9a3-4262-ba54-b517a7c2683d/_boards/board/t/6816f3cc-1ab5-4743-a4d4-6b7c8428e1a7/Microsoft.RequirementCategory)
NppColumnSort
=============

Column sorting plugin for Notepad++

Implements TextPad-like text sorting feature for Notepad++.
(http://notepad-plus-plus.org/)

The plugin lets you define a sort criterion based on an unlimited number of columns. For each column the comparison can be either numerical or lexicographical.

More information at:  http://william.famille-blum.org/blog/index.php?entry=entry110123-113226

Installation
--------------

Under the Notepad++ Plugin Manager look for "Column Sorting" plugin and click install.

Manual install
-----------------
You first need to build the project in Visual Studio. To install the plugin just copy the DLL under 'Unicode Release' to the Notepad++ plugin directory: 

> %ProgramFiles(x86)%\Notepad++\plugins for Windows 64bit;
> %ProgramFiles%\Notepad++\plugins for Windows 32bit.