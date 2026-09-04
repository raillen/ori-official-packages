# ori-imgui-widgets

ImGui **accessory widgets** for Ori (S3): rotary knob, boolean toggle, phase
spinner, and spectrum theme color helpers. Pure-Ori state machines with
headless test hooks; product UI composes sibling [`ori-imgui`](../ori-imgui).

**Package:** `widgets` · **0.1.0**  
**Modules:** `widgets.knob` · `widgets.toggle` · `widgets.spinner` · `widgets.spectrum`  
**Maturity:** **5 (Linux)** — G1..G7 met  
**Phase OS:** deferred (Linux first; multi-OS last per eco plan)

## Approach

| Module | Role |
|--------|------|
| **knob** | Rotary value in **milli** (range + normalize + angle for draw) |
| **toggle** | Boolean switch (`toggle` / `set_on`; disabled blocks mutation) |
| **spinner** | Indeterminate phase 0..999; `tick` by wall milli + speed |
| **spectrum** | Theme load MVP (dark/light/classic/spectrum) + RGBA helpers |

Prefer **pure Ori** — no C++ host, no FULL `.a`. Path-dep `imgui` is for product
draw composition only; headless CI never opens a window.

Upstream knobs (e.g. imgui-knobs) and theme packs stay optional; this package
ships the product G1 surface as pure Ori.

## Build / smoke (Linux)

```bash
export ORI_BIN=/path/to/ori
export ORI_RUNTIME_CDYLIB=/path/to/libori_runtime.so
export ORI_RUNTIME_LIB=/path/to/libori_runtime.a
export ORI_USE_SYSTEM_LINKER=1
# smoke forces AOT: path-dep imgui is static-only (no libori_imgui.so for JIT)

./tools/build_linux.sh   # pure Ori marker under lib/<triple>/
./tools/smoke_linux.sh   # check + headless AOT run + ≥4 tests
```

## API sketch

### knob

```ori
import widgets.knob as knob
var k = knob.create_unit(250)          -- min=0 max=1000
k = knob.set_value_milli(k, 500)
k = knob.nudge_value_milli(k, 50)
-- knob.value_milli_of(k), normalized_milli_of, angle_milli_deg_of
```

### toggle

```ori
import widgets.toggle as toggle
var t = toggle.create_labeled("Mute", false)
t = toggle.toggle(t)
-- toggle.is_on(t); disabled blocks toggle/set_on
```

### spinner

```ori
import widgets.spinner as spinner
var s = spinner.create_with_speed(1000)  -- milli-turns / second
s = spinner.tick(s, 16)
-- spinner.phase_milli_of(s) in 0..999; phase_angle_milli_deg_of for draw
```

### spectrum

```ori
import widgets.spectrum as spectrum
var th = spectrum.load_style(spectrum.STYLE_DARK)
-- spectrum.primary_of(th), accent_of, pack_rgba / unpack_rgba
-- spectrum.spectrum_color_at(t_milli), lerp_color(a, b, t_milli)
```

## Phase OS

**Linux-first.** Windows/macOS packaging is **deferred** (Phase OS last — non-blocking for maturity 5).

- Documented stub: [`tools/build_windows.ps1`](tools/build_windows.ps1)
- See ori-lang `docs/planning/PHASE-OS.md` for the multi-OS staging policy

## License

MIT (package).
