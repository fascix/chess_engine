<template>
  <div
    class="player-bar"
    :class="{ active }"
  >
    <div class="player-info">
      <span class="dot" :class="dotColor"></span>
      <span class="player-name">{{ name }}</span>
    </div>
    <div class="timer">{{ timeDisplay }}</div>
  </div>
</template>

<script setup>
defineProps({
  name: String,
  timeDisplay: String,
  active: Boolean,
  dotColor: String
})
</script>

<style scoped>
.player-bar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 8px 14px;
  border-radius: 4px;
  background: #423d39;
  color: #fff;
  transition: background 0.2s;
}
.player-bar.active {
  background: #55504b;
  box-shadow: inset 0 2px 0 #7fa650;
}
.player-info {
  display: flex;
  align-items: center;
  gap: 8px;
}
.dot {
  width: 12px;
  height: 12px;
  border-radius: 50%;
  display: inline-block;
}
.dot.white { background: #fff; border: 1px solid #888; }
.dot.black { background: #222; border: 1px solid #888; }
.player-name {
  font-weight: 600;
  font-size: 1rem;
}
.timer {
  font-size: 1.3rem;
  font-weight: 700;
  font-variant-numeric: tabular-nums;
  letter-spacing: 0.02em;
}
</style>
