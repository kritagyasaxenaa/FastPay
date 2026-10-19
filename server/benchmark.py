"""Bounded local-only latency benchmark for the FastPay demo transfer API."""

import argparse
import concurrent.futures
import ipaddress
import json
import math
import statistics
import time
import urllib.error
import urllib.parse
import urllib.request
import uuid


def percentile(samples, percentile_value):
    ordered = sorted(samples)
    return ordered[max(0, math.ceil(percentile_value * len(ordered)) - 1)]


def request_json(url, payload=None):
    body = json.dumps(payload).encode("utf-8") if payload is not None else None
    request = urllib.request.Request(
        url,
        data=body,
        headers={"Content-Type": "application/json", "Accept": "application/json"},
        method="POST" if payload is not None else "GET",
    )
    try:
        response = urllib.request.urlopen(request, timeout=10)
    except urllib.error.HTTPError as error:
        response = error
    return response.status, json.loads(response.read().decode("utf-8"))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", default="http://127.0.0.1:8000", help="Local API base URL")
    parser.add_argument("--requests", type=int, default=100, help="Number of simulated transfers (max 10000)")
    parser.add_argument("--concurrency", type=int, default=10, help="Concurrent clients (max 100)")
    parser.add_argument("--amount", default="0.01", help="INR per transfer; defaults to one paisa")
    args = parser.parse_args()

    parsed_url = urllib.parse.urlsplit(args.url)
    if parsed_url.scheme not in ("http", "https") or not parsed_url.hostname:
        parser.error("--url must be an http(s) URL")
    try:
        loopback = ipaddress.ip_address(parsed_url.hostname).is_loopback
    except ValueError:
        loopback = parsed_url.hostname.lower() == "localhost"
    if not loopback:
        parser.error("This benchmark only permits localhost or a loopback IP; it will not load a public server")
    if not 1 <= args.requests <= 10_000:
        parser.error("--requests must be between 1 and 10000")
    if not 1 <= args.concurrency <= 100:
        parser.error("--concurrency must be between 1 and 100")

    base_url = args.url.rstrip("/")
    status, health = request_json(base_url + "/api/v1/health")
    if status != 200 or health.get("mode") != "demo":
        parser.error("API health check failed or demo mode is disabled")

    def transfer(_index):
        payload = {
            "sender_id": "alice@fastpay",
            "receiver_id": "bob@fastpay",
            "amount": args.amount,
            "nonce": str(uuid.uuid4()),
        }
        started = time.perf_counter()
        status_code, result = request_json(base_url + "/api/v1/transactions/online", payload)
        elapsed_ms = (time.perf_counter() - started) * 1000
        if status_code != 200 or result.get("demo") is not True:
            raise RuntimeError(f"HTTP {status_code}: {result}")
        return elapsed_ms

    started = time.perf_counter()
    failures = []
    timings = []
    with concurrent.futures.ThreadPoolExecutor(max_workers=args.concurrency) as executor:
        futures = [executor.submit(transfer, index) for index in range(args.requests)]
        for future in concurrent.futures.as_completed(futures):
            try:
                timings.append(future.result())
            except Exception as error:  # Report failures instead of hiding them from the latency summary.
                failures.append(str(error))
    elapsed_s = time.perf_counter() - started

    print(f"endpoint=POST /api/v1/transactions/online mode=demo-only")
    print(f"requests={args.requests} concurrency={args.concurrency} successful={len(timings)} errors={len(failures)}")
    if timings:
        print(
            "latency_ms "
            f"p50={statistics.median(timings):.2f} "
            f"p95={percentile(timings, 0.95):.2f} "
            f"p99={percentile(timings, 0.99):.2f} "
            f"max={max(timings):.2f}"
        )
    print(f"throughput_requests_per_second={len(timings) / elapsed_s:.2f}")
    if failures:
        print("sample_error=" + failures[0])
        raise SystemExit(1)


if __name__ == "__main__":
    main()
