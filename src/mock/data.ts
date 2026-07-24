import type { PlatformData, SpecialEvent } from '@/types'
import { CampType, SpecialEventType } from '@/types'

export const mockPlatforms: Omit<PlatformData, 'dataStatus' | 'validUntil' | 'updateTime' | 'sourceProtocol'>[] = [
  {
    id: 'SHIP_001',
    name: 'Own Ship',
    lon: 121.5,
    lat: 31.2,
    altitude: 0,
    speed: 12.5,
    type: 'warship',
    category: 'destroyer',
    camp: CampType.Friendly,
    weapons: [
      { type: 'missile', count: 8 },
      { type: 'gun', count: 2 }
    ],
    sensors: [
      { type: 'radar', count: 1 },
      { type: 'sonar', count: 1 }
    ]
  },
  {
    id: 'SHIP_002',
    name: 'Merchant A',
    lon: 121.51,
    lat: 31.22,
    altitude: 0,
    speed: 8.0,
    type: 'merchant',
    category: 'cargo',
    camp: CampType.Neutral,
    weapons: [],
    sensors: []
  },
  {
    id: 'SHIP_003',
    name: 'Fishing B',
    lon: 121.48,
    lat: 31.18,
    altitude: 0,
    speed: 5.0,
    type: 'fishing',
    category: 'fishery',
    camp: CampType.Neutral,
    weapons: [],
    sensors: []
  },
  {
    id: 'SHIP_004',
    name: 'Enemy Ship',
    lon: 121.55,
    lat: 31.25,
    altitude: 0,
    speed: 15.0,
    type: 'warship',
    category: 'cruiser',
    camp: CampType.Enemy,
    weapons: [
      { type: 'missile', count: 16 },
      { type: 'gun', count: 4 }
    ],
    sensors: [
      { type: 'radar', count: 2 },
      { type: 'sonar', count: 1 }
    ]
  }
]

export const mockEvents: Omit<SpecialEvent, 'timestamp'>[] = [
  {
    eventId: 'EVENT_001',
    eventType: SpecialEventType.Alert,
    eventName: 'Enemy Detected',
    description: 'Enemy warship detected in vicinity',
    targetId: 'SHIP_004',
    sourceId: 'SHIP_001',
    extraData: {
      detectionRange: 5000,
      confidence: 0.95
    }
  },
  {
    eventId: 'EVENT_002',
    eventType: SpecialEventType.MissionStart,
    eventName: 'Patrol Mission Started',
    description: 'Ship started patrol mission',
    targetId: 'SHIP_001',
    sourceId: 'SHIP_001',
    extraData: {
      missionName: 'Area Patrol',
      duration: 3600
    }
  }
]
