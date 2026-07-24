export enum DataStatus {
  Normal = 'normal',
  Expired = 'expired',
  Invalid = 'invalid'
}

export enum ProtocolType {
  Unknown = 'unknown',
  HTTP = 'http',
  UDP = 'udp',
  TCP = 'tcp',
  WebSocket = 'websocket',
  DDS = 'dds',
  Redis = 'redis'
}

export enum CampType {
  Unknown = 'unknown',
  Friendly = 'friendly',
  Enemy = 'enemy',
  Neutral = 'neutral'
}

export enum SpecialEventType {
  Unknown = 'unknown',
  Attack = 'attack',
  Defense = 'defense',
  Alert = 'alert',
  MissionStart = 'missionStart',
  MissionEnd = 'missionEnd',
  Contact = 'contact',
  Lost = 'lost',
  Damage = 'damage',
  Repair = 'repair',
  Custom = 'custom'
}

export interface WeaponInfo {
  type: string
  count: number
}

export interface SensorInfo {
  type: string
  count: number
}

export interface PlatformData {
  id: string
  name: string
  lon: number
  lat: number
  altitude: number
  speed: number
  type: string
  category: string
  camp: CampType
  weapons: WeaponInfo[]
  sensors: SensorInfo[]
  dataStatus: DataStatus
  validUntil: number
  updateTime: number
  sourceProtocol: ProtocolType
}

export interface SpecialEvent {
  eventId: string
  eventType: SpecialEventType
  eventName: string
  description: string
  timestamp: number
  targetId: string
  sourceId: string
  extraData: Record<string, unknown>
}

export interface DynamicObjects {
  platforms: PlatformData[]
  events: SpecialEvent[]
  timestamp: number
}

export interface PropertyBox {
  id: string
  name: string
  isOwnShip: boolean
  lon: number
  lat: number
  speed: number
  x: number
  y: number
  isDragging: boolean
  dragOffsetX: number
  dragOffsetY: number
}

export interface ViewState {
  centerLon: number
  centerLat: number
  scale: number
  offsetX: number
  offsetY: number
}
