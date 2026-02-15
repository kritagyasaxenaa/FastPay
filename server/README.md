# FastPay FastAPI Server (skeleton)

Run with:

```bash
pip install -r requirements.txt
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

API docs: http://localhost:8000/docs

**Transaction ID and freezing:**
- Online transactions get a server-generated UUID `tx_id` (stored and returned).
- `POST /api/v1/transactions/verify-id` verifies that a client-held transaction ID matches the server record; on mismatch the account is frozen and 403 is returned.
- `POST /api/v1/users/me/freeze` reports account freeze (e.g. after offline verification failure).

See `../EXPLANATION_SERVER_AND_CACHE.txt` for full design (online storage, payment reception prompting, freezing, offline sync, and offline wallet cache + PIN encryption).
