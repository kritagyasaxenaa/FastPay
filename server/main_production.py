"""
FastPay Production Server with PostgreSQL Database
Supports multiple Android devices connecting simultaneously
"""

import uuid
import os
from contextlib import asynccontextmanager
from datetime import datetime, timezone
from typing import Optional, List
from fastapi import FastAPI, HTTPException, Depends
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
from pydantic import BaseModel
from sqlalchemy import create_engine, Column, String, DateTime, Boolean, Integer, UniqueConstraint, text
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import sessionmaker, Session

# Database setup
DATABASE_URL = os.getenv("DATABASE_URL", "sqlite:///./fastpay.db")
# Fix for Render's postgres:// instead of postgresql://
if DATABASE_URL.startswith("postgres://"):
    DATABASE_URL = DATABASE_URL.replace("postgres://", "postgresql://", 1)

engine = create_engine(DATABASE_URL, connect_args={"check_same_thread": False} if "sqlite" in DATABASE_URL else {})
SessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
Base = declarative_base()

# Database Models
class User(Base):
    __tablename__ = "users"
    user_id = Column(String, primary_key=True, index=True)
    phone_number = Column(String, unique=True, index=True)
    pin_hash = Column(String)  # Store hashed PIN
    is_frozen = Column(Boolean, default=False)
    created_at = Column(DateTime, default=datetime.utcnow)
    last_active = Column(DateTime, default=datetime.utcnow)

class Transaction(Base):
    __tablename__ = "transactions"
    tx_id = Column(String, primary_key=True, index=True)
    sender_id = Column(String, index=True)
    receiver_id = Column(String, index=True)
    amount = Column(String)
    nonce = Column(String)
    tx_type = Column(String)  # "online" or "offline"
    status = Column(String, default="completed")  # "pending", "completed", "failed"
    sender_signature = Column(String, nullable=True)
    receiver_receipt = Column(String, nullable=True)
    created_at = Column(DateTime, default=datetime.utcnow)

class DeviceToken(Base):
    __tablename__ = "device_tokens"
    user_id = Column(String, primary_key=True)
    token = Column(String)
    platform = Column(String, default="android")
    updated_at = Column(DateTime, default=datetime.utcnow)

class DemoBankAccount(Base):
    """Play-money account used only by the local/demo API."""
    __tablename__ = "demo_bank_accounts"
    account_id = Column(String, primary_key=True, index=True)
    currency = Column(String, nullable=False, default="INR")
    balance_paise = Column(Integer, nullable=False, default=0)
    updated_at = Column(DateTime, default=datetime.utcnow, onupdate=datetime.utcnow)

class DemoBankTransfer(Base):
    """Immutable record of a simulated transfer between demo accounts."""
    __tablename__ = "demo_bank_transfers"
    __table_args__ = (UniqueConstraint("idempotency_key", name="uq_demo_bank_transfer_idem"),)
    reference = Column(String, primary_key=True)
    idempotency_key = Column(String, nullable=False)
    sender_id = Column(String, nullable=False, index=True)
    receiver_id = Column(String, nullable=False, index=True)
    amount_paise = Column(Integer, nullable=False)
    currency = Column(String, nullable=False, default="INR")
    status = Column(String, nullable=False, default="completed")
    created_at = Column(DateTime, default=datetime.utcnow)

# Create tables
Base.metadata.create_all(bind=engine)

@asynccontextmanager
async def lifespan(_app):
    if os.getenv("FASTPAY_DEMO_MODE", "").lower() == "true":
        db = SessionLocal()
        try:
            ensure_demo_bank_accounts(db)
        finally:
            db.close()
    yield

# FastAPI app
app = FastAPI(title="FastPay API (demo only)", version="2.1.0", lifespan=lifespan)

# This API is a prototype, not a payment processor. Its current transaction
# endpoints do not authenticate users, check balances, or verify offline
# signatures. Refuse all API traffic except health checks unless the operator
# explicitly enables demo mode. Never enable this flag on a public deployment.
@app.middleware("http")
async def require_explicit_demo_mode(request, call_next):
    if (
        request.url.path.startswith("/api/v1/")
        and request.url.path != "/api/v1/health"
        and os.getenv("FASTPAY_DEMO_MODE", "").lower() != "true"
    ):
        return JSONResponse(
            status_code=503,
            content={
                "detail": "Payment API is disabled: this prototype is not production-ready.",
                "code": "payment_api_disabled",
            },
        )
    return await call_next(request)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Dependency
def get_db():
    db = SessionLocal()
    try:
        yield db
    finally:
        db.close()

# Pydantic models
class OnlineTransactionSubmit(BaseModel):
    sender_id: str
    receiver_id: str
    amount: str
    nonce: str
    pin_verification_token: Optional[str] = None

class OfflineTransactionItem(BaseModel):
    tx_id: str
    sender_id: str
    receiver_id: str
    amount: str
    nonce: str
    sender_signature: str
    receiver_receipt_signature: str

class OfflineSyncPayload(BaseModel):
    user_id: str
    transactions: List[OfflineTransactionItem]

class FreezePayload(BaseModel):
    user_id: str
    reason: str = "offline_verification_failed"
    details: Optional[str] = None

class DeviceTokenPayload(BaseModel):
    token: str
    platform: str = "android"

class VerifyTransactionIdPayload(BaseModel):
    user_id: str
    transaction_id: str
    sender_id: str
    receiver_id: str
    nonce: str
    amount: str

class UserRegistration(BaseModel):
    user_id: str
    phone_number: str
    pin_hash: str

# Helper functions
def ensure_not_frozen(user_id: str, db: Session):
    user = db.query(User).filter(User.user_id == user_id).first()
    if user and user.is_frozen:
        raise HTTPException(status_code=403, detail="Account frozen. Contact support.")

def notify_receiver(receiver_id: str, amount: str, sender_id: str, db: Session):
    """Send push notification to receiver (stub for now)"""
    device = db.query(DeviceToken).filter(DeviceToken.user_id == receiver_id).first()
    if device:
        print(f"[PUSH] Sending to {receiver_id}: Received {amount} from {sender_id}")
        # TODO: Implement FCM push notification here
    else:
        print(f"[NOTIFY] {receiver_id}: Received {amount} from {sender_id} (no device token)")

def ensure_demo_bank_accounts(db: Session):
    """Create deterministic play-money accounts when the demo API is used."""
    demo_accounts = (
        "sender@fastpay",
        "receiver@fastpay",
        "alice@fastpay",
        "bob@fastpay",
        "charlie@fastpay",
    )
    for account_id in demo_accounts:
        account = db.query(DemoBankAccount).filter(DemoBankAccount.account_id == account_id).first()
        if account is None:
            db.add(DemoBankAccount(account_id=account_id, currency="INR", balance_paise=100_000))
    db.commit()

def amount_to_paise(amount: str) -> int:
    """Parse a positive INR amount with at most two decimal places."""
    from decimal import Decimal, InvalidOperation

    try:
        value = Decimal(amount)
    except (InvalidOperation, TypeError):
        raise HTTPException(status_code=422, detail="amount_must_be_a_valid_decimal")
    if not value.is_finite() or value <= 0 or value.as_tuple().exponent < -2:
        raise HTTPException(status_code=422, detail="amount_must_be_positive_inr_with_at_most_two_decimals")
    return int(value * 100)

def format_inr(paise: int) -> str:
    return f"{paise // 100}.{paise % 100:02d}"

# Routes
@app.get("/")
def root():
    return {
        "status": "FastPay API demo prototype",
        "payment_api_enabled": os.getenv("FASTPAY_DEMO_MODE", "").lower() == "true",
    }

@app.post("/api/v1/users/register")
def register_user(user: UserRegistration, db: Session = Depends(get_db)):
    """Register a new user"""
    existing = db.query(User).filter(User.user_id == user.user_id).first()
    if existing:
        raise HTTPException(status_code=400, detail="User already exists")
    
    new_user = User(
        user_id=user.user_id,
        phone_number=user.phone_number,
        pin_hash=user.pin_hash
    )
    db.add(new_user)
    db.commit()
    return {"status": "registered", "user_id": user.user_id}

@app.post("/api/v1/transactions/online")
def submit_online_transaction(body: OnlineTransactionSubmit, db: Session = Depends(get_db)):
    """Simulate an online payment between the seeded play-money accounts."""
    if len(body.nonce) > 128 or not body.nonce.strip():
        raise HTTPException(status_code=422, detail="nonce_required_and_must_be_at_most_128_characters")
    amount_paise = amount_to_paise(body.amount)
    if body.sender_id == body.receiver_id:
        raise HTTPException(status_code=422, detail="sender_and_receiver_must_differ")

    ensure_demo_bank_accounts(db)
    ensure_not_frozen(body.sender_id, db)
    ensure_not_frozen(body.receiver_id, db)

    idempotency_key = f"{body.sender_id}:{body.nonce}"
    existing_transfer = db.query(DemoBankTransfer).filter(
        DemoBankTransfer.idempotency_key == idempotency_key
    ).first()
    if existing_transfer:
        same_request = (
            existing_transfer.receiver_id == body.receiver_id
            and existing_transfer.amount_paise == amount_paise
        )
        if not same_request:
            raise HTTPException(status_code=409, detail="idempotency_key_reused_for_different_payment")
        return {
            "tx_id": existing_transfer.reference,
            "status": existing_transfer.status,
            "demo": True,
            "bank_reference": existing_transfer.reference,
            "currency": existing_transfer.currency,
            "amount": format_inr(existing_transfer.amount_paise),
            "idempotent_replay": True,
        }

    sender = db.query(DemoBankAccount).filter(
        DemoBankAccount.account_id == body.sender_id
    ).with_for_update().first()
    receiver = db.query(DemoBankAccount).filter(
        DemoBankAccount.account_id == body.receiver_id
    ).with_for_update().first()
    if sender is None or receiver is None:
        raise HTTPException(status_code=404, detail="demo_bank_account_not_found")
    if sender.currency != receiver.currency or sender.currency != "INR":
        raise HTTPException(status_code=422, detail="unsupported_demo_currency")
    if sender.balance_paise < amount_paise:
        raise HTTPException(status_code=409, detail="insufficient_demo_funds")

    tx_id = str(uuid.uuid4())
    sender.balance_paise -= amount_paise
    receiver.balance_paise += amount_paise
    transfer = DemoBankTransfer(
        reference=tx_id,
        idempotency_key=idempotency_key,
        sender_id=body.sender_id,
        receiver_id=body.receiver_id,
        amount_paise=amount_paise,
        currency="INR",
        status="completed",
    )
    db.add(transfer)
    tx = Transaction(
        tx_id=tx_id,
        sender_id=body.sender_id,
        receiver_id=body.receiver_id,
        amount=format_inr(amount_paise),
        nonce=body.nonce,
        tx_type="online",
        status="completed"
    )
    db.add(tx)
    db.commit()

    notify_receiver(body.receiver_id, format_inr(amount_paise), body.sender_id, db)
    return {
        "tx_id": tx_id,
        "status": "completed",
        "demo": True,
        "bank_reference": tx_id,
        "currency": "INR",
        "amount": format_inr(amount_paise),
        "idempotent_replay": False,
    }

@app.get("/api/v1/demo/bank-accounts/{account_id}")
def get_demo_bank_account(account_id: str, db: Session = Depends(get_db)):
    """Return the play-money balance for a seeded demo account."""
    ensure_demo_bank_accounts(db)
    account = db.query(DemoBankAccount).filter(DemoBankAccount.account_id == account_id).first()
    if account is None:
        raise HTTPException(status_code=404, detail="demo_bank_account_not_found")
    return {
        "account_id": account.account_id,
        "balance": format_inr(account.balance_paise),
        "currency": account.currency,
        "demo": True,
    }

@app.get("/api/v1/transactions/online")
def list_online_transactions(user_id: str, since: Optional[str] = None, db: Session = Depends(get_db)):
    """List transactions for a user"""
    ensure_not_frozen(user_id, db)
    
    query = db.query(Transaction).filter(
        (Transaction.sender_id == user_id) | (Transaction.receiver_id == user_id)
    )
    
    if since:
        query = query.filter(Transaction.created_at >= since)
    
    transactions = query.order_by(Transaction.created_at.desc()).all()
    
    return {
        "transactions": [
            {
                "id": t.tx_id,
                "sender_id": t.sender_id,
                "receiver_id": t.receiver_id,
                "amount": t.amount,
                "nonce": t.nonce,
                "status": t.status,
                "type": t.tx_type,
                "created_at": t.created_at.isoformat()
            }
            for t in transactions
        ]
    }

@app.post("/api/v1/transactions/offline/sync")
def sync_offline_transactions(body: OfflineSyncPayload, db: Session = Depends(get_db)):
    """Sync offline transactions when device comes online"""
    ensure_not_frozen(body.user_id, db)
    
    accepted = []
    rejected = []
    
    for tx in body.transactions:
        # Check for duplicates
        existing = db.query(Transaction).filter(
            Transaction.sender_id == tx.sender_id,
            Transaction.receiver_id == tx.receiver_id,
            Transaction.nonce == tx.nonce
        ).first()
        
        if existing:
            rejected.append({"tx_id": tx.tx_id, "reason": "duplicate"})
            continue
        
        # Store offline transaction
        new_tx = Transaction(
            tx_id=tx.tx_id,
            sender_id=tx.sender_id,
            receiver_id=tx.receiver_id,
            amount=tx.amount,
            nonce=tx.nonce,
            tx_type="offline",
            status="completed",
            sender_signature=tx.sender_signature,
            receiver_receipt=tx.receiver_receipt_signature
        )
        db.add(new_tx)
        accepted.append(tx.tx_id)
    
    db.commit()
    return {"accepted": accepted, "rejected": rejected}

@app.post("/api/v1/transactions/verify-id")
def verify_transaction_id(body: VerifyTransactionIdPayload, db: Session = Depends(get_db)):
    """Verify transaction ID matches stored transaction"""
    ensure_not_frozen(body.user_id, db)
    
    tx = db.query(Transaction).filter(
        Transaction.sender_id == body.sender_id,
        Transaction.receiver_id == body.receiver_id,
        Transaction.nonce == body.nonce,
        Transaction.amount == body.amount
    ).first()
    
    if not tx:
        raise HTTPException(status_code=404, detail="Transaction not found")
    
    if tx.tx_id != body.transaction_id:
        # Freeze account on mismatch
        user = db.query(User).filter(User.user_id == body.user_id).first()
        if user:
            user.is_frozen = True
            db.commit()
        raise HTTPException(status_code=403, detail="Account frozen. Transaction ID mismatch.")
    
    return {"verified": True, "transaction_id": tx.tx_id}

@app.post("/api/v1/users/me/freeze")
def freeze_account(body: FreezePayload, db: Session = Depends(get_db)):
    """Freeze user account"""
    user = db.query(User).filter(User.user_id == body.user_id).first()
    if user:
        user.is_frozen = True
        db.commit()
    return {"status": "frozen", "message": "Account frozen. Contact support."}

@app.get("/api/v1/users/me/status")
def user_status(user_id: str, db: Session = Depends(get_db)):
    """Get user account status"""
    user = db.query(User).filter(User.user_id == user_id).first()
    if not user:
        return {"frozen": False, "exists": False}
    return {"frozen": user.is_frozen, "exists": True}

@app.post("/api/v1/users/me/device-token")
def register_device_token(body: DeviceTokenPayload, user_id: str, db: Session = Depends(get_db)):
    """Register device token for push notifications"""
    ensure_not_frozen(user_id, db)
    
    device = db.query(DeviceToken).filter(DeviceToken.user_id == user_id).first()
    if device:
        device.token = body.token
        device.platform = body.platform
        device.updated_at = datetime.utcnow()
    else:
        device = DeviceToken(user_id=user_id, token=body.token, platform=body.platform)
        db.add(device)
    
    db.commit()
    return {"registered": True}

@app.get("/api/v1/health")
def health_check(db: Session = Depends(get_db)):
    """Check API and database availability without exposing payment operations."""
    try:
        db.execute(text("SELECT 1"))
    except Exception as exc:
        raise HTTPException(status_code=503, detail="database_unavailable") from exc
    return {
        "status": "healthy",
        "mode": "demo" if os.getenv("FASTPAY_DEMO_MODE", "").lower() == "true" else "disabled",
        "timestamp": datetime.now(timezone.utc).isoformat(),
    }

# Run: uvicorn server_production:app --host 0.0.0.0 --port $PORT
