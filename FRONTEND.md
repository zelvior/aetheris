# Aetheris Frontend Development Guide (v1.0)

This guide covers building web user interfaces with Aetheris `component` declarations, reactive state, routing, forms, and deployment. See `BROWSER_COMPILATION_OR_WASM.md` for compiler output details and `NATIVE_GUI_AND_GRAPHICS.md` for desktop targets.

---

## 1. Project Setup

```bash
aeth new my_web_app --template=web
cd my_web_app
aeth dev --target=web        # dev server with hot reload on http://localhost:3000
aeth build --target=web --release -o dist/
```

```
my_web_app/
├── aeth.toml
├── src/
│   ├── main.aeth
│   ├── components/
│   └── pages/
├── assets/
└── tests/
```

---

## 2. Components & State

Components are the unit of UI. `mut state` declares reactive values; any change updates only the dependent nodes.

```aeth
component TodoApp ~
    mut state items: list[text] = [] ~
    mut state draft: text = "" ~

    fn add() ~
        if draft != "" ~
            items.push(draft) ~
            draft = "" ~
        end
    end

    render ~
        show Column() with style(gap: 8, maxWidth: 480) ~
            show Heading("Tasks", level: 1) ~
            show Input(value: draft, placeholder: "Add a task") with a11y(label: "New task") ~
            on input ? trigger (e) => draft = e.target.value ~
            show Button("Add") ~
            on click ? trigger add() ~

            for item in items ~
                show Text(item) ~
            end
        end
    end
end
```

### Props & Children

```aeth
component Card(title: text, tone: text = "neutral") ~
    render ~
        show Div() with style(border: "1px solid #CBD5E0", padding: 12) ~
            show Heading(title, level: 2) ~
            show Slot() ~
        end
    end
end

show Card(title: "Profile") ~
    show Text("Welcome back") ~
end
```

### Derived State & Effects

```aeth
derived total: f32 = items.sum(fn(i) ~ i.price end) ~
effect when query changes ~
    results = check await api.search(query) or [] ~
end
```

Effects are cancelled automatically when their dependencies change or the component unmounts.

---

## 3. Routing

```aeth
router App ~
    page "/"            => HomePage ~
    page "/users/:id"   => UserPage ~
    page "/settings"    => SettingsPage with guard(isLoggedIn) ~
    notFound            => NotFoundPage ~
end

show Link(to: "/settings") ~ show Text("Settings") ~ end
router.navigate("/users/42") ~
```

Route parameters are typed: `params.id` is `text`; use `params.id.toU64()` and handle the `Result`. Both History API and hash modes are supported; `--ssr` pre-renders pages to static HTML.

---

## 4. Data Fetching

```aeth
component UserList ~
    mut state users: Remote[list[User]] = Remote.Loading ~

    effect once ~
        users = match await http.get[list[User]]("/api/v1/users") ~
            case ok(list) => Remote.Ready(list) ~
            case err(e)   => Remote.Failed(e) ~
        end
    end

    render ~
        match users ~
            case Remote.Loading   => show Spinner() with a11y(label: "Loading users") ~
            case Remote.Failed(e) => show Text("Unable to load users: " + e) with a11y(live: "assertive") ~
            case Remote.Ready(l)  => for u in l ~ show Text(u.name) ~ end
        end
    end
end
```

Types can be shared with the backend (see `BACKEND.md`) so requests and responses are checked at compile time.

---

## 5. Forms & Validation

```aeth
form SignupForm(onSubmit: fn(SignupData)) ~
    field email: text = "" validate(isEmail, "Enter a valid email") ~
    field password: secret[text] = "" validate(minLen(12), "At least 12 characters") ~
    submit "Create account" ~
end
```

Validation messages are associated with fields through ARIA automatically.

---

## 6. Styling & Theming

* Inline: `with style(...)`.
* Scoped stylesheets: `style ~ .card { ... } end` inside a component (class names are hashed).
* Tokens: `theme Light ~ color.primary = #2B6CB0 ~ end`, switch via `Theme.set(Dark)`.
* Responsive: `with style(@media(maxWidth: 600): {columns: 1})`.
* Animations honor reduced-motion preferences (see `ACCESSIBILITY_AND_A11Y.md`).

---

## 7. Testing

```aeth
test "adds a task" ~
    let app = mount(TodoApp) ~
    app.type("Input", "Write docs") ~
    app.click("Add") ~
    assert app.text().contains("Write docs") ~
end
```

`aeth test --browser` runs component tests in a headless browser; `--snapshot` records DOM snapshots.

---

## 8. Performance Checklist

| Practice | Benefit |
| :--- | :--- |
| Keep `state` close to where it is used | Smaller update scope |
| Use keyed `for` loops: `for item in items key item.id` | Efficient list updates |
| Lazy-load routes: `page "/admin" => lazy AdminPage` | Smaller initial bundle |
| Use the asset pipeline for images/fonts | Optimized, hashed, cached assets |
| Run `aeth build --analyze` | Bundle size breakdown |

## 9. Deployment

`dist/` contains static files deployable to any CDN. Provide caching headers for hashed assets (`Cache-Control: immutable`). For SSR, run the server build via `aeth build --ssr` with the backend runtime described in `BACKEND.md`.
