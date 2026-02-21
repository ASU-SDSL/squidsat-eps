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
