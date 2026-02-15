#ifndef ULTRASOUNDHELPER_H
#define ULTRASOUNDHELPER_H

#include <QObject>
#include <QByteArray>
#include <QAudioSource>
#include <QAudioSink>
#include <QAudioFormat>
#include <QMediaDevices>
#include <QIODevice>

class UltrasoundHelper : public QObject
{
    Q_OBJECT
public:
    explicit UltrasoundHelper(QObject *parent = nullptr);
    ~UltrasoundHelper();

    // Receiver: start emitting ultrasound payload (header + public key)
    void startEmitting(const QByteArray &payload);
    void stopEmitting();

    // Sender: start recording from mic, buffer size 2x; when enough data, search header and extract key
    void startListening();
    void stopListening();
    
    // Check and request audio permission (Android)
    bool checkAudioPermission();
    void requestAudioPermission();

    bool isEmitting() const { return m_emitting; }
    bool isListening() const { return m_listening; }

signals:
    void keyReceived(const QByteArray &publicKeyFromMic);
    void error(const QString &message);
    void permissionRequired();

private slots:
    void onAudioDataReady();

private:
    void setupAudio();

    bool m_emitting = false;
    bool m_listening = false;
    
    QAudioFormat m_format;
    QAudioSource *m_audioSource;
    QAudioSink *m_audioSink;
    QIODevice *m_inputDevice;
    QIODevice *m_outputDevice;
    
    QByteArray m_emitPayload;
    QByteArray m_audioBuffer;
};

#endif // ULTRASOUNDHELPER_H
