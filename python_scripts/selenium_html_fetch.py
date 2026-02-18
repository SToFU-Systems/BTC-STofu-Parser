from __future__ import annotations

import json
import re
import time
from datetime import datetime, timezone
from pathlib import Path
from urllib.parse import urlparse
    
from selenium import webdriver
from selenium.webdriver.chrome.options import Options
from selenium.webdriver.support.ui import WebDriverWait


URLS = [
    "https://en.wikipedia.org/wiki/Python_(programming_language)",
    "https://en.wikipedia.org/wiki/Selenium_(software)",
    "https://en.wikipedia.org/wiki/Web_browser",
    "https://en.wikipedia.org/wiki/Artificial_intelligence",
]

OUT_DIR = Path("selenium_output")
OUT_DIR.mkdir(parents=True, exist_ok=True)
RESULTS_FILE = OUT_DIR / "results.jsonl"


def safe_name_from_url(url: str) -> str:
    p = urlparse(url)
    host = p.netloc or "site"
    path = p.path.strip("/").replace("/", "_") or "root"
    path = re.sub(r"[^a-zA-Z0-9._-]+", "_", path)
    return f"{host}__{path}"


def build_driver() -> webdriver.Chrome:
    opts = Options()
    # opts.add_argument("--headless=new")  # если нужно без окна
    opts.add_argument("--start-maximized")
    opts.add_argument("--disable-gpu")
    opts.add_argument("--no-sandbox")

    driver = webdriver.Chrome(options=opts)
    driver.set_page_load_timeout(60)
    return driver


def wait_dom_ready(driver: webdriver.Chrome, timeout_sec: int = 30) -> None:
    WebDriverWait(driver, timeout_sec).until(
        lambda d: d.execute_script("return document.readyState") == "complete"
    )


def save_html(out_path: Path, html: str) -> None:
    out_path.write_text(html, encoding="utf-8", errors="ignore")


def append_log(entry: dict) -> None:
    with RESULTS_FILE.open("a", encoding="utf-8") as f:
        f.write(json.dumps(entry, ensure_ascii=False) + "\n")


def main() -> None:
    driver = build_driver()
    base_handle = driver.current_window_handle

    try:
        # 1) открываем вкладки (пустые)
        for _ in range(len(URLS) - 1):
            driver.switch_to.new_window("tab")

        handles = driver.window_handles  # их стало len(URLS)

        # 2) по каждой вкладке: перейти на URL, дождаться загрузки, сохранить HTML, закрыть вкладку
        for handle, url in zip(handles, URLS):
            started = time.time()
            entry = {
                "ts": datetime.now(timezone.utc).isoformat(),
                "url": url,
                "ok": False,
                "file": None,
                "error": None,
            }

            try:
                driver.switch_to.window(handle)
                driver.get(url)
                wait_dom_ready(driver, timeout_sec=30)

                name = safe_name_from_url(url)
                html_path = OUT_DIR / f"{name}.html"
                save_html(html_path, driver.page_source)

                entry["ok"] = True
                entry["file"] = str(html_path)
            except Exception as e:
                entry["error"] = f"{type(e).__name__}: {e}"
            finally:
                entry["elapsed_sec"] = round(time.time() - started, 3)
                append_log(entry)

                if handle != base_handle:
                    try:
                        driver.close()
                    except Exception:
                        pass

    finally:
        driver.quit()


if __name__ == "__main__":
    main()
