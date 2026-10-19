import os
import tempfile
import unittest
import uuid

_database_fd, _database_path = tempfile.mkstemp(prefix="fastpay-tests-", suffix=".db")
os.close(_database_fd)
os.environ["DATABASE_URL"] = "sqlite:///" + _database_path
os.environ["FASTPAY_DEMO_MODE"] = "true"

from fastapi.testclient import TestClient

from server.main_production import app, engine, _public_demo_requests


client = TestClient(app)
client.__enter__()
os.environ["FASTPAY_DEMO_MODE"] = "false"


class ProductionSafetyGateTests(unittest.TestCase):
    def setUp(self):
        os.environ["FASTPAY_DEMO_MODE"] = "false"

    def test_health_is_available_when_payment_api_is_disabled(self):
        response = client.get("/api/v1/health")

        self.assertEqual(response.status_code, 200)
        self.assertEqual(response.json()["status"], "healthy")
        self.assertEqual(response.json()["mode"], "disabled")

    def test_payment_routes_fail_closed_by_default(self):
        response = client.post(
            "/api/v1/transactions/online",
            json={
                "sender_id": "sender@test",
                "receiver_id": "receiver@test",
                "amount": "1.00",
                "nonce": "test-nonce",
            },
        )

        self.assertEqual(response.status_code, 503)
        self.assertEqual(response.json()["code"], "payment_api_disabled")

    def test_root_does_not_claim_payment_api_is_enabled_by_default(self):
        response = client.get("/")

        self.assertEqual(response.status_code, 200)
        self.assertIs(response.json()["payment_api_enabled"], False)

    def test_demo_transfer_debits_and_credits_seeded_bank_accounts(self):
        os.environ["FASTPAY_DEMO_MODE"] = "true"
        try:
            sender_before = client.get("/api/v1/demo/bank-accounts/sender@fastpay").json()
            receiver_before = client.get("/api/v1/demo/bank-accounts/receiver@fastpay").json()
            payment = {
                "sender_id": "sender@fastpay",
                "receiver_id": "receiver@fastpay",
                "amount": "12.34",
                "nonce": str(uuid.uuid4()),
            }

            transfer = client.post("/api/v1/transactions/online", json=payment)
            sender_after = client.get("/api/v1/demo/bank-accounts/sender@fastpay").json()
            receiver_after = client.get("/api/v1/demo/bank-accounts/receiver@fastpay").json()
            replay = client.post("/api/v1/transactions/online", json=payment)

            self.assertEqual(transfer.status_code, 200, transfer.text)
            self.assertTrue(transfer.json()["demo"])
            self.assertFalse(transfer.json()["idempotent_replay"])
            self.assertEqual(sender_after["balance"], self._subtract(sender_before["balance"], "12.34"))
            self.assertEqual(receiver_after["balance"], self._add(receiver_before["balance"], "12.34"))
            self.assertEqual(replay.status_code, 200, replay.text)
            self.assertTrue(replay.json()["idempotent_replay"])
            self.assertEqual(replay.json()["tx_id"], transfer.json()["tx_id"])
            conflicting_replay = dict(payment, amount="12.35")
            conflict = client.post("/api/v1/transactions/online", json=conflicting_replay)
            self.assertEqual(conflict.status_code, 409)
            self.assertEqual(conflict.json()["detail"], "idempotency_key_reused_for_different_payment")
        finally:
            os.environ["FASTPAY_DEMO_MODE"] = "false"

    def test_demo_transfer_rejects_invalid_or_insufficient_amount(self):
        os.environ["FASTPAY_DEMO_MODE"] = "true"
        try:
            payment = {
                "sender_id": "sender@fastpay",
                "receiver_id": "receiver@fastpay",
                "amount": "100000.00",
                "nonce": str(uuid.uuid4()),
            }
            response = client.post("/api/v1/transactions/online", json=payment)

            self.assertEqual(response.status_code, 409)
            self.assertEqual(response.json()["detail"], "insufficient_demo_funds")
            payment["amount"] = "0.001"
            payment["nonce"] = str(uuid.uuid4())
            response = client.post("/api/v1/transactions/online", json=payment)
            self.assertEqual(response.status_code, 422)
        finally:
            os.environ["FASTPAY_DEMO_MODE"] = "false"

    def test_public_demo_only_exposes_fake_accounts_and_limited_transfers(self):
        os.environ["FASTPAY_DEMO_MODE"] = "true"
        os.environ["FASTPAY_PUBLIC_DEMO_MODE"] = "true"
        _public_demo_requests.clear()
        try:
            self.assertEqual(client.get("/api/v1/demo/bank-accounts/alice@fastpay").status_code, 200)
            self.assertEqual(client.get("/api/v1/demo/bank-accounts/not-a-demo-account").status_code, 404)
            self.assertEqual(client.post("/api/v1/users/register", json={
                "user_id": "public-test", "phone_number": "000", "pin_hash": "not-a-secret"
            }).status_code, 404)
            self.assertEqual(client.get("/docs").status_code, 404)

            payment = {
                "sender_id": "alice@fastpay",
                "receiver_id": "bob@fastpay",
                "amount": "50.01",
                "nonce": str(uuid.uuid4()),
            }
            too_large = client.post("/api/v1/transactions/online", json=payment)
            self.assertEqual(too_large.status_code, 422)
            self.assertEqual(too_large.json()["detail"], "public_demo_transfer_limit_is_50_inr")

            _public_demo_requests.clear()
            for index in range(10):
                limited_payment = dict(payment, amount="0.01", nonce=f"public-limit-{uuid.uuid4()}-{index}")
                self.assertEqual(client.post("/api/v1/transactions/online", json=limited_payment).status_code, 200)
            final_payment = dict(payment, amount="0.01", nonce=f"public-limit-final-{uuid.uuid4()}")
            limited = client.post("/api/v1/transactions/online", json=final_payment)
            self.assertEqual(limited.status_code, 429)
            self.assertEqual(limited.headers["retry-after"], "60")
        finally:
            os.environ["FASTPAY_DEMO_MODE"] = "false"
            os.environ["FASTPAY_PUBLIC_DEMO_MODE"] = "false"

    @staticmethod
    def _subtract(value, amount):
        from decimal import Decimal
        return f"{Decimal(value) - Decimal(amount):.2f}"

    @staticmethod
    def _add(value, amount):
        from decimal import Decimal
        return f"{Decimal(value) + Decimal(amount):.2f}"


def tearDownModule():
    client.__exit__(None, None, None)
    engine.dispose()
    try:
        os.unlink(_database_path)
    except FileNotFoundError:
        pass
