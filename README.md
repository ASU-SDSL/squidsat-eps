# SquidSat EPS Boilerplate

Minimal Zephyr boilerplate for STM32 (`nucleo_f103rb`) with a single blinky app.

## Why this is minimal

- One build system: `west` + CMake/Ninja (Zephyr standard)
- One app source file: `app/src/main.c`
- One board target: `nucleo_f103rb`
- One manifest with only required STM32 modules (`cmsis_6`, `hal_stm32`)

## 1. System-level dependencies (machine setup)

Install these once per laptop/workstation:

- Python 3.10+
- [uv](https://docs.astral.sh/uv/)
- Git
- CMake
- Ninja
- DTC (Device Tree Compiler)
- Zephyr SDK (with ARM toolchain)
- stlink tools (`st-flash`) for flashing (default in this repo)
- OpenOCD (optional fallback flasher)

Example package install commands:

```bash
# macOS (Homebrew)
brew install git uv cmake ninja dtc stlink openocd
```

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y git cmake ninja-build device-tree-compiler stlink-tools openocd python3 python3-venv
```

## 2. Project-level dependencies (`west` + `uv`)

From the repository root:

```bash
uv venv .venv
```

Activate virtual environment:

```bash
# macOS/Linux
source .venv/bin/activate
```

```powershell
# Windows PowerShell
.venv\Scripts\Activate.ps1
```

Install project Python tools and fetch Zephyr workspace:

```bash
uv pip install west

python -m west init -l app
python -m west update
uv pip install -r zephyr/scripts/requirements.txt
```

Notes:

- The workspace is pinned in `app/west.yml` (Zephyr `v4.3.0`).
- Manifest allowlist is intentionally small: `cmsis_6`, `hal_stm32`.

## 3. Cross-platform build steps

From repo root, with venv active:

```bash
python -m west build -p always -b nucleo_f103rb app
```

Build artifact used for flashing:

- `build/zephyr/zephyr.bin`

## 4. Cross-platform flashing steps

### Default (this repo): `st-flash`

macOS/Linux:

```bash
./flash.sh
```

Any platform with `st-flash` installed:

```bash
st-flash --connect-under-reset write build/zephyr/zephyr.bin 0x08000000
```

### Fallback: `west` + OpenOCD

```bash
python -m west flash --runner openocd
```

If `python -m west flash` tries `stm32cubeprogrammer` by default and fails, use `--runner openocd` explicitly as above.

## Project structure

```text
eps/
├── app/
│   ├── CMakeLists.txt
│   ├── prj.conf
│   ├── src/main.c
│   └── west.yml
├── flash.sh
└── README.md
```

## 5. Three-board broadcast test (functional addressing `0x18DB`)

Use three terminal sessions from repo root:

```bash
# Board 1 (sender: node 0x01)
./.venv/bin/python -m west build -p always -b nucleo_f103rb app -- -DCONF_FILE="prj.conf;node_a.conf"
st-flash --connect-under-reset write build/zephyr/zephyr.bin 0x08000000
```

```bash
# Board 2 (receiver: node 0x02)
./.venv/bin/python -m west build -p always -b nucleo_f103rb app -- -DCONF_FILE="prj.conf;node_b.conf"
st-flash --connect-under-reset write build/zephyr/zephyr.bin 0x08000000
```

```bash
# Board 3 (receiver: node 0x03)
./.venv/bin/python -m west build -p always -b nucleo_f103rb app -- -DCONF_FILE="prj.conf;node_c.conf"
st-flash --connect-under-reset write build/zephyr/zephyr.bin 0x08000000
```

Expected logs:

- Sender prints `TX broadcast seq=...`.
- All boards print `RX src=1 broadcast=1 ...` for each broadcast frame.
- Unicast demo messages continue to print with `broadcast=0`.

## 6. Multi-port serial monitor

Use the helper script to monitor all connected boards with `[1]/[2]/[3]` labels:

```bash
./.venv/bin/python monitor.py
```

Optional explicit ports:

```bash
./.venv/bin/python monitor.py --ports /dev/cu.usbmodem11203 /dev/cu.usbmodem11303 /dev/cu.usbmodem11403
```
