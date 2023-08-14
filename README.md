# IntelligentTB

Windows application to improve the automatic hide feature of the windows taskbar.
The taskbar will only be hidden when the current window intersects with it.
Otherwise the taskbar is shown like normal.
Inspired by [SmartTaskbar](https://github.com/ChanpleCai/SmartTaskbar). Written
completely using native Win32 API. The binary itself is only 20 KB in size and uses
about 1 MB of memory while running.

Tested on: Windows 11 23H2 (22631.2048)

## Features

- Low CPU/Memory usage
- Configurable via settings file
- Simple interface via tray icon

When right-clicking on the tray icon the following options are present:

- Disable/Enable: Temporary toggle for the application
- Settings: Opens the settings file (Promts to create a new one if it can't find)
- About: Opens the project Github-Page (this)
- Exit: Fully closes the application

## Settings

All settings are loaded at the start of the program from a `settings.ini` file.
This file has to be in the same folder as the executable. When the program
can't find the file, it uses default hardcoded settings. To adjust them,
click on the *Settings* entry in the tray menu. If no config file exists,
it will ask you to create one. After pressing *Yes* a editor with the file
should open up. The contents will look like the following:

```ini
[Settings]
TimerMs=200
Blacklist=Progman,XamlExplorerHostIslandWindow,Shell_TrayWnd,TopLevelWindowForOverflowXamlIsland,Windows.UI.Core.CoreWindow,WindowsDashboard,WorkerW
Whitelist=Shell_TrayWnd,MSTaskSwWClass,TrayNotifyWnd
EfficiencyMode=1
AutoStart=1
```

With the `TimerMs` setting can the update interval adjusted. This is the
frequency how often the program refreshes the state of the taskbar.
In `Blacklist` is a list of window classes that when in **foreground** and
intersecting should not hide the taskbar. `Whitelist` contains a list of 
class names for Windows that when **hovered** should show the taskbar. 
This whitelist for example is used to keep the taskbar shown while it is 
hovered with the mouse. Otherwise it would show and hide again while another
window intersects with it. With `EfficiencyMode` it is possible to
reduce the process priority and enable the new [EcoQoS](https://devblogs.microsoft.com/performance-diagnostics/introducing-ecoqos/)
feature. Enabling `AutoStart` will create a entry in the Registry
at `HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Run`.
This will enable the appilcation to launch when the computer starts.
Disabling this setting will remove the entry on the next start of
the application.