import { useState, useEffect, useRef } from 'react';
import './App.css';

function App() {
  const [ggwaveModule, setGgwaveModule] = useState(null);
  const [txData, setTxData] = useState('Hello javascript');
  const [rxData, setRxData] = useState('');
  const [isCapturing, setIsCapturing] = useState(false);
  const audioContextRef = useRef(null);
  const ggwaveInstanceRef = useRef(null);
  const recorderRef = useRef(null);
  const mediaStreamRef = useRef(null);

  useEffect(() => {
    const loadGgwaveModule = async () => {
      try {
        const module = await import('../../lib/ggwave/bindings/javascript/ggwave.js');
        const ggwave = await module.default();
        setGgwaveModule(ggwave);
      } catch (error) {
        console.error("Error loading ggwave module:", error);
      }
    };

    loadGgwaveModule();
  }, []);

  const ensureAudioContextAndGGWave = () => {
    if (!audioContextRef.current) {
      audioContextRef.current = new (window.AudioContext || window.webkitAudioContext)({ sampleRate: 48000 });
    }

    if (audioContextRef.current.state === 'suspended') {
      audioContextRef.current.resume();
    }

    if (!ggwaveInstanceRef.current && ggwaveModule) {
      const parameters = ggwaveModule.getDefaultParameters();
      parameters.sampleRateInp = audioContextRef.current.sampleRate;
      parameters.sampleRateOut = audioContextRef.current.sampleRate;
      ggwaveInstanceRef.current = ggwaveModule.init(parameters);
    }
  };

  const convertTypedArray = (src, type) => {
    const buffer = new ArrayBuffer(src.byteLength);
    new src.constructor(buffer).set(src);
    return new type(buffer);
  };

  const onSend = () => {
    ensureAudioContextAndGGWave();
    if (!ggwaveInstanceRef.current || !audioContextRef.current) return;

    stopCapture();

    const waveform = ggwaveModule.encode(ggwaveInstanceRef.current, txData, ggwaveModule.ProtocolId.GGWAVE_PROTOCOL_AUDIBLE_FAST, 10);

    const buf = convertTypedArray(waveform, Float32Array);
    const buffer = audioContextRef.current.createBuffer(1, buf.length, audioContextRef.current.sampleRate);
    buffer.getChannelData(0).set(buf);
    const source = audioContextRef.current.createBufferSource();
    source.buffer = buffer;
    source.connect(audioContextRef.current.destination);
    source.start(0);
  };

  const startCapture = () => {
    ensureAudioContextAndGGWave();
    if (!ggwaveInstanceRef.current || !audioContextRef.current) return;

    let constraints = {
      audio: {
        echoCancellation: false,
        autoGainControl: false,
        noiseSuppression: false
      }
    };

    navigator.mediaDevices.getUserMedia(constraints).then(function (e) {
      mediaStreamRef.current = audioContextRef.current.createMediaStreamSource(e);

      const bufferSize = 1024;
      const numberOfInputChannels = 1;
      const numberOfOutputChannels = 1;

      recorderRef.current = audioContextRef.current.createScriptProcessor(
        bufferSize,
        numberOfInputChannels,
        numberOfOutputChannels
      );

      recorderRef.current.onaudioprocess = function (e) {
        const source = e.inputBuffer;
        const res = ggwaveModule.decode(ggwaveInstanceRef.current, convertTypedArray(new Float32Array(source.getChannelData(0)), Int8Array));

        if (res && res.length > 0) {
          const decodedText = new TextDecoder("utf-8").decode(res);
          setRxData(decodedText);
        }
      };

      mediaStreamRef.current.connect(recorderRef.current);
      recorderRef.current.connect(audioContextRef.current.destination);
    }).catch(function (e) {
      console.error(e);
    });

    setRxData('Listening ...');
    setIsCapturing(true);
  };

  const stopCapture = () => {
    if (recorderRef.current) {
      recorderRef.current.disconnect(audioContextRef.current.destination);
      mediaStreamRef.current.disconnect(recorderRef.current);
      recorderRef.current = null;
    }

    setRxData('Audio capture is paused! Press the "Start capturing" button to analyze audio from the microphone');
    setIsCapturing(false);
  };

  return (
    <div id="main-container">
      Minimal <b>ggwave</b> example using Javascript bindings

      <br /><br />

      <div>Tx Data:</div> 
      <textarea 
        value={txData} 
        onChange={(e) => setTxData(e.target.value)} 
        style={{width:'300px', height:'100px'}}
      />
      <br />

      <button onClick={onSend}>Send</button>

      <br /><br />

      <div>Rx data:</div> 
      <textarea 
        value={rxData} 
        readOnly 
        style={{width:'300px', height:'100px'}}
      />
      <br />

      {!isCapturing ? (
        <button onClick={startCapture}>Start capturing</button>
      ) : (
        <button onClick={stopCapture}>Stop capturing</button>
      )}

      <br /><br />
    </div>
  );
}

export default App;
