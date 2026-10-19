# FastPay FastAPI prototype

This server is an isolated demo backend, not a payment processor. Its sample API does not authenticate users, check balances, or verify offline signatures. It must not receive real customer data, credentials, or funds.

By default, all `/api/v1/` endpoints are disabled and return HTTP 503. The root and health endpoints remain available for deployment checks. To run the unsafe demo API locally, explicitly set `FASTPAY_DEMO_MODE=true`; do not set this on a public or production service.

Run with:

```bash
pip install -r requirements.txt
uvicorn main_production:app --reload --host 127.0.0.1 --port 8000
```

API docs: http://localhost:8000/docs

For an isolated demo only, run with `FASTPAY_DEMO_MODE=true`. Do not expose the demo endpoints to the public internet. The demo seeds `sender@fastpay`, `receiver@fastpay`, `alice@fastpay`, `bob@fastpay`, and `charlie@fastpay` with INR 1,000.00 in play money. `POST /api/v1/transactions/online` debits the sender and credits the receiver; repeat requests with the same sender and nonce return the original transfer without a second debit. `GET /api/v1/demo/bank-accounts/{account_id}` shows the simulated balance. This is not connected to any bank or payment rail. `main.py` is an older in-memory prototype and should not be deployed.

Health check: `GET /api/v1/health`. It checks database connectivity and reports whether demo mode is enabled.
