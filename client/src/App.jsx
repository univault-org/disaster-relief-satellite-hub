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

  const generateUltrasonicSignal = (message) => {
    if (!audioContext) {
      setAudioStatus('Audio context not initialized')
      addLog('Audio context not initialized')
      return
    }

    try {
      addLog('Starting ultrasonic signal generation')
      const encoder = new TextEncoder()
      const data = encoder.encode(message)
      addLog(`Message encoded: ${data.length} bytes`)

      const bits = []
      for (let byte of data) {
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
    console.log('Order submitted:', order)
    addLog(`Order submitted: ${order}`)
    const message = "TEST:" + order
    addLog(`Preparing message: ${message}`)
    generateUltrasonicSignal(message)
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
