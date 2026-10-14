# Aetheris Backend Development Guide (v1.0)

This guide covers building HTTP services, APIs, background jobs, and data access with Aetheris `route` declarations and the async runtime. See `IO.md`, `ERROR_HANDLING.md`, and `SYSTEM_CALLS_AND_OS_INTERACTION.md` for the underlying subsystems.

---

## 1. Project Setup

```bash
aeth new my_api --template=api
cd my_api
aeth run src/main.aeth            # interpreter, fast iteration
aeth dev                          # hot reload
aeth build --target=native-64 --release -o my_api
```

```
my_api/
├── aeth.toml
├── src/
│   ├── main.aeth
│   ├── routes/
│   ├── models/
│   └── db/
├── migrations/
└── tests/
```

---

## 2. Routes

```aeth
route GET "/api/v1/health" ~
    reply { status: "ok" } with status 200 ~
end

route GET "/api/v1/users/:id" ~
    let id = check request.params.id.toU64() or reply "Invalid id" with status 400 ~
    let user = check await db.users.findById(id) or reply "User not found" with status 404 ~
    reply user with status 200 ~
end

route POST "/api/v1/users" ~
    let body = check json.decode[NewUser](request.body) or reply "Invalid JSON" with status 400 ~
    let id = check await db.users.insert(body) or reply "Failed to persist user" with status 500 ~
    reply { id: id } with status 201 ~
end
```

Every code path in a `route` must `reply`; the compiler rejects routes that can fall through (`L401`).

### Request Object

| Member | Description |
| :--- | :--- |
| `request.method`, `request.path` | HTTP method and path |
| `request.params` | Path parameters |
| `request.query` | Query string map |
| `request.headers` | Case-insensitive header map |
| `request.body` | Raw `text`; decode with `json.decode[T]` |
| `request.ip`, `request.id` | Client address and request ID |

### Reply Forms

```aeth
reply value with status 200 ~
reply value with status 200 headers { "Cache-Control": "max-age=60" } ~
reply file("report.pdf") with status 200 ~
reply stream(source) with status 200 ~
redirect "/login" with status 302 ~
```

---

## 3. Middleware

```aeth
middleware requireAuth ~
    let token = check request.headers.get("authorization") or reply "Unauthorized" with status 401 ~
    let user = check await auth.verify(token) or reply "Invalid token" with status 401 ~
    request.ctx.user = user ~
    next() ~
end

group "/api/v1/admin" with [requireAuth, rateLimit(100, per: 1m)] ~
    route GET "/stats" ~ reply await stats.collect() with status 200 ~ end
end
```

Built-in middleware: `cors`, `rateLimit`, `compress`, `requestLog`, `timeout`, `bodyLimit`, `securityHeaders`.

---

## 4. Server Startup

```aeth
import { server } from "std.net.http" ~

async fn main() -> Result[unit, text] ~
    let cfg = check loadConfig("config.toml") or err("config load failure") ~
    log.info("starting", port: cfg.port) ~

    check await db.connect(cfg.databaseUrl) or err("database unavailable") ~
    check await server.serve(port: cfg.port, tls: cfg.tls) ~
    return ok(()) ~
end
```

Graceful shutdown on SIGTERM is automatic: the server stops accepting connections, drains in-flight requests (default 10 s), then exits (see `PROCESS_SIGNAL_HANDLING.md`).

---

## 5. Data Access

```aeth
@derive(Json, Row)
struct User ~
    id: u64 ~
    name: text ~
    email: text ~
end

let adults = check await db.query[User]("SELECT * FROM users WHERE age >= ?", [18]) ~
```

* Queries use **bound parameters only**; string concatenation into SQL is a lint error (`L106`).
* Supported drivers: PostgreSQL, SQLite, MySQL; Redis for caching.
* Migrations: `aeth migrate new add_users`, `aeth migrate up`.
* Transactions:

```aeth
check await db.transaction(fn(tx) ~
    check await tx.exec("UPDATE accounts SET bal = bal - ? WHERE id = ?", [amt, from]) ~
    check await tx.exec("UPDATE accounts SET bal = bal + ? WHERE id = ?", [amt, to]) ~
    return ok(()) ~
end) ~
```

---

## 6. Concurrency & Background Work

```aeth
async fn sendWelcomeEmail(user: User) ~ ... end

route POST "/signup" ~
    let user = check await createUser(request.body) or reply "Signup failed" with status 500 ~
    spawn sendWelcomeEmail(user) ~                 // fire-and-forget task
    reply user with status 201 ~
end

schedule every 1h ~
    await cleanupExpiredSessions() ~
end
```

Shared mutable state requires explicit synchronization (`Mutex[T]`, `Atomic[T]`, or channels). Per-request arenas (see `CUSTOM_MEMORY_ALLOCATOR.md`) are used automatically for request-scoped allocation.

---

## 7. Security Checklist

| Practice | How |
| :--- | :--- |
| Input validation | Typed `json.decode[T]` with `validate` attributes |
| Authentication | Token verification middleware; hashing with `kdf.argon2id` |
| Transport | TLS 1.3 via `server.serve(tls: ...)` |
| Headers | `securityHeaders` middleware (HSTS, CSP, nosniff) |
| Limits | `bodyLimit`, `timeout`, `rateLimit` |
| Secrets | `secret[text]` type, loaded from environment or vault |
| Dependencies | `aeth audit` in CI |

See `CRYPTOGRAPHY_AND_SECURITY.md` for details.

---

## 8. Observability & Testing

```aeth
test "GET /health returns ok" ~
    let res = await testClient.get("/api/v1/health") ~
    assert res.status == 200 ~
end
```

* Request logs, metrics, and traces are wired to `std.log` and `std.telemetry` (see `LOGGING_AND_TELEMETRY.md`).
* `aeth test --integration` starts the app on an ephemeral port with a test database.

## 9. Deployment

| Option | Command |
| :--- | :--- |
| Static binary | `aeth build --target=native-64 --static --release` |
| Container | `aeth container build` (distroless image, typically under 15 MB) |
| WASI / edge | `aeth build --target=wasi` |
| Systemd | `aeth service install` generates a unit file |

Tune with `AETH_THREADS`, `AETH_HEAP`, and per-route concurrency limits. Use the profiler (`--profile`) and `aeth bench` for load testing.
