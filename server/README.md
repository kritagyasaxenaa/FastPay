# FastPay FastAPI prototype

This server is an isolated demo backend, not a payment processor. Its sample API does not authenticate users, check balances, or verify offline signatures. It must not receive real customer data, credentials, or funds.

By default, all `/api/v1/` endpoints are disabled and return HTTP 503. The root and health endpoints remain available for deployment checks. To run the unsafe demo API locally, explicitly set `FASTPAY_DEMO_MODE=true`; do not set this on a public or production service.

Run with:

```bash
cd server
pip install -r requirements.txt
FASTPAY_DEMO_MODE=true DATABASE_URL=sqlite:///./fastpay-demo.db \
  uvicorn main_production:app --reload --host 127.0.0.1 --port 8000
```

Then use `http://127.0.0.1:8000` in the app's **Server URL** field or call the API directly. For Android Emulator, the host machine is usually reachable as `http://10.0.2.2:8000`; the server must listen on a host interface reachable from the emulator. Keep it on a trusted local network and stop it after the demo.

Example simulated transfer:

```bash
curl -X POST http://127.0.0.1:8000/api/v1/transactions/online \
  -H 'Content-Type: application/json' \
  -d '{"sender_id":"sender@fastpay","receiver_id":"receiver@fastpay","amount":"12.34","nonce":"demo-001"}'
```

Check the simulated balances at `/api/v1/demo/bank-accounts/sender@fastpay` and `/api/v1/demo/bank-accounts/receiver@fastpay`. API docs: http://127.0.0.1:8000/docs

For an isolated demo only, run with `FASTPAY_DEMO_MODE=true`. Do not expose the demo endpoints to the public internet. The demo seeds `sender@fastpay`, `receiver@fastpay`, `alice@fastpay`, `bob@fastpay`, and `charlie@fastpay` with INR 1,000.00 in play money. `POST /api/v1/transactions/online` debits the sender and credits the receiver; repeat requests with the same sender and nonce return the original transfer without a second debit. `GET /api/v1/demo/bank-accounts/{account_id}` shows the simulated balance. This is not connected to any bank or payment rail. `main.py` is an older in-memory prototype and should not be deployed.

Health check: `GET /api/v1/health`. It checks database connectivity and reports whether demo mode is enabled.
