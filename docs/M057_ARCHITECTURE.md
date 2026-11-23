# M057 Architecture

M025 polling -> M022 endpoint transfer -> M015 event ring.
M022 records every transfer attempt and classifies matching, unmatched, timeout, doorbell, and completion-code outcomes. A timeout leaves the submitted TRB and DMA buffer quarantined. M057 reporting reads M022 counters and endpoint state.

M057 does not modify M031 framebuffer, M048 desktop startup, M049 boot policy, M050 event-loop scheduling, or M055 input-service registration.
