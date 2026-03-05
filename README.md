# A Slipstream fork/extension (DNS over QUIC over shadowsocks)

What problem does it solve?
- Tunnel traffic is encrypted using Shadowsocks
- More stable than dnstt.

Install build dependencies (Ubuntu/Debian).

Only tested on Ubuntu 24.04

```bash
sudo apt-get update
sudo apt-get install -y \
  python3-pip cmake git pkg-config libssl-dev ninja-build gcc g++
python3 -m pip install --user meson
```

### Configure and build

```bash
~/.local/bin/meson setup build
~/.local/bin/meson compile -C build
```

For optimized release binaries:

```bash
~/.local/bin/meson setup --buildtype=release -Db_lto=true --warnlevel=0 build-release
~/.local/bin/meson compile -C build-release
```

### Verify binaries

```bash
./build/slipstream-server --help
./build/slipstream-client --help
```



DNS server:
```bash
ns.fzserver.com A record to [server ip]
ns.fzserver.com NS record to ns.fzserver.com
```

server side service:

How to create a service like this that service will be restarted every 5 minutes?
```bash
[Unit]
Description=Slipstream DNS Server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
WorkingDirectory=/home/slipstream
ExecStart=/home/slipstream/build/slipstream-server --dns-listen-port=8853 --target-address=127.0.0.1:9090 --domain=ns.fzserver.com
Restart=on-failure
RestartSec=2

[Install]
WantedBy=multi-user.target
```


`9090` is the port of the Shadowsocks server running on the same machine. Slipstream tunnels traffic to this local Shadowsocks instance, which then handles the actual proxy connection.
- `8853` is the port of the Slipstream server.
- `ns.fzserver.com` is the NS domain name of the Slipstream server.
- `127.0.0.1` is the IP address of the machine running the Shadowsocks server.


## Iptables:

```bash
iptables -I INPUT -p udp --dport 8853 -j ACCEPT
iptables -t nat -I PREROUTING -i ens5 -p udp --dport 53 -j REDIRECT --to-ports 8853
```

`ens5` is the network interface name of the machine.


Client side service:
Create a folder named `slipstream` in your home directory and place the `slipstream-client-1.9` binary inside it:


```bash
[Unit]
Description=Slipstream DNS client
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
WorkingDirectory=/home/slipstream
ExecStart=/home/slipstream/slipstream-client-1.9 --resolver=8.8.8.8:53 --resolver=8.8.4.4:53 --domain=ns.fzserver.com --tcp-listen-port=7000
Restart=on-failure
RestartSec=2

[Install]
WantedBy=multi-user.target
```
- `slipstream-client-1.9` is the binary build of the Slipstream client.
- `8.8.8.8` and `8.8.4.4` are the DNS resolvers to use. It can be many as you want.
- `ns.fzserver.com` is the NS domain name of the Slipstream server.
- `7000` is the port of the shadowsocks server.

On xtls:

```json
{
  "protocol": "shadowsocks",
  "settings": {
    "servers": [
      {
        "address": "127.0.0.1",
        "port": 7000,
        "password": "your_password",
        "method": "2022-blake3-aes-256-gcm",
        "uot": true
      }
    ]
  },
  "tag": "xh6jn0dd",
  "streamSettings": {
    "network": "tcp",
    "security": "none",
    "tcpSettings": {
      "header": {
        "type": "none"
      }
    }
  },
  "mux": {
    "enabled": true,
    "concurrency": 8,
    "xudpConcurrency": 16,
    "xudpProxyUDP443": "allow"
  }
}
```

127.0.0.1:7000 is the outbound port of the shadowsocks server.


Add more DNS resolvers will add more speed.

```bash
./slipstream-client --resolver=8.8.8.8:53 --resolver=8.8.4.4:53 --resolver=9.9.9.9:53 --resolver=149.112.112.112:53 --resolver=4.2.2.1:53 --resolver=4.2.2.2:53 --resolver=4.2.2.3:53 --resolver=4.2.2.4:53 --resolver=4.2.2.5:53 --resolver=4.2.2.6:53 --resolver=208.67.222.222:53 --resolver=208.67.220.220:53 --resolver=64.6.64.6:53 --resolver=64.6.65.6:53 --resolver=156.154.70.1:53 --resolver=156.154.71.1:53 --resolver=129.250.35.250:53 --resolver=129.250.35.251:53 --resolver=223.5.5.5:53 --resolver=223.6.6.6:53 --resolver=119.29.29.29:53 --resolver=182.254.116.116:53 --resolver=180.76.76.76:53 --resolver=77.88.8.8:53 --resolver=77.88.8.1:53 --resolver=94.140.14.14:53 --resolver=94.140.15.15:53 --resolver=84.200.69.80:53 --resolver=84.200.70.40:53 --resolver=185.228.168.9:53 --resolver=185.228.169.9:53 --resolver=74.82.42.42:53 --resolver=66.220.18.42:53 --resolver=75.75.75.75:53 --resolver=75.75.76.76:53 --resolver=68.94.156.1:53 --resolver=68.94.157.1:53 --domain=ns.fzserver.com --tcp-listen-port=7000
```


---


## DEV NOTE


2026-03-04
Implemented a patch in [`src/slipstream_server.c`](src/slipstream_server.c) to address the RAM growth causes.

What changed:
- Added server-side `waiting_for_readable` guard on stream context to prevent repeated poller thread creation for the same stream.
- Reset `waiting_for_readable` in poller wake/exit paths so it can be re-armed safely.
- Fixed heap leak in `slipstream_io_copy`: `slipstream_io_copy_args` is now always freed on all exits.
- Hardened idle-socket check (`recv(MSG_PEEK)`) to only treat `EAGAIN/EWOULDBLOCK` when `recv < 0` and avoid stale `errno`.
- Fixed use-after-free risk on connection close callback by caching `thread_ctx` before freeing `server_ctx`.
- Freed `server_domain_name` during server shutdown.

Validation:
- Under stress, thread count still rises during active concurrent streams (expected), but returns to baseline after connections close.
- Temporary test processes were cleaned up.

Important for your service:
- Your systemd unit runs `/usr/local/bin/slipstream-server`, not `./build/slipstream-server`.
- Rebuild/install/deploy the updated binary, then restart service:
1. install updated binary to `/usr/local/bin/slipstream-server`
2. `sudo systemctl restart slipstream-server`
3. monitor with `ps -L -p <pid> ...` and RSS over time to confirm improvement

```bash
# 1) Get the server PID
PID=$(pgrep -f '/usr/local/bin/slipstream-server' | head -n1)
echo "$PID"
```

```bash
# 2) One-time snapshot of all threads in that process
ps -L -p "$PID" -o pid,tid,stat,pcpu,pmem,rss,vsz,wchan:30,comm
```

```bash
# 3) Live monitoring every 2s
watch -n 2 "ps -p $PID -o pid,nlwp,rss,vsz,%cpu,%mem,etime,cmd --no-headers; echo; ps -L -p $PID -o tid,stat,wchan:30 --no-headers | head -n 40"
```

What to watch:
- `nlwp` = thread count
- `rss` = resident RAM (KB)
- `wchan` = where threads are blocked (useful for leak/thread buildup patterns)

If it’s a systemd service, you can also get PID with:
```bash
systemctl show -p MainPID --value slipstream-server
```
#### Deploy the updated binary to production
From project root (`/home/slipstream`), do:

```bash
sudo install -m 0755 ./build/slipstream-server /usr/local/bin/slipstream-server
```

If you want a backup first:

```bash
sudo cp /usr/local/bin/slipstream-server /usr/local/bin/slipstream-server.bak
sudo install -m 0755 ./build/slipstream-server /usr/local/bin/slipstream-server
```

Then restart:

```bash
sudo systemctl restart slipstream-server
sudo systemctl status slipstream-server --no-pager
```


---

2026-03-03
**What has changed**

1. Fixed the assertion crash in SPCDNS:
- Changed class assertion in [codec.c](extern/SPCDNS/src/codec.c:379) from `<= 4` to `<= 65535`.
- This prevents aborting when non-`IN/CH/HS/CS` class queries are re-encoded.

2. Added defensive DNS class filtering in server decode:
- In [slipstream_server.c](src/slipstream_server.c:156), queries with class other than `CLASS_IN` are now refused (`RCODE_REFUSED`) early.
- This avoids feeding unsupported classes into the rest of the pipeline.

3. Hardened fd/pipe lifecycle to reduce `Bad file descriptor` noise:
- Initialized stream descriptors to `-1` in [slipstream_server.c](src/slipstream_server.c:222).
- Closed `fd`, `pipefd[0]`, and `pipefd[1]` safely in [slipstream_server.c](src/slipstream_server.c:265).
- Replaced noisy `printf` on `POLLNVAL` with debug log in [slipstream_server.c](src/slipstream_server.c:390).
- Closed socket on `connect()` failure in [slipstream_server.c](src/slipstream_server.c:438).
- Treated `EPIPE/EBADF/ECONNRESET` on send as normal teardown in [slipstream_server.c](src/slipstream_server.c:472).
- On stream FIN, close socket/pipe safely in [slipstream_server.c](src/slipstream_server.c:588).



----


Configured and tested. `slipstream-server` now restarts every 5 minutes.

./build/slipstream-client --tcp-listen-port 700 --resolver=4.2.2.1:53 --resolver=4.2.2.2:53 --resolver=8.8.4.4:53 --resolver=8.8.8.8:53 --resolver=4.2.2.3:53 --resolver=4.2.2.4:53 --resolver=4.2.2.5:53 --resolver=4.2.2.6:53 --domain ns.fzserver.com
Starting connection to 4.2.2.1

- Timer unit: [/etc/systemd/system/slipstream-server-restart.timer](/etc/systemd/system/slipstream-server-restart.timer)
- Service unit: [/etc/systemd/system/slipstream-server-restart.service](/etc/systemd/system/slipstream-server-restart.service)

Current status:
- `slipstream-server-restart.timer` is `active (waiting)`
- Next run is at the next 5-minute boundary.
- Manual test succeeded: `MainPID` changed from `460496` to `465672`.

Useful checks:
```bash
systemctl status slipstream-server-restart.timer --no-pager
systemctl list-timers --all --no-pager | grep slipstream-server-restart
journalctl -u slipstream-server-restart.service -f
```

If you want to stop this behavior later:
```bash
sudo systemctl disable --now slipstream-server-restart.timer
```

Edit the timer file and change `OnCalendar`.

Current file: [/etc/systemd/system/slipstream-server-restart.timer](/etc/systemd/system/slipstream-server-restart.timer)

Example values:
- Every 2 minutes: `OnCalendar=*:0/2`
- Every 10 minutes: `OnCalendar=*:0/10`
- Every 15 minutes: `OnCalendar=*:0/15`

Then reload and restart timer:

```bash
sudo systemctl daemon-reload
sudo systemctl restart slipstream-server-restart.timer
systemctl status slipstream-server-restart.timer --no-pager
```

If you want, tell me the exact interval and I’ll set it now.
----



![GitHub Release](https://img.shields.io/github/v/release/EndPositive/slipstream?include_prereleases&sort=semver&display_name=tag)
![GitHub License](https://img.shields.io/github/license/EndPositive/slipstream)

A high-performance covert channel over DNS, powered by QUIC multipath.

<p align="center">
  <picture align="center">
    <source media="(prefers-color-scheme: dark)" srcset="docs/assets/file_transfer_times_dark.png">
    <source media="(prefers-color-scheme: light)" srcset="docs/assets/file_transfer_times_light.png">
    <img alt="Shows a bar chart with benchmark results." src="docs/assets/file_transfer_times_light.png">
  </picture>
</p>

<p align="center">
  <i>Exfiltrating a 10 MB file over a single DNS resolver.</i>
</p>

## Highlights

* Adaptive congestion control for rate-limited resolvers
* Parallel routing over multiple multiple rate-limited resolvers
* 60% lower header overhead than DNSTT

## Installation

Get the latest binaries [GitHub releases](https://github.com/EndPositive/slipstream/releases/latest) or pull the latest version from the [GitHub Container Registry](https://github.com/users/EndPositive/packages?repo_name=slipstream).

## Documentation

slipstream's documentation is available at [endpositive.github.io/slipstream](https://endpositive.github.io/slipstream/).

# Acknowledgements

David Fifield's DNSTT and Turbo Tunnel concept has been a massive source of inspiration.
Although slipstream inherits no code, this work could not have been possible without his ideas.
