"""
커밋 메시지를 파싱해서 docs/gantt.svg(개발 일지)를 자동 업데이트하는 스크립트.

커밋 메시지 형식:
  feat(전투): 전투 루프 구현
  fix(UI): HP 바 렌더링 버그 수정
  refactor(플레이어): Player 클래스 분리
  docs(readme): 주석 추가
"""

import subprocess
import re
import os
import textwrap
from datetime import datetime, timezone, timedelta

README_PATH = "README.md"
SVG_PATH = "docs/gantt.svg"

SECTION_STATUS = {
    "feat":     "done",
    "fix":      "crit",
    "refactor": "active",
    "docs":     None,
    "chore":    None,
}

KST = timezone(timedelta(hours=9))


def get_latest_commits():
    before = os.environ.get("BEFORE_SHA", "")
    after  = os.environ.get("GITHUB_SHA", "HEAD")
    if before and before != "0" * 40:
        log_range = f"{before}..{after}"
    else:
        log_range = "-1"
    result = subprocess.run(
        ["git", "log", log_range, "--pretty=format:%s"],
        capture_output=True, text=True
    )
    return result.stdout.strip().splitlines()


def parse_commit(message: str):
    m = re.match(r"^(\w+)\(([^)]+)\):\s*(.+)$", message.strip())
    if not m:
        return None
    commit_type, section, description = m.groups()
    if commit_type not in SECTION_STATUS:
        return None
    return commit_type, section, description


# ── SVG 간트 ────────────────────────────────────────────────

def load_svg_tasks(svg_path: str) -> list[dict]:
    """기존 SVG에서 태스크 목록을 복원한다."""
    if not os.path.exists(svg_path):
        return []
    with open(svg_path, encoding="utf-8") as f:
        raw = f.read()
    tasks = []
    for m in re.finditer(
        r'data-date="([^"]+)" data-section="([^"]+)" data-status="([^"]+)" data-desc="([^"]+)"',
        raw
    ):
        tasks.append({"date": m.group(1), "section": m.group(2),
                      "status": m.group(3), "desc": m.group(4)})
    return tasks


def _wrap(text: str, width: int = 28) -> list[str]:
    return textwrap.wrap(text, width) or [text]


def build_svg(tasks: list[dict]) -> str:
    if not tasks:
        tasks_to_show = []
    else:
        tasks_to_show = tasks[-40:]          # 최대 40개 표시

    ROW_H   = 28
    HDR_H   = 48
    PAD     = 16
    LABEL_W = 160
    BAR_W   = 340
    W       = LABEL_W + BAR_W + PAD * 3
    H       = HDR_H + ROW_H * max(len(tasks_to_show), 1) + PAD * 2

    # 날짜 범위
    if tasks_to_show:
        dates = sorted(t["date"] for t in tasks_to_show)
        d_min = dates[0]
        d_max = dates[-1]
    else:
        today = datetime.now(KST).strftime("%Y-%m-%d")
        d_min = d_max = today

    def date_x(date_str: str) -> float:
        d0 = datetime.strptime(d_min, "%Y-%m-%d")
        d1 = datetime.strptime(d_max, "%Y-%m-%d")
        d  = datetime.strptime(date_str, "%Y-%m-%d")
        span = max((d1 - d0).days, 1)
        return PAD * 2 + LABEL_W + (d - d0).days / span * (BAR_W - 24)

    STATUS_COLOR = {
        "done":   "#4A90D9",
        "crit":   "#E05C5C",
        "active": "#50B86C",
        "other":  "#A0A0A0",
    }

    lines = [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{W}" height="{H}" '
        f'style="font-family:'Segoe UI',sans-serif;background:#1e1e2e;">',
        # 메타데이터 (태스크 복원용)
    ]
    for t in tasks_to_show:
        lines.append(
            f'<!-- task data-date="{t["date"]}" data-section="{t["section"]}" '
            f'data-status="{t["status"]}" data-desc="{t["desc"]}" -->'
        )

    # 헤더
    lines += [
        f'<rect width="{W}" height="{HDR_H}" fill="#13131f"/>',
        f'<text x="{W//2}" y="30" text-anchor="middle" '
        f'font-size="15" font-weight="bold" fill="#cdd6f4">Tower of Omens — 개발 일지</text>',
        f'<text x="{W//2}" y="44" text-anchor="middle" '
        f'font-size="10" fill="#6c7086">{d_min} ~ {d_max}</text>',
    ]

    # 행
    for i, t in enumerate(tasks_to_show):
        y     = HDR_H + PAD + i * ROW_H
        color = STATUS_COLOR.get(t["status"], STATUS_COLOR["other"])
        x_bar = date_x(t["date"])

        # 배경 줄무늬
        if i % 2 == 0:
            lines.append(f'<rect x="0" y="{y}" width="{W}" height="{ROW_H}" fill="#181825"/>')

        # 섹션 레이블
        label = f'{t["section"]}'
        lines.append(
            f'<text x="{PAD}" y="{y + ROW_H//2 + 4}" '
            f'font-size="11" fill="#cdd6f4">{label}</text>'
        )

        # 바
        lines.append(
            f'<rect x="{x_bar}" y="{y+4}" width="120" height="{ROW_H-8}" '
            f'rx="4" fill="{color}" opacity="0.85"/>'
        )

        # 설명 텍스트
        short_desc = t["desc"][:22] + ("…" if len(t["desc"]) > 22 else "")
        lines.append(
            f'<text x="{x_bar + 6}" y="{y + ROW_H//2 + 4}" '
            f'font-size="10" fill="#ffffff">{short_desc}</text>'
        )

        # 날짜
        lines.append(
            f'<text x="{W - PAD}" y="{y + ROW_H//2 + 4}" '
            f'text-anchor="end" font-size="9" fill="#6c7086">{t["date"]}</text>'
        )

    lines.append('</svg>')
    return "\n".join(lines)


# ── 작업 로그 (README 테이블) ────────────────────────────────

def parse_existing_log(content: str) -> list[str]:
    m = re.search(r"<!-- LOG_START -->\n(.*?)\n<!-- LOG_END -->", content, re.DOTALL)
    if not m:
        return []
    rows = []
    for line in m.group(1).splitlines():
        if line.startswith("| ") and "---" not in line and "날짜" not in line:
            rows.append(line)
    return rows


def build_log(rows: list[str]) -> str:
    header = "| 날짜 | 섹션 | 작업 내용 |\n|------|------|-----------|"
    return header + "\n" + "\n".join(rows)


def replace_block(content: str, start_tag: str, end_tag: str, new_block: str) -> str:
    pattern = re.compile(
        rf"({re.escape(start_tag)}\n).*?(\n{re.escape(end_tag)})",
        re.DOTALL
    )
    return pattern.sub(rf"\1{new_block}\2", content)


# ── 메인 ────────────────────────────────────────────────────

def main():
    commits = get_latest_commits()
    if not commits:
        print("파싱할 커밋 없음. 종료.")
        return

    today   = datetime.now(KST).strftime("%Y-%m-%d")
    tasks   = load_svg_tasks(SVG_PATH)

    with open(README_PATH, encoding="utf-8") as f:
        readme = f.read()
    log_rows = parse_existing_log(readme)

    changed = False
    for msg in commits:
        parsed = parse_commit(msg)
        if not parsed:
            print(f"  skip: {msg}")
            continue
        commit_type, section, description = parsed
        print(f"  ✅ {commit_type}({section}): {description}")

        status = SECTION_STATUS.get(commit_type, "done") or "other"
        tasks.append({"date": today, "section": section,
                      "status": status, "desc": description})
        log_rows.append(f"| {today} | {section} | {description} |")
        changed = True

    if not changed:
        print("업데이트할 내용 없음.")
        return

    # SVG 저장
    os.makedirs("docs", exist_ok=True)
    with open(SVG_PATH, "w", encoding="utf-8") as f:
        f.write(build_svg(tasks))
    print(f"SVG 저장: {SVG_PATH}")

    # README 로그 블록 갱신
    new_log = build_log(log_rows)
    readme  = replace_block(readme, "<!-- LOG_START -->", "<!-- LOG_END -->", new_log)
    with open(README_PATH, "w", encoding="utf-8") as f:
        f.write(readme)
    print("README 업데이트 완료!")


if __name__ == "__main__":
    main()
