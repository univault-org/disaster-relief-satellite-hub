import SHA256 from 'crypto-js/sha256';

export const generateKey = (deviceId, udarpProfile, environmentData, history) => {
  const relevantEntry = history.find(entry => entry.deviceA === deviceId || entry.deviceB === deviceId);

  const combinedData = JSON.stringify({
    udarpProfile,
    environmentData,
    relevantEntry,
    timestamp: new Date().toISOString()
  });

  console.log('Device ID:', deviceId);
  console.log('Relevant Entry:', JSON.stringify(relevantEntry, null, 2));
  console.log('Combined Data:', combinedData);

  const key = SHA256(combinedData).toString();
  console.log('Generated Key:', key);

  return key;
};
