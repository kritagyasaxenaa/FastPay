"""
FastPay Production Server with PostgreSQL Database
Supports multiple Android devices connecting simultaneously
"""

import uuid
import os
from datetime import datetime
from typing import Optional, List
from fastapi import FastAPI, HTTPException, Depends
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from sqlalchemy import create_engine, Column, String, DateTime, Boolean
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

# Create tables
Base.metadata.create_all(bind=engine)

# FastAPI app
app = FastAPI(title="FastPay Production API", version="2.0.0")

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

# Routes
@app.get("/")
def root():
    return {"status": "FastPay API v2.0", "database": "connected"}

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
    """Submit an online transaction"""
    ensure_not_frozen(body.sender_id, db)
    ensure_not_frozen(body.receiver_id, db)
    
    tx_id = str(uuid.uuid4())
    tx = Transaction(
        tx_id=tx_id,
        sender_id=body.sender_id,
        receiver_id=body.receiver_id,
        amount=body.amount,
        nonce=body.nonce,
        tx_type="online",
        status="completed"
    )
    db.add(tx)
    db.commit()
    
    notify_receiver(body.receiver_id, body.amount, body.sender_id, db)
    return {"tx_id": tx_id, "status": "completed"}

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
def health_check():
    """Health check endpoint"""
    return {"status": "healthy", "timestamp": datetime.utcnow().isoformat()}

# Run: uvicorn server_production:app --host 0.0.0.0 --port $PORT
