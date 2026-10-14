# Aetheris Code Examples & Real-World Patterns (v1.0)

This document provides complete, production-grade source examples demonstrating the versatility of **Aetheris** (`.aeth`) across systems programming, backend web services, reactive frontend interfaces, and AI model workflows.

---

## 1. Systems & Low-Level Embedded Example

### Hardware LED Control & Direct Memory Write (32-Bit ARM Target)
This snippet targets embedded microcontrollers, using pointer arithmetic (`uptr`) and memory address writes without heap allocation or garbage collection overhead.

```aeth
// Target: --target=native-32
const GPIO_BASE_ADDR: uptr = 0x40020000 ~
const MODER_OFFSET: u32 = 0x00 ~
const ODR_OFFSET: u32 = 0x14 ~

fn delayCycles(mut count: u32) ~
    while count > 0 ~
        count = count - 1 ~
    end
end

fn toggleLed(pin: u8) ~
    let gpioModeReg: uptr = GPIO_BASE_ADDR + MODER_OFFSET ~
    let gpioOutReg: uptr = GPIO_BASE_ADDR + ODR_OFFSET ~

    // Set pin direction to output mode via bitwise operations
    *(gpioModeReg as *mut u32) = *(gpioModeReg as *mut u32) | (1 << pin) ~

    loop ~
        // Set LED high
        *(gpioOutReg as *mut u32) = *(gpioOutReg as *mut u32) | (1 << pin) ~
        delayCycles(1000000) ~

        // Clear LED low
        *(gpioOutReg as *mut u32) = *(gpioOutReg as *mut u32) & ~(1 << pin) ~
        delayCycles(1000000) ~
    end
end
```

---

## 2. High-Performance Asynchronous Backend API

### REST Microservice with Deterministic Error Handling
A backend web service defining HTTP routes, interacting with a database, and handling fallible calls explicitly using `check` and `reply ... with status`.

```aeth
struct UserPayload ~
    username: text ~
    email: text ~
    age: u16 ~
end

route POST "/api/v1/users" ~
    let payload = check json.decode[UserPayload](request.body) or reply "Invalid JSON structure" with status 400 ~

    if payload.age < 18 ~
        reply "User must be an adult" with status 422 ~
    end

    let userId = check db.users.insert(payload) or reply "Failed to persist user" with status 500 ~

    reply {
        id: userId,
        status: "created"
    } with status 201 ~
end

route GET "/api/v1/users/:id" ~
    let userId = request.params.id ~
    let user = check db.users.findById(userId) or reply "User not found" with status 404 ~

    reply user with status 200 ~
end
```

---

## 3. Web UI & Reactive Frontend Component

### Interactive Counter & Live Search UI (Web & WASM Target)
A reactive component written in Aetheris that compiles down to WebAssembly or lightweight JavaScript.

```aeth
component SearchDashboard ~
    mut state searchQuery: text = "" ~
    mut state resultCount: i32 = 0 ~

    fn handleSearchUpdate(newText: text) ~
        searchQuery = newText ~
        resultCount = searchQuery.length() * 3 ~
    end

    render ~
        show Div() with style(padding: "20px", fontFamily: "sans-serif") ~
            show Heading("Aetheris Live Search") with style(color: "#2B6CB0") ~

            show Input(value: searchQuery, placeholder: "Type to query...") ~
            on input ? trigger (e) => handleSearchUpdate(e.target.value) ~

            if resultCount > 0 ~
                show Text("Found " + resultCount + " matching records for '" + searchQuery + "'") ~
            else ~
                show Text("No search query entered yet.") with style(color: "#A0AEC0") ~
            end
        end
    end
end
```

---

## 4. AI & Deep Learning Tensor Pipeline

### Neural Network Declaration and Training (GPU/SIMD Target)
Declaring a multi-layer deep learning model and executing forward pass inference with zero runtime boilerplate.

```aeth
neural SentimentClassifier ~
    layer Dense(inputs: 512, outputs: 256, activation: "relu") ~
    layer Dropout(rate: 0.20) ~
    layer Dense(inputs: 256, outputs: 64, activation: "relu") ~
    layer Dense(inputs: 64, outputs: 2, activation: "softmax") ~

    fit(dataset: "data/reviews.csv", epochs: 15, batchSize: 32) ~
end

fn evaluateReview(embeddingTensor: tensor[f32]) -> Result[f32, text] ~
    if embeddingTensor.shape()[0] != 512 ~
        return err("Tensor dimensions must match 512 inputs") ~
    end

    let confidenceScore = infer SentimentClassifier with embeddingTensor ~
    return ok(confidenceScore) ~
end
```

---

## 5. Multi-Module Project Integration

### Main Application Entrypoint Combining Modules

```aeth
import { startTcpServer } from "std.net" ~
import { loadConfig } from "./config.aeth" ~

fn main() -> Result[unit, text] ~
    print("[INIT] Loading configuration...") ~
    let config = check loadConfig("config.json") or err("Configuration load failure") ~

    print("[INIT] Booting server on port " + config.port + "...") ~
    check startTcpServer(config.port) ~

    return ok(()) ~
end
```