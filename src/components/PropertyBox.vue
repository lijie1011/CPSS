<script setup lang="ts">
import { ref, computed, watch } from 'vue'
import { useDataStore } from '@/stores/dataStore'
import { useViewStore } from '@/stores/viewStore'
import type { PropertyBox as PropertyBoxType, PlatformData } from '@/types'
import { CampType } from '@/types'

const props = defineProps<{
  box: PropertyBoxType
  platform: PlatformData | undefined
}>()

const dataStore = useDataStore()
const viewStore = useViewStore()

const isDragging = ref(false)
const dragOffset = ref({ x: 0, y: 0 })
const position = ref({ x: props.box.x, y: props.box.y })

watch(() => [props.box.x, props.box.y], ([newX, newY]) => {
  if (!isDragging.value) {
    position.value = { x: newX, y: newY }
  }
})

const campStr = computed(() => {
  if (!props.platform) return 'Unknown'
  switch (props.platform.camp) {
    case CampType.Friendly: return 'Friendly'
    case CampType.Enemy: return 'Enemy'
    case CampType.Neutral: return 'Neutral'
    default: return 'Unknown'
  }
})

const events = computed(() => {
  if (!props.platform) return []
  return dataStore.getEventsByTargetId(props.platform.id)
})

const handleMouseDown = (e: MouseEvent) => {
  isDragging.value = true
  dragOffset.value = {
    x: e.clientX - position.value.x,
    y: e.clientY - position.value.y
  }
  viewStore.setDragging(props.box.id, true)
}

const handleMouseMove = (e: MouseEvent) => {
  if (!isDragging.value) return
  position.value = {
    x: e.clientX - dragOffset.value.x,
    y: e.clientY - dragOffset.value.y
  }
  viewStore.updatePropertyBoxPosition(props.box.id, position.value.x, position.value.y)
}

const handleMouseUp = () => {
  isDragging.value = false
  viewStore.setDragging(props.box.id, false)
}

const handleDblClick = () => {
  viewStore.destroyPropertyBox(props.box.id)
}
</script>

<template>
  <div
    class="property-box"
    :style="{ left: `${position.x}px`, top: `${position.y}px` }"
    @mousedown="handleMouseDown"
    @mousemove="handleMouseMove"
    @mouseup="handleMouseUp"
    @mouseleave="handleMouseUp"
    @dblclick="handleDblClick"
  >
    <div class="box-header">
      <span class="header-title">Property</span>
    </div>
    <div class="box-content">
      <div class="info-row">
        <span class="label">Name:</span>
        <span class="value">{{ box.name }}</span>
      </div>
      <div class="info-row">
        <span class="label">ID:</span>
        <span class="value">{{ box.id }}</span>
      </div>
      <div class="info-row">
        <span class="label">Camp:</span>
        <span class="value" :class="`camp-${campStr.toLowerCase()}`">{{ campStr }}</span>
      </div>
      <div class="info-row">
        <span class="label">Longitude:</span>
        <span class="value">{{ box.lon.toFixed(6) }}</span>
      </div>
      <div class="info-row">
        <span class="label">Latitude:</span>
        <span class="value">{{ box.lat.toFixed(6) }}</span>
      </div>
      <div class="info-row">
        <span class="label">Speed:</span>
        <span class="value">{{ box.speed.toFixed(1) }} kn</span>
      </div>
      <div v-for="event in events" :key="event.eventId" class="info-row event-row">
        <span class="label">Event:</span>
        <span class="value event-value">{{ event.eventName }}</span>
      </div>
    </div>
  </div>
</template>

<style scoped>
.property-box {
  position: absolute;
  background: rgba(255, 255, 255, 0.95);
  border: 1px solid #333;
  border-radius: 6px;
  padding: 0;
  min-width: 200px;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.3);
  cursor: move;
  z-index: 100;
}

.box-header {
  background: #2c3e50;
  color: white;
  padding: 8px 12px;
  border-radius: 6px 6px 0 0;
}

.header-title {
  font-weight: bold;
  font-size: 14px;
}

.box-content {
  padding: 10px 12px;
}

.info-row {
  display: flex;
  justify-content: space-between;
  margin-bottom: 6px;
  font-size: 12px;
}

.info-row:last-child {
  margin-bottom: 0;
}

.label {
  color: #666;
  font-weight: 500;
}

.value {
  color: #333;
  font-weight: 600;
}

.camp-friendly {
  color: #27ae60;
}

.camp-enemy {
  color: #c0392b;
}

.camp-neutral {
  color: #f39c12;
}

.event-row {
  margin-top: 8px;
  padding-top: 8px;
  border-top: 1px dashed #ddd;
}

.event-value {
  color: #e74c3c;
}
</style>
