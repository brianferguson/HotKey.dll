HotKey.dll
=============

HotKey.dll is a plugin for [Rainmeter](http://www.rainmeter.net) that will run an [Action](#options) when a key (or combination of keys) are pressed and released. The plugin can also determine the "state" of 3 toggle keys (Caps Lock, Scroll Lock, and Num Lock).

#####Note:
This plugin requires Rainmeter version 3.0.2 (r2161) or higher. Make sure to set the "Minimum Rainmeter Version" to `3.0.2.2161` in the [Skin Packager](http://docs.rainmeter.net/manual/publishing-skins).


Contents
-
* [Features](#features)
* [Options](#options)
* [Pre-defined HotKey Keywords](#pre-defined-hotkey-keywords)
* [Changes](#changes)
* [Download](#download)
* [Build Instructions](#build-instructions)
* [Examples](#examples)


Features
-
Here are some of the features of the HotKey plugin:

* Performs an [action](http://docs.rainmeter.net/manual-beta/skins/option-types#Action) once the hot key is pressed and released.
* Can get the status (on or off) of the 3 toggle keys (Caps Lock, Scroll Lock, Num Lock). The [number value](http://docs.rainmeter.net/manual-beta/measures#Values) of the plugin will be `1` when the toggle key is in the "on" state, and `0` when in the "off" state. To use the special toggle cases, add the word "Status" after the key. Example: `HotKey=CapsLock Status`. 
* Hotkeys can be a letter, number, or the [pre-defined keywords](#pre-defined-hotkey-keywords). You can represent any keyboard key by using its number equivilant (in either hex, octal, binary or base 10). See the list [here](http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx). Example: `HotKey=Shift 0x41` which translates to "SHIFT A"

#####Notes:
* If `SHIFT`/`CTRL`/`ALT` is used with its L/R variations, the L/R will be ignored.
* There are only 3 special toggle cases: `CapsLock Status`, `ScrollLock Status`, and `NumLock Status`. The [number value](http://docs.rainmeter.net/manual-beta/measures#Values) of the plugin will be `1` when the toggle key is in the "on" state, and `0` when in the "off" state.
* The [number value](http://docs.rainmeter.net/manual-beta/measures#Values) of the plugin will always be `0` except in the special toggle cases.
* The `Fn` on some laptop keyboards cannot be detected.
* The mouse button's will not work by themselves, they require another non-mouse key to be used in combination with the mouse button.
* On some keyboards, when NumLock is off, the keys will represent other keys (usually the navigation keys, like "Home").
* On some keyboards, when `SHIFT` is used with a Numeric Keypad key, the HotKey may not work. Example: `HotKey=Shift Num6` will not work because the plugin thinks the SHIFT and Numpad 6 need to be pressed, while the system thinks you pressed the Right Arrow key.
* There may be cases where an elvated process will "block" the plugin from seeing a key being pressed.
* Due to the plugin needing to compare which keys were pressed vs. which keys trigger an action, there is a slight chance that some hot keys will not be detected correctly when typing really fast or having a large amount of HotKey measures.


Options
-
* **HotKey** - Key or combination of keys (separated by a space) needed to be pressed and release to run the Action.
* **Action** - Action to be taken when the right HotKey has been pressed and released.


Pre-defined HotKey Keywords
-
* **Mouse Buttons** - `LBUTTON`, `RBUTTON`, `MBUTTON`, `XBUTTON1`, `XBUTTON2` (Note: These can only be used in combination with other non-mouse keys.)
* **Numeric Keypad** - `NUM0`, `NUM1`, `NUM2`, `NUM3`, `NUM4`, `NUM5`, `NUM6`, `NUM7`, `NUM8`, `NUM9`, `MULT`, `ADD`, `SUBTRACT`, `DECIMAL`, `DIVIDE`
* **Navigation/Arrows** - `PAGEUP`, `PAGEDOWN`, `END`, `HOME`, `LEFT`, `UP`, `RIGHT`, `DOWN`, `INSERT`, `DELETE`, `PAUSE`(Break), `PRINTSCREEN`(Sys Req.)
* **F Keys** - `F1`, `F2`, ... `F24`
* **Keyboard** - `BACKSPACE`, `TAB`, `ENTER`(Return), `CAPSLOCK`, `NUMLOCK`, `SCROLLLOCK`, `ESCAPE`, `SPACE`, `LWIN`, `RWIN`, `MENU`(next to Windows key), `SHIFT`, `LSHIFT`, `RSHIFT`, `CTRL`, `LCTRL`, `RCTRL`, `ALT`, `LALT`, `RALT`, `COLON`(;:), `PLUS`(=+), `MINUS`(-_), `COMMA`(,<), `PERIOD`(.>), `FORWARDSLASH`(/?), `BACKSLASH`(\|), `BACKTICK`((&#x60;~), `LBRACKET`([{), `RBRACKET`(]}), `QUOTE`('")


Changes
-
Here is a list of the major changes to the plugin.

#####Version:
* **0.0.0.1** - Initial Beta Version.


Download
-
####To download the current source code:

* Using git: `git clone git@github.com:brianferguson/HotKey.dll.git`
* Download as a [.zip](https://github.com/brianferguson/HotKey.dll/zipball/master)

####To download current plugin (.dll):

* [32-bit version](https://github.com/brianferguson/HotKey.dll/blob/master/PluginHotKey/x32/Release/HotKey.dll?raw=true)
* [64-bit version](https://github.com/brianferguson/HotKey.dll/blob/master/PluginHotKey/x64/Release/HotKey.dll?raw=true)


Build Instructions
-
This plugin can be built using any version of Visual Studio 2013. If you don't already have VS2013, you can download the free "Visual Studio Express 2013 for Windows Desktop" version [here](http://www.visualstudio.com/downloads/download-visual-studio-vs).

After Visual Studio has been installed and updated, open `HotKey.sln` at the root of the repository to build.


Examples
-

####Example 1:
The status of the 3 toggle keys are at the top of the skin. To change the background color, press and release the hot keys.

```ini
[Rainmeter]
Update=1000
BackgroundMode=2
SolidColor=#BackgroundColor#

[Variables]
BackgroundColor=50,50,50,255
Key1=CTRL ALT PAGEUP
Key2=RCTRL Num6
Key3=SHIFT B
Key4=LWIN BACKSPACE
Key5=MBUTTON SCROLLLOCK

;For the toggle keys, the "Action" forces Rainmeter to update the measure so that the IfConditions
;	get evaluated, else the IfConditions would not evaluate until the next update cycle.
[CapsLock]
Measure=Plugin
Plugin=HotKey
HotKey=CapsLock Status
Action=!UpdateMeasure CapsLock
IfCondition=CapsLock = 1
IfTrueAction=[!SetOption CapsMeter FontColor "255,0,0,255"][!UpdateMeter CapsMeter][!Redraw]
IfFalseAction=[!SetOption CapsMeter FontColor "255,255,255,255"][!UpdateMeter CapsMeter][!Redraw]

[ScrollLock]
Measure=Plugin
Plugin=HotKey
HotKey=ScrollLock Status
Action=!UpdateMeasure ScrollLock
IfCondition=ScrollLock = 1
IfTrueAction=[!SetOption ScrollMeter FontColor "255,0,0,255"][!UpdateMeter ScrollMeter][!Redraw]
IfFalseAction=[!SetOption ScrollMeter FontColor "255,255,255,255"][!UpdateMeter ScrollMeter][!Redraw]

[NumLock]
Measure=Plugin
Plugin=HotKey
HotKey=Numlock Status
Action=!UpdateMeasure NumLock
IfCondition=NumLock = 1
IfTrueAction=[!SetOption NumMeter FontColor "255,0,0,255"][!UpdateMeter NumMeter][!Redraw]
IfFalseAction=[!SetOption NumMeter FontColor "255,255,255,255"][!UpdateMeter NumMeter][!Redraw]

[Red]
Measure=Plugin
Plugin=HotKey
HotKey=#Key1#
Action=[!SetOption Background SolidColor "255,0,0,255"][!UpdateMeter Background][!Redraw]

[Green]
Measure=Plugin
Plugin=HotKey
HotKey=#Key2#
Action=[!SetOption Background SolidColor "0,255,0,255"][!UpdateMeter Background][!Redraw]

[Blue]
Measure=Plugin
Plugin=HotKey
HotKey=#Key3#
Action=[!SetOption Background SolidColor "0,0,255,255"][!UpdateMeter Background][!Redraw]

[Black]
Measure=Plugin
Plugin=HotKey
HotKey=#Key4#
Action=[!SetOption Background SolidColor "0,0,0,255"][!UpdateMeter Background][!Redraw]

[Reset]
Measure=Plugin
Plugin=HotKey
HotKey=#Key5#
Action=[!SetOption Background SolidColor "#BackgroundColor#"][!UpdateMeter Background][!Redraw]

[Style]
FontColor=255,255,255,255
X=18R
Y=5
DynamicVariables=1
AntiAlias=1

[Style1]
X=5
Y=2R
FontSize=8

[CapsMeter]
Meter=String
MeterStyle=Style
Text=[Caps]

[ScrollMeter]
Meter=String
MeterStyle=Style
Text=[Scroll]

[NumMeter]
Meter=String
MeterStyle=Style
Text=[Num]

[Background]
Meter=Image
SolidColor=#BackgroundColor#
X=0
Y=5R
W=200
H=90

[Key1Label]
Meter=String
MeterStyle=Style | Style1
Text=Red: #Key1#
Y=5r

[Key2Label]
Meter=String
MeterStyle=Style | Style1
Text=Green: #Key2#

[Key3Label]
Meter=String
MeterStyle=Style | Style1
Text=Blue: #Key3#

[Key4Label]
Meter=String
MeterStyle=Style | Style1
Text=Black: #Key4#

[Key5Label]
Meter=String
MeterStyle=Style | Style1
Text=Reset: #Key5#
```
