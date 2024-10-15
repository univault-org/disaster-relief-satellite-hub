import { useState, useEffect } from 'react'
import './App.css'

function App() {
  const [order, setOrder] = useState('')
  const [audioContext, setAudioContext] = useState(null)

  useEffect(() => {
    setAudioContext(new (window.AudioContext || window.webkitAudioContext)())
  }, [])

  const generateUltrasonicSignal = (message) => {
    if (!audioContext) return;

    const encoder = new TextEncoder();
    const data = encoder.encode(message);
    const bits = [];
    for (let byte of data) {
      for (let i = 0; i < 8; i++) {
        bits.push((byte & (1 << i)) !== 0);
      }
    }

    const sampleRate = 48000;
    const f0 = 15000;
    const df = 1000;
    const samplesPerFrame = 480;

    const buffer = audioContext.createBuffer(1, bits.length * samplesPerFrame, sampleRate);
    const channel = buffer.getChannelData(0);

    bits.forEach((bit, index) => {
      const frequency = bit ? f0 + df : f0;
      for (let i = 0; i < samplesPerFrame; i++) {
        const t = i / sampleRate;
        channel[index * samplesPerFrame + i] = Math.sin(2 * Math.PI * frequency * t);
      }
    });

    const source = audioContext.createBufferSource();
    source.buffer = buffer;
    source.connect(audioContext.destination);
    source.start();
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    console.log('Order submitted:', order);
    const message = "TEST:" + order; // Prepend "TEST:" to the order
    generateUltrasonicSignal(message);
  }

  return (
    <div className="App">
      <h1>Disaster Relief Order System</h1>
      <form onSubmit={handleSubmit}>
        <textarea
          value={order}
          onChange={(e) => setOrder(e.target.value)}
          placeholder="Enter your order here"
          rows="4"
          cols="50"
        />
        <br />
        <button type="submit">Submit Order and Send Key</button>
      </form>
    </div>
  )
}

export default App
