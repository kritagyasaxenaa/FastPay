# ⚡ FastPay - Quick Deployment Reference

## 🎯 Goal
Host FastPay server + database so multiple Android phones can connect and transact.

---

## 📋 3-Step Deployment

### 1️⃣ Deploy Database (2 minutes)
```
Render.com → New → PostgreSQL
Name: fastpay-db
Plan: Free
✓ Copy Internal Database URL
```

### 2️⃣ Deploy Server (3 minutes)
```
Render.com → New → Web Service
Connect GitHub repo
Root: server/
Build: pip install -r requirements.txt
Start: uvicorn main_production:app --host 0.0.0.0 --port $PORT
Environment: DATABASE_URL = [paste database URL]
✓ Copy Server URL (https://xxx.onrender.com)
```

### 3️⃣ Build APK (20 minutes)
```
Edit server_config.h → Update PRODUCTION_SERVER URL
GitHub → Actions → Run "Build Android APK"
Download APK from Artifacts
```

---

## 📱 Install on Phones

**Same APK for all phones!**

1. Transfer APK to phone
2. Install (enable unknown sources if needed)
3. Open app → Set PIN
4. Done! App connects to server automatically

---

## ✅ Test Multi-Phone

**Phone 1:**
```
Online Mode → Send ₹500 → Enter PIN → Send
```

**Phone 2:**
```
History tab → See transaction appear!
```

---

## 🔧 After Deployment

**Update server URL in code:**
```cpp
// server_config.h line 9
const QString PRODUCTION_SERVER = "https://YOUR-APP.onrender.com";
```

**Then rebuild APK!**

---

## 📊 Monitor

**Server Health:**
```
https://your-server.onrender.com/api/v1/health
```

**View Transactions:**
```sql
-- Connect to database
SELECT * FROM transactions ORDER BY created_at DESC LIMIT 10;
```

**Server Logs:**
```
Render Dashboard → Your Service → Logs tab
```

---

## 🐛 Quick Fixes

| Problem | Solution |
|---------|----------|
| App can't connect | Check server URL in server_config.h |
| Server sleeping | Normal on free tier, wakes in ~30s |
| No transactions appearing | Verify both phones on internet |
| APK won't install | Enable "Unknown sources" |

---

## 💰 Cost

**Free Forever:**
- Up to ~10-20 concurrent users
- 750 hours/month server time
- 1GB database storage

**Paid ($14/month):**
- 24/7 availability
- Unlimited users
- More storage

---

## 🚀 Files You Need

```
FastPayQt_Improved/
├── server/
│   ├── main_production.py  ← Deploy this to Render
│   └── requirements.txt
├── server_config.h          ← Update SERVER URL here
└── .github/workflows/       ← For APK building
```

---

## 📞 Quick Links

**Render:** https://render.com
**GitHub Actions:** github.com/YOUR_REPO/actions
**Database Client:** dbeaver.io (free)

---

## ⏱️ Total Time: ~30 minutes

- Deploy database: 2 min
- Deploy server: 3 min  
- Build APK: 20 min
- Install on phones: 5 min

**That's it! You're live! 🎉**
