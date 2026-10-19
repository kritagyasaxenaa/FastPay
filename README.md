# FastPay - Payment Flow Prototype

A React Native Android prototype for exploring a simulated payment flow. The Android app is in [`mobile/`](mobile/README.md). Legacy Qt/C++ sources remain in the repository but are no longer the Android app. FastPay is not a real payment product and must not be used to move money or store production credentials.

## Production status

**Not production-ready.** The Android app contains demo transaction flows and placeholder cryptography. The server's simulated balances are play money, users are not authenticated, and offline signatures are not verified. The production-named API disables all `/api/v1/` operations by default except `/api/v1/health`. Setting `FASTPAY_DEMO_MODE=true` enables the simulator for isolated demonstrations only. Never set that flag on a public deployment.

The Android client is built with React Native and Expo. It uses native Android networking and UI components and connects only to the explicitly labeled simulated ledger. No real bank, UPI, or payment rail is integrated.

## Simulated bank demo

For local demonstrations, run the API with `FASTPAY_DEMO_MODE=true`. It seeds test accounts with INR 1,000.00 and simulates transfers by debiting and crediting the in-app ledger. The transfer API supports retries without duplicate debits and exposes demo balances at `/api/v1/demo/bank-accounts/{account_id}`. It is play money only; it does not connect to banks, UPI, or other payment rails. Keep demo mode off on public deployments.

## Run and build Android

Follow the React Native / Expo instructions in [`mobile/README.md`](mobile/README.md). Start the local demo API using [`server/README.md`](server/README.md). The Android Emulator URL is `http://10.0.2.2:8000`; cleartext is allowed only for that emulator host. The restricted public phone-test deployment uses HTTPS and exposes only fake-account demo routes; it must never receive real customer data, credentials, or funds.
