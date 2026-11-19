#!/usr/bin/env python3
"""symbolize.py MAPS — put function names on the user-space frames in bpftrace output.

bpftrace prints ustack(perf) frames as "<hex-address> <symbol> (<path>)". Inside a container it
knows the traced thread only by its host PID, which does not exist in the container's /proc, so
every frame comes out as "[unknown]". The lab saves its own /proc/self/maps (PKTD_MAPS=<file>);
this script maps each address to a file and offset and asks addr2line for the function.
Reads bpftrace output on stdin, writes it to stdout with frames rewritten as
"<hex-address> <function> (<path>)"; every other line passes through unchanged.
"""
import re
import subprocess
import sys

FRAME = re.compile(r'^(\s+)([0-9a-f]{6,16}) .*$')


def load_maps(path):
    maps = []
    with open(path) as f:
        for line in f:
            p = line.split()
            if len(p) >= 6 and 'x' in p[1] and p[5].startswith('/'):
                lo, hi = (int(x, 16) for x in p[0].split('-'))
                maps.append((lo, hi, int(p[2], 16), p[5]))
    return maps


def main():
    maps = load_maps(sys.argv[1])
    text = sys.stdin.read().splitlines()
    wanted = {}  # path -> set of file offsets
    where = {}   # address -> (path, offset)
    for line in text:
        m = FRAME.match(line)
        if not m:
            continue
        addr = int(m.group(2), 16)
        for lo, hi, off, path in maps:
            if lo <= addr < hi:
                # A return address points just past the call; look up the call itself.
                where[addr] = (path, addr - lo + off - 1)
                wanted.setdefault(path, set()).add(addr - lo + off - 1)
                break
    names = {}
    for path, offs in wanted.items():
        offs = sorted(offs)
        out = subprocess.run(['addr2line', '-f', '-e', path] + [hex(o) for o in offs],
                             capture_output=True, text=True).stdout.splitlines()
        for i, o in enumerate(offs):
            fn = out[2 * i] if 2 * i < len(out) else '??'
            names[(path, o)] = '[unknown]' if fn == '??' else fn
    for line in text:
        m = FRAME.match(line)
        if m and int(m.group(2), 16) in where:
            path, o = where[int(m.group(2), 16)]
            line = f'{m.group(1)}{m.group(2)} {names[(path, o)]} ({path})'
        print(line)


if __name__ == '__main__':
    main()
