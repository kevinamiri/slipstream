---
title: Introduction
nav_order: 0
---

# Introduction

slipstream is a novel DNS tunnel leveraging the QUIC protocol under the hood.

DNS tunneling is a technique that allows for data to be transmitted over DNS queries and responses.
Such a tunnel can be set up in any network with access to a DNS resolver, even when the client does not have direct access to the internet.
DNS tunneling enables attackers to exfiltrate data, establish Command and Control communication, or bypass network restrictions.
Unfortunately, the DNS protocol is quite limited, with a maximum message size, the use of a request-response model, and employed rate-limiting by resolvers.
To overcome these limitations, tools such as Iodine, or OzymanDNS design custom reliable protocols on top of DNS to allow for streaming continuous data over the tunnel.
However, they are often limited, with throughput often not exceeding several hundred kilobytes per second.
By carefully implementing QUIC compatibility layers, we're able to leverage QUIC's reliability guarantees and introduce a high-performance DNS tunnel.

## Quick start

Follow the [installation](/slipstream/installation/) and [usage instruction](/slipstream/usage/) guides.

## Benchmarks

In our [benchmark](/slipstream/benchmark/), we show that slipstream achieves a 10-fold time decrease in exfiltrating a 10 MB file, while using up to 15% less queries.
For downloading files into the restricted network, slipstream uses up to 37% less queries.
Using QUIC congestion control, it accurately determines the query rate of the used DNS resolver.
Finally, QUIC multipath allows slipstream to combine the bandwidth of multiple resolvers to largely improve its performance, even when those resolvers have different round-trip times, loss rates, or rate limits.
