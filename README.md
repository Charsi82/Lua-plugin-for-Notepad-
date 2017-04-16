# Lua plugin for Notepad++
  
* Syntax verification of Lua scripts.
 
* Run the script from the current file.
* Print result in output console.

 Version Lua - **5.3.4**  (http://www.lua.org).
 
 Additional libs:
	- lfs - ver. 1.6.3 Copyright Kepler Project 2003 (http://www.keplerproject.org/luafilesystem)	

 Changes in Lua:
 - string.format can work with pattren %b for boolean values  
   (for example, print(string.format("%b_%b"), true, false) --> "true_false")
 - printf(fmt,...) same as print(string.format(fmt,...))
 - clear_console() - clear output console
 - show_console() - show console if it's hidden
 - messagebox (text[, caption[, wnd_type]] ) - show message box.  
 Caption is empty as default. Integer value wnd_type may be in interval 0..6.
 Result the function is the code of the pressed button.

# How to use  
 - Copy **lua_npp_lang.ini** and **lua_npp.dll** in plugins folder for N++.  
 Open a Lua file and goto menu Plugins - Lua utils - Verify syntax. Verification result can be seen in the console.  
 - You can switch language of interface between english and russian.  
 - To quickly close the console, you can double-click on it ;)

# Requirements
  vcredist2013 (https://www.microsoft.com/ru-RU/download/details.aspx?id=40784)

# Download (x86 and x64)
  link - https://yadi.sk/d/UIMIb8_8vZtMD
