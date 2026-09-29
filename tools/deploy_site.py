#!/usr/bin/env python3
"""Upload the progress website (site/) to the web host over SFTP, FTPS or FTP.

    python tools/deploy_site.py            # upload site/ if it changed since the last upload
    python tools/deploy_site.py --force    # upload even if unchanged
    python tools/deploy_site.py --dry-run  # show what would be uploaded

Settings come from deploy.ini in the repository root (git-ignored; copy
deploy.ini.example), or from the file named by the DEPLOY_CONFIG environment
variable.  The password can also come from the DEPLOY_PASSWORD environment
variable instead of the file.  Without a config file this does nothing, so the
git hooks can call it unconditionally.

Files are uploaded under a temporary name and then renamed, so visitors never
see a half-written page.  SFTP needs paramiko (`pip install paramiko`); FTP
and FTPS use Python's own ftplib.
"""

from __future__ import annotations

import argparse
import configparser
import ftplib
import hashlib
import os
import posixpath
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
SITE = ROOT / "site"
STAMP = ROOT / "build" / "deploy.last"


def load_config() -> configparser.SectionProxy | None:
    """Find and read the deploy settings.

    @return the [site] section, or None when there is no config file
    """
    cands = [os.environ.get("DEPLOY_CONFIG"), ROOT / "deploy.ini", Path.home() / ".3dw-deploy.ini"]
    for c in cands:
        if c and Path(c).is_file():
            cp = configparser.ConfigParser()
            cp.read(c, encoding="utf-8")
            if "site" not in cp:
                raise SystemExit(f"{c}: no [site] section")
            return cp["site"]
    return None


def site_files() -> list[Path]:
    """Every file under site/, parents before children."""
    return sorted(p for p in SITE.rglob("*") if p.is_file())


def digest(files: list[Path]) -> str:
    """Hash of the names and contents of the files, to skip unchanged uploads."""
    h = hashlib.sha256()
    for f in files:
        h.update(f.relative_to(SITE).as_posix().encode() + b"\0" + f.read_bytes())
    return h.hexdigest()


class SftpTarget:
    def __init__(self, cfg, password):
        try:
            import paramiko
        except ImportError:
            raise SystemExit("SFTP needs paramiko: pip install paramiko")
        self.ssh = paramiko.SSHClient()
        self.ssh.load_system_host_keys()
        known = cfg.get("known_hosts")
        if known:
            self.ssh.load_host_keys(os.path.expanduser(known))
        self.ssh.set_missing_host_key_policy(paramiko.RejectPolicy())
        self.ssh.connect(cfg["host"], port=cfg.getint("port", 22), username=cfg["user"],
                         password=password or None,
                         key_filename=os.path.expanduser(cfg["key_file"]) if cfg.get("key_file") else None,
                         timeout=30)
        self.sftp = self.ssh.open_sftp()

    def mkdir(self, path):
        try:
            self.sftp.stat(path)
        except IOError:
            self.sftp.mkdir(path)

    def put(self, local: Path, remote: str):
        tmp = remote + ".uploading"
        self.sftp.put(str(local), tmp)
        try:
            self.sftp.posix_rename(tmp, remote)
        except IOError:
            try:
                self.sftp.remove(remote)
            except IOError:
                pass
            self.sftp.rename(tmp, remote)

    def close(self):
        self.sftp.close()
        self.ssh.close()


class FtpTarget:
    def __init__(self, cfg, password, tls: bool):
        port = cfg.getint("port", 21)
        self.ftp = ftplib.FTP_TLS(timeout=30) if tls else ftplib.FTP(timeout=30)
        self.ftp.connect(cfg["host"], port)
        self.ftp.login(cfg["user"], password or "")
        if tls:
            self.ftp.prot_p()
        self.ftp.set_pasv(cfg.getboolean("passive", True))

    def mkdir(self, path):
        try:
            self.ftp.mkd(path)
        except ftplib.error_perm:
            pass            # already there

    def put(self, local: Path, remote: str):
        tmp = remote + ".uploading"
        with open(local, "rb") as fh:
            self.ftp.storbinary(f"STOR {tmp}", fh)
        try:
            self.ftp.delete(remote)
        except ftplib.error_perm:
            pass
        self.ftp.rename(tmp, remote)

    def close(self):
        try:
            self.ftp.quit()
        except ftplib.all_errors:
            self.ftp.close()


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--force", action="store_true", help="upload even if nothing changed")
    ap.add_argument("--dry-run", action="store_true")
    a = ap.parse_args(argv)

    cfg = load_config()
    if cfg is None:
        print("deploy: no deploy.ini, skipping upload")
        return 0
    files = site_files()
    if not files:
        print("deploy: site/ is empty - run tools/website.py first")
        return 1
    h = digest(files)
    if not a.force and STAMP.is_file() and STAMP.read_text().strip() == h:
        print("deploy: site unchanged since the last upload")
        return 0

    proto = cfg.get("protocol", "sftp").lower()
    remote_dir = cfg.get("remote_dir", "3dw").rstrip("/")
    print(f"deploy: {len(files)} file(s) -> {proto}://{cfg.get('host')}/{remote_dir}/")
    if a.dry_run:
        for f in files:
            print("  ", f.relative_to(SITE).as_posix())
        return 0

    password = os.environ.get("DEPLOY_PASSWORD") or cfg.get("password", "")
    if proto == "sftp":
        t = SftpTarget(cfg, password)
    elif proto in ("ftps", "ftp"):
        t = FtpTarget(cfg, password, tls=(proto == "ftps"))
    else:
        raise SystemExit(f"deploy: unknown protocol {proto!r} (sftp, ftps or ftp)")
    try:
        made = set()
        for f in files:
            rel = f.relative_to(SITE).as_posix()
            remote = posixpath.join(remote_dir, rel)
            d = posixpath.dirname(remote)
            parts = d.split("/")
            for i in range(1, len(parts) + 1):
                sub = "/".join(parts[:i])
                if sub and sub not in made:
                    t.mkdir(sub)
                    made.add(sub)
            t.put(f, remote)
            print("  uploaded", rel)
    finally:
        t.close()
    STAMP.parent.mkdir(exist_ok=True)
    STAMP.write_text(h + "\n")
    print("deploy: done")
    return 0


if __name__ == "__main__":
    sys.exit(main())
