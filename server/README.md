# FastPay FastAPI prototype

This server is an isolated demo backend, not a payment processor. Its sample API does not authenticate users, check balances, or verify offline signatures. It must not receive real customer data, credentials, or funds.

By default, all `/api/v1/` endpoints are disabled and return HTTP 503. The root and health endpoints remain available for deployment checks. To run the unsafe demo API locally, explicitly set `FASTPAY_DEMO_MODE=true`; do not set this on a public or production service.

Run with:

```bash
pip install -r requirements.txt
uvicorn main_production:app --reload --host 127.0.0.1 --port 8000
```

API docs: http://localhost:8000/docs

For an isolated demo only, run with `FASTPAY_DEMO_MODE=true`. Do not expose the demo endpoints to the public internet. `main.py` is an older in-memory prototype and should not be deployed.

Health check: `GET /api/v1/health`. It checks database connectivity and reports whether demo mode is enabled.
