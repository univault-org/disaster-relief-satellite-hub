import { useState, useEffect } from 'react'
import './App.css'

function App() {
  const [audioContext, setAudioContext] = useState(null)
  const [logs, setLogs] = useState([])
  const [isTransmitting, setIsTransmitting] = useState(false)

  useEffect(() => {
    const ctx = new (window.AudioContext || window.webkitAudioContext)();
    setAudioContext(ctx);
  }, []);

  const addLog = (message) => {
    setLogs(prevLogs => [...prevLogs, `${new Date().toISOString()}: ${message}`]);
  };

  const generateEncryptionKey = () => {
    const key = new Uint8Array(16);
    crypto.getRandomValues(key);
    return key;
  };

  const generateSignal = (key) => {
    if (!audioContext) return;

    const sampleRate = audioContext.sampleRate;
    const duration = 5; // 5 seconds total
    const f0 = 18000; // Base frequency
    const df = 1000; // Frequency shift
    const amplitude = 0.8;

    const buffer = audioContext.createBuffer(1, sampleRate * duration, sampleRate);
    const channel = buffer.getChannelData(0);

    // Generate preamble (0.5 seconds)
    const preambleDuration = sampleRate * 0.5;
    for (let i = 0; i < preambleDuration; i++) {
      const t = i / sampleRate;
      const preambleFreq = f0 + (df * i / preambleDuration);
      channel[i] = amplitude * Math.sin(2 * Math.PI * preambleFreq * t);
    }

    // Add a short silence after the preamble (0.1 seconds)
    const silenceDuration = sampleRate * 0.1;
    for (let i = preambleDuration; i < preambleDuration + silenceDuration; i++) {
      channel[i] = 0;
    }

    // Add start marker (0.1 seconds)
    const markerDuration = sampleRate * 0.1;
    for (let i = preambleDuration + silenceDuration; i < preambleDuration + silenceDuration + markerDuration; i++) {
      const t = (i - (preambleDuration + silenceDuration)) / sampleRate;
      channel[i] = amplitude * Math.sin(2 * Math.PI * (f0 + df) * t);
    }

    // Generate data signal (4.3 seconds)
    const bitsPerSecond = 32; // 32 bits per second
    const samplesPerBit = sampleRate / bitsPerSecond;
    let phase = 0;
    for (let i = preambleDuration + silenceDuration + markerDuration; i < buffer.length; i++) {
      const dataIndex = i - (preambleDuration + silenceDuration + markerDuration);
      const bitIndex = Math.floor(dataIndex / (samplesPerBit * 2)); // Each bit is repeated
      const byteIndex = Math.floor(bitIndex / 8);
      const bitInByte = 7 - (bitIndex % 8);
      
      if (byteIndex < key.length) {
        const bit = (key[byteIndex] & (1 << bitInByte)) !== 0;
        const frequency = bit ? f0 + df : f0;
        phase += 2 * Math.PI * frequency / sampleRate;
        channel[i] = amplitude * Math.sin(phase);
      }
    }

    const source = audioContext.createBufferSource();
    source.buffer = buffer;
    source.connect(audioContext.destination);
    return source;
  };

  const startTransmission = () => {
    if (isTransmitting) return;
    setIsTransmitting(true);

    const key = generateEncryptionKey();
    const keyHex = Array.from(key).map(b => b.toString(16).padStart(2, '0')).join('');
    addLog(`Generated encryption key: ${keyHex}`);

    const f0 = 18000; // Base frequency
    const df = 1000; // Frequency shift
    const amplitude = 0.8; // Amplitude

    addLog(`Transmission parameters: f0=${f0}, df=${df}, amplitude=${amplitude}`);

    const source = generateSignal(key);
    source.start();

    // Add audible beep at the start of transmission
    const oscillator = audioContext.createOscillator();
    oscillator.type = 'sine';
    oscillator.frequency.setValueAtTime(440, audioContext.currentTime); // 440 Hz - A4 note
    oscillator.connect(audioContext.destination);
    oscillator.start();
    oscillator.stop(audioContext.currentTime + 0.2); // Beep for 0.2 seconds

    addLog('Started 4.5-second transmission');
    setTimeout(() => {
      setIsTransmitting(false);
      addLog('Transmission ended');
    }, 4500);
  };

  return (
    <div className="App">
      <h1>Ultrasonic Test Signal Transmitter</h1>
      <button onClick={startTransmission} disabled={isTransmitting} style={{ backgroundColor: isTransmitting ? 'gray' : 'blue' }}>
        {isTransmitting ? 'Transmitting...' : 'Start 5s Test Signal'}
      </button>
      <div>
        <h3>Logs:</h3>
        {logs.map((log, index) => <div key={index}>{log}</div>)}
      </div>
    </div>
  )
}

export default App;
