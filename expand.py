import os
import re
import sys
from pathlib import Path

SCRIPT_DIR = Path(__file__).resolve().parent

if (SCRIPT_DIR / "Library").is_dir():
    KYOPRO_HOME = SCRIPT_DIR
else:
    KYOPRO_HOME = SCRIPT_DIR.parent

INCLUDE_RE = re.compile(
    r'^\s*#\s*include\s*(?:<(Library/[^>]+)>|"([^"\n]+)")\s*(?://.*)?$'
)

PRAGMA_ONCE_RE = re.compile(
    r'^\s*#\s*pragma\s+once\s*(?://.*)?$'
)

IF_RE = re.compile(r'^\s*#\s*(if|ifdef|ifndef)\b')
ENDIF_RE = re.compile(r'^\s*#\s*endif\b')


def expand(src):
    expanded = set()
    active = set()
    res = []

    paths = os.environ.get(
        "CPLUS_INCLUDE_PATH", str(KYOPRO_HOME)
    )
    include_dirs = [
        Path(p) for p in paths.split(os.pathsep)
    ]

    def find_header(name, current, quoted):
        dirs = (
            ([current.parent] if quoted else [])
            + include_dirs
        )

        for directory in dirs:
            candidate = directory / name

            if candidate.is_file():
                return candidate.resolve()

        raise FileNotFoundError(
            f"{current}: header not found: {name}"
        )

    def dfs(path, is_header=False):
        path = path.resolve()

        if path in active:
            raise RuntimeError(f"Circular include: {path}")

        if path in expanded:
            return

        lines = path.read_text(
            encoding="utf-8"
        ).splitlines()

        if is_header and not any(
            PRAGMA_ONCE_RE.fullmatch(line)
            for line in lines
        ):
            raise RuntimeError(
                f"Missing #pragma once: {path}"
            )

        active.add(path)
        expanded.add(path)

        depth = 0
        name = os.path.relpath(path, KYOPRO_HOME)

        if is_header:
            res.append(f"// ===== BEGIN {name} =====")

        for line in lines:

            if is_header and PRAGMA_ONCE_RE.fullmatch(line):
                if depth != 0:
                    raise RuntimeError(
                        f"Conditional #pragma once: {path}"
                    )
                continue

            if IF_RE.match(line):
                depth += 1
            elif ENDIF_RE.match(line):
                depth -= 1

            m = INCLUDE_RE.fullmatch(line)

            if m:
                if depth != 0:
                    raise RuntimeError(
                        f"Conditional include: {path}: {line}"
                    )

                quoted = m.group(2) is not None
                header_name = m.group(2) if quoted else m.group(1)

                header = find_header(
                    header_name, path, quoted
                )

                dfs(header, True)

            else:
                res.append(line)

        if is_header:
            res.append(f"// ===== END {name} =====")
            res.append("")

        active.remove(path)

    dfs(src)

    res.append("")
    res.append("// Expanded by https://github.com/Hassy935/kyopro_template/blob/main/expand.py")

    return "\n".join(res)+"\n"


def main():
    if len(sys.argv) != 2:
        print("usage: expand.py <src>", file=sys.stderr)
        return 1

    try:
        result = expand(Path(sys.argv[1]))
    except (OSError, RuntimeError, ValueError) as e:
        print(f"[expand] {e}", file=sys.stderr)
        return 1

    sys.stdout.write(result)
    return 0


if __name__ == "__main__":
    sys.exit(main())