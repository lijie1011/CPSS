<script setup lang="ts">
import { computed } from 'vue'
import type { PropertyBox as PropertyBoxType } from '@/types'
import L from 'leaflet'

const props = defineProps<{
  box: PropertyBoxType
  width: number
  height: number
  map: L.Map | null
}>()

const shipPosition = computed(() => {
  if (!props.map) return { x: 0, y: 0 }
  const point = props.map.latLngToContainerPoint([props.box.lat, props.box.lon])
  return { x: point.x, y: point.y }
})

const boxCenter = computed(() => ({
  x: props.box.x + 100,
  y: props.box.y + 60
}))

const lineVisible = computed(() => {
  return props.map !== null && props.box.lon !== 0 && props.box.lat !== 0
})
</script>

<template>
  <svg v-if="lineVisible" class="connecting-line" style="position: absolute; top: 0; left: 0; width: 100%; height: 100%; pointer-events: none;">
    <line
      :x1="shipPosition.x"
      :y1="shipPosition.y"
      :x2="boxCenter.x"
      :y2="boxCenter.y"
      stroke="white"
      stroke-width="2"
      stroke-dasharray="5,5"
    />
  </svg>
</template>

<style scoped>
.connecting-line {
  z-index: 1000;
}
</style>
