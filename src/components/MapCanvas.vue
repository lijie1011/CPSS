<script setup lang="ts">
import { ref, onMounted, onUnmounted, watch, nextTick } from 'vue'
import L from 'leaflet'
import 'leaflet/dist/leaflet.css'
import { useDataStore } from '@/stores/dataStore'
import { useViewStore } from '@/stores/viewStore'
import { CampType, SpecialEventType } from '@/types'
import PropertyBox from './PropertyBox.vue'
import ConnectingLine from './ConnectingLine.vue'

const dataStore = useDataStore()
const viewStore = useViewStore()

const mapRef = ref<HTMLDivElement | null>(null)
let map: L.Map | null = null
let platformMarkers: Map<string, L.Marker | L.CircleMarker> = new Map()
let eventMarkers: Map<string, L.Marker> = new Map()

const getCampColor = (camp: CampType): string => {
  switch (camp) {
    case CampType.Friendly: return '#00ff00'
    case CampType.Enemy: return '#ff0000'
    case CampType.Neutral: return '#ffff00'
    default: return '#888888'
  }
}

const getEventIcon = (eventType: SpecialEventType): { icon: string; color: string } => {
  switch (eventType) {
    case SpecialEventType.Alert: return { icon: '!', color: '#ff0000' }
    case SpecialEventType.Attack: return { icon: 'A', color: '#8b0000' }
    case SpecialEventType.Defense: return { icon: 'D', color: '#0000ff' }
    case SpecialEventType.Contact: return { icon: 'C', color: '#00ffff' }
    case SpecialEventType.Damage: return { icon: 'X', color: '#ff8c00' }
    case SpecialEventType.MissionStart: return { icon: 'M', color: '#00ff00' }
    case SpecialEventType.MissionEnd: return { icon: 'E', color: '#808080' }
    case SpecialEventType.Lost: return { icon: '?', color: '#ff00ff' }
    case SpecialEventType.Repair: return { icon: 'R', color: '#90ee90' }
    default: return { icon: '*', color: '#ffff00' }
  }
}

const createShipIcon = (color: string, isOwnShip: boolean): L.DivIcon => {
  if (isOwnShip) {
    const svg = `<svg width="30" height="20" viewBox="0 0 30 20">
      <polygon points="15,0 5,8 8,10 5,12 15,20 25,12 22,10 25,8" fill="${color}" stroke="${color}" stroke-width="2"/>
    </svg>`
    return L.divIcon({
      html: svg,
      className: 'ship-icon',
      iconSize: [30, 20],
      iconAnchor: [15, 10]
    })
  }
  const svg = `<svg width="24" height="24" viewBox="0 0 24 24">
    <circle cx="12" cy="12" r="10" fill="${color}" stroke="${color}" stroke-width="2"/>
    <circle cx="12" cy="12" r="5" fill="rgba(255,255,255,0.3)"/>
  </svg>`
  return L.divIcon({
    html: svg,
    className: 'platform-icon',
    iconSize: [24, 24],
    iconAnchor: [12, 12]
  })
}

const createEventIcon = (iconText: string, color: string): L.DivIcon => {
  const svg = `<svg width="24" height="24" viewBox="0 0 24 24">
    <rect x="2" y="2" width="20" height="20" fill="${color}" rx="4"/>
    <text x="12" y="17" text-anchor="middle" fill="white" font-size="14" font-weight="bold">${iconText}</text>
  </svg>`
  return L.divIcon({
    html: svg,
    className: 'event-icon',
    iconSize: [24, 24],
    iconAnchor: [12, 12]
  })
}

const updatePlatforms = () => {
  if (!map) return
  
  for (const platform of dataStore.validPlatforms) {
    const existing = platformMarkers.get(platform.id)
    const color = getCampColor(platform.camp)
    const isOwnShip = platform.id === 'SHIP_001'
    
    if (existing) {
      existing.setLatLng([platform.lat, platform.lon])
    } else {
      const marker = L.marker([platform.lat, platform.lon], {
        icon: createShipIcon(color, isOwnShip),
        title: platform.name
      }).addTo(map)
      
      marker.on('click', (e) => {
        e.originalEvent.stopPropagation()
        const point = map!.latLngToContainerPoint([platform.lat, platform.lon])
        viewStore.createPropertyBox(platform.id, platform.name, isOwnShip, platform.lon, platform.lat, platform.speed, point.x, point.y)
      })
      
      platformMarkers.set(platform.id, marker)
    }
  }
  
  const validIds = new Set(dataStore.validPlatforms.map(p => p.id))
  for (const [id, marker] of platformMarkers) {
    if (!validIds.has(id)) {
      map.removeLayer(marker)
      platformMarkers.delete(id)
    }
  }
}

const updateEvents = () => {
  if (!map) return
  
  for (const event of dataStore.events) {
    const platform = dataStore.getPlatformById(event.targetId)
    if (!platform) continue
    
    const existing = eventMarkers.get(event.eventId)
    const { icon, color } = getEventIcon(event.eventType)
    
    if (existing) {
      existing.setLatLng([platform.lat, platform.lon])
    } else {
      const marker = L.marker([platform.lat, platform.lon], {
        icon: createEventIcon(icon, color),
        title: `${event.eventType}: ${platform.name}`
      }).addTo(map)
      eventMarkers.set(event.eventId, marker)
    }
  }
  
  const validIds = new Set(dataStore.events.map(e => e.eventId))
  for (const [id, marker] of eventMarkers) {
    if (!validIds.has(id)) {
      map.removeLayer(marker)
      eventMarkers.delete(id)
    }
  }
}

const handleMapClick = (e: L.LeafletMouseEvent) => {
  const target = e.originalEvent.target as HTMLElement
  if (target.closest('.leaflet-marker-icon')) return
  
  viewStore.hideAllPropertyBoxes()
}

const updatePropertyBoxPositions = () => {
  if (!map) return
  
  for (const box of viewStore.propertyBoxes) {
    const platform = dataStore.getPlatformById(box.id)
    if (platform && !box.isDragging) {
      const point = map.latLngToContainerPoint([platform.lat, platform.lon])
      viewStore.updatePropertyBoxPosition(box.id, point.x + 30, point.y - 100)
    }
  }
}

onMounted(async () => {
  await nextTick()
  
  if (!mapRef.value) return
  
  map = L.map(mapRef.value, {
    zoomControl: false,
    attributionControl: false
  }).setView([31.2, 121.5], 10)
  
  L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    maxZoom: 19,
    attribution: '© OpenStreetMap'
  }).addTo(map)
  
  L.tileLayer('https://tiles.openseamap.org/seamark/{z}/{x}/{y}.png', {
    maxZoom: 19,
    attribution: '© OpenSeaMap'
  }).addTo(map)
  
  map.on('click', handleMapClick)
  
  map.on('move', () => {
    updatePropertyBoxPositions()
  })
  
  map.on('zoom', () => {
    updatePropertyBoxPositions()
  })
  
  updatePlatforms()
  updateEvents()
})

onUnmounted(() => {
  if (map) {
    map.remove()
    map = null
  }
  platformMarkers.clear()
  eventMarkers.clear()
})

watch(() => dataStore.lastUpdateTime, () => {
  updatePlatforms()
  updateEvents()
  updatePropertyBoxPositions()
})

watch(() => viewStore.propertyBoxes, () => {
  updatePropertyBoxPositions()
}, { deep: true })

const handleZoomIn = () => {
  map?.zoomIn()
}

const handleZoomOut = () => {
  map?.zoomOut()
}

const handleReset = () => {
  map?.setView([31.2, 121.5], 10)
}

defineExpose({
  handleZoomIn,
  handleZoomOut,
  handleReset
})
</script>

<template>
  <div ref="mapRef" class="map-container">
    <div 
      v-for="box in viewStore.propertyBoxes" 
      :key="box.id"
      class="property-box-wrapper"
    >
      <ConnectingLine 
        :box="box"
        :width="0"
        :height="0"
        :map="map"
      />
      <PropertyBox
        :box="box"
        :platform="dataStore.getPlatformById(box.id)"
      />
    </div>
  </div>
</template>

<style scoped>
.map-container {
  width: 100%;
  height: 100%;
  position: relative;
}

.property-box-wrapper {
  position: absolute;
  top: 0;
  left: 0;
  pointer-events: none;
}

.property-box-wrapper > * {
  pointer-events: auto;
}

:deep(.ship-icon) {
  background: transparent;
  border: none;
}

:deep(.platform-icon) {
  background: transparent;
  border: none;
}

:deep(.event-icon) {
  background: transparent;
  border: none;
}
</style>
