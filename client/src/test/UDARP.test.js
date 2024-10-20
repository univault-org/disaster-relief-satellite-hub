import { describe, it, expect, beforeEach, afterEach, vi } from 'vitest';
import { generateKey } from '../lib/keyGen';

// Mock the history data
const mockHistory = [
  {
    timestamp: "2023-05-01T10:00:00Z",
    deviceA: "device1",
    deviceB: "device2",
    udarpHash: "a1b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6",
    result: "success",
    environmentHash: "q1w2e3r4t5y6u7i8o9p0"
  },
  {
    timestamp: "2023-05-02T14:30:00Z",
    deviceA: "device1",
    deviceB: "device3",
    udarpHash: "b2c3d4e5f6g7h8i9j0k1l2m3n4o5p6q7",
    result: "success",
    environmentHash: "w2e3r4t5y6u7i8o9p0a1"
  }
];

describe('UDARP Key Generation', () => {
  const mockUdarpProfile = 'mockUdarpProfile';
  const mockEnvironmentData = {
    temperature: 25,
    humidity: 60,
    noise: 40
  };

  beforeEach(() => {
    // Mock the Date.now() to return a fixed timestamp
    vi.useFakeTimers();
    vi.setSystemTime(new Date('2023-05-03T12:00:00Z'));
  });

  afterEach(() => {
    vi.useRealTimers();
  });

  it('Two devices with same history generate the same key', () => {
    const key1 = generateKey('device1', mockUdarpProfile, mockEnvironmentData, mockHistory);
    const key2 = generateKey('device2', mockUdarpProfile, mockEnvironmentData, mockHistory);

    expect(key1).toBe(key2);
  });

  it('Third device with different history generates a different key', () => {
    const key1 = generateKey('device1', mockUdarpProfile, mockEnvironmentData, mockHistory);
    const key3 = generateKey('device3', mockUdarpProfile, mockEnvironmentData, mockHistory);

    expect(key1).not.toBe(key3);
  });

  it('Same device generates different keys with different UDARP profiles', () => {
    const key1 = generateKey('device1', mockUdarpProfile, mockEnvironmentData, mockHistory);
    const key1Different = generateKey('device1', 'differentUdarpProfile', mockEnvironmentData, mockHistory);

    expect(key1).not.toBe(key1Different);
  });

  it('Same device generates different keys with different environment data', () => {
    const key1 = generateKey('device1', mockUdarpProfile, mockEnvironmentData, mockHistory);
    const key1Different = generateKey('device1', mockUdarpProfile, { ...mockEnvironmentData, temperature: 30 }, mockHistory);

    expect(key1).not.toBe(key1Different);
  });
});
