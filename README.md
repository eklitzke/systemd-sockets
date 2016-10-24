# Systemd Socket Activation Test

This is a test I did to see what happens with systemd socket activation when
trying to activate on multiple ports.

The C++ program here will try to bind on ports 8000, 8001, and 8002. It can also
accept the sockets via systemd socket activation.

To set it up initially:

```bash
mkdir -p ~/.config/systemd/user
cp systemd-sockets.{service,socket} ~/.config/systemd/user
systemctl --user daemon-reload
systemctl --user start systemd-sockets.socket
systemctl --user start systemd-sockets
journalctl --user -u systemd-sockets
```

Now you can verify that the service inherited the expected ports. Now if you add
8002 to `~/.config/systemd/user/systemd-sockets.socket` and run:

```bash
# This will fail
systemctl --user restart systemd-sockets.socket
```

However, this will work:

```bash
# This will work
systemctl --user stop systemd-sockets
systemctl --user restart systemd-sockets.socket
systemctl --user start systemd-sockets
```
