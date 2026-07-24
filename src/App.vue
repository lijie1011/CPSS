<script setup lang="ts">
import { ref, onMounted, onUnmounted } from 'vue'
import { useDataStore } from '@/stores/dataStore'
import MapCanvas from '@/components/MapCanvas.vue'
import Toolbar from '@/components/Toolbar.vue'
import LegendDialog from '@/components/LegendDialog.vue'

const dataStore = useDataStore()

const showLegend = ref(false)
let updateTimer: number
const mapCanvasRef = ref<InstanceType<typeof MapCanvas> | null>(null)

onMounted(() => {
  dataStore.loadMockData()
  updateTimer = window.setInterval(() => {
    dataStore.updateMockData()
  }, 1000)
})

onUnmounted(() => {
  if (updateTimer) {
    clearInterval(updateTimer)
  }
})

const handleShowLegend = () => {
  showLegend.value = true
}

const handleZoomIn = () => {
  mapCanvasRef.value?.handleZoomIn()
}

const handleZoomOut = () => {
  mapCanvasRef.value?.handleZoomOut()
}

const handleReset = () => {
  mapCanvasRef.value?.handleReset()
}
</script>

<template>
  <div class="app-container">
    <header class="app-header">
      <h1>CPSS - Command Post Support System</h1>
      <div class="header-info">
        <span class="platform-count">Platforms: {{ dataStore.validPlatforms.length }}</span>
        <span class="update-time">Last Update: {{ new Date(dataStore.lastUpdateTime).toLocaleTimeString() }}</span>
      </div>
    </header>
    
    <main class="app-main">
      <aside class="sidebar">
        <Toolbar 
          @show-legend="handleShowLegend"
          @zoom-in="handleZoomIn"
          @zoom-out="handleZoomOut"
          @reset="handleReset"
        />
      </aside>
      
      <section class="map-section">
        <MapCanvas ref="mapCanvasRef" />
      </section>
    </main>
    
    <LegendDialog 
      :visible="showLegend" 
      @close="showLegend = false" 
    />
  </div>
</template>

<style scoped>
.app-container {
  display: flex;
  flex-direction: column;
  width: 100%;
  height: 100%;
  background: #0f0f1a;
}

.app-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 15px 20px;
  background: #1a1a2e;
  border-bottom: 1px solid #2a2a4a;
}

.app-header h1 {
  margin: 0;
  font-size: 18px;
  color: #ffffff;
  font-weight: 600;
}

.header-info {
  display: flex;
  gap: 20px;
  font-size: 13px;
  color: #aaa;
}

.platform-count {
  background: #2a2a4a;
  padding: 4px 12px;
  border-radius: 12px;
}

.app-main {
  display: flex;
  flex: 1;
  overflow: hidden;
}

.sidebar {
  width: 80px;
  padding: 15px;
  display: flex;
  flex-direction: column;
  align-items: center;
}

.map-section {
  flex: 1;
  overflow: hidden;
}
</style>
