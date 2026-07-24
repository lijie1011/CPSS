import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import type { PlatformData, SpecialEvent, DynamicObjects } from '@/types'
import { DataStatus, ProtocolType } from '@/types'
import { mockPlatforms, mockEvents } from '@/mock/data'

export const useDataStore = defineStore('data', () => {
  const platforms = ref<PlatformData[]>([])
  const events = ref<SpecialEvent[]>([])
  const lastUpdateTime = ref<number>(0)

  const validPlatforms = computed(() => {
    const now = Date.now()
    return platforms.value.filter(p => 
      p.dataStatus !== DataStatus.Expired && 
      (p.validUntil <= 0 || p.validUntil > now)
    )
  })

  const getPlatformById = (id: string): PlatformData | undefined => {
    return platforms.value.find(p => p.id === id)
  }

  const getEventsByTargetId = (targetId: string): SpecialEvent[] => {
    return events.value.filter(e => e.targetId === targetId)
  }

  const updatePlatforms = (newPlatforms: PlatformData[]) => {
    const platformMap = new Map<string, PlatformData>()
    platforms.value.forEach(p => platformMap.set(p.id, p))
    newPlatforms.forEach(p => platformMap.set(p.id, p))
    platforms.value = Array.from(platformMap.values())
    lastUpdateTime.value = Date.now()
  }

  const updateEvents = (newEvents: SpecialEvent[]) => {
    const eventMap = new Map<string, SpecialEvent>()
    events.value.forEach(e => eventMap.set(e.eventId, e))
    newEvents.forEach(e => eventMap.set(e.eventId, e))
    events.value = Array.from(eventMap.values())
  }

  const updateData = (data: DynamicObjects) => {
    updatePlatforms(data.platforms)
    updateEvents(data.events)
  }

  const addPlatform = (platform: PlatformData) => {
    const index = platforms.value.findIndex(p => p.id === platform.id)
    if (index >= 0) {
      platforms.value[index] = platform
    } else {
      platforms.value.push(platform)
    }
    lastUpdateTime.value = Date.now()
  }

  const removePlatform = (id: string) => {
    platforms.value = platforms.value.filter(p => p.id !== id)
  }

  const addEvent = (event: SpecialEvent) => {
    const index = events.value.findIndex(e => e.eventId === event.eventId)
    if (index >= 0) {
      events.value[index] = event
    } else {
      events.value.push(event)
    }
  }

  const removeEvent = (eventId: string) => {
    events.value = events.value.filter(e => e.eventId !== eventId)
  }

  const loadMockData = () => {
    platforms.value = mockPlatforms.map(p => ({
      ...p,
      dataStatus: DataStatus.Normal,
      validUntil: Date.now() + 5000,
      updateTime: Date.now(),
      sourceProtocol: ProtocolType.Unknown
    }))
    events.value = mockEvents.map(e => ({
      ...e,
      timestamp: Date.now()
    }))
    lastUpdateTime.value = Date.now()
  }

  const updateMockData = () => {
    platforms.value = platforms.value.map(p => {
      const speedFactor = p.id === 'SHIP_001' ? 0 : 0.001
      const angle = Math.random() * Math.PI * 2
      return {
        ...p,
        lon: p.lon + Math.cos(angle) * speedFactor,
        lat: p.lat + Math.sin(angle) * speedFactor,
        speed: p.speed + (Math.random() - 0.5) * 0.5,
        updateTime: Date.now(),
        validUntil: Date.now() + 5000
      }
    })
    lastUpdateTime.value = Date.now()
  }

  const clearAll = () => {
    platforms.value = []
    events.value = []
    lastUpdateTime.value = 0
  }

  return {
    platforms,
    events,
    lastUpdateTime,
    validPlatforms,
    getPlatformById,
    getEventsByTargetId,
    updatePlatforms,
    updateEvents,
    updateData,
    addPlatform,
    removePlatform,
    addEvent,
    removeEvent,
    loadMockData,
    updateMockData,
    clearAll
  }
})
