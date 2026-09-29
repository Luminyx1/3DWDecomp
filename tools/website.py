#!/usr/bin/env python3
"""Generate the progress website (a single static page) from build/report.json.

    python tools/website.py            # -> site/index.html
    python tools/website.py -o out.html

Run tools/progress.py (or the pre-commit hook) first so build/report.json is
current; the post-commit hook runs this too.  The page is self-contained
(inline CSS and a few lines of script for the night-mode toggle) apart from the web font, so it can be copied to any
web server as-is.

Layout (docs/WEBSITE.md): overall % (XX.XX%) and a progress bar, then each
library, then the ten best files of every sub-library of Game, al/Library and
al/Project with the sub-library's total.  The look borrows Super Mario 3D
World's bright sky, clouds, glossy HUD bars and checkered floors - drawn in
CSS, no game assets.
"""

from __future__ import annotations

import argparse
import datetime
import html
import json
import subprocess
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent

LIBRARIES = [  # (category id, label, colour)
    ("sead", "sead", "#ff5a5f"),
    ("game", "Game", "#ffb400"),
    ("nw", "NintendoWare", "#2fbf71"),
    ("agl", "agl", "#3a8dff"),
    ("eui", "eui", "#9b5de5"),
    ("al", "al (ActionLibrary)", "#ff7a1a"),
    ("aal", "aal", "#00c2c7"),
    ("erepo", "erepo", "#f15bb5"),
]
SECTIONS = [  # (unit name prefix, title)
    ("src/Game/", "Game"),
    ("src/al/Library/", "al / Library"),
    ("src/al/Project/", "al / Project"),
]


def num(v) -> int:
    return int(v or 0)


def pct(m: int, t: int) -> float:
    return 100.0 * m / t if t else 0.0


def git_rev() -> str:
    try:
        return subprocess.run(["git", "rev-parse", "--short", "HEAD"], cwd=ROOT, capture_output=True,
                              text=True).stdout.strip()
    except OSError:
        return ""


def bar(p: float, colour: str = "", cls: str = "bar") -> str:
    style = f"--p:{max(p, 0.0):.3f}%" + (";--min:10px" if p > 0 else "") + (f";--c:{colour}" if colour else "")
    return f'<div class="{cls}" style="{style}"><div class="fill"></div></div>'


def build(report: dict) -> str:
    m = report["measures"]
    total = pct(num(m.get("matched_code")), num(m.get("total_code")))
    cats = {c["id"]: c["measures"] for c in report.get("categories", [])}

    lib_cards = []
    for cid, label, colour in LIBRARIES:
        c = cats.get(cid, {})
        tc, mc = num(c.get("total_code")), num(c.get("matched_code"))
        tf, mf = num(c.get("total_functions")), num(c.get("matched_functions"))
        p = pct(mc, tc)
        lib_cards.append(
            f'<div class="card lib" style="--c:{colour}"><div class="band"></div>'
            f'<h3>{html.escape(label)}</h3><div class="pct">{p:.2f}%</div>{bar(p, colour, "bar small")}'
            f'<div class="sub">{mf:,} / {tf:,} functions</div></div>')

    # sub-libraries
    groups: dict[str, dict[str, list]] = {t: defaultdict(list) for _, t in SECTIONS}
    for u in report.get("units", []):
        name = u["name"]
        for prefix, title in SECTIONS:
            if name.startswith(prefix):
                rest = name[len(prefix):]
                sub = rest.split("/", 1)[0] if "/" in rest else "(root)"
                um = u.get("measures", {})
                groups[title][sub].append((rest.rsplit("/", 1)[-1], num(um.get("matched_code")),
                                           num(um.get("total_code"))))
                break

    sections = []
    for _, title in SECTIONS:
        cards = []
        for sub in sorted(groups[title], key=str.upper):
            files = groups[title][sub]
            tm, tt = sum(f[1] for f in files), sum(f[2] for f in files)
            sp = pct(tm, tt)
            best = sorted(files, key=lambda f: (-pct(f[1], f[2]), -f[1], f[0].upper()))[:10]
            rows = "".join(
                f'<li><span class="fname">{html.escape(f)}</span>{bar(pct(fm, ft), "", "bar tiny")}'
                f'<span class="fp">{pct(fm, ft):.2f}%</span></li>' for f, fm, ft in best)
            cards.append(
                f'<details class="card folder"><summary><span class="fold">{html.escape(sub)}</span>'
                f'<span class="count">{len(files)} file{"s" if len(files) != 1 else ""}</span><span class="fp big">{sp:.2f}%</span>'
                f'{bar(sp, "", "bar small")}</summary><ol>{rows}</ol></details>')
        sections.append(f'<section><h2>{html.escape(title)}</h2><div class="folders">{"".join(cards)}</div></section>')

    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%d %H:%M UTC")
    rev = git_rev()
    return TEMPLATE.format(
        total=f"{total:.2f}", bar=bar(total, "", "bar main"),
        matched=f"{num(m.get('matched_code')):,}", code=f"{num(m.get('total_code')):,}",
        fmatched=f"{num(m.get('matched_functions')):,}", ftotal=f"{num(m.get('total_functions')):,}",
        libs="".join(lib_cards), sections="".join(sections), night_rules=NIGHT_RULES,
        stamp=stamp, rev=f" &middot; {html.escape(rev)}" if rev else "")


# 3D World's night courses: deep blue-violet sky, a moon where the sun was,
# twinkling stars, a glowing tile floor and dark glassy HUD panels.
NIGHT_TOKENS = """
  --sky-top: #070b2e; --sky-mid: #1a1f5c; --sky-low: #3b2a74;
  --ink: #eef3ff; --muted: #a9b6e6; --card: #1c2257;
  --shadow: 0 10px 0 rgba(0, 0, 20, .45), 0 18px 40px rgba(0, 0, 0, .45);
  --navy: #5b4bd6; --navy-2: #2e2383; --track: #0f1440; --gloss: rgba(160,180,255,.12);
  --tile-a: #232a7a; --tile-b: #151a55; --haze: 40,32,100; --cloud: #3a3f86;
  --cloud-shadow: rgba(10, 10, 40, .5); --outline: #ffe27a; --accent: #ffe27a;
  --glow: radial-gradient(circle, #fffbe6 0%, #fff4c2 7%, rgba(255,240,170,.35) 9%,
          rgba(170,160,255,.18) 22%, rgba(0,0,0,0) 50%);
  --stars:
    radial-gradient(3.4px 3.4px at 8% 12%, #fff 50%, transparent 51%),
    radial-gradient(2.2px 2.2px at 22% 30%, #fff 50%, transparent 51%),
    radial-gradient(3.4px 3.4px at 35% 8%, #ffe9a8 50%, transparent 51%),
    radial-gradient(2.2px 2.2px at 47% 22%, #fff 50%, transparent 51%),
    radial-gradient(3.4px 3.4px at 58% 40%, #cfe0ff 50%, transparent 51%),
    radial-gradient(2.2px 2.2px at 66% 14%, #fff 50%, transparent 51%),
    radial-gradient(3.4px 3.4px at 78% 33%, #fff 50%, transparent 51%),
    radial-gradient(2.2px 2.2px at 88% 6%, #ffe9a8 50%, transparent 51%),
    radial-gradient(3.4px 3.4px at 94% 44%, #fff 50%, transparent 51%),
    radial-gradient(2.2px 2.2px at 14% 48%, #cfe0ff 50%, transparent 51%),
    radial-gradient(2.2px 2.2px at 40% 52%, #fff 50%, transparent 51%),
    radial-gradient(3.4px 3.4px at 72% 55%, #fff 50%, transparent 51%);
  color-scheme: dark;
"""
NIGHT_EXTRA = """
.bar .fill {{ box-shadow: inset 0 -3px 0 rgba(0,0,0,.2), 0 0 16px var(--c); }}
.floor {{ filter: drop-shadow(0 0 18px rgba(120,110,255,.35)); }}
.cloud {{ opacity: .45; }}
"""
NIGHT_RULES = (
    ".mode .night-only {{ display: none; }}\n"
    ":root[data-theme=\"night\"] {{" + NIGHT_TOKENS + "}}\n"
    ":root[data-theme=\"night\"] .mode .day-only {{ display: none; }}\n"
    ":root[data-theme=\"night\"] .mode .night-only {{ display: inline-block; }}\n"
    + "".join(":root[data-theme=\"night\"] " + r.strip() + "\n" for r in NIGHT_EXTRA.strip().splitlines())
    + "@media (prefers-color-scheme: dark) {{\n"
    ":root:not([data-theme=\"day\"]) {{" + NIGHT_TOKENS + "}}\n"
    ":root:not([data-theme=\"day\"]) .mode .day-only {{ display: none; }}\n"
    ":root:not([data-theme=\"day\"]) .mode .night-only {{ display: inline-block; }}\n"
    + "".join(":root:not([data-theme=\"day\"]) " + r.strip() + "\n" for r in NIGHT_EXTRA.strip().splitlines())
    + "}}\n"
).replace("{{", "{").replace("}}", "}")


TEMPLATE = """<!doctype html>
<html lang="en">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>3DWDecomp Progress</title>
<link rel="preconnect" href="https://fonts.googleapis.com">
<link href="https://fonts.googleapis.com/css2?family=Fredoka:wght@400;600;700&display=swap" rel="stylesheet">
<style>
:root {{
  --sky-top: #3cb8ff; --sky-mid: #8fdcff; --sky-low: #e6f8ff;
  --ink: #16325c; --muted: #4a6a93; --card: #ffffff;
  --gold-1: #fff27a; --gold-2: #ffc400; --gold-3: #ff8a00;
  --shadow: 0 10px 0 rgba(22, 50, 92, .12), 0 18px 40px rgba(20, 90, 160, .25);
  --navy: #1b4fa0; --navy-2: #0e2f66; --track: #e3f1fb; --gloss: rgba(255,255,255,.7);
  --tile-a: #ffffff; --tile-b: #bfe9ff; --haze: 230,248,255; --cloud: #fff;
  --cloud-shadow: rgba(160, 210, 240, .45); --outline: #ffd23f;
  --glow: radial-gradient(circle, rgba(255,255,230,.95) 0%, rgba(255,245,180,.55) 18%, rgba(255,255,255,0) 55%);
  --stars: none; color-scheme: light;
}}
* {{ box-sizing: border-box; }}
html, body {{ margin: 0; }}
body {{
  font-family: "Fredoka", "Trebuchet MS", system-ui, sans-serif;
  color: var(--ink);
  background: linear-gradient(180deg, var(--sky-top) 0%, var(--sky-mid) 45%, var(--sky-low) 80%);
  min-height: 100vh; overflow-x: hidden; position: relative;
}}
/* sun bloom */
body::before {{
  content: ""; position: fixed; top: -18vmax; right: -12vmax; width: 60vmax; height: 60vmax;
  background: var(--glow);
  pointer-events: none; z-index: 0;
}}
/* night sky stars (none by day) */
body::after {{
  content: ""; position: fixed; inset: 0; z-index: 0; pointer-events: none;
  background: var(--stars); background-size: 340px 260px; opacity: .9; animation: twinkle 6s ease-in-out infinite alternate;
}}
@keyframes twinkle {{ from {{ opacity: .55; }} to {{ opacity: 1; }} }}
@media (prefers-reduced-motion: reduce) {{ body::after {{ animation: none; }} }}
.mode {{
  position: absolute; top: 18px; right: 16px; z-index: 2; cursor: pointer;
  font: 600 15px "Fredoka", system-ui, sans-serif; color: var(--ink);
  background: var(--card); border: 3px solid var(--outline); border-radius: 999px;
  padding: 6px 14px 6px 10px; box-shadow: 0 4px 0 var(--navy);
}}
.mode:active {{ translate: 0 3px; box-shadow: 0 1px 0 var(--navy); }}
.mode .i {{ width: 1.4em; text-align: center; }}
.mode .day-only, .mode .night-only {{ display: inline-block; }}
{night_rules}
/* checkered floor in perspective, like a 3D World course */
.floor {{
  position: fixed; left: -50%; right: -50%; bottom: -10vh; height: 55vh; z-index: 0;
  background:
    linear-gradient(180deg, rgba(var(--haze),1) 0%, rgba(var(--haze),0) 30%),
    repeating-conic-gradient(var(--tile-a) 0 25%, var(--tile-b) 0 50%) 0 0 / 120px 120px;
  transform: perspective(600px) rotateX(62deg); transform-origin: 50% 100%;
  box-shadow: inset 0 40px 60px rgba(var(--haze),.9);
  pointer-events: none;
}}
.cloud {{
  position: fixed; z-index: 0; pointer-events: none; opacity: .95;
  width: 260px; height: 90px; border-radius: 90px; background: var(--cloud);
  box-shadow: 0 12px 0 var(--cloud-shadow);
  animation: drift 90s linear infinite;
}}
.cloud::before, .cloud::after {{
  content: ""; position: absolute; background: var(--cloud); border-radius: 50%;
}}
.cloud::before {{ width: 120px; height: 120px; top: -60px; left: 40px; }}
.cloud::after  {{ width: 90px;  height: 90px;  top: -40px; right: 45px; }}
.c1 {{ top: 9vh;  left: -20vw; transform: scale(.9); }}
.c2 {{ top: 24vh; left: -35vw; transform: scale(.6); animation-duration: 130s; animation-delay: -60s; opacity: .8; }}
.c3 {{ top: 4vh;  left: -30vw; transform: scale(.45); animation-duration: 160s; animation-delay: -20s; opacity: .7; }}
@keyframes drift {{ from {{ translate: 0 0; }} to {{ translate: 150vw 0; }} }}
@media (prefers-reduced-motion: reduce) {{ .cloud {{ animation: none; left: 10vw; }} }}

main {{ position: relative; z-index: 1; max-width: 1100px; margin: 0 auto; padding: 32px 16px 120px; }}
header {{ text-align: center; margin: 12px 0 28px; }}
.title {{
  font-size: clamp(40px, 8vw, 76px); font-weight: 700; margin: 0; letter-spacing: 1px;
  color: #fff;
  text-shadow: 0 4px 0 var(--navy), 0 -2px 0 var(--navy), 3px 0 0 var(--navy), -3px 0 0 var(--navy),
               0 10px 18px rgba(10, 50, 110, .35);
}}
.tag {{ margin: 6px 0 0; font-size: 18px; font-weight: 600; color: #fff; text-shadow: 0 2px 0 rgba(20,70,140,.5); }}

.card {{
  background: var(--card); border-radius: 26px; box-shadow: var(--shadow);
  position: relative; overflow: hidden;
}}
.card::after {{ /* soft top highlight */
  content: ""; position: absolute; inset: 0 0 auto 0; height: 45%;
  background: linear-gradient(180deg, var(--gloss), rgba(255,255,255,0));
  pointer-events: none;
}}
.hero {{ padding: 28px 28px 30px; text-align: center; border: 6px solid var(--card); outline: 4px solid var(--outline); }}
.hero .big {{
  font-size: clamp(56px, 12vw, 112px); font-weight: 700; line-height: 1;
  background: linear-gradient(180deg, var(--gold-1), var(--gold-2) 55%, var(--gold-3));
  -webkit-background-clip: text; background-clip: text; color: transparent;
  filter: drop-shadow(0 4px 0 #c45a00) drop-shadow(0 10px 12px rgba(200, 100, 0, .3));
}}
.hero .what {{ color: var(--muted); font-weight: 600; margin-top: 6px; }}

.bar {{
  --c: var(--gold-2); height: 30px; border-radius: 999px; margin: 18px 0 8px;
  background: var(--track); box-shadow: inset 0 3px 6px rgba(20, 60, 110, .25);
  overflow: hidden; position: relative;
}}
.bar .fill {{
  width: var(--p); min-width: var(--min, 0px); height: 100%; border-radius: 999px;
  background: linear-gradient(180deg, color-mix(in srgb, var(--c) 45%, #fff) 0%, var(--c) 55%,
              color-mix(in srgb, var(--c) 75%, #000) 100%);
  box-shadow: inset 0 -3px 0 rgba(0,0,0,.12), 0 0 12px color-mix(in srgb, var(--c) 60%, transparent);
  position: relative;
}}
.bar .fill::after {{ /* glossy stripe */
  content: ""; position: absolute; left: 10px; right: 10px; top: 4px; height: 30%;
  border-radius: 999px; background: rgba(255,255,255,.6);
}}
.bar.main {{ height: 38px; --c: #ffc400; border: 4px solid var(--card); box-shadow: inset 0 3px 6px rgba(20,60,110,.3), 0 4px 0 var(--navy); }}
.bar.small {{ height: 16px; margin: 10px 0 6px; }}
.bar.tiny {{ height: 10px; margin: 0; flex: 1 1 auto; --c: #3a8dff; }}

h2 {{
  display: inline-block; margin: 44px 0 16px; padding: 8px 22px; border-radius: 999px;
  background: var(--navy); color: #fff; font-size: 26px; box-shadow: 0 5px 0 var(--navy-2);
}}
.libs {{ display: grid; grid-template-columns: repeat(auto-fill, minmax(230px, 1fr)); gap: 18px; margin-top: 26px; }}
.lib {{ padding: 26px 20px 18px; }}
.lib .band {{ position: absolute; inset: 0 0 auto 0; height: 12px; background: var(--c); z-index: 2; }}
.lib h3 {{ margin: 4px 0 0; font-size: 20px; }}
.lib .pct {{ font-size: 40px; font-weight: 700; color: var(--c); text-shadow: 0 3px 0 rgba(0,0,0,.08); }}
.sub {{ color: var(--muted); font-size: 14px; }}

.folders {{ display: grid; grid-template-columns: repeat(auto-fill, minmax(320px, 1fr)); gap: 16px; }}
.folder {{ padding: 0; }}
.folder summary {{
  list-style: none; cursor: pointer; padding: 16px 20px 12px; display: flex; flex-wrap: wrap;
  align-items: baseline; gap: 4px 10px; position: relative; z-index: 1;
}}
.folder summary::-webkit-details-marker {{ display: none; }}
.folder summary .bar {{ flex-basis: 100%; }}
.fold {{ font-weight: 700; font-size: 19px; }}
.count {{ color: var(--muted); font-size: 13px; }}
.fp {{ margin-left: auto; font-weight: 700; font-variant-numeric: tabular-nums; }}
.fp.big {{ font-size: 20px; color: var(--accent, var(--navy)); }}
.folder ol {{ margin: 0; padding: 4px 20px 18px 42px; position: relative; z-index: 1; }}
.folder li {{ display: flex; align-items: center; gap: 10px; padding: 5px 0; font-size: 14px; }}
.folder li .fname {{ flex: 0 1 46%; overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }}
.folder li .fp {{ flex: 0 0 64px; text-align: right; font-weight: 600; }}
footer {{ text-align: center; margin-top: 48px; color: #fff; font-weight: 600; text-shadow: 0 2px 0 rgba(20,70,140,.5); }}
@media (max-width: 520px) {{ .folders {{ grid-template-columns: 1fr; }} .hero {{ padding: 20px 14px; }} }}
</style>
</head>
<body>
<script>
/* night mode: follows the system setting until toggled; the choice is remembered */
(function () {{
  var t = null;
  try {{ t = localStorage.getItem("theme"); }} catch (e) {{}}
  if (t === "night" || t === "day") document.documentElement.setAttribute("data-theme", t);
}})();
function toggleNight() {{
  var el = document.documentElement, cur = el.getAttribute("data-theme");
  if (!cur) cur = matchMedia("(prefers-color-scheme: dark)").matches ? "night" : "day";
  var next = cur === "night" ? "day" : "night";
  el.setAttribute("data-theme", next);
  try {{ localStorage.setItem("theme", next); }} catch (e) {{}}
}}
</script>
<button class="mode" type="button" onclick="toggleNight()" aria-label="Toggle night mode">
  <span class="i day-only">&#9790;</span><span class="i night-only">&#9728;</span><span class="day-only">Night</span><span class="night-only">Day</span>
</button>
<div class="floor"></div>
<div class="cloud c1"></div><div class="cloud c2"></div><div class="cloud c3"></div>
<main>
  <header>
    <h1 class="title">3DWDecomp</h1>
    <p class="tag">Super Mario 3D World + Bowser's Fury &mdash; matching decompilation</p>
  </header>
  <div class="card hero">
    <div class="big">{total}%</div>
    {bar}
    <div class="what">{matched} / {code} bytes of code &middot; {fmatched} / {ftotal} functions</div>
  </div>
  <div class="libs">{libs}</div>
  {sections}
  <footer>Updated {stamp}{rev}</footer>
</main>
</body>
</html>
"""


def main(argv=None) -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--report", default=str(ROOT / "build" / "report.json"))
    ap.add_argument("-o", "--out", default=str(ROOT / "site" / "index.html"))
    a = ap.parse_args(argv)
    report = json.loads(Path(a.report).read_text())
    out = Path(a.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(build(report), encoding="utf-8")
    print(f"wrote {out}")
    return 0


if __name__ == "__main__":
    import sys
    sys.exit(main())
