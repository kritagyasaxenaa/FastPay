"""
FastPay FastAPI server (skeleton).

Responsibilities:
- Online transaction storage
- Prompting on payment reception (notifications / push)
- Account freezing (POST /users/me/freeze)
- Offline transaction storage when clients sync (POST /transactions/offline/sync)
"""

import uuid
from fastapi import FastAPI, HTTPException, Depends
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from typing import Optional
from datetime import datetime

app = FastAPI(title="FastPay API", version="1.0.0")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# ---------------------------------------------------------------------------
# In-memory store (replace with DB: SQLAlchemy, etc.)
# ---------------------------------------------------------------------------
ONLINE_TRANSACTIONS = []
OFFLINE_TRANSACTIONS = []
FROZEN_USERS = set()
DEVICE_TOKENS = {}  # user_id -> { "token": "...", "platform": "android" }

# ---------------------------------------------------------------------------
# Pydantic models
# ---------------------------------------------------------------------------

class OnlineTransactionSubmit(BaseModel):
    sender_id: str
    receiver_id: str
    amount: str
    nonce: str  # timestamp "YYYY-MM-DD HH:MM:SS"
    pin_verification_token: Optional[str] = None

class OfflineTransactionItem(BaseModel):
    tx_id: str
    sender_id: str
    receiver_id: str
    amount: str
    nonce: str
    sender_signature: str
    receiver_receipt_signature: str
    created_at: Optional[str] = None

class OfflineSyncPayload(BaseModel):
    user_id: str
    transactions: list[OfflineTransactionItem]

class FreezePayload(BaseModel):
    user_id: str  # who to freeze (in production from JWT)
    reason: str = "offline_verification_failed"
    nonce: Optional[str] = None
    details: Optional[str] = None

class DeviceTokenPayload(BaseModel):
    token: str
    platform: str = "android"

class VerifyTransactionIdPayload(BaseModel):
    """Verify that the given transaction_id matches the one stored for this transaction."""
    user_id: str
    transaction_id: str
    sender_id: str
    receiver_id: str
    nonce: str
    amount: str

# ---------------------------------------------------------------------------
# Helpers: check frozen, notify receiver (stub)
# ---------------------------------------------------------------------------

def ensure_not_frozen(user_id: str):
    if user_id in FROZEN_USERS:
        raise HTTPException(status_code=403, detail="Account frozen. Contact support.")

def notify_receiver_payment_received(receiver_id: str, amount: str, sender_id: str):
    """Trigger payment reception prompt: push / WebSocket / etc."""
    # TODO: FCM push to DEVICE_TOKENS.get(receiver_id), or send via WebSocket
    print(f"[NOTIFY] {receiver_id}: You received {amount} from {sender_id}")

# ---------------------------------------------------------------------------
# Routes
# ---------------------------------------------------------------------------

@app.post("/api/v1/transactions/online")
def submit_online_transaction(body: OnlineTransactionSubmit):
    """Store online transaction and prompt receiver."""
    ensure_not_frozen(body.sender_id)
    ensure_not_frozen(body.receiver_id)
    # TODO: validate nonce (replay window), PIN token, balances
    tx_id = str(uuid.uuid4())
    tx = {
        "id": tx_id,
        "sender_id": body.sender_id,
        "receiver_id": body.receiver_id,
        "amount": body.amount,
        "nonce": body.nonce,
        "status": "completed",
        "created_at": datetime.utcnow().isoformat(),
    }
    ONLINE_TRANSACTIONS.append(tx)
    notify_receiver_payment_received(body.receiver_id, body.amount, body.sender_id)
    return {"tx_id": tx["id"], "status": "completed"}

@app.get("/api/v1/transactions/online")
def list_online_transactions(user_id: str, since: Optional[str] = None):
    """List online transactions for a user (e.g. for history / polling)."""
    ensure_not_frozen(user_id)
    out = [t for t in ONLINE_TRANSACTIONS if t["sender_id"] == user_id or t["receiver_id"] == user_id]
    if since:
        out = [t for t in out if t.get("created_at", "") >= since]
    return {"transactions": out}

@app.post("/api/v1/transactions/offline/sync")
def sync_offline_transactions(body: OfflineSyncPayload):
    """Store offline transactions when client comes online."""
    ensure_not_frozen(body.user_id)
    accepted = []
    rejected = []
    for tx in body.transactions:
        key = (tx.sender_id, tx.receiver_id, tx.nonce)
        if any(t["sender_id"] == tx.sender_id and t["receiver_id"] == tx.receiver_id and t["nonce"] == tx.nonce
               for t in OFFLINE_TRANSACTIONS):
            rejected.append({"tx_id": tx.tx_id, "reason": "duplicate"})
            continue
        # TODO: verify sender_signature and receiver_receipt_signature
        OFFLINE_TRANSACTIONS.append({
            "tx_id": tx.tx_id,
            "sender_id": tx.sender_id,
            "receiver_id": tx.receiver_id,
            "amount": tx.amount,
            "nonce": tx.nonce,
            "synced_at": datetime.utcnow().isoformat(),
        })
        accepted.append(tx.tx_id)
    return {"accepted": accepted, "rejected": rejected}

@app.post("/api/v1/transactions/verify-id")
def verify_transaction_id(body: VerifyTransactionIdPayload):
    """
    Verify that the provided transaction_id matches the one stored for this transaction.
    If it does not match (e.g. tampering or inconsistency), the account is frozen and 403 is returned.
    """
    ensure_not_frozen(body.user_id)
    for tx in ONLINE_TRANSACTIONS:
        if (tx["sender_id"] == body.sender_id and tx["receiver_id"] == body.receiver_id
                and tx["nonce"] == body.nonce and tx["amount"] == body.amount):
            if tx["id"] != body.transaction_id:
                FROZEN_USERS.add(body.user_id)
                raise HTTPException(
                    status_code=403,
                    detail="Account frozen. Transaction ID mismatch detected."
                )
            return {"verified": True, "transaction_id": tx["id"]}
    raise HTTPException(status_code=404, detail="Transaction not found")

@app.post("/api/v1/users/me/freeze")
def report_freeze(body: FreezePayload):
    """Report account freeze after failed offline verification. In production, user_id from JWT."""
    FROZEN_USERS.add(body.user_id)
    return {"status": "frozen", "message": "Account frozen. Contact support."}

@app.get("/api/v1/users/me/status")
def user_status(user_id: str):
    """Return frozen status (and optional balance). In production, user_id from JWT."""
    return {"frozen": user_id in FROZEN_USERS}

@app.post("/api/v1/users/me/device-token")
def register_device_token(body: DeviceTokenPayload, user_id: str = "me"):
    """Register device for payment reception push notifications. Pass user_id as query."""
    ensure_not_frozen(user_id)
    DEVICE_TOKENS[user_id] = {"token": body.token, "platform": body.platform}
    return {"registered": True}

# ---------------------------------------------------------------------------
# Run: uvicorn main:app --reload --host 0.0.0.0 --port 8000
# ---------------------------------------------------------------------------
