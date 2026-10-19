import os
import tempfile
import unittest
import uuid

_database_fd, _database_path = tempfile.mkstemp(prefix="fastpay-tests-", suffix=".db")
os.close(_database_fd)
os.environ["DATABASE_URL"] = "sqlite:///" + _database_path
os.environ["FASTPAY_DEMO_MODE"] = "false"

from fastapi.testclient import TestClient

from server.main_production import app, engine


client = TestClient(app)


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

    @staticmethod
    def _subtract(value, amount):
        from decimal import Decimal
        return f"{Decimal(value) - Decimal(amount):.2f}"

    @staticmethod
    def _add(value, amount):
        from decimal import Decimal
        return f"{Decimal(value) + Decimal(amount):.2f}"


def tearDownModule():
    engine.dispose()
    try:
        os.unlink(_database_path)
    except FileNotFoundError:
        pass
