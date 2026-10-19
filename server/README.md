# FastPay FastAPI prototype

This server is a simulated-bank demo backend, not a payment processor. It does not authenticate end users or verify offline signatures. It must never receive real customer data, credentials, or funds.

By default, all `/api/v1/` endpoints are disabled and return HTTP 503. The root and health endpoints remain available for deployment checks. Set `FASTPAY_DEMO_MODE=true` only for an isolated demo. For the phone-test deployment, also set `FASTPAY_PUBLIC_DEMO_MODE=true`: it exposes only the preseeded fake-account balance and transfer routes, hides API docs, limits requests to 10 transfers/minute and 60 balance reads/minute per client IP, and caps each transfer at INR 50. All other API routes remain blocked. This is abuse resistance for a public play-money demo, not real authentication or a production security boundary.

Run with:

```bash
cd server
pip install -r requirements.txt
FASTPAY_DEMO_MODE=true DATABASE_URL=sqlite:///./fastpay-demo.db \
  uvicorn main_production:app --reload --host 127.0.0.1 --port 8000
```

Then use `http://127.0.0.1:8000` in the app's **Server URL** field or call the API directly. For Android Emulator, the host machine is usually reachable as `http://10.0.2.2:8000`; the server must listen on a host interface reachable from the emulator. For public phone testing, use an HTTPS host and explicitly enable both demo flags. Do not store real user data or enable bank/payment rails.

Example simulated transfer:

```bash
curl -X POST http://127.0.0.1:8000/api/v1/transactions/online \
  -H 'Content-Type: application/json' \
  -d '{"sender_id":"sender@fastpay","receiver_id":"receiver@fastpay","amount":"12.34","nonce":"demo-001"}'
```

Check the simulated balances at `/api/v1/demo/bank-accounts/sender@fastpay` and `/api/v1/demo/bank-accounts/receiver@fastpay`. API docs: http://127.0.0.1:8000/docs

To measure demo transfer latency, keep the local server running in one terminal and run this from the `server` directory in another:

```bash
python benchmark.py --requests 100 --concurrency 10
```

The benchmark permits only localhost/loopback URLs, checks that demo mode is active, and reports p50/p95/p99, throughput, and errors. It moves play money between the seeded Alice and Bob accounts, so repeated runs change their balances. Delete `fastpay-demo.db` to reset them to INR 1,000.00 each.

The demo seeds `sender@fastpay`, `receiver@fastpay`, `alice@fastpay`, `bob@fastpay`, and `charlie@fastpay` with INR 1,000.00 in play money. `POST /api/v1/transactions/online` debits the sender and credits the receiver; repeat requests with the same sender and nonce return the original transfer without a second debit. `GET /api/v1/demo/bank-accounts/{account_id}` shows the simulated balance. This is not connected to any bank or payment rail. `main.py` is an older in-memory prototype and should not be deployed.

Health check: `GET /api/v1/health`. It checks database connectivity and reports whether demo mode is enabled.
