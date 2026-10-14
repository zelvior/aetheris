# Aetheris Data Streaming & Pipelines Specification (v1.0)

Aetheris provides typed streams and a pipeline operator (`|>`) for processing data incrementally with bounded memory and built-in backpressure.

---

## 1. Core Abstractions

| Type | Description |
| :--- | :--- |
| `Stream[T]` | Asynchronous sequence of `T` values that can fail |
| `Source[T]` | Producer of a stream (file, socket, sensor, channel) |
| `Sink[T]` | Consumer of a stream |
| `Channel[T]` | Bounded multi-producer, multi-consumer queue |

All stream operations return `Result` at the failure points; a stream ends with `None`/EOF or an `err`.

---

## 2. The Pipeline Operator `|>`

`a |> f(args)` passes `a` as the first argument of `f`. Pipelines are lazy and run when attached to a sink or consumed with `await`.

```aeth
async fn countErrors(path: text) -> Result[u64, text] ~
    let n = check await source.lines(path)
        |> filter(fn(line) ~ line.contains("ERROR") end)
        |> count() or err("read failed") ~
    return ok(n) ~
end
```

Precedence: `|>` binds looser than `or` and `+`, tighter than assignment.

---

## 3. Stream Operators

| Operator | Description |
| :--- | :--- |
| `map(f)` | Transform each item |
| `filter(pred)` | Keep matching items |
| `flatMap(f)` | Map to streams and flatten |
| `batch(n)` / `window(size, step)` | Group items |
| `take(n)` / `skip(n)` | Limit or skip items |
| `chunk(bytes)` | Re-chunk byte streams |
| `fold(init, f)` | Reduce to a value |
| `merge(other)` / `zip(other)` | Combine streams |
| `throttle(rate)` / `debounce(dur)` | Rate control |
| `parallel(n, f)` | Process items on `n` workers, order-preserving |
| `tee()` | Duplicate stream |
| `retry(policy)` | Re-run failed stages |
| `timeout(dur)` | Fail if no item arrives |

---

## 4. Sources and Sinks

| Sources | Sinks |
| :--- | :--- |
| `source.file`, `source.lines` | `sink.file` |
| `source.tcp`, `source.udp`, `source.ws` | `sink.tcp`, `sink.ws` |
| `source.http(url)` | `sink.http(url)` |
| `source.stdin` | `sink.stdout` |
| `source.uart`, `source.adc`, `source.i2c` (native-32) | `sink.uart`, `sink.dac` |
| `source.channel(ch)` | `sink.channel(ch)` |
| `source.db(query)` | `sink.db(table)` |
| `source.csv`, `source.jsonl` | `sink.csv`, `sink.jsonl` |

---

## 5. Backpressure & Memory

Streams are **pull-based with bounded buffers**: a slow sink automatically slows the source.

```aeth
let ch = Channel[Event].bounded(1024) ~     // producers await when full
```

| Policy | Behavior when buffer is full |
| :--- | :--- |
| `Block` (default) | Producer waits |
| `DropNewest` | Discard incoming item |
| `DropOldest` | Evict oldest item |
| `Error` | Return `err(Overflow)` |

Buffer sizes are static where possible, so `native-32` pipelines allocate once at startup using fixed pools. The compiler reports the worst-case pipeline memory (`--report-memory`).

---

## 6. Example: ETL Pipeline

```aeth
@derive(Json)
struct Reading ~ sensor: text ~ value: f32 ~ ts: i64 ~ end

async fn run() -> Result[unit, text] ~
    check await source.jsonl[Reading]("readings.jsonl")
        |> filter(fn(r) ~ r.value > 0.0 end)
        |> window(size: 60, step: 60)
        |> map(averageWindow)
        |> compress.zstd(level: 3)
        |> sink.file("hourly.avg.zst") ~
    return ok(()) ~
end
```

## 7. Example: Real-Time Sensor Loop (native-32)

```aeth
fn main() ~
    source.adc(channel: 3, rate: 1000)
        |> window(size: 16, step: 1)
        |> map(movingAverage)
        |> sink.uart(0) ~
end
```

Embedded pipelines execute without heap allocation and are driven by timer interrupts instead of an async executor.

---

## 8. Errors & Cancellation

* An `err` in any stage terminates downstream stages and propagates to the awaiting caller.
* `recover(fn(e) ~ ... end)` can replace errors with fallback items.
* Cancel by dropping the stream handle or calling `handle.cancel()`; sources release resources deterministically.
* `checkpoint(store)` stores offsets so long-running pipelines resume after restart (at-least-once; use `dedupe(key)` for effectively-once semantics).

## 9. Observability

Each stage exposes counters (items in/out, latency, queue depth) through `std.telemetry` (see `LOGGING_AND_TELEMETRY.md`). `aeth pipeline graph` renders the stage graph as DOT or Mermaid.
