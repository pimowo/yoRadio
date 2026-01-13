#pragma once
// ACK timeout for play/pause (milliseconds)
#define BT_ACK_TIMEOUT_MS 3000
// Heartbeat timeout: consider BT disconnected if no messages for this period (ms)
// Increased to 60s to avoid spurious disconnects on noisy links
#define BT_HEARTBEAT_TIMEOUT_MS 60000

void bluetooth_handle_line(const char *line);
