# file: serpapi_fetch.py
"""
Requirements:
  pip install playwright httpx
  playwright install chromium

This script:
- Calls SerpApi to get organic result URLs for each query
- Launches Chrome/Chromium
- Opens N tabs at once for N URLs
- Loads all pages concurrently in one session
- Saves each page's full HTML to its own .html file
- Closes each tab only after saving
- Shuts down the browser cleanly

Setup (PowerShell):
  $env:SERPAPI_KEY="YOUR_KEY"

Setup (bash/zsh):
  export SERPAPI_KEY="YOUR_KEY"
"""

from __future__ import annotations

import asyncio
import os
import re
from pathlib import Path
from typing import Any, Iterable

import httpx
from playwright.async_api import async_playwright, TimeoutError as PlaywrightTimeoutError


QUERIES: list[str] = [
    "Artificial intelligence wikipedia",
    "Quantum mechanics wikipedia",
]

SERPAPI_BASE_URL = "https://serpapi.com/search.json"
SERP_ENGINE = "google"
RESULTS_PER_QUERY = 10
SERP_TIMEOUT_S = 30.0

CUSTOM_USER_AGENT = (
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
    "AppleWebKit/537.36 (KHTML, like Gecko) "
    "Chrome/121.0.0.0 Safari/537.36 MyCustomAgent/1.0"
)

OUTPUT_DIR = Path("serpapi_output")
NAV_TIMEOUT_MS = 45_000  # per page navigation timeout


def _safe_filename(s: str, max_len: int = 140) -> str:
    s = s.strip().lower()
    s = re.sub(r"^https?://", "", s)
    s = re.sub(r"[^\w\-\.]+", "_", s)
    s = re.sub(r"_+", "_", s).strip("_")
    return (s[:max_len] or "page") + ".html"


def _safe_dirname(s: str, max_len: int = 80) -> str:
    s = s.strip().lower()
    s = re.sub(r"[^\w\-]+", "_", s)
    s = re.sub(r"_+", "_", s).strip("_")
    return s[:max_len] or "query"


def _require_serpapi_key() -> str:
    key = os.getenv("SERPAPI_KEY")
    if not key:
        raise SystemExit(
            "Missing SERPAPI_KEY.\n"
            "PowerShell:  $env:SERPAPI_KEY=\"YOUR_KEY\"\n"
            "bash/zsh:    export SERPAPI_KEY=\"YOUR_KEY\""
        )
    return key


async def _serpapi_links(query: str, api_key: str) -> list[str]:
    params: dict[str, Any] = {
        "engine": SERP_ENGINE,
        "q": query,
        "num": RESULTS_PER_QUERY,
        "api_key": api_key,
    }

    async with httpx.AsyncClient(timeout=SERP_TIMEOUT_S) as client:
        r = await client.get(SERPAPI_BASE_URL, params=params)
        r.raise_for_status()
        payload = r.json()

    organic = payload.get("organic_results") or []
    urls: list[str] = []
    for item in organic:
        link = item.get("link")
        if isinstance(link, str) and link.startswith(("http://", "https://")):
            urls.append(link)
        if len(urls) >= RESULTS_PER_QUERY:
            break

    return urls


async def _fetch_save_close(page, url: str, out_path: Path) -> None:
    try:
        await page.goto(url, wait_until="networkidle", timeout=NAV_TIMEOUT_MS)
    except PlaywrightTimeoutError:
        pass

    html = await page.content()
    out_path.write_text(html, encoding="utf-8")
    await page.close()


async def _save_urls_multi_tabs(urls: Iterable[str], out_dir: Path) -> None:
    urls = list(urls)
    if not urls:
        return

    out_dir.mkdir(parents=True, exist_ok=True)

    async with async_playwright() as p:
        browser = await p.chromium.launch(channel="chrome", headless=True)

        context = await browser.new_context(
            user_agent=CUSTOM_USER_AGENT,
        )

        pages = [await context.new_page() for _ in urls]

        tasks = []
        for i, (page, url) in enumerate(zip(pages, urls), start=1):
            filename = f"{i:03d}_{_safe_filename(url)}"
            out_path = out_dir / filename
            tasks.append(_fetch_save_close(page, url, out_path))

        await asyncio.gather(*tasks)

        await context.close()
        await browser.close()


async def main(queries: Iterable[str]) -> None:
    queries = [q.strip() for q in queries if q.strip()]
    if not queries:
        raise SystemExit("No queries provided in QUERIES list.")

    api_key = _require_serpapi_key()
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    for query in queries:
        urls = await _serpapi_links(query, api_key)
        query_dir = OUTPUT_DIR / _safe_dirname(query)
        await _save_urls_multi_tabs(urls, query_dir)


if __name__ == "__main__":
    asyncio.run(main(QUERIES))
