<script setup lang="ts">
const emit = defineEmits<{
  (e: 'showLegend'): void
  (e: 'zoomIn'): void
  (e: 'zoomOut'): void
  (e: 'reset'): void
}>()
</script>

<template>
  <div class="toolbar">
    <button class="tool-btn" @click="emit('zoomIn')" title="Zoom In">
      <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
        <line x1="12" y1="5" x2="12" y2="19" />
        <line x1="5" y1="12" x2="19" y2="12" />
      </svg>
      <span>Zoom In</span>
    </button>
    
    <button class="tool-btn" @click="emit('zoomOut')" title="Zoom Out">
      <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
        <line x1="12" y1="5" x2="12" y2="19" />
      </svg>
      <span>Zoom Out</span>
    </button>
    
    <button class="tool-btn" @click="emit('reset')" title="Reset View">
      <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
        <circle cx="12" cy="12" r="10" />
        <path d="M12 6v6l4 2" />
      </svg>
      <span>Reset</span>
    </button>
    
    <button class="tool-btn" @click="emit('showLegend')" title="Event Legend">
      <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
        <circle cx="12" cy="12" r="10" />
        <path d="M9.09 9a3 3 0 0 1 5.83 1c0 2-3 3-3 3" />
        <line x1="12" y1="17" x2="12.01" y2="17" />
      </svg>
      <span>Help</span>
    </button>
  </div>
</template>

<style scoped>
.toolbar {
  display: flex;
  flex-direction: column;
  gap: 10px;
  padding: 15px;
  background: rgba(26, 26, 46, 0.95);
  border-radius: 8px;
}

.tool-btn {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 5px;
  padding: 12px 16px;
  background: #2a2a4a;
  border: none;
  border-radius: 6px;
  color: #ffffff;
  cursor: pointer;
  transition: all 0.2s;
}

.tool-btn:hover {
  background: #3a3a6a;
  transform: translateY(-2px);
}

.tool-btn:active {
  transform: translateY(0);
}

.tool-btn svg {
  flex-shrink: 0;
}

.tool-btn span {
  font-size: 11px;
}
</style>
