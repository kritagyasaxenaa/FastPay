import os
import tempfile
import unittest

os.environ.setdefault(
    "DATABASE_URL",
    "sqlite:///" + os.path.join(tempfile.gettempdir(), "fastpay-safety-tests.db"),
)
os.environ["FASTPAY_DEMO_MODE"] = "false"

from fastapi.testclient import TestClient

from server.main_production import app


client = TestClient(app)


class ProductionSafetyGateTests(unittest.TestCase):
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
