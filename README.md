NppColumnSort
=============

Column sorting plugin for Notepad++

Implements TextPad-like text sorting feature for Notepad++.
(http://notepad-plus-plus.org/)

The plugin lets you define a sort criterion based on an unlimited number of columns. For each column the comparison can be either numerical or lexicographical.

More information at:  http://william.famille-blum.org/blog/index.php?entry=entry110123-113226

Build status
------------

[![Build status](https://ci.appveyor.com/api/projects/status/ksp8pd28oqhi298p?svg=true)](https://ci.appveyor.com/project/blumu/nppcolumnsort)

Installation
--------------

Under the Notepad++ Plugin Manager look for "Column Sorting" plugin and click install.

Manual install
-----------------
You first need to build the project in Visual Studio. To install the plugin just copy the DLL under 'Unicode Release' to the Notepad++ plugin directory: 

> %ProgramFiles(x86)%\Notepad++\plugins for Windows 64bit;
> %ProgramFiles%\Notepad++\plugins for Windows 32bit.
