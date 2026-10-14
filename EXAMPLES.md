# Aetheris Code Examples & Patterns

This document contains real-world code examples demonstrating Aetheris (`.aeth`) syntax, statement terminators (`~`), block boundaries (`end`), frontend components, HTTP REST routes, async tasks, and AI model inference.

---

## 1. Backend REST API Service

```aeth
// HTTP REST Route definition
route GET "/api/v1/health" ~
    let statusMsg: text = "Service Operational" ~
    reply statusMsg with status 200 ~
end

route POST "/api/v1/calculate" ~
    let base: i32 = 8000 ~
    let offset: i32 = 80 ~
    let total: i32 = base + offset ~
    reply total with status 201 ~
end
```

To run and test:
```bash
./aeth run src/api.aeth
```

---

## 2. Frontend Web UI Component

```aeth
// Declarative UI Component
component CounterApp ~
    render ~
        show Heading("Aetheris Counter App") ~
        show Button("Increment Count") ~
    end
end
```

To build and serve:
```bash
./aeth dev src/ui.aeth
```

---

## 3. Async Task Spawning & Concurrent Execution

```aeth
async fn fetchUserData(id: i32) -> text ~
    return "User Record #" + id ~
end

fn main() -> i32 ~
    let userData: text = await fetchUserData(42) ~
    print("Fetched: ", userData) ~
    spawn fetchUserData(100) ~
    return 0 ~
end
```

To execute:
```bash
./aeth run src/async_demo.aeth
```

---

## 4. AI Tensor Model & Inference

```aeth
neural SentimentClassifier ~
end

fn main() -> i32 ~
    let confidence: f32 = infer SentimentClassifier with 1024 ~
    print("Classification score: ", confidence) ~
    return 0 ~
end
```
