export type DemoBalance = {
  account_id: string;
  balance: string;
  currency: string;
  demo: true;
};

export type DemoTransfer = {
  tx_id: string;
  bank_reference: string;
  status: string;
  amount: string;
  currency: string;
  demo: true;
  idempotent_replay: boolean;
};

type ErrorResponse = { detail?: string; message?: string };

async function apiRequest<T>(baseUrl: string, path: string, init?: RequestInit): Promise<T> {
  const controller = new AbortController();
  const timeout = setTimeout(() => controller.abort(), 10_000);

  try {
    const response = await fetch(`${baseUrl.replace(/\/+$/, '')}${path}`, {
      ...init,
      signal: controller.signal,
      headers: { Accept: 'application/json', 'Content-Type': 'application/json', ...init?.headers },
    });
    const data = (await response.json()) as T & ErrorResponse;
    if (!response.ok) {
      throw new Error(data.detail || data.message || `Server returned ${response.status}`);
    }
    return data;
  } catch (error) {
    if (error instanceof Error && error.name === 'AbortError') {
      throw new Error('The server took too long to respond. Check that the local demo server is running.');
    }
    if (error instanceof TypeError) {
      throw new Error('Could not reach the demo server. Check the server URL and make sure the API is running.');
    }
    throw error;
  } finally {
    clearTimeout(timeout);
  }
}

export async function fetchBalance(baseUrl: string, accountId: string): Promise<DemoBalance> {
  const balance = await apiRequest<DemoBalance>(
    baseUrl,
    `/api/v1/demo/bank-accounts/${encodeURIComponent(accountId)}`,
  );
  if (balance.demo !== true) throw new Error('This server did not identify itself as the demo ledger.');
  return balance;
}

export async function transferDemoMoney(
  baseUrl: string,
  senderId: string,
  receiverId: string,
  amount: string,
): Promise<DemoTransfer> {
  const response = await apiRequest<DemoTransfer>(baseUrl, '/api/v1/transactions/online', {
    method: 'POST',
    body: JSON.stringify({
      sender_id: senderId,
      receiver_id: receiverId,
      amount,
      nonce: `${Date.now()}-${Math.random().toString(36).slice(2, 14)}`,
    }),
  });
  if (response.demo !== true) throw new Error('The server did not confirm a simulated payment.');
  return response;
}
