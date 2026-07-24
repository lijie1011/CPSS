<script setup lang="ts">
import { CampType, SpecialEventType } from '@/types'

defineProps<{
  visible: boolean
}>()

const emit = defineEmits<{
  (e: 'close'): void
}>()

const campTypes = [
  { type: CampType.Friendly, name: 'Friendly', color: '#00ff00' },
  { type: CampType.Enemy, name: 'Enemy', color: '#ff0000' },
  { type: CampType.Neutral, name: 'Neutral', color: '#ffff00' },
  { type: CampType.Unknown, name: 'Unknown', color: '#888888' }
]

const eventTypes = [
  { type: SpecialEventType.Alert, icon: '!', name: 'Alert', color: '#ff0000' },
  { type: SpecialEventType.Attack, icon: 'A', name: 'Attack', color: '#8b0000' },
  { type: SpecialEventType.Defense, icon: 'D', name: 'Defense', color: '#0000ff' },
  { type: SpecialEventType.Contact, icon: 'C', name: 'Contact', color: '#00ffff' },
  { type: SpecialEventType.Damage, icon: 'X', name: 'Damage', color: '#ff8c00' },
  { type: SpecialEventType.MissionStart, icon: 'M', name: 'Mission Start', color: '#00ff00' },
  { type: SpecialEventType.MissionEnd, icon: 'E', name: 'Mission End', color: '#808080' },
  { type: SpecialEventType.Lost, icon: '?', name: 'Lost', color: '#ff00ff' },
  { type: SpecialEventType.Repair, icon: 'R', name: 'Repair', color: '#90ee90' },
  { type: SpecialEventType.Custom, icon: '*', name: 'Custom', color: '#ffff00' }
]

const instructions = [
  'Click on a target to show property box',
  'Double-click on property box to close it',
  'Drag property box to reposition',
  'Click on blank area to hide all property boxes',
  'Mouse wheel to zoom in/out',
  'Drag to pan the map'
]
</script>

<template>
  <Teleport to="body">
    <div v-if="visible" class="dialog-overlay" @click="emit('close')">
      <div class="dialog-content" @click.stop>
        <div class="dialog-header">
          <h2>Event Legend</h2>
          <button class="close-btn" @click="emit('close')">×</button>
        </div>
        
        <div class="dialog-body">
          <div class="legend-section">
            <h3>Camp Colors</h3>
            <div class="legend-grid">
              <div v-for="camp in campTypes" :key="camp.type" class="legend-item">
                <div class="color-box" :style="{ background: camp.color }"></div>
                <span>{{ camp.name }}</span>
              </div>
            </div>
          </div>
          
          <div class="legend-section">
            <h3>Event Icons</h3>
            <div class="legend-grid">
              <div v-for="event in eventTypes" :key="event.type" class="legend-item">
                <div class="event-box" :style="{ background: event.color }">
                  {{ event.icon }}
                </div>
                <span>{{ event.name }}</span>
              </div>
            </div>
          </div>
          
          <div class="legend-section">
            <h3>Instructions</h3>
            <ul class="instructions-list">
              <li v-for="(instr, index) in instructions" :key="index">
                {{ instr }}
              </li>
            </ul>
          </div>
        </div>
        
        <div class="dialog-footer">
          <button class="ok-btn" @click="emit('close')">OK</button>
        </div>
      </div>
    </div>
  </Teleport>
</template>

<style scoped>
.dialog-overlay {
  position: fixed;
  top: 0;
  left: 0;
  right: 0;
  bottom: 0;
  background: rgba(0, 0, 0, 0.5);
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1000;
}

.dialog-content {
  background: #1a1a2e;
  border-radius: 12px;
  padding: 20px;
  max-width: 500px;
  width: 90%;
  max-height: 80vh;
  overflow-y: auto;
  color: white;
}

.dialog-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  margin-bottom: 20px;
  padding-bottom: 10px;
  border-bottom: 1px solid #3a3a5a;
}

.dialog-header h2 {
  margin: 0;
  font-size: 18px;
}

.close-btn {
  background: none;
  border: none;
  color: #888;
  font-size: 24px;
  cursor: pointer;
  padding: 0 5px;
}

.close-btn:hover {
  color: white;
}

.legend-section {
  margin-bottom: 20px;
}

.legend-section h3 {
  font-size: 14px;
  margin-bottom: 10px;
  color: #aaa;
}

.legend-grid {
  display: grid;
  grid-template-columns: repeat(2, 1fr);
  gap: 10px;
}

.legend-item {
  display: flex;
  align-items: center;
  gap: 10px;
}

.color-box {
  width: 20px;
  height: 20px;
  border-radius: 4px;
  border: 1px solid rgba(255, 255, 255, 0.3);
}

.event-box {
  width: 24px;
  height: 24px;
  border-radius: 4px;
  display: flex;
  align-items: center;
  justify-content: center;
  color: white;
  font-weight: bold;
  font-size: 12px;
}

.instructions-list {
  list-style: none;
  padding: 0;
  margin: 0;
}

.instructions-list li {
  padding: 5px 0;
  font-size: 12px;
  color: #aaa;
  border-bottom: 1px solid rgba(255, 255, 255, 0.1);
}

.instructions-list li:last-child {
  border-bottom: none;
}

.dialog-footer {
  text-align: right;
  padding-top: 10px;
  border-top: 1px solid #3a3a5a;
}

.ok-btn {
  background: #2a2a4a;
  border: none;
  color: white;
  padding: 8px 20px;
  border-radius: 4px;
  cursor: pointer;
}

.ok-btn:hover {
  background: #3a3a6a;
}
</style>
