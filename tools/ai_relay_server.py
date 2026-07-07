#!/usr/bin/env python3
"""Tiny HTTP relay for the HMI-Board AI chat demo."""

from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
import json
import os
import re
import urllib.error
import urllib.request


HOST = os.environ.get("AI_RELAY_BIND", "0.0.0.0")
PORT = int(os.environ.get("AI_RELAY_PORT", "8080"))
API_BASE = os.environ.get("AI_API_BASE", "https://api.openai.com/v1/chat/completions")
API_KEY = os.environ.get("AI_API_KEY") or os.environ.get("OPENAI_API_KEY")
MODEL = os.environ.get("AI_MODEL", "gpt-4o-mini")


class Handler(BaseHTTPRequestHandler):
    def do_POST(self):
        if self.path != "/chat":
            self.send_text(404, "Not found")
            return

        length = int(self.headers.get("Content-Length", "0") or "0")
        raw = self.rfile.read(length)

        try:
            payload = json.loads(raw.decode("utf-8"))
            prompt = str(payload.get("prompt", "")).strip()
        except Exception:
            self.send_text(400, "Bad request")
            return

        if not prompt:
            self.send_text(400, "Prompt is empty")
            return

        if not API_KEY:
            self.send_text(500, "Set AI_API_KEY on PC first")
            return

        try:
            answer = call_ai(prompt)
        except Exception as exc:
            print("AI request failed:", exc)
            self.send_text(502, "AI request failed: %s" % exc)
            return

        self.send_text(200, answer)

    def log_message(self, fmt, *args):
        print("%s - %s" % (self.address_string(), fmt % args))

    def send_text(self, code, text):
        body = text.encode("utf-8", errors="replace")
        self.send_response(code)
        self.send_header("Content-Type", "text/plain; charset=utf-8")
        self.send_header("Content-Length", str(len(body)))
        self.end_headers()
        self.wfile.write(body)


def call_ai(prompt):
    data = {
        "model": MODEL,
        "messages": [
            {"role": "system", "content": "You are a concise assistant for a small embedded HMI demo."},
            {"role": "user", "content": prompt},
        ],
        "temperature": 0.7,
        "max_tokens": 180,
    }
    req = urllib.request.Request(
        API_BASE,
        data=json.dumps(data).encode("utf-8"),
        headers={
            "Authorization": "Bearer %s" % API_KEY,
            "Content-Type": "application/json",
        },
        method="POST",
    )

    try:
        with urllib.request.urlopen(req, timeout=30) as resp:
            result = json.loads(resp.read().decode("utf-8"))
    except urllib.error.HTTPError as exc:
        detail = exc.read().decode("utf-8", errors="replace")
        detail = re.sub(r"<[^>]+>", " ", detail)
        detail = " ".join(detail.split())
        raise RuntimeError("HTTP %s %s" % (exc.code, detail[:160]))

    return result["choices"][0]["message"]["content"].strip()


if __name__ == "__main__":
    print("AI relay listening on http://%s:%d/chat" % (HOST, PORT))
    print("Using API_BASE=%s MODEL=%s" % (API_BASE, MODEL))
    ThreadingHTTPServer((HOST, PORT), Handler).serve_forever()
