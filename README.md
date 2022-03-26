# RaiseExplorer

A small process that sits in the background and waits for you to hit Ctrl+Shift+E. Once you do, it finds all explorer windows on the current virtual desktop and brings them to the front all at once.

There's no UI. To install it, compile it and place the executable somewhere. Then open 'shell:startup' with the run dialog (Windows Key + R). Place a shortcut to the executable in that directory and Windows will start it up on boot. To quit you kill it with the task manager.

## Why?

When working with files I find myself with multiple explorer windows open, and I want to interact with all of them at once. I wrote this code to avoid having to activate multiple windows from the taskbar.