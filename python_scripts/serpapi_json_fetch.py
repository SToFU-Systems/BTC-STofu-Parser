from __future__ import annotations

import json
import os
import re
import time
from dataclasses import dataclass
from datetime import datetime, timezone
from pathlib import Path
from typing import Any, Dict, List, Optional

import requests

OUT_DIR = Path("serpapi_output")
OUT_DIR.mkdir(parents=True, exist_ok=True)

RESULTS_FILE = OUT_DIR / "results.jsonl"
API_KEY = os.getenv("SERPAPI_KEY")  # export SERPAPI_KEY="..."
BASE_URL = "https://serpapi.com/search.json"

QUERIES: List[str] = [
    "python selenium webdriver",
    "qt qrc resources audio",   
    "c++ unordered_map vs map",
]

DEFAULT_PARAMS = {
    "engine": "google",
    "hl": "en",
    "gl": "us",
    "num": "10",
}


def now_utc_iso() -> str:
    return datetime.now(timezone.utc).isoformat()

def safe_filename(s: str, max_len: int = 120) -> str:
    s = s.strip()
    s = re.sub(r"\s+", " ", s)
    s = re.sub(r"[^a-zA-Z0-9._ -]+", "_", s)
    s = s.replace(" ", "_")
    return (s[:max_len] if len(s) > max_len else s) or "query"

def append_jsonl(path: Path, obj: Dict[str, Any]) -> None:
    with path.open("a", encoding="utf-8") as f:
        f.write(json.dumps(obj, ensure_ascii=False) + "\n")

@dataclass
class FetchResult:
    ok: bool
    status_code: int
    data: Optional[Dict[str, Any]]
    error: Optional[str]


def serpapi_search(query: str, timeout_sec: int = 30) -> FetchResult:
    if not API_KEY:
        return FetchResult(False, 0, None, "SERPAPI_KEY env var is not set")

    params = dict(DEFAULT_PARAMS)
    params["q"] = query
    params["api_key"] = API_KEY

    try:
        r = requests.get(BASE_URL, params=params, timeout=timeout_sec)
        status = r.status_code

        data = r.json()

        if status != 200:
            return FetchResult(False, status, data, f"HTTP {status}")
        return FetchResult(True, status, data, None)

    except requests.RequestException as e:
        return FetchResult(False, 0, None, f"RequestException: {e}")
    except ValueError as e:
        return FetchResult(False, 0, None, f"JSON decode error: {e}")


def main() -> None:
    for q in QUERIES:
        started = time.time()

        res = serpapi_search(q)

        entry: Dict[str, Any] = {
            "ts": now_utc_iso(),
            "query": q,
            "ok": res.ok,
            "status_code": res.status_code,
            "elapsed_sec": round(time.time() - started, 3),
            "file": None,
            "error": res.error,
        }

        if res.data is not None:
            fname = safe_filename(q) + ".json"
            out_path = OUT_DIR / fname
            out_path.write_text(json.dumps(res.data, ensure_ascii=False, indent=2), encoding="utf-8")
            entry["file"] = str(out_path)

        append_jsonl(RESULTS_FILE, entry)

        time.sleep(0.3)


if __name__ == "__main__":
    main()
