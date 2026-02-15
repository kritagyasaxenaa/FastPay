#include "ultrasoundhelper.h"
#include <QDebug>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QCoreApplication>
#endif

UltrasoundHelper::UltrasoundHelper(QObject *parent) 
    : QObject(parent)
    , m_audioSource(nullptr)
    , m_audioSink(nullptr)
    , m_inputDevice(nullptr)
    , m_outputDevice(nullptr)
{
    setupAudio();
}

UltrasoundHelper::~UltrasoundHelper()
{
    stopListening();
    stopEmitting();
}

void UltrasoundHelper::setupAudio()
{
    // Audio format for ultrasound (20kHz sampling)
    m_format.setSampleRate(44100);
    m_format.setChannelCount(1);
    m_format.setSampleFormat(QAudioFormat::Int16);
    
    // Check if format is supported
    QAudioDevice defaultInputDevice = QMediaDevices::defaultAudioInput();
    if (!defaultInputDevice.isFormatSupported(m_format)) {
        qWarning() << "Default audio format not supported, trying to use nearest";
        m_format = defaultInputDevice.preferredFormat();
    }
}

bool UltrasoundHelper::checkAudioPermission()
{
#ifdef Q_OS_ANDROID
    // Check if RECORD_AUDIO permission is granted
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (!activity.isValid()) {
        qWarning() << "Cannot get Android context";
        return false;
    }

    QJniObject permission = QJniObject::fromString("android.permission.RECORD_AUDIO");
    
    jint result = activity.callMethod<jint>(
        "checkSelfPermission",
        "(Ljava/lang/String;)I",
        permission.object<jstring>()
    );
    
    return (result == 0); // PERMISSION_GRANTED = 0
#else
    return true; // Desktop - no permission needed
#endif
}

void UltrasoundHelper::requestAudioPermission()
{
#ifdef Q_OS_ANDROID
    QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (!activity.isValid()) {
        qWarning() << "Cannot get Android context for permission request";
        return;
    }

    // Request RECORD_AUDIO permission
    QJniObject permission = QJniObject::fromString("android.permission.RECORD_AUDIO");
    
    // Create array with single permission
    QJniObject permissionArray = QJniObject::callStaticObjectMethod(
        "java/lang/reflect/Array",
        "newInstance",
        "(Ljava/lang/Class;I)Ljava/lang/Object;",
        QJniObject::callStaticObjectMethod("java/lang/String", "class", "()Ljava/lang/Class;").object(),
        1
    );
    
    QJniObject::callStaticMethod<void>(
        "java/lang/reflect/Array",
        "set",
        "(Ljava/lang/Object;ILjava/lang/Object;)V",
        permissionArray.object(),
        0,
        permission.object()
    );
    
    // Request permission
    activity.callMethod<void>(
        "requestPermissions",
        "([Ljava/lang/String;I)V",
        permissionArray.object<jobjectArray>(),
        1001 // Request code
    );
#endif
}

void UltrasoundHelper::startEmitting(const QByteArray &payload)
{
    if (m_emitting) {
        stopEmitting();
    }
    
    m_emitting = true;
    m_emitPayload = payload;
    
    qDebug() << "Starting ultrasound emission with" << payload.size() << "bytes";
    
    // Initialize audio output
    QAudioDevice outputDevice = QMediaDevices::defaultAudioOutput();
    m_audioSink = new QAudioSink(outputDevice, m_format, this);
    
    // TODO: Generate ultrasound signal from payload
    // For now, this is a placeholder - actual implementation would:
    // 1. Convert payload to ultrasound frequencies (18-22 kHz)
    // 2. Generate audio waveform
    // 3. Write to QIODevice from m_audioSink->start()
    
    m_outputDevice = m_audioSink->start();
    if (!m_outputDevice) {
        qWarning() << "Failed to start audio output";
        stopEmitting();
        return;
    }
    
    qDebug() << "Ultrasound emission started";
}

void UltrasoundHelper::stopEmitting()
{
    if (!m_emitting) return;
    
    m_emitting = false;
    
    if (m_audioSink) {
        m_audioSink->stop();
        m_audioSink->deleteLater();
        m_audioSink = nullptr;
    }
    
    m_outputDevice = nullptr;
    m_emitPayload.clear();
    
    qDebug() << "Ultrasound emission stopped";
}

void UltrasoundHelper::startListening()
{
    // Check permission first on Android
#ifdef Q_OS_ANDROID
    if (!checkAudioPermission()) {
        qWarning() << "RECORD_AUDIO permission not granted, requesting...";
        requestAudioPermission();
        emit permissionRequired();
        return;
    }
#endif

    if (m_listening) {
        stopListening();
    }
    
    m_listening = true;
    m_audioBuffer.clear();
    
    qDebug() << "Starting ultrasound listening...";
    
    // Initialize audio input
    QAudioDevice inputDevice = QMediaDevices::defaultAudioInput();
    m_audioSource = new QAudioSource(inputDevice, m_format, this);
    
    m_inputDevice = m_audioSource->start();
    if (!m_inputDevice) {
        qWarning() << "Failed to start audio input";
        stopListening();
        return;
    }
    
    // Connect to read audio data as it arrives
    connect(m_inputDevice, &QIODevice::readyRead, this, &UltrasoundHelper::onAudioDataReady);
    
    qDebug() << "Ultrasound listening started";
}

void UltrasoundHelper::stopListening()
{
    if (!m_listening) return;
    
    m_listening = false;
    
    if (m_inputDevice) {
        disconnect(m_inputDevice, &QIODevice::readyRead, this, &UltrasoundHelper::onAudioDataReady);
    }
    
    if (m_audioSource) {
        m_audioSource->stop();
        m_audioSource->deleteLater();
        m_audioSource = nullptr;
    }
    
    m_inputDevice = nullptr;
    m_audioBuffer.clear();
    
    qDebug() << "Ultrasound listening stopped";
}

void UltrasoundHelper::onAudioDataReady()
{
    if (!m_inputDevice || !m_listening) return;
    
    // Read audio data from microphone
    QByteArray chunk = m_inputDevice->readAll();
    if (chunk.isEmpty()) return;
    
    m_audioBuffer.append(chunk);
    
    // TODO: Process audio buffer to detect ultrasound patterns
    // For now, this is a placeholder - actual implementation would:
    // 1. Apply FFT to detect 18-22 kHz frequencies
    // 2. Decode the ultrasound signal back to data
    // 3. Extract header and payload
    // 4. Emit keyReceived signal when complete
    
    // Placeholder: simulate detection after collecting 2x expected size
    const int expectedSize = 4096; // Adjust based on actual payload size
    if (m_audioBuffer.size() >= expectedSize * 2) {
        // Extract mock key (in real implementation, decode from audio)
        QByteArray extractedKey = m_audioBuffer.left(32); // Placeholder
        emit keyReceived(extractedKey);
        
        // Clear buffer after extraction
        m_audioBuffer.clear();
        qDebug() << "Ultrasound key extracted (placeholder)";
    }
}
