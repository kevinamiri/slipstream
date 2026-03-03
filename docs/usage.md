---
title: Usage instructions
nav_order: 30
---

# Usage instructions

slipstream is designed to tunnel TCP traffic over DNS messages.
Since DNS is a distributed system, we can abuse existing DNS infrastructure in the tunnel.
For example, the figure below shows multiple network routes using a public DNS resolver to pass through the local or country scoped firewall.
This is especially useful when a network mandates the use of a DNS resolver as assigned in the DHCP configuration.

![DNS tunnel network setup](/slipstream/assets/network.png)

slipstream consists of a server and client binary.

#### Server

The server is the one to be placed on the outside of the restricted network.
It will act as the authoritative nameserver for a given domain.
It will forward received connections to a TCP service specified in the CLI arguments.

```shell
$ slipstream-server
  --target-address=x.x.x.x:yy \ # TCP address of the service to access
  --domain=test.com
```

#### Client

The client is placed inside the restricted network.


```shell
$ slipstream-client \
  --resolver-address=x.x.x.x:yy \ # Address of public DNS resolver or DHCP assigned resolver
  --domain=test.com
```


### Configuration of DNS records

Assumming you own `test.com`, you should configure the DNS records such that your slipstream server is configured as the authoritative nameserver of that domain.
For example, add a NS record for `test.com` pointing to `ns.test.com`.
Then add an A record on `ns.test.com` pointing to your slipstream server IP.

```
@   IN  NS  ns.test.com.
ns  IN  A   x.x.x.x:yy ; # Address of slipstream server
```

### Direct connection

It is also possible to setup a direct connection between the client and the server.
This allows to impersonate DNS traffic on port 53 without actually using any public infrastructure.
This is a similar trick to using WireGuard on port 53, additionally encoding as DNS traffic.

```shell
$ slipstream-client \
  --congestion-control=bbr \ # Faster better than dcubic in direct connections
  --resolver-address=x.x.x.x:yy \ # Address of slipstream server
  --domain=test.com
```

## Example data transfer

An example of a sending data from the client to the server over a direct slipstream connection.

```shell
$ nc -l -p 5201
$ slipstream-server \
  --dns-listen-port=8853 \
  --target-address=127.0.0.1:5201 \
  --domain=test.com
```

```shell
$ slipstream-client \
  --congestion-control=bbr \
  --tcp-listen-port=7000 \
  --resolver=127.0.0.1:8853 \
  --domain=test.com
Adding 127.0.0.1:8853
Starting connection to 127.0.0.1
Initial connection ID: 54545454
Listening on port 7000...
Connection completed, almost ready.
Connection confirmed.
```

You can then connect to the slipstream client on port 7000 as if you were connecting to the nc client on port 5201.

```shell
$ base64 /dev/urandom | head -c 5000000 | nc 127.0.0.1 7000

# slipstream client wakes up
[0:9] accept: connection
[0:9] wakeup
[0:9] activate: stream
[0:9] recv->quic_send: empty, disactivate
[0:9] wakeup
[0:9] activate: stream
[0:9] recv->quic_send: empty, disactivate
[0:9] wakeup
[0:9] activate: stream
[0:9] recv->quic_send: empty, disactivate
[0:9] recv: closed stream
```
```shell
# base64 data arrives on the server
S9w3u5up+c39u6vrkBtxKbSxOJA2UElczDgc3x4h3TtZtzvgMX05Ig4whEYDvY5MP8g4dJ1QsXX1
fSDm0y6mOlQ4fQhYchkyKt18fV0tpBkLrPwv6MkW+IaksKe7Qo61s3gxu2jrPBlC1yxML+rYZU93
MYNB7rFC6s3a0eHmfdsfbtBbFIF809X91fqd6gYiKPtWAHc0J5OsEyqMI3QcUGSDJd4Sw+iAC5X7
```
