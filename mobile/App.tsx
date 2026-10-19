import React, { useCallback, useEffect, useMemo, useState } from 'react';
import {
  ActivityIndicator,
  KeyboardAvoidingView,
  Platform,
  Pressable,
  ScrollView,
  StatusBar,
  StyleSheet,
  Text,
  TextInput,
  View,
} from 'react-native';
import { DemoBalance, DemoTransfer, fetchBalance, transferDemoMoney } from './src/api';

const SENDER_ID = 'alice@fastpay';
const DEFAULT_RECEIVER = 'bob@fastpay';
const DEFAULT_API = process.env.EXPO_PUBLIC_API_URL || 'http://10.0.2.2:8000';

const colors = {
  ink: '#17233B',
  muted: '#718099',
  canvas: '#F4F7FB',
  white: '#FFFFFF',
  blue: '#315CFF',
  paleBlue: '#EAF0FF',
  green: '#10845A',
  paleGreen: '#E4F7EF',
  border: '#E8EDF4',
  orange: '#935A15',
  paleOrange: '#FFF3DF',
};

function formatMoney(value?: string, currency = 'INR') {
  const parsed = Number(value);
  if (!Number.isFinite(parsed)) return '—';
  return new Intl.NumberFormat('en-IN', { style: 'currency', currency, minimumFractionDigits: 2 }).format(parsed);
}

export default function App() {
  const [apiUrl, setApiUrl] = useState(DEFAULT_API);
  const [draftUrl, setDraftUrl] = useState(DEFAULT_API);
  const [receiver, setReceiver] = useState(DEFAULT_RECEIVER);
  const [amount, setAmount] = useState('');
  const [senderBalance, setSenderBalance] = useState<DemoBalance | null>(null);
  const [receiverBalance, setReceiverBalance] = useState<DemoBalance | null>(null);
  const [lastTransfer, setLastTransfer] = useState<DemoTransfer | null>(null);
  const [busy, setBusy] = useState(false);
  const [refreshing, setRefreshing] = useState(false);
  const [notice, setNotice] = useState('');
  const [noticeIsError, setNoticeIsError] = useState(false);
  const [showServer, setShowServer] = useState(false);

  const canSend = useMemo(() => {
    const parsed = Number(amount);
    return Number.isFinite(parsed) && parsed > 0 && /^\d+(\.\d{1,2})?$/.test(amount.trim()) &&
      receiver.trim().length > 0 && receiver.trim() !== SENDER_ID && !busy;
  }, [amount, busy, receiver]);

  const loadBalances = useCallback(async (url = apiUrl, recipientId = receiver.trim() || DEFAULT_RECEIVER) => {
    setRefreshing(true);
    try {
      const [sender, recipient] = await Promise.all([
        fetchBalance(url, SENDER_ID),
        fetchBalance(url, recipientId),
      ]);
      setSenderBalance(sender);
      setReceiverBalance(recipient);
      setNotice('Balances are up to date.');
      setNoticeIsError(false);
    } catch (error) {
      setNotice(error instanceof Error ? error.message : 'Could not load demo balances.');
      setNoticeIsError(true);
    } finally {
      setRefreshing(false);
    }
  }, [apiUrl]);

  useEffect(() => {
    void loadBalances(DEFAULT_API, DEFAULT_RECEIVER);
  }, [loadBalances]);

  const saveServerUrl = () => {
    const cleaned = draftUrl.trim().replace(/\/+$/, '');
    if (!/^https?:\/\//i.test(cleaned)) {
      setNotice('Enter a server address beginning with http:// or https://.');
      setNoticeIsError(true);
      return;
    }
    setApiUrl(cleaned);
    setDraftUrl(cleaned);
    setShowServer(false);
    void loadBalances(cleaned);
  };

  const sendPayment = async () => {
    if (!canSend) return;
    setBusy(true);
    setLastTransfer(null);
    setNotice('Sending simulated payment…');
    setNoticeIsError(false);
    try {
      const transfer = await transferDemoMoney(apiUrl, SENDER_ID, receiver.trim(), amount.trim());
      setLastTransfer(transfer);
      setAmount('');
      setNotice(transfer.idempotent_replay ? 'This payment was already processed.' : 'Demo payment complete.');
    } catch (error) {
      setNotice(error instanceof Error ? error.message : 'Payment could not be completed.');
      setNoticeIsError(true);
    } finally {
      setBusy(false);
    }
    try {
      const [sender, recipient] = await Promise.all([
        fetchBalance(apiUrl, SENDER_ID),
        fetchBalance(apiUrl, receiver.trim()),
      ]);
      setSenderBalance(sender);
      setReceiverBalance(recipient);
    } catch {
      setNotice((previous) => previous.startsWith('Demo payment complete') || previous.startsWith('This payment was already')
        ? `${previous} Balance refresh failed.`
        : previous);
    }
  };

  return (
    <View style={styles.screen}>
      <StatusBar barStyle="dark-content" backgroundColor={colors.canvas} />
      <KeyboardAvoidingView style={styles.flex} behavior={Platform.OS === 'ios' ? 'padding' : undefined}>
        <ScrollView contentContainerStyle={styles.content} keyboardShouldPersistTaps="handled">
          <View style={styles.topBar}>
            <View style={styles.brandRow}>
              <View style={styles.brandMark}><Text style={styles.brandMarkText}>F</Text></View>
              <View>
                <Text style={styles.brand}>FastPay</Text>
                <Text style={styles.brandSub}>Simple, nearby payments</Text>
              </View>
            </View>
            <Pressable accessibilityRole="button" accessibilityLabel="Server settings" onPress={() => setShowServer(!showServer)} style={styles.settingsButton}>
              <Text style={styles.settingsGlyph}>•••</Text>
            </Pressable>
          </View>

          <View style={styles.demoBanner}>
            <View style={styles.demoDot} />
            <Text style={styles.demoText}>DEMO MODE · Play money only</Text>
          </View>

          <View style={styles.balanceCard}>
            <View style={styles.balanceTop}>
              <Text style={styles.cardEyebrow}>YOUR DEMO BALANCE</Text>
              <Pressable onPress={() => void loadBalances()} disabled={refreshing} style={styles.refreshButton}>
                {refreshing ? <ActivityIndicator size="small" color={colors.blue} /> : <Text style={styles.refreshText}>↻  Refresh</Text>}
              </Pressable>
            </View>
            <Text style={styles.balanceAmount}>{formatMoney(senderBalance?.balance, senderBalance?.currency)}</Text>
            <View style={styles.balanceDivider} />
            <View style={styles.accountRow}>
              <View style={styles.avatar}><Text style={styles.avatarText}>A</Text></View>
              <View style={styles.accountInfo}>
                <Text style={styles.accountName}>Alice's demo wallet</Text>
                <Text style={styles.accountId}>{SENDER_ID}</Text>
              </View>
              <View style={styles.activePill}><Text style={styles.activePillText}>ACTIVE</Text></View>
            </View>
          </View>

          <View style={styles.sectionHead}>
            <Text style={styles.sectionTitle}>Send money</Text>
            <Text style={styles.sectionCaption}>Instant demo transfer</Text>
          </View>

          <View style={styles.formCard}>
            <Text style={styles.fieldLabel}>TO</Text>
            <View style={styles.recipientInputRow}>
              <View style={styles.recipientAvatar}><Text style={styles.recipientAvatarText}>{(receiver.trim()[0] || 'B').toUpperCase()}</Text></View>
              <TextInput
                accessibilityLabel="Recipient demo account"
                autoCapitalize="none"
                autoCorrect={false}
                onChangeText={setReceiver}
                placeholder="recipient@fastpay"
                placeholderTextColor="#9AA6B8"
                style={styles.recipientInput}
                value={receiver}
              />
              <Text style={styles.chevron}>⌄</Text>
            </View>
            <View style={styles.fieldDivider} />
            <Text style={styles.fieldLabel}>AMOUNT</Text>
            <View style={styles.amountRow}>
              <Text style={styles.currencySign}>₹</Text>
              <TextInput
                accessibilityLabel="Payment amount in rupees"
                keyboardType="decimal-pad"
                onChangeText={(value) => setAmount(value.replace(/[^0-9.]/g, ''))}
                placeholder="0.00"
                placeholderTextColor="#B2BDCD"
                style={styles.amountInput}
                value={amount}
              />
              <Text style={styles.currencyCode}>INR</Text>
            </View>
            <View style={styles.quickAmounts}>
              {['50', '100', '500'].map((value) => (
                <Pressable key={value} onPress={() => setAmount(value)} style={[styles.quickChip, amount === value && styles.quickChipSelected]}>
                  <Text style={[styles.quickChipText, amount === value && styles.quickChipTextSelected]}>₹{value}</Text>
                </Pressable>
              ))}
            </View>
            <Pressable accessibilityRole="button" disabled={!canSend} onPress={() => void sendPayment()} style={[styles.sendButton, !canSend && styles.sendButtonDisabled]}>
              {busy ? <ActivityIndicator color={colors.white} /> : <Text style={styles.sendButtonText}>Send demo payment  →</Text>}
            </Pressable>
            <View style={styles.secureNote}>
              <Text style={styles.secureIcon}>ⓘ</Text>
              <Text style={styles.secureText}>This transfer uses simulated balances. No bank or payment network is contacted.</Text>
            </View>
          </View>

          {(notice.length > 0 || lastTransfer) && (
            <View style={[styles.notice, noticeIsError ? styles.noticeError : styles.noticeSuccess]}>
              <Text style={styles.noticeTitle}>{noticeIsError ? 'Could not connect' : notice}</Text>
              {lastTransfer && <Text style={styles.noticeBody}>Reference {lastTransfer.bank_reference.slice(0, 18)} · {formatMoney(lastTransfer.amount, lastTransfer.currency)}</Text>}
            </View>
          )}

          <View style={styles.sectionHead}>
            <Text style={styles.sectionTitle}>Recipient account</Text>
            <Text style={styles.sectionCaption}>Demo ledger</Text>
          </View>
          <View style={styles.recipientBalanceCard}>
            <View style={styles.recipientAvatarLarge}><Text style={styles.recipientAvatarTextLarge}>{(receiver.trim()[0] || 'B').toUpperCase()}</Text></View>
            <View style={styles.recipientBalanceInfo}>
              <Text style={styles.recipientName}>{receiver.trim() || 'Choose recipient'}</Text>
              <Text style={styles.recipientSub}>Available demo balance</Text>
            </View>
            <Text style={styles.recipientBalance}>{receiverBalance?.account_id === receiver.trim()
              ? formatMoney(receiverBalance.balance, receiverBalance.currency)
              : '—'}</Text>
          </View>

          {showServer && (
            <View style={styles.serverCard}>
              <Text style={styles.fieldLabel}>DEMO SERVER</Text>
              <TextInput autoCapitalize="none" autoCorrect={false} onChangeText={setDraftUrl} style={styles.serverInput} value={draftUrl} />
              <Text style={styles.serverHint}>Use the hosted HTTPS demo URL, or 10.0.2.2:8000 in the Android Emulator.</Text>
              <Pressable onPress={saveServerUrl} style={styles.saveButton}><Text style={styles.saveButtonText}>Save and connect</Text></Pressable>
            </View>
          )}

          <Text style={styles.footer}>FastPay is a demonstration app. Do not use real financial information.</Text>
        </ScrollView>
      </KeyboardAvoidingView>
    </View>
  );
}

const styles = StyleSheet.create({
  flex: { flex: 1 },
  screen: { flex: 1, backgroundColor: colors.canvas },
  content: { paddingHorizontal: 22, paddingTop: 12, paddingBottom: 34 },
  topBar: { flexDirection: 'row', alignItems: 'center', justifyContent: 'space-between', marginBottom: 18 },
  brandRow: { flexDirection: 'row', alignItems: 'center', gap: 11 },
  brandMark: { height: 42, width: 42, borderRadius: 14, backgroundColor: colors.blue, alignItems: 'center', justifyContent: 'center' },
  brandMarkText: { color: colors.white, fontSize: 22, fontWeight: '800' },
  brand: { color: colors.ink, fontWeight: '800', fontSize: 18, letterSpacing: -0.3 },
  brandSub: { color: colors.muted, fontSize: 11, marginTop: 2 },
  settingsButton: { width: 42, height: 42, backgroundColor: colors.white, borderRadius: 14, alignItems: 'center', justifyContent: 'center', borderWidth: 1, borderColor: colors.border },
  settingsGlyph: { color: colors.ink, fontSize: 17, fontWeight: '900', letterSpacing: 2 },
  demoBanner: { flexDirection: 'row', alignItems: 'center', alignSelf: 'flex-start', backgroundColor: colors.paleOrange, paddingHorizontal: 11, paddingVertical: 7, borderRadius: 20, marginBottom: 14 },
  demoDot: { width: 7, height: 7, borderRadius: 4, backgroundColor: '#D18A2D', marginRight: 7 },
  demoText: { color: colors.orange, fontSize: 10, fontWeight: '800', letterSpacing: 0.45 },
  balanceCard: { backgroundColor: colors.ink, borderRadius: 23, padding: 20, marginBottom: 26, overflow: 'hidden' },
  balanceTop: { flexDirection: 'row', justifyContent: 'space-between', alignItems: 'center' },
  cardEyebrow: { color: '#ABB8D0', fontSize: 10, fontWeight: '700', letterSpacing: 1 },
  refreshButton: { height: 29, minWidth: 78, alignItems: 'center', justifyContent: 'center', backgroundColor: '#2B3954', borderRadius: 10, paddingHorizontal: 9 },
  refreshText: { color: '#D9E2F3', fontSize: 11, fontWeight: '700' },
  balanceAmount: { color: colors.white, fontSize: 34, fontWeight: '800', letterSpacing: -1.2, marginTop: 15, marginBottom: 20 },
  balanceDivider: { height: 1, backgroundColor: '#34425C', marginBottom: 15 },
  accountRow: { flexDirection: 'row', alignItems: 'center' },
  avatar: { width: 34, height: 34, borderRadius: 12, alignItems: 'center', justifyContent: 'center', backgroundColor: '#4C6290' },
  avatarText: { color: colors.white, fontWeight: '800', fontSize: 14 },
  accountInfo: { flex: 1, marginLeft: 10 },
  accountName: { color: colors.white, fontSize: 12, fontWeight: '700' },
  accountId: { color: '#AAB6CA', fontSize: 10, marginTop: 3 },
  activePill: { backgroundColor: '#1F654F', paddingVertical: 5, paddingHorizontal: 8, borderRadius: 8 },
  activePillText: { color: '#9DE8C9', fontSize: 8, fontWeight: '800', letterSpacing: 0.5 },
  sectionHead: { flexDirection: 'row', alignItems: 'baseline', justifyContent: 'space-between', marginBottom: 12 },
  sectionTitle: { fontSize: 16, fontWeight: '800', color: colors.ink, letterSpacing: -0.2 },
  sectionCaption: { fontSize: 10, color: colors.muted },
  formCard: { backgroundColor: colors.white, borderRadius: 22, padding: 18, borderWidth: 1, borderColor: colors.border, marginBottom: 16 },
  fieldLabel: { color: '#8B98AA', fontSize: 9, fontWeight: '800', letterSpacing: 1 },
  recipientInputRow: { flexDirection: 'row', alignItems: 'center', marginTop: 11, marginBottom: 14 },
  recipientAvatar: { width: 34, height: 34, borderRadius: 12, backgroundColor: colors.paleBlue, alignItems: 'center', justifyContent: 'center' },
  recipientAvatarText: { color: colors.blue, fontWeight: '800', fontSize: 14 },
  recipientInput: { flex: 1, color: colors.ink, fontWeight: '700', marginLeft: 10, paddingVertical: 4, fontSize: 13 },
  chevron: { color: '#97A4B5', fontSize: 19, marginRight: 2 },
  fieldDivider: { height: 1, backgroundColor: colors.border, marginBottom: 15 },
  amountRow: { flexDirection: 'row', alignItems: 'center', marginTop: 3 },
  currencySign: { color: colors.ink, fontSize: 28, fontWeight: '700', marginRight: 6 },
  amountInput: { color: colors.ink, fontSize: 32, fontWeight: '800', minWidth: 70, flex: 1, paddingVertical: 3, letterSpacing: -0.8 },
  currencyCode: { color: colors.muted, backgroundColor: colors.canvas, overflow: 'hidden', paddingHorizontal: 9, paddingVertical: 6, borderRadius: 8, fontSize: 10, fontWeight: '800' },
  quickAmounts: { flexDirection: 'row', gap: 8, marginTop: 9, marginBottom: 17 },
  quickChip: { paddingHorizontal: 12, paddingVertical: 7, borderRadius: 9, borderWidth: 1, borderColor: colors.border, backgroundColor: colors.white },
  quickChipSelected: { backgroundColor: colors.paleBlue, borderColor: '#C8D5FF' },
  quickChipText: { color: '#66758C', fontSize: 11, fontWeight: '700' },
  quickChipTextSelected: { color: colors.blue },
  sendButton: { height: 52, borderRadius: 15, backgroundColor: colors.blue, alignItems: 'center', justifyContent: 'center', shadowColor: colors.blue, shadowOpacity: 0.18, shadowRadius: 9, shadowOffset: { width: 0, height: 5 }, elevation: 3 },
  sendButtonDisabled: { backgroundColor: '#AAB8EB', shadowOpacity: 0, elevation: 0 },
  sendButtonText: { color: colors.white, fontSize: 13, fontWeight: '800', letterSpacing: 0.1 },
  secureNote: { flexDirection: 'row', alignItems: 'flex-start', marginTop: 13 },
  secureIcon: { color: colors.orange, fontSize: 12, marginRight: 6 },
  secureText: { color: colors.muted, fontSize: 10, lineHeight: 15, flex: 1 },
  notice: { padding: 13, borderRadius: 13, marginBottom: 21 },
  noticeSuccess: { backgroundColor: colors.paleGreen },
  noticeError: { backgroundColor: '#FFF0EE' },
  noticeTitle: { color: colors.ink, fontSize: 12, fontWeight: '700' },
  noticeBody: { color: colors.muted, fontSize: 10, marginTop: 4 },
  recipientBalanceCard: { backgroundColor: colors.white, borderRadius: 17, padding: 14, borderColor: colors.border, borderWidth: 1, flexDirection: 'row', alignItems: 'center' },
  recipientAvatarLarge: { height: 40, width: 40, borderRadius: 14, backgroundColor: '#F0EAFE', alignItems: 'center', justifyContent: 'center' },
  recipientAvatarTextLarge: { color: '#7951C8', fontSize: 15, fontWeight: '800' },
  recipientBalanceInfo: { flex: 1, marginLeft: 10 },
  recipientName: { color: colors.ink, fontSize: 11, fontWeight: '700' },
  recipientSub: { color: colors.muted, fontSize: 9, marginTop: 4 },
  recipientBalance: { color: colors.ink, fontSize: 12, fontWeight: '800' },
  serverCard: { backgroundColor: colors.white, padding: 17, borderRadius: 17, borderWidth: 1, borderColor: colors.border, marginTop: 16 },
  serverInput: { color: colors.ink, borderWidth: 1, borderColor: colors.border, borderRadius: 10, paddingHorizontal: 11, paddingVertical: 10, fontSize: 12, marginTop: 9 },
  serverHint: { color: colors.muted, fontSize: 10, marginTop: 7 },
  saveButton: { backgroundColor: colors.ink, borderRadius: 10, alignItems: 'center', paddingVertical: 11, marginTop: 12 },
  saveButtonText: { color: colors.white, fontSize: 11, fontWeight: '700' },
  footer: { color: '#99A5B6', textAlign: 'center', fontSize: 9, lineHeight: 14, marginTop: 22, paddingHorizontal: 18 },
});
