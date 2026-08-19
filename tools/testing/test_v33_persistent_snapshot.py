#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, os, re, shutil, subprocess, sys, tempfile
from pathlib import Path

KV_RE=re.compile(r'([A-Za-z0-9_-]+)=([^ ]*)')

def parse_line(text: str, prefix: str) -> dict[str,str]:
    rows=[line.strip() for line in text.splitlines() if line.startswith(prefix)]
    if not rows:
        raise RuntimeError(f'missing {prefix!r} line')
    return dict(KV_RE.findall(rows[-1]))

def run(headless: Path, rom: Path, script_text: str, state: Path) -> str:
    with tempfile.NamedTemporaryFile('w',delete=False,prefix='civ-v33-snapshot-',suffix='.txt') as f:
        f.write(script_text); script=Path(f.name)
    try:
        env=os.environ.copy(); env['CIVILIZATION_STATE_DIR']=str(state)
        cp=subprocess.run([str(headless),str(rom),str(script)],env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=120,check=False)
        if cp.returncode!=0:
            raise RuntimeError(f'headless returned {cp.returncode}\n{cp.stdout}')
        return cp.stdout
    finally:
        script.unlink(missing_ok=True)

def main() -> int:
    ap=argparse.ArgumentParser()
    ap.add_argument('--headless',type=Path,required=True)
    ap.add_argument('--rom',type=Path,required=True)
    a=ap.parse_args()
    with tempfile.TemporaryDirectory(prefix='civ-v33-state-') as td:
        state=Path(td)
        out1=run(a.headless,a.rom,
                 'play\nframe-run 30\npause\nstatus\nframe-hash\naudio-status\nsnapshot-save 1\nsnapshot-path 1\nquit\n',state)
        snap=state/'Snapshots'/'Snapshot1.civsnap'
        if not snap.is_file() or snap.stat().st_size<1024:
            raise RuntimeError('persistent snapshot file was not created')
        snapshot_sha=hashlib.sha256(snap.read_bytes()).hexdigest()
        status1=parse_line(out1,'status '); frame1=parse_line(out1,'frame-hash '); audio1=parse_line(out1,'audio-status ')
        if 'snapshot-save 1 ok=1' not in out1 or 'snapshot-path 1 ok=1 exists=1' not in out1:
            raise RuntimeError('snapshot save/path command did not confirm persistence')

        # A new process proves this is not an in-memory frontend slot.
        out2=run(a.headless,a.rom,'snapshot-load 1\nstatus\nframe-hash\naudio-status\nsnapshot-path 1\nquit\n',state)
        status2=parse_line(out2,'status '); frame2=parse_line(out2,'frame-hash '); audio2=parse_line(out2,'audio-status ')
        if 'snapshot-load 1 ok=1' not in out2:
            raise RuntimeError('fresh-process snapshot load failed')
        status_keys=('instr','pc','e','p','a','x','y','s','d','dbr','failed','frame','master','scanline','hclock','field','natural')
        for key in status_keys:
            if status1.get(key)!=status2.get(key):
                raise RuntimeError(f'status mismatch {key}: {status1.get(key)} != {status2.get(key)}')
        if frame1.get('hash')!=frame2.get('hash') or frame1.get('frame')!=frame2.get('frame'):
            raise RuntimeError('framebuffer state mismatch after cross-process restore')
        audio_keys=('ready','sync-master','core-master','delta','smp-pc','smp-cycles','smp-instr','aot','pcm','nonzero','first-nonzero','hash','aot-failed','barriers')
        for key in audio_keys:
            if audio1.get(key)!=audio2.get(key):
                raise RuntimeError(f'audio mismatch {key}: {audio1.get(key)} != {audio2.get(key)}')
        if audio2.get('delta')!='0' or audio2.get('aot-failed')!='0' or audio2.get('barriers')!='0':
            raise RuntimeError('restored Full Static audio is not synchronized/clean')

        # Corrupt a copy and prove integrity guards reject it without harming slot 1.
        bad=state/'Snapshots'/'Snapshot2.civsnap'; shutil.copyfile(snap,bad)
        data=bytearray(bad.read_bytes()); data[len(data)//2]^=0x80; bad.write_bytes(data)
        env=os.environ.copy(); env['CIVILIZATION_STATE_DIR']=str(state)
        with tempfile.NamedTemporaryFile('w',delete=False,prefix='civ-v33-bad-',suffix='.txt') as f:
            f.write('snapshot-load 2\nquit\n'); badscript=Path(f.name)
        try:
            cp=subprocess.run([str(a.headless),str(a.rom),str(badscript)],env=env,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=120,check=False)
        finally:
            badscript.unlink(missing_ok=True)
        if cp.returncode==0 or 'snapshot-load 2 ok=0' not in cp.stdout:
            raise RuntimeError('corrupted snapshot was not rejected')

        print(f'v33 persistent snapshot PASS sha256={snapshot_sha} size={snap.stat().st_size} frame={status2.get("frame")} master={status2.get("master")}')
    return 0

if __name__=='__main__':
    try: sys.exit(main())
    except Exception as e:
        print(f'v33 persistent snapshot FAIL: {e}',file=sys.stderr); sys.exit(1)
