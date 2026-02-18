# file: save_pages_multi_tabs.py
"""
Requirements:
  py -m pip install playwright
  py -m playwright install chromium

This script:
- Launches Chrome/Chromium
- Sets a custom User-Agent
- Opens N tabs at once for N URLs
- Loads all pages concurrently in one session
- Saves each page's full HTML to its own .html file
- Closes each tab only after saving
- Shuts down the browser cleanly

URLs source:
- reads from queries.json рядом со скриптом (JSON array of strings)
"""

from __future__ import annotations

import asyncio
import json
import re
from pathlib import Path
from typing import Iterable

from playwright.async_api import async_playwright, TimeoutError as PlaywrightTimeoutError

# нужно почитать нужно ли 1 или можна 3 юзера агента поставить.
# посмотреть где скрывает окна браузера
CUSTOM_USER_AGENT = (
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) "
    "AppleWebKit/537.36 (KHTML, like Gecko) "
    "Chrome/121.0.0.0 Safari/537.36 MyCustomAgent/1.0"
)

OUTPUT_DIR = Path("selenium_output")
NAV_TIMEOUT_MS = 45_000  # per page navigation timeout
LINKS_JSON = Path(__file__).with_name("queries.json")


def load_urls() -> list[str]:
    if not LINKS_JSON.exists():
        raise SystemExit(f"Не найден файл: {LINKS_JSON}\nСоздай queries.json рядом со скриптом.")

    try:
        data = json.loads(LINKS_JSON.read_text(encoding="utf-8"))
    except json.JSONDecodeError as e:
        raise SystemExit(f"queries.json невалидный JSON: {e}")

    if not isinstance(data, list) or not all(isinstance(u, str) for u in data):
        raise SystemExit("queries.json должен быть JSON-массивом строк (список ссылок).")

    urls = [u.strip() for u in data if u.strip()]
    if not urls:
        raise SystemExit("queries.json пустой: нет ссылок.")

    # уберём дубликаты, сохранив порядок
    seen = set()
    uniq: list[str] = []
    for u in urls:
        if u not in seen:
            seen.add(u)
            uniq.append(u)

    return uniq


def _safe_filename(s: str, max_len: int = 140) -> str:
    s = s.strip().lower()
    s = re.sub(r"^https?://", "", s)
    s = re.sub(r"[^\w\-\.]+", "_", s)
    s = re.sub(r"_+", "_", s).strip("_")
    return (s[:max_len] or "page") + ".html"


async def _fetch_save_close(page, url: str, out_path: Path) -> None:
    try:
        await page.goto(url, wait_until="networkidle", timeout=NAV_TIMEOUT_MS)
    except PlaywrightTimeoutError:
        # Still try to capture whatever loaded so far
        pass

    html = await page.content()
    out_path.write_text(html, encoding="utf-8")
    await page.close()


async def main(urls: Iterable[str]) -> None:
    urls = list(urls)
    if not urls:
        raise SystemExit("No URLs provided.")

    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)

    async with async_playwright() as p:
        # "channel='chrome'" uses installed Chrome if available; otherwise Playwright falls back to Chromium.
        browser = await p.chromium.launch(channel="chrome", headless=True)

        context = await browser.new_context(
            user_agent=CUSTOM_USER_AGENT,
        )

        # 1) Open ALL tabs first (must match URL count).
        pages = [await context.new_page() for _ in urls]

        # 2) Navigate + save + close each tab concurrently.
        tasks = []
        for i, (page, url) in enumerate(zip(pages, urls), start=1):
            filename = f"{i:03d}_{_safe_filename(url)}"
            out_path = OUTPUT_DIR / filename
            tasks.append(_fetch_save_close(page, url, out_path))

        await asyncio.gather(*tasks)

        await context.close()
        await browser.close()


if __name__ == "__main__":
    asyncio.run(main(load_urls()))
