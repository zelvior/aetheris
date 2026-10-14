# Aetheris Logging & Telemetry Specification (v1.0)

`std.log` provides structured, low-overhead logging; `std.telemetry` provides metrics and distributed tracing compatible with OpenTelemetry.

---

## 1. Log Levels

| Level | Use |
| :--- | :--- |
| `trace` | Very fine-grained debugging |
| `debug` | Developer diagnostics |
| `info` | Normal operational events |
| `warn` | Recoverable anomalies |
| `error` | Failures requiring attention |

Levels below the compile-time minimum (`--log-min=info`) are removed from the binary entirely. The runtime level is set via `AETH_LOG=debug` or `AETH_LOG=net=trace,db=warn` for per-module control.

---

## 2. Structured Logging

```aeth
import { log } from "std.log" ~

fn handleLogin(user: text, ok: bool) ~
    log.info("login attempt", user: user, success: ok, ip: request.ip) ~
    if not ok ~
        log.warn("login failed", user: user) ~
    end
end
```

Output formats:

```text
2026-10-14T09:30:11.482Z INFO  auth  login attempt user="ada" success=true ip="203.0.113.7"
```

```json
{"ts":"2026-10-14T09:30:11.482Z","level":"info","module":"auth","msg":"login attempt","user":"ada","success":true}
```

`secret[T]` values are rejected by logging calls; fields named `password`, `token`, or `authorization` are redacted automatically.

---

## 3. Sinks

| Sink | Targets | Notes |
| :--- | :--- | :--- |
| `stderr` / `stdout` | Native | Default; auto color when TTY |
| File | Native-64 | Rotation by size or time, optional compression |
| `syslog` / `journald` | Linux | Priority mapping |
| Browser console | Web | Level mapped to `console.*` |
| UART / SWO | native-32 | Ring buffer drained in idle loop |
| OTLP exporter | native-64, web | Sends logs to a collector |

```aeth
log.configure(Config {
    format: Json,
    sinks: [Sink.stderr(), Sink.file("/var/log/app.log", rotate: 50.mb)],
    level: Level.Info,
}) ~
```

### Embedded Logging

On `native-32`, `log.info("temp", value: t)` is compiled to a compact binary record (message ID + fields, no format strings stored on device). The `aeth log decode` tool reconstructs text using the build's `.logmap` file, keeping flash usage minimal.

---

## 4. Metrics

```aeth
import { telemetry } from "std.telemetry" ~

let requests = telemetry.counter("http_requests_total", labels: ["route", "status"]) ~
let latency  = telemetry.histogram("http_latency_ms", buckets: [5, 10, 50, 100, 500]) ~

requests.inc(route: "/api/v1/users", status: 200) ~
latency.observe(12.4) ~
```

| Instrument | Description |
| :--- | :--- |
| `counter` | Monotonic count |
| `gauge` | Value that goes up and down |
| `histogram` | Distribution with buckets |

Metrics are exposed via a Prometheus endpoint (`telemetry.serve(9090)`) or pushed through OTLP.

---

## 5. Tracing

```aeth
async fn fetchOrder(id: u64) -> Result[Order, text] ~
    span "fetchOrder" with attrs(orderId: id) ~
        let row = check await db.orders.find(id) ~
        return ok(row) ~
    end
end
```

Spans nest automatically across `await` points. Trace context propagates through HTTP headers using W3C `traceparent`. Sampling is configured with `telemetry.sample(rate: 0.05)`.

---

## 6. Privacy & Consent

* Telemetry about the application is **opt-in by the application developer**.
* The Aetheris toolchain itself collects **no usage data** unless the user explicitly runs `aeth telemetry enable`.
* PII scrubbing rules can be attached to any exporter.

## 7. Performance

Log calls on disabled levels cost a single branch. Enabled calls write into per-thread buffers flushed asynchronously; the hot path performs no heap allocation.
