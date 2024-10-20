import { useState, useEffect } from 'react';
import PropTypes from 'prop-types';
import { generateKey } from './lib/keyGen';
import history from './history.json';

const UDARP = ({ deviceId, onKeyGenerated }) => {
  const [key, setKey] = useState('');

  const handleGenerateKey = () => {
    // Simulated UDARP profile (in a real scenario, this would be generated from audio data)
    const udarpProfile = Math.random().toString(36).substring(7);
    
    // Simulated environmental data
    const environmentData = {
      temperature: Math.round(Math.random() * 30 + 10), // 10-40 degrees
      humidity: Math.round(Math.random() * 100), // 0-100%
      noise: Math.round(Math.random() * 50 + 30) // 30-80 dB
    };

    const generatedKey = generateKey(deviceId, udarpProfile, environmentData, history);
    setKey(generatedKey);
    onKeyGenerated(generatedKey);

    console.log('Generated Key:', generatedKey);
  };

  useEffect(() => {
    handleGenerateKey();
  }, [deviceId]);



  return (
    <div>
      <h2>UDARP Key Generation</h2>
      <p>Device ID: {deviceId}</p>
        <p>Generated Key: {key}</p>
      <button onClick={handleGenerateKey}>Generate Key</button>
    </div>
  );
};

// Add prop validation
UDARP.propTypes = {
  deviceId: PropTypes.string.isRequired,
  onKeyGenerated: PropTypes.func.isRequired,
};

export default UDARP;
