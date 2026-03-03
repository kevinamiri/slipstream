# Slipstream fork (DNS over QUIC over shadowsocks)


DNS server:
```bash
ns.fzserver.com A record to [server ip]
ns.fzserver.com NS record to ns.fzserver.com
```







create a folder inside home directory called slipstream and put the slipstream-client-1.9 inside it.

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
