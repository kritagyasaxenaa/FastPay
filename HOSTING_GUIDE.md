# 🚀 FastPay - Complete Hosting Guide for Multiple Phones

## Overview

This guide shows you how to:
1. ✅ Deploy the FastAPI server with PostgreSQL database (FREE)
2. ✅ Build the Android APK
3. ✅ Configure multiple phones to connect to your server
4. ✅ Test transactions between phones

---

## Part 1: Deploy Server + Database (100% FREE on Render)

### Step 1: Sign Up for Render

1. Go to https://render.com
2. Click "Get Started for Free"
3. Sign up with GitHub

### Step 2: Create PostgreSQL Database

1. Click "New +" → "PostgreSQL"
2. Configure:
   - **Name:** `fastpay-db`
   - **Database:** `fastpay`
   - **User:** `fastpay` (auto-generated)
   - **Region:** Choose closest to you
   - **Plan:** Free
3. Click "Create Database"
4. **IMPORTANT:** Copy the "Internal Database URL" - you'll need this!
   - It looks like: `postgres://fastpay:xxxxx@dpg-xxxxx/fastpay`

### Step 3: Deploy FastAPI Server

1. **Push your code to GitHub first:**
   ```bash
   cd FastPayQt_Improved
   git init
   git add .
   git commit -m "Initial commit"
   git remote add origin https://github.com/YOUR_USERNAME/fastpay.git
   git push -u origin main
   ```

2. **In Render Dashboard:**
   - Click "New +" → "Web Service"
   - Click "Connect GitHub" → Select your repository
   - Configure:
     - **Name:** `fastpay-api`
     - **Region:** Same as database
     - **Branch:** `main`
     - **Root Directory:** `server`
     - **Runtime:** Python 3
     - **Build Command:** `pip install -r requirements.txt`
     - **Start Command:** `uvicorn main_production:app --host 0.0.0.0 --port $PORT`
   
3. **Add Environment Variable:**
   - Scroll to "Environment Variables"
   - Click "Add Environment Variable"
   - **Key:** `DATABASE_URL`
   - **Value:** Paste the database URL you copied earlier
   - Click "Add"

4. **Create Web Service**
   - Click "Create Web Service"
   - Wait 2-3 minutes for deployment
   - **Copy your server URL:** `https://fastpay-api.onrender.com`

### Step 4: Verify Server is Running

Visit: `https://your-app-name.onrender.com/api/v1/health`

You should see:
```json
{
  "status": "healthy",
  "timestamp": "2024-xx-xx..."
}
```

✅ **Server is live!**

---

## Part 2: Build Android APK

### Method 1: GitHub Actions (Recommended - No Installation)

1. **Ensure code is on GitHub** (from Step 3.1 above)

2. **Update server URL in code:**
   - Edit `server_config.h` line 9:
     ```cpp
     const QString PRODUCTION_SERVER = "https://YOUR-APP-NAME.onrender.com";
     ```
   - Commit and push:
     ```bash
     git add server_config.h
     git commit -m "Update production server URL"
     git push
     ```

3. **Build APK:**
   - Go to your GitHub repo
   - Click "Actions" tab
   - Click "Build Android APK" workflow
   - Click "Run workflow" → "Run workflow"
   - Wait 15-20 minutes ⏳

4. **Download APK:**
   - Click on the completed workflow run (green ✓)
   - Scroll to "Artifacts"
   - Download "FastPay-Android-APK"
   - Extract the ZIP to get your APK

### Method 2: Local Build with Qt Creator

1. Install Qt 6.6+ with Android support
2. Open `FastPayQt/CMakeLists.txt` in Qt Creator
3. Edit `server_config.h` with your server URL
4. Select Android kit (arm64-v8a)
5. Build → Build Project
6. Find APK in build folder

---

## Part 3: Install on Multiple Phones

### Phone 1 Setup

1. **Transfer APK** to phone (USB, Google Drive, etc.)
2. **Enable "Install from unknown sources":**
   - Settings → Security → Unknown sources → Enable
3. **Install APK** by tapping on it
4. **Open FastPay app**
5. **Set PIN:** Enter a 4-6 digit PIN (e.g., `123456`)
6. **App is ready!**

### Phone 2, 3, 4... Setup

Repeat the same steps on each phone:
1. Transfer same APK
2. Install
3. Set PIN (can be different for each phone)

**All phones will automatically connect to your hosted server!**

---

## Part 4: Test Multi-Phone Transactions

### Test Scenario: Send Money from Phone 1 to Phone 2

**On Phone 1 (Sender):**
1. Open FastPay
2. Tap "Online Mode"
3. Tap "Send Payment"
4. Enter amount: `500`
5. Enter PIN: `123456`
6. Tap "Send"

**On Phone 2 (Receiver):**
1. Open FastPay
2. Go to "History" tab
3. **You should see the transaction appear!**

### Test Offline Mode

**On Phone 1 (Receiver):**
1. Open FastPay
2. Tap "Offline Mode"
3. Tap "Emit Payment Code"
4. Hold phone steady

**On Phone 2 (Sender):**
1. Tap "Offline Mode"
2. Tap "Scan Code" or "Listen"
3. Hold phones close together (< 30cm)
4. Enter amount
5. Transaction completes!

---

## Part 5: Monitoring & Management

### View Database Records

1. **Go to Render Dashboard**
2. **Click your database** (`fastpay-db`)
3. **Click "Connect"** → Copy the External Database URL
4. **Use any PostgreSQL client:**
   - DBeaver (free)
   - pgAdmin (free)
   - Or connect via command line

**View all transactions:**
```sql
SELECT * FROM transactions ORDER BY created_at DESC;
```

**View all users:**
```sql
SELECT user_id, phone_number, is_frozen, created_at FROM users;
```

### Server Logs

1. **Render Dashboard** → Your web service
2. **Click "Logs" tab**
3. See real-time server activity

### Server Management

**Restart server:**
- Render Dashboard → Your service → "Manual Deploy" → "Deploy"

**Check health:**
- Visit: `https://your-server.onrender.com/api/v1/health`

---

## Common Issues & Fixes

### "Connection Error" in App

**Problem:** App can't reach server

**Fix:**
1. Check server is running: Visit `/api/v1/health`
2. Verify URL in `server_config.h` matches your Render URL
3. Rebuild APK with correct URL
4. Check phone has internet connection

### Server Sleeping (Free Tier)

**Problem:** First request after inactivity takes ~30 seconds

**Solution:**
- This is normal on free tier
- Server "wakes up" after first request
- Consider upgrading to paid tier for 24/7 availability

### Database Connection Failed

**Problem:** Server logs show database errors

**Fix:**
1. Verify `DATABASE_URL` environment variable is set correctly
2. Ensure database is in same region as web service
3. Check database is running in Render dashboard

### APK Not Installing

**Problem:** "App not installed" error

**Fix:**
1. Uninstall old version first
2. Enable "Unknown sources"
3. Check Android version (need 5.0+)
4. Try different browser for download

### Transactions Not Appearing

**Problem:** Send from Phone 1, nothing on Phone 2

**Debug:**
1. Check both phones have internet
2. Verify both using same server URL
3. Check server logs for errors
4. Try restarting app on both phones

---

## Architecture Overview

```
┌─────────────┐         ┌──────────────────┐         ┌─────────────┐
│  Phone 1    │         │  Render Server   │         │  Phone 2    │
│  (Android)  │◄───────►│  FastAPI +       │◄───────►│  (Android)  │
│             │  HTTPS  │  PostgreSQL      │  HTTPS  │             │
└─────────────┘         └──────────────────┘         └─────────────┘
                                │
                                │ Stores
                                ▼
                        ┌──────────────┐
                        │ PostgreSQL   │
                        │ Database     │
                        │ - Users      │
                        │ - Transactions│
                        │ - Devices    │
                        └──────────────┘
```

---

## Production Checklist

Before going live:

- [ ] Server deployed and healthy
- [ ] Database connected
- [ ] Server URL updated in `server_config.h`
- [ ] APK built with production server
- [ ] Tested on at least 2 phones
- [ ] Transaction history working
- [ ] Offline transactions sync when online
- [ ] Account freeze mechanism tested
- [ ] Database backups enabled (Render auto-backs up)

---

## Scaling & Upgrades

### When to Upgrade

**Free Tier Limits:**
- Server sleeps after 15 min inactivity
- 750 hours/month runtime
- 256MB RAM
- Database: 1GB storage, 97 hours/month

**Upgrade when:**
- Need 24/7 availability → Upgrade to $7/month
- More than 100 users → Upgrade database
- Heavy traffic → Scale to multiple instances

### Cost Estimate

**For 100-1000 users:**
- Web Service: $7/month (Starter)
- PostgreSQL: $7/month (Essential, 256MB RAM)
- **Total: $14/month**

---

## API Endpoints Reference

**Health Check:**
```
GET /api/v1/health
```

**Register User:**
```
POST /api/v1/users/register
{
  "user_id": "alice@fastpay",
  "phone_number": "+1234567890",
  "pin_hash": "..."
}
```

**Submit Online Transaction:**
```
POST /api/v1/transactions/online
{
  "sender_id": "alice@fastpay",
  "receiver_id": "bob@fastpay",
  "amount": "500",
  "nonce": "2024-02-07 12:30:45"
}
```

**Get Transaction History:**
```
GET /api/v1/transactions/online?user_id=alice@fastpay
```

**Check Account Status:**
```
GET /api/v1/users/me/status?user_id=alice@fastpay
```

---

## Support & Next Steps

**Need Help?**
- Check server logs in Render dashboard
- Test API endpoints with curl or Postman
- Verify database has records

**Improvements:**
- Add push notifications (FCM)
- Implement user authentication (JWT)
- Add transaction receipts
- Build iOS version

---

## 🎉 You're Live!

Your FastPay system is now:
- ✅ Hosted on production server
- ✅ Connected to PostgreSQL database  
- ✅ Accessible from anywhere
- ✅ Ready for multiple phones
- ✅ Completely FREE (on free tier)

**Share your server URL with friends and start testing!**

Server URL: `https://your-app-name.onrender.com`
