### Input group

The Linux "input" group prescribes access to input devices such as keyboards, mice, and touchscreens. By default, these devices are owned by the "root" user and the "input" group, with permissions set to allow read and write access for the owner and group. This means that members of the "input" group can read from and write to these devices, allowing them to interact with the input hardware.

Use the following command to add your user to the "input" group, which will grant you the necessary permissions to access input devices:

``` bash
sudo usermod -aG input $USER
```

It does not take effect until you log out and log back in. After that, you should have the appropriate permissions to access input devices without needing elevated privileges.
