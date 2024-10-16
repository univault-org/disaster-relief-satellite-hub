import { useState, useEffect } from 'react'
import './App.css'

function App() {
  const [order, setOrder] = useState('')
  const [audioContext, setAudioContext] = useState(null)
  const [audioStatus, setAudioStatus] = useState('Not initialized')
  const [logs, setLogs] = useState([])

  useEffect(() => {
    const initAudio = () => {
      if (!audioContext) {
        try {
          const newAudioContext = new (window.AudioContext || window.webkitAudioContext)()
          setAudioContext(newAudioContext)
          setAudioStatus('Initialized')
          addLog('Audio context initialized')
        } catch (error) {
          console.error('Failed to initialize audio context:', error)
          setAudioStatus('Initialization failed')
          addLog('Audio initialization failed: ' + error.message)
        }
      }
    }

    document.addEventListener('click', initAudio, { once: true })

    return () => {
      document.removeEventListener('click', initAudio)
    }
  }, [audioContext])

  const addLog = (message) => {
    setLogs(prevLogs => [...prevLogs, `${new Date().toISOString()}: ${message}`])
  }

  const generateKnownBitPattern = () => {
    // Generate a 128-bit pattern (16 bytes)
    const pattern = new Uint8Array([
      0xAA, 0x55, 0xAA, 0x55, // Alternating 10101010 and 01010101
      0xFF, 0x00, 0xFF, 0x00, // Alternating all 1s and all 0s
      0x12, 0x34, 0x56, 0x78, // Increasing pattern
      0xFE, 0xDC, 0xBA, 0x98  // Decreasing pattern
    ]);
    return pattern;
  }

  const generateUltrasonicSignal = () => {
    if (!audioContext) {
      setAudioStatus('Audio context not initialized')
      addLog('Audio context not initialized')
      return
    }

    try {
      addLog('Starting ultrasonic signal generation')
      const pattern = generateKnownBitPattern();
      addLog(`Known pattern generated: ${pattern.length} bytes`)

      const bits = []
      for (let byte of pattern) {
        for (let i = 0; i < 8; i++) {
          bits.push((byte & (1 << i)) !== 0)
        }
      }
      addLog(`Bits generated: ${bits.length} bits`)

      const sampleRate = 48000
      const f0 = 5000
      const df = 1000
      const samplesPerFrame = 480

      addLog(`Audio parameters: sampleRate=${sampleRate}, f0=${f0}, df=${df}, samplesPerFrame=${samplesPerFrame}`)

      const buffer = audioContext.createBuffer(1, bits.length * samplesPerFrame, sampleRate)
      const channel = buffer.getChannelData(0)
      addLog(`Audio buffer created: ${buffer.length} samples`)

      bits.forEach((bit, index) => {
        const frequency = bit ? f0 + df : f0
        for (let i = 0; i < samplesPerFrame; i++) {
          const t = i / sampleRate
          channel[index * samplesPerFrame + i] = Math.sin(2 * Math.PI * frequency * t)
        }
      })
      addLog('Signal waveform generated')

      const source = audioContext.createBufferSource()
      source.buffer = buffer
      source.connect(audioContext.destination)
      setAudioStatus('Generating signal...')
      addLog('Starting signal playback')
      source.onended = () => {
        setAudioStatus('Signal transmission complete')
        addLog('Signal playback ended')
      }
      source.start()
      addLog('Signal playback started')
    } catch (error) {
      console.error('Failed to generate ultrasonic signal:', error)
      setAudioStatus('Signal generation failed')
      addLog('Signal generation failed: ' + error.message)
    }
  }

  const handleSubmit = (e) => {
    e.preventDefault()
    if (!audioContext) {
      alert('Please click anywhere on the page to initialize audio before submitting.')
      return
    }
    addLog('Generating known bit pattern signal')
    generateUltrasonicSignal()
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
        <button type="submit" style={{
          backgroundColor: '#4CAF50',
          border: 'none',
          color: 'white',
          padding: '15px 32px',
          textAlign: 'center',
          textDecoration: 'none',
          display: 'inline-block',
          fontSize: '16px',
          margin: '4px 2px',
          cursor: 'pointer'
        }}>
          Submit Order and Send Signal
        </button>
      </form>
      {!audioContext && (
        <p>Click anywhere on the page to initialize audio.</p>
      )}
      <p>Audio Status: {audioStatus}</p>
      <div style={{ marginTop: '20px' }}>
        <h3>Logs:</h3>
        <div style={{ height: '200px', overflowY: 'scroll', border: '1px solid #ccc', padding: '10px' }}>
          {logs.map((log, index) => (
            <div key={index}>{log}</div>
          ))}
        </div>
      </div>
    </div>
  )
}

export default App
