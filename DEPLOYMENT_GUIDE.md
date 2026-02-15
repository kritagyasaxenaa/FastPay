# 🚀 FastPay - Complete Deployment Guide

## Table of Contents
1. [Quick Demo (5 minutes)](#quick-demo)
2. [Deploy Server (Free Hosting)](#deploy-server)
3. [Deploy Web Demo (Free Hosting)](#deploy-web-demo)
4. [Build Android APK](#build-android-apk)
5. [Multi-Device Testing](#multi-device-testing)

---

## 🎯 Quick Demo (5 Minutes)

### Test Locally on Your Computer

1. **Start the Server:**
```bash
cd server
pip install -r requirements.txt
uvicorn main:app --reload --host 0.0.0.0 --port 8000
```

2. **Open Web Demo:**
   - Open `demo/index.html` in your browser
   - Or run: `python3 -m http.server 3000` in the demo folder
   - Visit: http://localhost:3000

3. **Test with Multiple Browser Tabs:**
   - Tab 1: Select "Phone 1 (Alice)"
   - Tab 2: Select "Phone 2 (Bob)"
   - Send money from Alice to Bob!

---

## 🌐 Deploy Server (FREE Hosting)

### Option 1: Deploy to Render (Recommended - 100% Free)

1. **Create Account:**
   - Go to https://render.com
   - Sign up with GitHub

2. **Deploy:**
   - Click "New +" → "Web Service"
   - Connect your GitHub repository
   - Configure:
     - **Name:** fastpay-api
     - **Environment:** Python 3
     - **Build Command:** `pip install -r requirements.txt`
     - **Start Command:** `uvicorn server.main:app --host 0.0.0.0 --port $PORT`
     - **Region:** Choose closest to you
   - Click "Create Web Service"

3. **Get Your API URL:**
   - After deployment, you'll get: `https://fastpay-api.onrender.com`
   - Copy this URL - you'll need it!

**Note:** Free tier spins down after inactivity (wakes up in ~1 min on first request)

### Option 2: Deploy to Railway (Also Free)

1. Go to https://railway.app
2. Click "Start a New Project" → "Deploy from GitHub"
3. Select your repository
4. Railway auto-detects Python and deploys!
5. Get your URL from the deployment

### Option 3: Deploy to Fly.io (Free 3 Apps)

```bash
# Install Fly CLI
curl -L https://fly.io/install.sh | sh

# Login
fly auth login

# Deploy
cd server
fly launch
fly deploy
```

---

## 🎨 Deploy Web Demo (FREE Hosting)

### Option 1: Netlify (Easiest - Drag & Drop)

1. **Go to https://app.netlify.com**
2. **Drag & drop** the `demo` folder
3. **Update API URL:**
   - After deploy, click "Site settings" → "Environment variables"
   - Or edit `demo/index.html` line 232:
     ```javascript
     const API_URL = 'https://your-server-url.onrender.com/api/v1';
     ```
4. **Redeploy** if needed
5. **Share the URL!** (e.g., `https://fastpay-demo.netlify.app`)

### Option 2: Vercel (GitHub Integration)

1. Go to https://vercel.com
2. Import from GitHub
3. Deploy the `demo` folder
4. Update API URL in `index.html`

### Option 3: GitHub Pages (100% Free, No signup)

1. **Create a new repo** or use existing
2. **Upload demo folder** to the repo
3. **Settings → Pages:**
   - Source: Deploy from a branch
   - Branch: main
   - Folder: /demo
4. **Visit:** `https://yourusername.github.io/fastpay/`

---

## 📱 Build Android APK

### Method 1: GitHub Actions (No Installation Required)

1. **Upload project to GitHub**
2. **Add this file:** `.github/workflows/build-android.yml`
3. **Go to Actions tab** → Run workflow
4. **Wait 15-20 minutes**
5. **Download APK** from Artifacts

### Method 2: Qt Creator (Local Build)

1. Install Qt 6 with Android support
2. Open `FastPayQt/CMakeLists.txt`
3. Select Android kit (arm64-v8a)
4. Build → Build Project
5. Find APK in build folder

---

## 🔥 Multi-Device Testing Guide

### Scenario 1: Test with Phones on Same WiFi

1. **Deploy server** to Render/Railway
2. **Update API URL** in both:
   - Web demo (`demo/index.html`)
   - Android app (if built, in settings)
3. **Open demo on multiple phones:**
   - Phone 1: Visit demo URL → Select "Phone 1 (Alice)"
   - Phone 2: Visit demo URL → Select "Phone 2 (Bob)"
   - Phone 3: Visit demo URL → Select "Phone 3 (Charlie)"
4. **Send payments** between devices!

### Scenario 2: Test Android App + Web Demo

1. **Install APK** on Phone 1
2. **Open web demo** on Phone 2 browser
3. Configure both to use your deployed server
4. Send money from app to web and vice versa!

### Scenario 3: Demo for Presentation

1. **Use a projector/TV:**
   - Open demo on big screen
   - Show "Phone 1" view
2. **Use your phone:**
   - Open demo
   - Select "Phone 2"
   - Send payment
3. **Watch transaction appear** on big screen in real-time!

---

## 🎭 Demo Credentials

**Default test users:**
- **Alice:** ID: `alice@fastpay`, PIN: `123456`
- **Bob:** ID: `bob@fastpay`, PIN: `123456`
- **Charlie:** ID: `charlie@fastpay`, PIN: `123456`

---

## 🐛 Troubleshooting

### "Connection Error" in Web Demo
- Make sure server is running
- Check API_URL in `demo/index.html` matches your server
- Server on Render? First request takes ~1 min to wake up

### "CORS Error" in Browser Console
- Server has CORS enabled for all origins
- If still getting error, check server logs

### APK Build Fails
- Check GitHub Actions logs
- Ensure all files uploaded correctly
- Try the fixed workflow file included

### Server Crashes
- Check logs on Render/Railway dashboard
- Ensure requirements.txt has all dependencies
- Free tier has memory limits (should be fine for testing)

---

## 📊 What's Included

```
FastPayQt_Improved/
├── FastPayQt/          # Qt Android app source
│   ├── CMakeLists.txt
│   ├── main.cpp
│   └── android/        # Android-specific files
├── Crypto/             # Cryptography modules
├── server/             # FastAPI backend
│   ├── main.py
│   └── requirements.txt
├── demo/               # Web demo interface
│   └── index.html
└── .github/
    └── workflows/      # GitHub Actions for APK build
```

---

## 🚀 Quick Start Summary

**For a 5-minute demo:**
```bash
# Terminal 1 - Start server
cd server && pip install -r requirements.txt
uvicorn main:app --host 0.0.0.0 --port 8000

# Terminal 2 - Start web demo
cd demo && python3 -m http.server 3000

# Open browser
# Tab 1: http://localhost:3000 (Select Phone 1)
# Tab 2: http://localhost:3000 (Select Phone 2)
# Send money from Phone 1 to Phone 2!
```

**For production demo:**
1. Deploy server to Render (free)
2. Deploy demo to Netlify (free)
3. Share demo URL with anyone!
4. Build APK with GitHub Actions
5. Install on phones

---

## 🎉 You're Ready!

Your FastPay system is now:
- ✅ Hosted online (accessible from anywhere)
- ✅ Multi-device ready (test with multiple phones)
- ✅ Professional looking (great for demos)
- ✅ 100% FREE (using free tiers)

**Share your demo:** Just send the Netlify URL to anyone!

Questions? Check the troubleshooting section or raise an issue on GitHub.

Happy testing! 💰📱
