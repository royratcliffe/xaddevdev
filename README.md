### Input group

The Linux "input" group prescribes access to input devices such as keyboards, mice, and touchscreens. By default, these devices are owned by the "root" user and the "input" group, with permissions set to allow read and write access for the owner and group. This means that members of the "input" group can read from and write to these devices, allowing them to interact with the input hardware.

Use the following command to add your user to the "input" group, which will grant you the necessary permissions to access input devices:

``` bash
sudo usermod -aG input $USER
```

It does not take effect until you log out and log back in. After that, you should have the appropriate permissions to access input devices without needing elevated privileges.

## Hot Plugging

The implementation takes a causiously pragmatic approach to handling the case
where an input device is not initially plugged in. When the program starts, it
sets up an inotify watch on the `/dev/input` directory to monitor for new device
files. When a new device appears, Linux will first create its corresponding
`/dev/input` entry: a character device file, e.g. `/dev/input/event0`.

Linux then changes the file's attributes to make it owned by the "input" group
and readable-writable by that group. The `xaddevdev` program detects the new device file,
and then waits for it to become accessible before trying to read from it. This
allows the program to handle devices that are plugged in *after* it has started,
without needing to restart the program or manually trigger a rescan of input
devices.

There is a trick, however. It takes a moment for the device file to become
accessible after it is created. The kernel *asynchronously* updates the file's
permissions in a separate step after creating it, and there is a small window of
time where the file exists but is not yet readable/writable by the "input"
group. The program sees the file system events for the new device file
asynchronously as well. If the program tries to read from the device file before
it becomes accessible, it fails with a permission error.
