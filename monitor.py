#!/usr/bin/env python3
import argparse
import glob
import queue
import re
import signal
import sys
import threading
import time

try:
    import serial
except ImportError:
    print("pyserial is required. Install with: ./.venv/bin/pip install pyserial", file=sys.stderr)
    sys.exit(1)


NODE_LABELS = {
    0x01: "1",
    0x02: "2",
    0x03: "3",
}
ANSI_RE = re.compile(r"\x1b\[[0-9;]*m")
TS_RE = re.compile(r"^\[([0-9:.,]+)\]\s*")
RX_RE = re.compile(
    r"RX src=(\d+)\s+broadcast=(\d+)\s+node_id=(\d+)\s+seq=(\d+)\s+uptime_ms=(\d+)\s+type=(\d+)"
)
TX_U_RE = re.compile(r"TX unicast seq=(\d+)\s+target=(\d+)\s+bytes=(\d+)")
TX_B_RE = re.compile(r"TX broadcast seq=(\d+)\s+bytes=(\d+)")


def default_ports():
    return sorted(set(glob.glob("/dev/cu.usbmodem*") + glob.glob("/dev/tty.usbmodem*")))


def label_for_node(node):
    return NODE_LABELS.get(node, f"NODE{node}")


def reader_worker(port, baud, out_q, stop_evt):
    node = None
    saw_broadcast_tx = False
    tx_targets = set()
    node_re = re.compile(r"node=(\d+)")
    tx_target_re = re.compile(r"TX unicast .* target=(\d+)")

    try:
        ser = serial.Serial(port, baudrate=baud, timeout=0.2)
    except Exception as exc:
        out_q.put((time.time(), "ERR", port, f"open failed: {exc}", f"open failed: {exc}"))
        return

    out_q.put((time.time(), "INFO", port, "opened", "opened"))
    with ser:
        while not stop_evt.is_set():
            try:
                raw = ser.readline()
            except Exception as exc:
                out_q.put((time.time(), "ERR", port, f"read failed: {exc}", f"read failed: {exc}"))
                return

            if not raw:
                continue

            line = raw.decode("utf-8", errors="replace").rstrip()
            clean = ANSI_RE.sub("", line)
            if node is None:
                m = node_re.search(clean)
                if m:
                    node = int(m.group(1))
            if node is None:
                if "TX broadcast" in clean:
                    saw_broadcast_tx = True
                m = tx_target_re.search(clean)
                if m:
                    tx_targets.add(int(m.group(1)))

                # Infer from known traffic plan:
                # A -> broadcast + targets 2,3
                # B -> targets 1,3
                # C -> targets 1,2
                if saw_broadcast_tx:
                    node = 1
                elif tx_targets == {1, 3}:
                    node = 2
                elif tx_targets == {1, 2}:
                    node = 3

            label = label_for_node(node) if node is not None else "?"
            out_q.put((time.time(), label, port, line, clean))


def maybe_summarize(label, clean_line):
    ts = ""
    line = clean_line
    m_ts = TS_RE.match(line)
    if m_ts:
        ts = m_ts.group(1)
        line = line[m_ts.end():]

    if "<inf> eps:" not in line:
        return None

    if "TX broadcast" in line:
        m = TX_B_RE.search(line)
        if not m:
            return None
        seq, nbytes = m.groups()
        src = "?" if label == "?" else label
        return f"{ts} TX BC  {src}->* seq={seq} bytes={nbytes}"

    if "TX unicast" in line:
        m = TX_U_RE.search(line)
        if not m:
            return None
        seq, target, nbytes = m.groups()
        src = "?" if label == "?" else label
        dst = label_for_node(int(target))
        return f"{ts} TX UNI {src}->{dst} seq={seq} bytes={nbytes}"

    if "RX src=" in line:
        m = RX_RE.search(line)
        if not m:
            return None
        src, is_bc, node_id, seq, _uptime, msg_type = m.groups()
        src_l = label_for_node(int(src))
        local = "?" if label == "?" else label
        mtype = "HB" if msg_type == "0" else "ST"
        if is_bc == "1":
            return f"{ts} RX BC  {src_l}->* at {local} seq={seq} type={mtype}"
        return f"{ts} RX UNI {src_l}->{local} seq={seq} type={mtype}"

    return None


def main():
    parser = argparse.ArgumentParser(description="Monitor multiple Nucleo serial ports with node labels")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    parser.add_argument("--ports", nargs="*", help="Ports to open (default: auto-detect usbmodem ports)")
    parser.add_argument(
        "--raw",
        action="store_true",
        help="Print raw log lines only (disable parsed summaries)",
    )
    args = parser.parse_args()

    ports = args.ports if args.ports else default_ports()
    if not ports:
        print("No serial ports found.", file=sys.stderr)
        return 1

    print("Monitoring ports:")
    for p in ports:
        print(f"  - {p}")
    print("Press Ctrl+C to stop.\n")

    stop_evt = threading.Event()
    q = queue.Queue()
    threads = []

    def handle_sigint(_sig, _frame):
        stop_evt.set()

    signal.signal(signal.SIGINT, handle_sigint)

    for p in ports:
        t = threading.Thread(target=reader_worker, args=(p, args.baud, q, stop_evt), daemon=True)
        t.start()
        threads.append(t)

    try:
        while not stop_evt.is_set():
            try:
                _, label, port, line, clean = q.get(timeout=0.2)
            except queue.Empty:
                continue
            if label in ("INFO", "ERR"):
                print(f"[{label}] {port} | {line}")
                continue

            if args.raw:
                print(f"[{label}] {port} | {line}")
                continue

            summary = maybe_summarize(label, clean)
            if summary is not None:
                print(f"[{label}] {summary}")
            else:
                print(f"[{label}] {port} | {line}")
    finally:
        stop_evt.set()
        for t in threads:
            t.join(timeout=1.0)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
