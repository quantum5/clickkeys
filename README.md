# clickkeys

A tool to use keyboard buttons to click the mouse, for when your mouse button is broken.

Sometimes, your mouse button does not register when clicked, and sometimes it registers
as multiple separate clicks. This can be annoying, but you may not be able to replace it
right away. This program solves the issue.

## Features

![clickkeys about dialog](https://quantum2.xyz/wp-content/uploads/2017/07/clickkeys.png)

* A press of the pause/break key is translated into a single instaneous click of the primary button (usually left).
    * This is helpful to click without stopping the mouse.
    * <kbd>**⊞**</kbd>+<kbd>Alt</kbd>+<kbd>Shift</kbd>+<kbd>M</kbd> can be used to toggle this feature.
* The context menu key press/release is directly translated into press/release of the primary mouse button.
    * This is helpful for dragging.
    * <kbd>Alt</kbd>+<kbd>Shift</kbd>+<kbd>M</kbd> can be used to toggle this feature.
* <kbd>Ctrl</kbd>+<kbd>Alt</kbd>+<kbd>Shift</kbd>+<kbd>M</kbd> can be used to toggle this feature.

## Compliation

Using Visual C++ command line:

```
$ nmake
```

The resulting files will be produced in `clickkeys.exe`.
