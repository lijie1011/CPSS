import { defineStore } from 'pinia'
import { ref, computed } from 'vue'
import type { PropertyBox, ViewState } from '@/types'

export const useViewStore = defineStore('view', () => {
  const viewState = ref<ViewState>({
    centerLon: 121.5,
    centerLat: 31.2,
    scale: 4000000,
    offsetX: 0,
    offsetY: 0
  })

  const propertyBoxes = ref<PropertyBox[]>([])

  const screenScale = computed(() => {
    return 500000 / viewState.value.scale
  })

  const geoToScreen = (lon: number, lat: number, width: number, height: number): { x: number; y: number } => {
    const x = ((lon - viewState.value.centerLon) * 111320 * Math.cos(viewState.value.centerLat * Math.PI / 180) / viewState.value.scale) * width + width / 2
    const y = ((viewState.value.centerLat - lat) * 111320 / viewState.value.scale) * width + height / 2
    return { x, y }
  }

  const screenToGeo = (x: number, y: number, width: number, height: number): { lon: number; lat: number } => {
    const lon = (x - width / 2) * viewState.value.scale / (width * 111320 * Math.cos(viewState.value.centerLat * Math.PI / 180)) + viewState.value.centerLon
    const lat = viewState.value.centerLat - (y - height / 2) * viewState.value.scale / (width * 111320)
    return { lon, lat }
  }

  const zoomIn = () => {
    viewState.value.scale = Math.max(10000, viewState.value.scale * 0.8)
  }

  const zoomOut = () => {
    viewState.value.scale = Math.min(100000000, viewState.value.scale * 1.25)
  }

  const resetView = () => {
    viewState.value = {
      centerLon: 121.5,
      centerLat: 31.2,
      scale: 4000000,
      offsetX: 0,
      offsetY: 0
    }
  }

  const setCenter = (lon: number, lat: number) => {
    viewState.value.centerLon = lon
    viewState.value.centerLat = lat
  }

  const pan = (dx: number, dy: number, width: number) => {
    const deltaLon = dx * viewState.value.scale / (width * 111320 * Math.cos(viewState.value.centerLat * Math.PI / 180))
    const deltaLat = -dy * viewState.value.scale / (width * 111320)
    viewState.value.centerLon -= deltaLon
    viewState.value.centerLat -= deltaLat
  }

  const createPropertyBox = (id: string, name: string, isOwnShip: boolean, lon: number, lat: number, speed: number, x: number, y: number) => {
    const existing = propertyBoxes.value.find(b => b.id === id)
    if (existing) {
      destroyPropertyBox(id)
      return false
    }
    propertyBoxes.value.push({
      id,
      name,
      isOwnShip,
      lon,
      lat,
      speed,
      x: x + 30,
      y: y - 100,
      isDragging: false,
      dragOffsetX: 0,
      dragOffsetY: 0
    })
    return true
  }

  const destroyPropertyBox = (id: string) => {
    propertyBoxes.value = propertyBoxes.value.filter(b => b.id !== id)
  }

  const findPropertyBox = (id: string): PropertyBox | undefined => {
    return propertyBoxes.value.find(b => b.id === id)
  }

  const updatePropertyBoxPosition = (id: string, x: number, y: number) => {
    const box = propertyBoxes.value.find(b => b.id === id)
    if (box) {
      box.x = x
      box.y = y
    }
  }

  const updatePropertyBoxData = (id: string, lon: number, lat: number, speed: number) => {
    const box = propertyBoxes.value.find(b => b.id === id)
    if (box && !box.isDragging) {
      box.lon = lon
      box.lat = lat
      box.speed = speed
    }
  }

  const hideAllPropertyBoxes = () => {
    propertyBoxes.value = []
  }

  const setDragging = (id: string, dragging: boolean, offsetX: number = 0, offsetY: number = 0) => {
    const box = propertyBoxes.value.find(b => b.id === id)
    if (box) {
      box.isDragging = dragging
      box.dragOffsetX = offsetX
      box.dragOffsetY = offsetY
    }
  }

  return {
    viewState,
    propertyBoxes,
    screenScale,
    geoToScreen,
    screenToGeo,
    zoomIn,
    zoomOut,
    resetView,
    setCenter,
    pan,
    createPropertyBox,
    destroyPropertyBox,
    findPropertyBox,
    updatePropertyBoxPosition,
    updatePropertyBoxData,
    hideAllPropertyBoxes,
    setDragging
  }
})
